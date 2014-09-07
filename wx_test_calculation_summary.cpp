// Test calculation summary features.
//
// Copyright (C) 2014 Gregory W. Chicares.
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
#include "wx_test_case.hpp"
#include "wx_test_mvc_dialog.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cstddef>                          // std::size_t

namespace
{

// Names of the columns used when not using the built-in calculation summary.
char const* const custom_columns_names[] =
    {"AttainedAge"
    ,"PolicyYear"
    ,"Outlay"
    ,"CSVNet_Current"
    ,"AcctVal_Current"
    ,"CSVNet_Guaranteed"
    ,"AcctVal_Guaranteed"
    ,"EOYDeathBft_Current"
    ,"EOYDeathBft_Guaranteed"
    ,"NetWD"
    ,"NewCashLoan"
    ,"LoanIntAccrued_Current"
    };

std::size_t const number_of_custom_columns
    = sizeof custom_columns_names / sizeof(custom_columns_names[0]);

// Special name used when the column is not used at all.
char const* const magic_null_column_name = "[none]";


// Change the calculation summary settings in the preferences dialog to use, or
// not use, the built-in defaults.
void use_builtin_calculation_summary(bool b)
{
    wxUIActionSimulator ui;
    ui.Char('f', wxMOD_CONTROL);    // "File|Preferences"

    class ChangeCalculationSummaryInPreferencesDialog
        :public ExpectMvcDialog
    {
    public:
        ChangeCalculationSummaryInPreferencesDialog
            (bool use_builtin_summary)
            :use_builtin_summary_(use_builtin_summary)
            {
            }

        virtual void DoRunDialog(MvcController* dialog) const
            {
            wxUIActionSimulator ui;

            // Go to the "Use built-in calculation summary" checkbox.
            ui.Char(WXK_TAB);
            wxYield();

            // Disable the checkbox initially as we need it to be disabled to
            // change the values of the column controls.
            ui.Char('-');
            wxYield();

            DoUpdateDialogUI(dialog);

            // Update the columns controls when using them.
            for(std::size_t n = 0; n < number_of_custom_columns; ++n)
                {
                ui.Char(WXK_TAB);
                wxYield();

                wxString const column_name
                    (use_builtin_summary_
                    ? magic_null_column_name
                    : custom_columns_names[n]
                    );

                LMI_ASSERT(ui.Select(column_name));

                DoUpdateDialogUI(dialog);
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
            }

    private:
        bool const use_builtin_summary_;
    };

    wxTEST_DIALOG
        (wxYield()
        ,ChangeCalculationSummaryInPreferencesDialog(b)
        );
}

} // Unnamed namespace.

LMI_WX_TEST_CASE(calculation_summary)
{
    configurable_settings const& settings = configurable_settings::instance();


    use_builtin_calculation_summary(true);

    LMI_ASSERT(settings.calculation_summary_columns().empty());
    LMI_ASSERT(settings.use_builtin_calculation_summary());


    use_builtin_calculation_summary(false);

    // Concatenate all the custom column names together. Notice that the
    // trailing space is intentional as it is present in the configurable
    // settings file too.
    std::string all_custom_columns;
    for(std::size_t n = 0; n < number_of_custom_columns; ++n)
        {
        all_custom_columns += custom_columns_names[n];
        all_custom_columns += ' ';
        }

    LMI_ASSERT(settings.calculation_summary_columns() == all_custom_columns);
    LMI_ASSERT(!settings.use_builtin_calculation_summary());
}
