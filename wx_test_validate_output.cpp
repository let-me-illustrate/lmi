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
#include "wx_test_mvc_dialog.hpp"
#include "wx_test_new.hpp"

#include <wx/filename.h>
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

    void assert_exists() const
        {
        LMI_ASSERT(fs::exists(path_));
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

LMI_WX_TEST_CASE(validate_output_illustration)
{
    std::string const&
        ext = configurable_settings::instance().spreadsheet_file_extension();

    // Build the path existence of which we're going to check and ensure that
    // it doesn't exist before the start of the test.
    output_file_existence_checker unnamed_trace("unnamed.monthly_trace" + ext);

    struct enter_comment_in_illustration_dialog
        :public ExpectMvcDialog
    {
        virtual void DoRunDialog(MvcController* dialog) const
            {
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
            }
    };

    // Create a new illustration with the special comment.

    // Double parentheses circumvent the most vexing parse.
    wx_test_new_illustration ill((enter_comment_in_illustration_dialog()));
    ill.close_discard_changes();

    // And check that this resulted in the creation of the expected file.
    unnamed_trace.assert_exists();

    // Open an existing illustration already containing the same comment.
    wxUIActionSimulator ui;

    output_file_existence_checker
        existing_trace("MonthlyTrace.monthly_trace" + ext);

    wxFileName fn(configurable_settings::instance().default_input_filename());
    fn.SetFullName("MonthlyTrace.ill");

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
        ,wxExpectModal<wxMessageDialog>(wxID_OK)          // Ignore warning.
        ,wxExpectDismissableModal<MvcController>(wxID_OK) // Accept defaults.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxTEST_DIALOG(wxYield(), wxExpectModal<wxMessageDialog>(wxNO));

    existing_trace.assert_exists();
}

LMI_WX_TEST_CASE(validate_output_mec)
{
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

    unnamed_output.assert_exists();

    // And when opening an existing one.
    output_file_existence_checker existing_output("MecTesting.mec" + ext);

    wxFileName fn(configurable_settings::instance().default_input_filename());
    fn.SetFullName("MecTesting.mec");

    ui.Char('o', wxMOD_CONTROL);    // "File|Open"
    wxTEST_DIALOG
        (wxYield()
         ,wxExpectModal<wxFileDialog>(fn.GetFullPath())
         ,wxExpectDismissableModal<MvcController>(wxID_OK) // Accept defaults.
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    existing_output.assert_exists();
}
