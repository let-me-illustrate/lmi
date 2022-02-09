// Validate a variety of input sequences in the GUI input dialog.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
#include "input.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <algorithm>                    // find()
#include <iostream>

/// Validate a variety of input sequences in the GUI input dialog.
///
/// Test a broad variety of input sequences. For now, use the set in
/// the user manual:
///   https://www.nongnu.org/lmi/sequence_input.html
/// but hard code them here--later they might differ, e.g. if we
/// decide to add extra tests here.
///
/// First, create a temporary '.ill' document:
///   File | New | Illustration
/// Then paste each input sequence into the appropriate field and test
/// it thus:
///  - Click the ellipsis button; press OK to close its dialog.
///  - Click OK to run the illustration. This step is tested because
///    it triggers downstream validation.
/// Reopen the tabbed dialog for each subsequent test. When done,
/// close the illustration without saving it.

namespace
{

// Combine the input sequence itself with the field it should be entered into.
struct input_sequence_test_data
{
    char const* field;
    char const* sequence;
};

input_sequence_test_data const test_cases[] =
{
    // These sequences correspond to the examples from the user manual.
    { "SpecifiedAmount" ,"sevenpay 7; 250000 retirement; 100000 #10; 75000 @95; 50000" },
    { "SpecifiedAmount" ,"100000; 110000; 120000; 130000; 140000; 150000"              },
    { "PaymentMode"     ,"annual; monthly"                                             },
    { "Payment"         ,"10000 20; 0"                                                 },
    { "Payment"         ,"10000 10; 5000 15; 0"                                        },
    { "Payment"         ,"10000 @70; 0"                                                },
    { "Payment"         ,"10000 retirement; 0"                                         },
    { "Payment"         ,"0 retirement; 5000"                                          },
    { "Payment"         ,"0 retirement; 5000 maturity"                                 },
    { "Withdrawal"      ,"0 retirement; 5000 #10; 0"                                   },
    { "Withdrawal"      ,"0,[0,retirement);10000,[retirement,#10);0"                   },

    // This is an additional sequence used solely in the present TU.
    { "ProjectedSalary" ,"100000; 105000; 110000 retirement; 0"                        },
};

} // Unnamed namespace.

LMI_WX_TEST_CASE(input_sequences)
{
    wx_test_new_illustration ill;

    struct test_sequence_dialog : public wxExpectModalBase<MvcController>
    {
        explicit test_sequence_dialog(input_sequence_test_data const& test_data)
            :test_data_ {test_data}
        {}

        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            char const* const field_name = test_data_.field;
            if(!dialog->FindWindow(field_name))
                {
                // Check whether the field name is valid at all. If it
                // isn't, then the input model must have changed, so
                // warn that this test must be updated.
                Input const dummy_input;
                std::vector<std::string> const& names = dummy_input.member_names();
                if(std::find(names.begin(), names.end(), field_name) == names.end())
                    {
                    std::cout
                        << "WARNING: unknown field name '"
                        << field_name
                        << "': 'test_cases' array probably needs to be updated."
                        << std::endl
                        ;
                    }

                // However, it is not an error if the field used by
                // this input sequence doesn't exist in the currently-
                // used skin--not all skins have all the fields--so
                // just skip it and continue testing the other ones.
                return wxID_CANCEL;
                }

            // Focus the field in which the sequence should be entered.
            wx_test_focus_controller_child(*dialog, field_name);

            // Type the sequence into it.
            wxUIActionSimulator ui;
            ui.Text(test_data_.sequence);
            wxYield();

            // Switch to the ellipsis button which should be next to it.
            ui.Char(WXK_TAB);
            wxYield();

            // Show the dialog for sequence entry and dismiss it immediately.
            ui.Char(WXK_SPACE);
            wxTEST_DIALOG
                (wxYield()
                ,wxExpectDismissableModal<wxDialog>(wxOK).
                    Describe("sequence entry dialog" + sequence_describe())
                );

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "cell properties dialog" + sequence_describe();
            }

        // Helper providing the description of the sequence tested by this
        // particular dialog, including it in the various descriptions should
        // make it easier to find the exact failing test if anything goes
        // wrong.
        wxString sequence_describe() const
            {
            return wxString::Format(" for sequence \"%s\"" ,test_data_.sequence);
            }

        input_sequence_test_data const& test_data_;
    };

    wxUIActionSimulator ui;
    for(auto const& test : test_cases)
        {
        ui.Char('e', wxMOD_CONTROL); // "Illustration|Edit Cell"
        wxTEST_DIALOG(wxYield(), test_sequence_dialog(test));
        }

    ill.close_discard_changes();
}
