// Validate existence and naming conventions of output files.
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
#include "mvc_controller.hpp"
#include "uncopyable_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <boost/filesystem/operations.hpp>

namespace
{

/// Class helping to check for the expected output file existence.
///
/// This class takes care of ensuring that the file doesn't exist when it is
/// constructed and provides a way to check for the existence of the file
/// later. It also cleans up the file when it is destroyed.

class output_file_existence_checker
    :private lmi::uncopyable<output_file_existence_checker>
{
  public:
    output_file_existence_checker(std::string const& path)
        :path_(path)
        {
        fs::remove(path_);
        }

    bool exists() const
        {
        return fs::exists(path_);
        }

    ~output_file_existence_checker()
        {
        try
            {
            fs::remove(path_);
            }
        catch(...)
            {
            }
        }

  private:
    fs::path path_;
};

} // Unnamed namespace.

/*
    Add test to validate existence of the expected output files.

    This implements the following item of the testing specification:

        14. Validate existence and naming conventions of output for
            named and unnamed files.

          A. File | New | Illustration | enter 'idiosyncrasyZ' in
             'Comments' | OK
             Expected results:
               file 'unnamed.monthly_trace.tsv' exists

          B. File | Open | 'MonthlyTrace.ill' | press 'OK' to
              [dismiss message box] | press 'OK' to run illustration
             Expected results:
               file 'MonthlyTrace.monthly_trace.tsv' exists

          C. File | New | MEC testing | OK
             Expected results:
               file 'unnamed.mec.tsv' exists

          D. File | Open | 'MecTesting.mec' | OK
             Expected results:
               file 'MecTesting.mec.tsv' exists

    The currently configured spreadsheet file extension is used instead of the
    hard-coded "tsv", otherwise the tests are implemented exactly as specified.
 */
LMI_WX_TEST_CASE(validate_output_illustration)
{
    std::string const&
        ext = configurable_settings::instance().spreadsheet_file_extension();

    // Build the path existence of which we're going to check and ensure that
    // it doesn't exist before the start of the test.
    output_file_existence_checker unnamed_trace("unnamed.monthly_trace" + ext);

    struct enter_comment_in_illustration_dialog
        :public wxExpectModalBase<MvcController>
    {
        virtual int OnInvoked(MvcController* dialog) const
            {
            dialog->Show();
            wxYield();

            wxUIActionSimulator ui;

            // Go to the first page: as the dialog remembers its last opened
            // page, ensure that we are always on the one we need.
            ui.Char(WXK_HOME);
            wxYield();

            // It is difficult to focus the text entry that we're interested
            // directly from keyboard, so cheat a little and focus it
            // programmatically.
            wxWindow* const comments = wxWindow::FindWindowByName
                ("Comments"
                ,dialog
                );
            LMI_ASSERT(comments);

            comments->SetFocus();
            wxYield();

            ui.Text("idiosyncrasyZ");
            wxYield();

            return wxID_OK;
            }
    };

    // Create a new illustration with the special comment.

    // Double parentheses circumvent the most vexing parse.
    wx_test_new_illustration ill((enter_comment_in_illustration_dialog()));
    ill.close_discard_changes();

    // And check that this resulted in the creation of the expected file.
    LMI_ASSERT(unnamed_trace.exists());

    // Open an existing illustration already containing the same comment.
    wxUIActionSimulator ui;

    output_file_existence_checker
        existing_trace("MonthlyTrace.monthly_trace" + ext);

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(get_test_file_path_for("MonthlyTrace.ill"))
        ,wxExpectModal<wxMessageDialog>(wxID_OK)          // Ignore warning.
        ,wxExpectDismissableModal<MvcController>(wxID_OK) // Accept defaults.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxTEST_DIALOG(wxYield(), wxExpectModal<wxMessageDialog>(wxNO));

    LMI_ASSERT(existing_trace.exists());
}

LMI_WX_TEST_CASE(validate_output_mec)
{
    skip_if_not_supported("unnamed.mec");

    std::string const&
        ext = configurable_settings::instance().spreadsheet_file_extension();

    // Test creation of the output file when opening a new MEC testing document.
    output_file_existence_checker unnamed_output("unnamed.mec" + ext);

    wxUIActionSimulator ui;
    ui.Char('n', wxMOD_CONTROL);    // "File|New"
    ui.Char('m');                   // "MEC testing"

    wxTEST_DIALOG
        (wxYield()
        ,wxExpectDismissableModal<MvcController>(wxID_OK)
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    LMI_ASSERT(unnamed_output.exists());

    // And when opening an existing one.
    output_file_existence_checker existing_output("MecTesting.mec" + ext);

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
         ,wxExpectModal<wxFileDialog>(get_test_file_path_for("MecTesting.mec"))
         ,wxExpectDismissableModal<MvcController>(wxID_OK) // Accept defaults.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    LMI_ASSERT(existing_output.exists());
}
