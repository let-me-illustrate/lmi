// Test calculation summary features.
//
// Copyright (C) 2014, 2015 Gregory W. Chicares.
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
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_utility.hpp"

#include <wx/ffile.h>
#include <wx/html/htmlpars.h>
#include <wx/html/htmlwin.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cstddef>                          // std::size_t

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

std::size_t const number_of_default_columns
    = sizeof default_columns_info / sizeof(default_columns_info[0]);

// Names and titles of the columns used when not using the built-in calculation
// summary.
name_and_title const custom_columns_info[] =
    {{ "PolicyYear"             , "Policy Year"                 }
    ,{ "AttainedAge"            , "Attained Age"                }
    ,{ "Outlay"                 , "Net Outlay"                  }
    ,{ "CSVNet_Current"         , "Curr Net Cash Surr Value"    }
    ,{ "AcctVal_Current"        , "Curr Account Value"          }
    ,{ "CSVNet_Guaranteed"      , "Guar Net Cash Surr Value"    }
    ,{ "AcctVal_Guaranteed"     , "Guar Account Value"          }
    ,{ "EOYDeathBft_Current"    , "Curr EOY Death Benefit"      }
    ,{ "EOYDeathBft_Guaranteed" , "Guar EOY Death Benefit"      }
    ,{ "NetWD"                  , "Withdrawal"                  }
    ,{ "NewCashLoan"            , "Annual Loan"                 }
    ,{ "LoanIntAccrued_Current" , "Curr Loan Int Accrued"       }
    };

std::size_t const number_of_custom_columns
    = sizeof custom_columns_info / sizeof(custom_columns_info[0]);

// Special name used when the column is not used at all.
char const* const magic_null_column_name = "[none]";

// Change the calculation summary settings in the preferences dialog to use, or
// not use, the built-in defaults.
void use_builtin_calculation_summary(bool b)
{
    wxUIActionSimulator ui;
    ui.Char('f', wxMOD_CONTROL);    // "File|Preferences"

    class ChangeCalculationSummaryInPreferencesDialog
        :public wxExpectModalBase<MvcController>
    {
      public:
        ChangeCalculationSummaryInPreferencesDialog
            (bool use_builtin_summary)
            :use_builtin_summary_(use_builtin_summary)
            {
            }

        virtual int OnInvoked(MvcController* dialog) const
            {
            dialog->Show();
            wxYield();

            wxUIActionSimulator ui;

            // Go to the "Use built-in calculation summary" checkbox.
            ui.Char(WXK_TAB);
            wxYield();

            // Disable the checkbox initially as we need it to be disabled to
            // change the values of the column controls.
            ui.Char('-');
            wxYield();

            // Update the columns controls when using them.
            for(std::size_t n = 0; n < number_of_custom_columns; ++n)
                {
                ui.Char(WXK_TAB);
                wxYield();

                wxString const column_name
                    (use_builtin_summary_
                    ? magic_null_column_name
                    : custom_columns_info[n].name
                    );

                LMI_ASSERT(ui.Select(column_name));

                wxYield();
                }

            // Finally return to the initial checkbox.
            for(std::size_t n = 0; n < number_of_custom_columns; ++n)
                {
                ui.Char(WXK_TAB, wxMOD_SHIFT);
                }

            wxYield();

            // And set it to the desired value.
            ui.Char(use_builtin_summary_ ? '+' : '-');
            wxYield();

            return wxID_OK;
            }

      private:
        bool const use_builtin_summary_;
    };

    wxTEST_DIALOG
        (wxYield()
        ,ChangeCalculationSummaryInPreferencesDialog(b)
        );
}

void check_calculation_summary_columns
    (std::size_t number_of_columns
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
    size_t pos = html.find("<hr>\n<table");
    LMI_ASSERT(pos != wxString::npos);

    pos = html.find("\n<td", pos);
    LMI_ASSERT(pos != wxString::npos);

    pos++;                                          // skip the new line

    // We have found the place where the columns are described in the HTML,
    // iterate over all of them.
    for(std::size_t n = 0; n < number_of_columns; ++n)
        {
        LMI_ASSERT_EQUAL(wxString(html, pos, 3), "<td");

        pos = html.find(">", pos);                  // end of the <td> tag
        LMI_ASSERT(pos != wxString::npos);

        pos++;                                      // <td> tag contents

        size_t const next = html.find("\n", pos);   // the next line start
        LMI_ASSERT(next != wxString::npos);

        // Extract the column title from the rest of the line.
        wxString title;
        LMI_ASSERT(wxString(html, pos, next - pos).EndsWith(" </td>", &title));

        LMI_ASSERT_EQUAL(title, columns_info[n].title);

        pos = next + 1;
        }

    LMI_ASSERT_EQUAL(wxString(html, pos, 5), "</tr>");
}

// Save the current clipboard contents to a file with the given name,
// overwriting it if necessary.
void save_clipboard(wxString const& filename)
{
    wxFFile f(filename, "w");
    LMI_ASSERT(f.IsOpened());
    LMI_ASSERT(f.Write(wxString(ClipboardEx::GetText())));
    LMI_ASSERT(f.Close());
}

// Save the illustration calculation summary and full data to files with the
// given prefix.
void save_illustration_data(wxString const& prefix)
{
    wxUIActionSimulator ui;
    ui.Char('c', wxMOD_CONTROL); // "Illustration|Copy calculation summary"
    wxYield();
    save_clipboard(prefix + "IllSummary.txt");

    ui.Char('d', wxMOD_CONTROL); // "Illustration|Copy full illustration data"
    wxYield();
    save_clipboard(prefix + "IllFull.txt");
}

} // Unnamed namespace.

/*
    Start of the calculation summary unit test.

    This partially implements the first half of the item 8 of the
    testing specification:

        8. Validate Calculation summary and a few of its features.

          A. File | Preferences | check 'Use built-in calculation summary'
             File | New | Illustration | Ok
             Expected results:
               These columns display in the calculation summary view:
                 Policy Year
                 Net Outlay
                 Curr Account Value
                 Curr Net Cash Surr Value
                 Curr EOY Death Benefit
               Inspect 'configurable_settings.xml' for:
                 <calculation_summary_columns></calculation_summary_columns>

          B. File | Preferences | uncheck 'Use built-in calculation summary'
             select twelve different columns | inspect display for those columns
             Expected results:
               These columns display in the calculation summary view:
                 Policy Year
                 Attained Age
                 Net Outlay
                 Curr Net Cash Surr Value
                 Curr Account Value
                 Guar Net Cash Surr Value
                 Guar Account Value
                 Curr EOY Death Benefit
                 Guar EOY Death Benefit
                 Withdrawal
                 Annual Loan
                 Curr Loan Int Accrued
               Inspect 'configurable_settings.xml' for:
                 <calculation_summary_columns>AttainedAge PolicyYear Outlay \
                  CSVNet_Current AcctVal_Current CSVNet_Guaranteed \
                  AcctVal_Guaranteed EOYDeathBft_Current EOYDeathBft_Guaranteed \
                  NetWD NewCashLoan LoanIntAccrued_Current </calculation_summary_columns>

          C. File | New | Illustration
             Illustration | Copy calculation summary
             paste the output for inspection, then
             Illustration | Copy full illustration data
             paste the output for inspection

          D. File | New | Census
             Census | Run case
             Illustration | Copy calculation summary
             paste the output for inspection, then
             Illustration | Copy full illustration data
             paste the output for inspection

    The output is pasted into 4 files called {New,Calc}Ill{Summary,Full}.txt in
    the current working directory.
 */
LMI_WX_TEST_CASE(calculation_summary)
{
    configurable_settings const& settings = configurable_settings::instance();

    use_builtin_calculation_summary(true);

    LMI_ASSERT(settings.calculation_summary_columns().empty());
    LMI_ASSERT(settings.use_builtin_calculation_summary());

    check_calculation_summary_columns
        (number_of_default_columns
        ,default_columns_info
        );

    use_builtin_calculation_summary(false);

    // Concatenate all the custom column names together. Notice that the
    // trailing space is intentional as it is present in the configurable
    // settings file too.
    std::string all_custom_columns;
    for(std::size_t n = 0; n < number_of_custom_columns; ++n)
        {
        all_custom_columns += custom_columns_info[n].name;
        all_custom_columns += ' ';
        }

    LMI_ASSERT_EQUAL(settings.calculation_summary_columns(), all_custom_columns);
    LMI_ASSERT(!settings.use_builtin_calculation_summary());

    check_calculation_summary_columns
        (number_of_custom_columns
        ,custom_columns_info
        );

    wxUIActionSimulator ui;

    wx_test_new_illustration ill;
    save_illustration_data("New");
    ill.close();

    wx_test_new_census census;
    ui.Char('r', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Run case"
    wxYield();

    save_illustration_data("Calc");

    // Close the illustration opened by "Run case".
    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    census.close();
}
