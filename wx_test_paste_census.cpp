// Test pasting spreadsheet data into a census.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "calendar_date.hpp"
#include "data_directory.hpp"
#include "mvc_controller.hpp"
#include "ssize_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"
#include "wx_utility.hpp"

#include <wx/app.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/mdi.h>
#include <wx/radiobox.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>

namespace
{

// Helper function to find the wxGrid used for the census display.
//
// Precondition: the currently active window must be a CensusView.
wxGrid* find_census_grid_window()
{
    wxWindow* const top_window = wxTheApp->GetTopWindow();
    LMI_ASSERT(top_window);

    wxMDIParentFrame* const
        parent_frame = dynamic_cast<wxMDIParentFrame*>(top_window);
    LMI_ASSERT(parent_frame);

    wxMDIChildFrame* const child_frame = parent_frame->GetActiveChild();
    LMI_ASSERT(child_frame);

    wxWindowList const& census_children = child_frame->GetChildren();
    wxWindowList::const_iterator z = census_children.begin();
    LMI_ASSERT(z != census_children.end());

    wxGrid* const grid = dynamic_cast<wxGrid*>(*z);
    LMI_ASSERT(grid);

    return grid;
}

// Helper for building the diagnostic message.
std::string build_not_found_message(std::set<std::string> const& remaining)
{
    std::ostringstream message;
    bool const only_one = remaining.size() == 1;
    message << (only_one ? "column" : "columns");

    bool first = true;
    for(auto const& i : remaining)
        {
        if(!first)
            {
            message << ",";
            first = false;
            }

        message << " '" << i << "'";
        }

    message << " " << (only_one ? "was" : "were") << " not found" << " ";

    return message.str();
}

// Check for the presence of all columns with the given name and, if specified,
// for the absence of the given one.
//
// The 'when' parameter is used solely for the diagnostic messages in case of
// the check failure.
void check_grid_columns
    (wxGrid* grid
    ,char const* when
    ,std::set<std::string> const& expected
    ,std::string const& unexpected = std::string()
    )
{
    std::set<std::string> remaining(expected.begin(), expected.end());

    for(int n = 0; n < grid->GetNumberCols(); ++n)
        {
        std::string const title = grid->GetColLabelValue(n).ToStdString();
        LMI_ASSERT_WITH_MSG
            (title != unexpected
            ,"column '" << title << "' unexpectedly found " << when
            );

        // Notice that it is not an error if the column is not in the expected
        // columns set, it is not exhaustive.
        remaining.erase(title);
        }

    LMI_ASSERT_WITH_MSG
        (remaining.empty()
        ,build_not_found_message(remaining) << when
        );
}

// Find the index of the column with the given title.
//
// Throws an exception if the column is not found.
int find_table_column_by_title
    (wxGrid* grid
    ,std::string const& title
    )
{
    for(int n = 0; n < grid->GetNumberCols(); ++n)
        {
        if(grid->GetColLabelValue(n).ToStdString() == title)
            {
            return n;
            }
        }

    throw std::runtime_error("column " + title + " not found");
}

} // Unnamed namespace.

/// Test pasting spreadsheet data into a census.
///
/// Create a set of data that might reasonably be copied from a
/// spreadsheet. Initially at least, use the data in the user manual:
///   https://www.nongnu.org/lmi/pasting_to_a_census.html
/// Hardcode the data here; don't read them from the user manual.
/// (That didactic example was designed mainly to fit on a web page
/// and to make sense to end users. Some day we might want to make
/// this automated test more comprehensive, without changing the web
/// page.)
///
/// Place that data set on the clipboard and simulate
///   File | New | Census
///   Census | Paste census
/// Make sure it has the expected number of rows. Also check that all
/// the columns pasted are shown by verifying each header: some time
/// ago, the "Payment" and "Death Benefit Option" columns were lost
/// due to a defect. All pasted columns should be shown, along with
/// several others that vary in step with issue-age differences.
///
/// The census manager shows only columns that vary across cells,
/// notably taking into account the case and class default cells.
/// Test this in two ways. First:
///   Census | Edit class defaults [requires focusing a row]
///   change gender to "Unisex"
///   apply to every cell: Yes
/// Verify the expected result: the gender column is still shown
/// (because "Unisex" is not yet the class default), and its value is
/// "Unisex" in every row. Second:
///   Census | Edit case defaults
///   change underwriting class to any different value
///   apply to every cell: Yes
/// Verify the expected result: the underwriting-class column is no
/// longer shown.
///
/// Then save the file in 'gui_test_path'; verify that it exists.

LMI_WX_TEST_CASE(paste_census)
{
    // Update this file (and the webpage cited above) in 2040--see:
    //   https://lists.nongnu.org/archive/html/lmi/2020-06/msg00037.html
    // If ccache is used with 'CCACHE_SLOPPINESS=time_macros' to
    // prevent this file from being recompiled, then this static
    // assertion [spaces inserted in macro name to pass concinnity
    // test]:
    //   // _ _ DATE _ _[9] is second-to-last digit of a four-digit year
    //   static_assert('4' != _ _ DATE _ _[9]);
    // won't work, so use this non-static assertion instead:
    LMI_ASSERT(calendar_date().year() < 2040);

    // The column titles are the user-visible strings corresponding to the
    // internal column names actually used in the census data below.
    std::set<std::string> column_titles;
    column_titles.insert("Gender");
    column_titles.insert("Date Of Birth");
    column_titles.insert("Employee Class");
    column_titles.insert("Specified Amount");

    char const* const census_data =
        "Gender\tDateOfBirth\tEmployeeClass\tSpecifiedAmount\n"
        "\n"
        "Female\t19851231\tClerical\t100000, @85; 50000\n"
        "Male\t19801130\tClerical\t200000, @85; 50000\n"
        "Female\t19751029\tTechnical\t300000, @85; 50000\n"
        "Male\t19700928\tTechnical\t400000, @85; 50000\n"
        "Female\t19650827\tSupervisor\t500000, @85; 50000\n"
        "Male\t19600726\tAttorney\t600000, @85; 75000\n"
        "Female\t19550625\tPresident\t700000, @85; 100000\n"
        ;

    int const number_of_rows =
        bourn_cast<int>
            (
            std::count
                (census_data
                ,census_data + std::strlen(census_data)
                ,'\n'
                )
            )
        - 1 // Not counting the header.
        - 1 // Nor the empty line after it.
        ;
    LMI_ASSERT(0 < number_of_rows);

    // Put the data to paste on clipboard.
    ClipboardEx::SetText(census_data);

    // Create a new census.
    wx_test_new_census census;

    // Paste data into it.
    wxUIActionSimulator ui;
    ui.Char('s', wxMOD_CONTROL | wxMOD_SHIFT); // "Census | Paste census"
    wxYield();

    // Find the model containing the cells and check that it was filled in
    // correctly.
    wxGrid* const grid_window = find_census_grid_window();
    wxGridTableBase* const table = grid_window->GetTable();
    LMI_ASSERT(table);
    LMI_ASSERT_EQUAL(table->GetNumberRows(), number_of_rows);

    check_grid_columns
        (grid_window
        ,"after pasting initial census data"
        ,column_titles
        );

    // Change class defaults: this requires a selection, so ensure we have one
    // by clicking somewhere inside the control.
    ui.MouseMove
        (grid_window->ClientToScreen
            (wxPoint
                (10 * grid_window->GetCharWidth()
                , 3 * grid_window->GetCharHeight()
                )
            )
        );
    ui.MouseClick();
    wxYield();

    LMI_ASSERT_EQUAL(lmi::ssize(grid_window->GetSelectedRows()), 1);

    ui.Char('e', wxMOD_CONTROL | wxMOD_ALT); // "Census|Edit class defaults"

    struct change_gender_in_class_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            // We can't find directly the radio button we're interested in,
            // because it's not a real wxWindow, so we need to find the radio
            // box containing it.
            wxWindow* const gender_window = wx_test_focus_controller_child
                (*dialog
                ,"Gender"
                );

            wxRadioBox* const
                gender_radiobox = dynamic_cast<wxRadioBox*>(gender_window);
            LMI_ASSERT(gender_radiobox);

            wxUIActionSimulator ui;
            // Select the last, "Unisex", radio button, by simulating
            // down-arrow twice: female --> male, then male --> unisex.
            ui.Char(WXK_DOWN);
            wxYield();
            ui.Char(WXK_DOWN);
            wxYield();

            LMI_ASSERT_EQUAL(gender_radiobox->GetSelection(), 2);

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "class defaults dialog";
            }
    };

    // The menu command above should have opened the "Class defaults" dialog and
    // our code dealing with it above is supposed to result in an appearance of
    // "Apply all changes to every cell?" message box for which we provide an
    // affirmative answer.
    wxTEST_DIALOG
        (wxYield()
        ,change_gender_in_class_defaults_dialog()
        ,wxExpectModal<wxMessageDialog>(wxYES).
            Describe("message box asking whether to apply gender changes to all")
        );

    // Check that all columns, including the "Gender" one, are still shown.
    check_grid_columns
        (grid_window
        ,"after changing gender in class defaults"
        ,column_titles
        );

    // Verify that the "Gender" column value is "Unisex" in every row now.
    int const gender_column = find_table_column_by_title(grid_window, "Gender");
    LMI_ASSERT_EQUAL(table->GetNumberRows(), number_of_rows);
    // Only the first two rows are affected, because only they belong
    // to the first employee class.
    for(int row = 0; row < 2; ++row)
        {
        LMI_ASSERT_EQUAL(table->GetValue(row, gender_column), "Unisex");
        }

    // Change the case defaults to get rid of the underwriting class.
    ui.Char('e', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Edit case defaults"

    struct change_class_in_case_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            // We can't find directly the radio button we're interested in,
            // because it's not a real wxWindow, so we need to find the radio
            // box containing it.
            wxWindow* const class_window = wx_test_focus_controller_child
                (*dialog
                ,"UnderwritingClass"
                );

            wxRadioBox* const
                class_radiobox = dynamic_cast<wxRadioBox*>(class_window);
            LMI_ASSERT(class_radiobox);

            wxUIActionSimulator ui;
            ui.Char(WXK_UP); // Select the first, "Preferred", radio button.
            wxYield();

            LMI_ASSERT_EQUAL(class_radiobox->GetSelection(), 0);

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "case defaults dialog";
            }
    };

    // The menu command above should have opened the "Case defaults" dialog and
    // our code dealing with it above is supposed to result in an appearance of
    // "Apply all changes to every cell?" message box for which we provide an
    // affirmative answer.
    wxTEST_DIALOG
        (wxYield()
        ,change_class_in_case_defaults_dialog()
        ,wxExpectModal<wxMessageDialog>(wxYES).
            Describe("message box asking whether to apply class changes to all")
        );

    // Check that we still have the same cells but that now the underwriting
    // class column has disappeared as its value has been fixed.
    LMI_ASSERT_EQUAL(table->GetNumberRows(), number_of_rows);

    column_titles.erase("Underwriting Class");
    check_grid_columns
        (grid_window
        ,"after changing class in case defaults"
        ,column_titles
        ,"Underwriting Class"
        );

    // Finally save the census with the pasted data for later inspection.
    std::string const census_file_name = get_test_file_path_for("PasteCensus.cns");
    output_file_existence_checker output_cns(census_file_name);

    ui.Char('a', wxMOD_CONTROL);    // "File|Save as"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(census_file_name).
            Describe("census save file dialog")
        );

    LMI_ASSERT(output_cns.exists());

    census.close();
}
