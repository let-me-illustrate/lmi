// Test calculation summary.
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
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "ssize_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_utility.hpp"

#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/ffile.h>
#include <wx/html/htmlpars.h>
#include <wx/html/htmlwin.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

namespace
{

struct name_and_title
{
    char const* name;
    char const* title;
};

// Names and titles of the columns used by default.
name_and_title const default_columns_info[] =
    {{ "PolicyYear"             , "Policy Year"                 }
    ,{ "Outlay"                 , "Net Outlay"                  }
    ,{ "AcctVal_Current"        , "Curr Account Value"          }
    ,{ "CSVNet_Current"         , "Curr Net Cash Surr Value"    }
    ,{ "EOYDeathBft_Current"    , "Curr EOY Death Benefit"      }
    };

int const number_of_default_columns = lmi::ssize(default_columns_info);

// Names and titles of the columns used when not using the built-in calculation
// summary.
name_and_title const custom_columns_info[] =
    {{ "PolicyYear"             , "Policy Year"                 }
    ,{ "NewCashLoan"            , "Annual Loan"                 }
    };

int const number_of_custom_columns = lmi::ssize(custom_columns_info);

// Special name used when the column is not used at all. This is the same
// string used in preferences_model.cpp, but we duplicate it here as we don't
// have access to it.
char const* const empty_column_name = "[none]";

// Total number of configurable summary columns. This, again, duplicates the
// number [implicitly] used in preferences_model.cpp.
int const total_number_of_columns = 12;

// Base class for all the tests working with the preferences dialog. It
// defines both a simpler interface for the derived classes to define the
// tests with the preferences dialog, and also provides a helper run() function
// which shows the preferences dialog and performs these checks.
class expect_preferences_dialog_base
    :public wxExpectModalBase<MvcController>
{
  public:
    expect_preferences_dialog_base()
        :dialog_       {nullptr}
        ,use_checkbox_ {nullptr}
        {
        }

    expect_preferences_dialog_base(expect_preferences_dialog_base const&) = delete;
    expect_preferences_dialog_base& operator=(expect_preferences_dialog_base const&) = delete;

    void run() const
        {
        wxUIActionSimulator ui;
        ui.Char('f', wxMOD_CONTROL);    // "File|Preferences"

        wxTEST_DIALOG(wxYield(), *this);
        }

    int OnInvoked(MvcController* dialog) const override
        {
        // OnInvoked() is const but it doesn't make much sense for
        // OnPreferencesInvoked() to be const as it is going to modify the
        // dialog, so cast away this constness once and for all.
        expect_preferences_dialog_base* const
            self = const_cast<expect_preferences_dialog_base*>(this);

        self->dialog_ = dialog;

        dialog->Show();
        wxYield();

        wxWindow* const use_window = wx_test_focus_controller_child
            (*dialog
            ,"UseBuiltinCalculationSummary"
            );

        self->use_checkbox_ = dynamic_cast<wxCheckBox*>(use_window);
        LMI_ASSERT(use_checkbox_);

        return self->OnPreferencesInvoked();
        }

    wxString GetDefaultDescription() const override
        {
        return "preferences dialog";
        }

  protected:
    virtual int OnPreferencesInvoked() = 0;

    // Helpers for the derived classes OnPreferencesInvoked().
    void set_use_builtin_summary(bool use)
        {
        // Under MSW we could use "+" and "-" keys to set the checkbox value
        // unconditionally, but these keys don't work under the other
        // platforms, so it's simpler to use the space key which can be used on
        // all platforms to toggle the checkbox -- but then we must do it only
        // if really needed.
        if(use_checkbox_->GetValue() != use)
            {
            use_checkbox_->SetFocus();

            wxUIActionSimulator ui;
            ui.Char(WXK_SPACE);
            wxYield();
            }
        }

    wxComboBox* focus_column_combobox(int n)
        {
            wxWindow* const column_window = wx_test_focus_controller_child
                (*dialog_
                ,wxString::Format("CalculationSummaryColumn%02u", n).c_str()
                );

            wxComboBox* const
                column_combobox = dynamic_cast<wxComboBox*>(column_window);
            LMI_ASSERT_WITH_MSG
                (column_combobox
                ,"control for column #" << n << "is not a wxComboBox"
                );

            return column_combobox;
        }

    // These variables are only valid inside the overridden
    // OnPreferencesInvoked() function.
    MvcController* dialog_;
    wxCheckBox* use_checkbox_;
};

void check_calculation_summary_columns
    (int number_of_columns
    ,name_and_title const columns_info[]
    )
{
    // Create a new illustration.
    wx_test_new_illustration ill;

    // Find the window displaying HTML contents of the illustration view.
    wxWindow* const focus = wxWindow::FindFocus();
    LMI_ASSERT(focus);

    wxHtmlWindow* const htmlwin = dynamic_cast<wxHtmlWindow*>(focus);
    LMI_ASSERT(htmlwin);

    // And get the HTML from it.
    wxHtmlParser* const parser = htmlwin->GetParser();
    LMI_ASSERT(parser);
    LMI_ASSERT(parser->GetSource());

    wxString const html = *parser->GetSource();

    // We don't need the window any more.
    ill.close();

    // Find the start of the table after the separating line.
    auto pos = html.find("<hr>\n<table");
    LMI_ASSERT(pos != wxString::npos);

    pos = html.find("\n<td", pos);
    LMI_ASSERT(pos != wxString::npos);

    ++pos;                                          // skip the new line

    // We have found the place where the columns are described in the HTML,
    // iterate over all of them.
    for(int n = 0; n < number_of_columns; ++n)
        {
        LMI_ASSERT_EQUAL(wxString(html, pos, 3), "<td");

        pos = html.find(">", pos);                     // end of the <td> tag
        LMI_ASSERT(pos != wxString::npos);

        ++pos;                                         // <td> tag contents

        auto const next = html.find("\n", pos); // the next line start
        LMI_ASSERT(next != wxString::npos);

        // Extract the column title from the rest of the line.
        wxString title;
        LMI_ASSERT(wxString(html, pos, next - pos).EndsWith(" </td>", &title));

        LMI_ASSERT_EQUAL(title, columns_info[n].title);

        pos = next + 1;
        }

    LMI_ASSERT_EQUAL(wxString(html, pos, 5), "</tr>");
}

} // Unnamed namespace.

// Deferred ideas:
//
// Someday, test supplemental-report column selections similarly.
//
// To test backward compatibility, modify 'configurable_settings'
// directly, adding a field that was formerly removed, and setting
// the version number to a version that offered that field.

/// Test calculation summary.
///
/// Iff the '--distribution' option is specified, then:
///   File | Preferences
/// make sure that "Use built-in calculation summary" is checked, and
/// that the saved selections (those that would become active if the
/// checkbox were unchecked) exactly match the default selections
/// given by default_calculation_summary_columns().
///
/// Display an illustration, to see calculation-summary effects:
/// File | New | Illustration | OK
///
/// File | Preferences
/// uncheck "Use built-in calculation summary"
/// set all "Column" controls to "[none]"
/// in "Column 2" (two, not zero), select "NewCashLoan"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Annual Loan
///
/// File | Preferences
/// Verify that "NewCashLoan" has moved from "Column 2" to "Column 0"
/// check "Use built-in calculation summary"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Net Outlay
///   Curr Account Value
///   Curr Net Cash Surr Value
///   Curr EOY Death Benefit
///
/// File | Preferences
/// uncheck "Use built-in calculation summary"
/// Verify that "Column 0" is "NewCashLoan" and the rest are "[none]"
/// OK
/// Verify that the columns shown in the open illustration are exactly
///   Policy Year
///   Annual Loan

LMI_WX_TEST_CASE(calculation_summary)
{
    configurable_settings& c = configurable_settings::instance();
    std::string const saved_builtin = c["use_builtin_calculation_summary"].str();
    std::string const saved_columns = c["calculation_summary_columns"    ].str();

    if(is_distribution_test())
        {
        // Not only is this the expected value in the GUI, but we also want to be
        // sure that effective_calculation_summary_columns() returns the default
        // columns in the code below -- and this is only the case when we are using
        // the built-in calculation summary.
        LMI_ASSERT(c.use_builtin_calculation_summary());

        struct verify_builtin_calculation_summary : expect_preferences_dialog_base
        {
            int OnPreferencesInvoked() override
                {
                LMI_ASSERT_EQUAL(use_checkbox_->GetValue(), true);

                std::vector<std::string> const&
                    summary_columns = effective_calculation_summary_columns();

                for(int n = 0; n < number_of_custom_columns; ++n)
                    {
                    wxString const& column = focus_column_combobox(n)->GetValue();
                    if(n < lmi::ssize(summary_columns))
                        {
                        LMI_ASSERT_EQUAL(column, summary_columns[n]);
                        }
                    else
                        {
                        LMI_ASSERT_EQUAL(column, empty_column_name);
                        }
                    }

                return wxID_CANCEL;
                }
        };

        verify_builtin_calculation_summary().run();
        }

    wx_test_new_illustration ill;

    // Use a single "NewCashLoan" custom column in third position.
    struct set_custom_columns_in_preferences_dialog : expect_preferences_dialog_base
    {
        int OnPreferencesInvoked() override
            {
            set_use_builtin_summary(false);

            wxUIActionSimulator ui;
            for(int n = 0; n < total_number_of_columns; ++n)
                {
                focus_column_combobox(n);
                ui.Select(n == 2 ? "NewCashLoan" : empty_column_name);
                }

            return wxID_OK;
            }
    };

    set_custom_columns_in_preferences_dialog().run();

    check_calculation_summary_columns
        (number_of_custom_columns
        ,custom_columns_info
        );

    // Now switch to using the default columns.
    struct use_builtin_calculation_summary : expect_preferences_dialog_base
    {
        int OnPreferencesInvoked() override
            {
            // Before returning to the built-in summary, check that our custom
            // value for the column #2 moved into the position #0 (because the
            // first two columns were left unspecified).
            LMI_ASSERT_EQUAL
                (focus_column_combobox(0)->GetValue()
                ,"NewCashLoan"
                );

            // And all the rest of the columns are (still) empty.
            for(int n = 1; n < total_number_of_columns; ++n)
                {
                LMI_ASSERT_EQUAL
                    (focus_column_combobox(n)->GetValue()
                    ,empty_column_name
                    );
                }

            set_use_builtin_summary(true);

            return wxID_OK;
            }
    };

    use_builtin_calculation_summary().run();

    check_calculation_summary_columns
        (number_of_default_columns
        ,default_columns_info
        );

    // Finally, switch back to the previously configured custom columns.
    struct use_custom_calculation_summary : expect_preferences_dialog_base
    {
      public:
        int OnPreferencesInvoked() override
            {
            set_use_builtin_summary(false);

            // The custom columns shouldn't have changed.
            LMI_ASSERT_EQUAL
                (focus_column_combobox(0)->GetValue()
                ,"NewCashLoan"
                );

            for(int n = 1; n < total_number_of_columns; ++n)
                {
                LMI_ASSERT_EQUAL
                    (focus_column_combobox(n)->GetValue()
                    ,empty_column_name
                    );
                }

            return wxID_OK;
            }
    };

    use_custom_calculation_summary().run();

    check_calculation_summary_columns
        (number_of_custom_columns
        ,custom_columns_info
        );

    c["use_builtin_calculation_summary"] = saved_builtin;
    c["calculation_summary_columns"    ] = saved_columns;
    c.save();

    ill.close();
}
