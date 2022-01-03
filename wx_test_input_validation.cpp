// Test validation of input ranges in a census.
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
#include "bourn_cast.hpp"
#include "global_settings.hpp"
#include "mvc_controller.hpp"
#include "wx_test_case.hpp"
#include "wx_test_document.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <cstring>
#include <stdexcept>

/// Test validation of the COI multiplier input field.
///
/// Open the file CoiMultiplier.ill provided with the distribution and test
/// various values of the "CurrentCoiMultiplier" field in the dialog invoked by
///     Illustration | Edit cell
///
/// The following inputs must result in an error:
///     (a) Empty string value.
///     (b) Negative value.
///     (c) Zero value.
///
/// The following input must result in an error in the normal execution case
/// but not when one of the special back door command line arguments is
/// specified:
///     (d) Positive value less than the minimum (which is 0.9).
///
/// Finally, these inputs must not trigger any errors:
///     (e) Exactly the minimum value.
///     (f) Value of 1.
///     (g) Value greater than 1.
///
/// Errors are tested by catching the exceptions and examining their associated
/// messages and not by checking for the message boxes displayed by the program
/// because these message boxes are shown from OnExceptionInMainLoop() function
/// of the application object which behaves differently in the test suite.

namespace
{

/// The field value with the expected error message (or at least its
/// variable part--see below for how the full error message is
/// constructed), or nullptr if no error should be given.

struct coi_multiplier_test_data
{
    char const* value;
    char const* error;
};

coi_multiplier_test_data const test_cases[] =
{
    { ""    ,"COI multiplier entered is '', but it must contain at least one number other than zero." },
    { "-1"  ,"Lowest COI multiplier entered is -1, but 0.9 is the lowest multiplier allowed." },
    { "0"   ,"COI multiplier entered is '0', but it must contain at least one number other than zero." },
    { "0.8" ,"Lowest COI multiplier entered is 0.8, but 0.9 is the lowest multiplier allowed." },
    { "0.9" ,nullptr },
    { "1"   ,nullptr },
    { "1.1" ,nullptr },
};

} // Unnamed namespace.

LMI_WX_TEST_CASE(input_validation)
{
    skip_if_not_distribution();

    wx_test_existing_illustration ill(get_test_file_path_for("CoiMultiplier.ill"));

    struct test_coi_multiplier_dialog : public wxExpectModalBase<MvcController>
    {
        explicit test_coi_multiplier_dialog(char const* value)
            :value_ {value}
        {}

        test_coi_multiplier_dialog(test_coi_multiplier_dialog const&) = delete;
        test_coi_multiplier_dialog& operator=(test_coi_multiplier_dialog const&) = delete;

        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            wx_test_focus_controller_child(*dialog, "CurrentCoiMultiplier");

            wxUIActionSimulator ui;
            if(*value_ == '\0')
                {
                // Special case of the empty value: we must clear the entry
                // contents in this case, but emulating the input of "nothing"
                // wouldn't be enough to do it, so do it manually instead.
                ui.Char(WXK_SPACE);
                ui.Char(WXK_BACK);
                }
            else
                {
                ui.Text(value_);
                }
            wxYield();

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return wxString::Format
                ("edit cell dialog for testing COI multiplier \"%s\""
                ,value_
                );
            }

        char const* const value_;
    };

    for(auto const& td : test_cases)
        {
        // This flag is used to assert that all expected exceptions were
        // generated at the end of the loop. The reason for using it instead of
        // just asserting directly inside the "try" statement is that failing
        // assert also throws an exception which would have been caught by our
        // own "catch" clause and while we could test for it and rethrow the
        // exception if we recognize it as ours, it is finally simpler to just
        // avoid catching it in the first place.
        bool check_for_expected_exception = false;
        try
            {
            wxUIActionSimulator ui;
            ui.Char('e', wxMOD_CONTROL); // "Illustration|Edit Cell"
            wxTEST_DIALOG
                (wxYield()
                ,test_coi_multiplier_dialog(td.value)
                );

            // A special case: when using one of the special command line back
            // door options, the test for the minimal COI multiplier value is
            // skipped and doesn't result in the expected error -- which is
            // itself expected, so don't fail the test in this case.
            if
                (
                    !global_settings::instance().mellon()
                ||  std::strcmp(td.value, "0.8") != 0
                )
                {
                // Outside of this special case, do verify that we didn't miss
                // an expected exception below.
                check_for_expected_exception = true;
                }
            }
        catch(std::domain_error& e)
            {
            // This is another special case: normally a negative value would
            // fail the check comparing it with the lowest multiplier allowed,
            // however this check is disabled when one of the special command
            // line back door options is used. In this case the negative value
            // still doesn't pass a subsequent check in coi_rate_from_q(),
            // which is expected and doesn't constitute a test failure.
            // Anything else does however.
            LMI_ASSERT_WITH_MSG
                (
                    global_settings::instance().mellon()
                &&  td.value
                &&  td.value[0] == '-'
                &&  std::strcmp(e.what(), "q is negative.") == 0
                ,"COI multiplier value \""
                    << td.value
                    << "\" resulted in an unexpected domain error ("
                    << e.what()
                    << ")"
                );
            }
        catch(std::runtime_error& e)
            {
            std::string const error_message = e.what();

            LMI_ASSERT_WITH_MSG
                (td.error
                ,"COI multiplier value \""
                    << td.value
                    << "\" unexpectedly resulted in an error ("
                    << error_message
                    << ")"
                );

            // The error message always starts with the same prefix, discard it
            // to make the failure messages below in case of a difference
            // between the expected and actual errors more concise.
            char const* const
                error_prefix = "Input validation problems for '':\n";
            int error_prefix_len = bourn_cast<int>(std::strlen(error_prefix));

            LMI_ASSERT_WITH_MSG
                (error_message.substr(0, error_prefix_len) == error_prefix
                ,"Error message for COI multiplier value \""
                    << td.value
                    << "\" doesn't contain the expected prefix ("
                    << e.what()
                    << ")"
                );

            // The error message contains a line of the form "[file %s, line
            // %d]" at the end which we want to ignore, as the line number and
            // possibly the file name can change and are irrelevant to this
            // check anyhow, so find this line presence and ignore it in
            // comparison.
            std::string::size_type const
                loc_pos = error_message.find("\n\n[file", error_prefix_len);
            LMI_ASSERT_WITH_MSG
                (loc_pos != std::string::npos
                ,"Error message for COI multiplier value \""
                    << td.value
                    << "\" unexpectedly doesn't contain location information ("
                    << e.what()
                    << ")"
                );

            // Finally check that what remains, i.e. the real error message,
            // conforms to the expected one.
            std::string const validation_error = error_message.substr
                (error_prefix_len
                ,loc_pos - error_prefix_len
                );

            LMI_ASSERT_EQUAL(validation_error, td.error);
            }

        if(check_for_expected_exception)
            {
            LMI_ASSERT_WITH_MSG
                (!td.error
                ,"COI multiplier value \""
                    << td.value
                    << "\" didn't generate the expected error ("
                    << td.error
                    << ")"
                );
            }
        }

    ill.close_discard_changes();
}
