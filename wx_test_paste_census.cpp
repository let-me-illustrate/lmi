// Test pasting spreadsheet data into a census.
//
// Copyright (C) 2014, 2015, 2016 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "mvc_controller.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"
#include "wx_utility.hpp"

#include <wx/app.h>
#include <wx/dataview.h>
#include <wx/dialog.h>
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

// Helper function to find the wxDataViewCtrl used for the census display.
//
// Precondition: the currently active window must be a CensusView.
wxDataViewCtrl* find_census_list_window()
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

    wxDataViewCtrl* const dvc = dynamic_cast<wxDataViewCtrl*>(*z);
    LMI_ASSERT(dvc);

    return dvc;
}

// Retrieve the list model from list window.
//
// Precondition: this wxDataViewCtrl must actually use a list model.
wxDataViewListModel* get_census_list_model(wxDataViewCtrl* dvc)
{
    wxDataViewModel* const model = dvc->GetModel();
    LMI_ASSERT(model);

    wxDataViewListModel* const
        list_model = dynamic_cast<wxDataViewListModel*>(model);
    LMI_ASSERT(list_model);

    return list_model;
}

// Helper for building the diagnostic message in check_list_columns().
std::string build_not_found_message(std::set<std::string> const& remaining)
{
    std::ostringstream message;
    bool const only_one = remaining.size() == 1;
    message << (only_one ? "column" : "columns");

    typedef std::set<std::string>::const_iterator ssci;
    for(ssci i = remaining.begin(); i != remaining.end(); ++i)
        {
        if(i != remaining.begin())
            {
            message << ",";
            }

        message << " '" << *i << "'";
        }

    message << " " << (only_one ? "was" : "were") << " not found" << " ";

    return message.str();
}

// Check for the presence of all columns with the given name and, if specified,
// for the absence of the given one.
//
// The 'when' parameter is used solely for the diagnostic messages in case of
// the check failure.
void check_list_columns
    (wxDataViewCtrl* dvc
    ,char const* when
    ,std::set<std::string> const& expected
    ,std::string const& unexpected = std::string()
    )
{
    std::set<std::string> remaining(expected.begin(), expected.end());

    unsigned int const num_columns = dvc->GetColumnCount();
    for(unsigned int n = 0; n < num_columns; ++n)
        {
        std::string const title = dvc->GetColumn(n)->GetTitle().ToStdString();
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
unsigned int find_model_column_by_title
    (wxDataViewCtrl* dvc
    ,std::string const& title
    )
{
    unsigned int const num_columns = dvc->GetColumnCount();
    for(unsigned int n = 0; n < num_columns; ++n)
        {
        wxDataViewColumn const* column = dvc->GetColumn(n);
        if(column->GetTitle().ToStdString() == title)
            {
            return column->GetModelColumn();
            }
        }

    throw std::runtime_error("column " + title + " not found");
}

} // Unnamed namespace.

/// Test pasting spreadsheet data into a census.
///
/// Create a set of data that might reasonably be copied from a
/// spreadsheet. Initially at least, use the data in the user manual:
///   http://www.nongnu.org/lmi/pasting_to_a_census.html
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
        "Female\t19851231\tClerical\t100000, @65; 50000\n"
        "Male\t19801130\tClerical\t200000, @65; 50000\n"
        "Female\t19751029\tTechnical\t300000, @65; 50000\n"
        "Male\t19700928\tTechnical\t400000, @65; 50000\n"
        "Female\t19650827\tSupervisor\t500000, @65; 50000\n"
        "Male\t19600726\tAttorney\t600000, @65; 75000\n"
        "Female\t19550625\tPresident\t700000, @65; 100000\n"
        ;

    std::size_t const number_of_rows = std::count
        (census_data
        ,census_data + std::strlen(census_data)
        ,'\n'
        )
        - 1 // Not counting the header.
        - 1 // Nor the empty line after it.
        ;

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
    wxDataViewCtrl* const list_window = find_census_list_window();
    wxDataViewListModel* const list_model = get_census_list_model(list_window);
    LMI_ASSERT_EQUAL(list_model->GetCount(), number_of_rows);

    check_list_columns
        (list_window
        ,"after pasting initial census data"
        ,column_titles
        );

    // Change class defaults: this requires a selection, so ensure we have one
    // by clicking somewhere inside the control.
    ui.MouseMove
        (list_window->ClientToScreen
            (wxPoint
                (10*list_window->GetCharWidth()
                ,3*list_window->GetCharHeight()
                )
            )
        );
    ui.MouseClick();
    wxYield();

    LMI_ASSERT_EQUAL(list_window->GetSelectedItemsCount(), 1);

    ui.Char('e', wxMOD_CONTROL | wxMOD_ALT); // "Census|Edit class defaults"

    struct change_gender_in_class_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        virtual int OnInvoked(MvcController* dialog) const
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

        virtual wxString GetDefaultDescription() const
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
    check_list_columns
        (list_window
        ,"after changing gender in class defaults"
        ,column_titles
        );

    // Verify that the "Gender" column value is "Unisex" in every row now.
    unsigned int const
        gender_column = find_model_column_by_title(list_window, "Gender");
    LMI_ASSERT_EQUAL(list_model->GetCount(), number_of_rows);
    // Only the first two rows are affected, because only they belong
    // to the first employee class.
    for(std::size_t row = 0; row < 2; ++row)
        {
        wxVariant value;
        list_model->GetValueByRow(value, row, gender_column);
        LMI_ASSERT_EQUAL(value.GetString(), "Unisex");
        }

    // Change the case defaults to get rid of the underwriting class.
    ui.Char('e', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Edit case defaults"

    struct change_class_in_case_defaults_dialog
        :public wxExpectModalBase<MvcController>
    {
        virtual int OnInvoked(MvcController* dialog) const
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

        virtual wxString GetDefaultDescription() const
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
    LMI_ASSERT_EQUAL(list_model->GetCount(), number_of_rows);

    column_titles.erase("Underwriting Class");
    check_list_columns
        (list_window
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

