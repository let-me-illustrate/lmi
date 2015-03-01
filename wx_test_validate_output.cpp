// Test creation and naming of spreadsheet output files.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "mvc_controller.hpp"
#include "uncopyable_lmi.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

// Consider renaming this file to 'wx_test_spreadsheet_output.cpp'
// e.g., because its purpose is to test *spreadsheet* output only.
// To us at least, to "validate" a file suggests checking its
// contents, while "MEC output" suggests a '.mec.xml' file and
// "illustration output" most likely means a PDF file, so names like
//   validate_output_illustration
//   validate_output_mec
// suggest something quite different from what those functions do.

/// Test creation and naming of spreadsheet output files.
///
/// For simplicity, this description uses extension '.tsv' for
/// "spreadsheet" output; the actual tests use the configured
/// spreadsheet_file_extension().
///
/// File | New | Illustration
///   Comments: "idiosyncrasyZ"
///   OK
/// Verify that this file was created:
///   unnamed.monthly_trace.tsv
///
/// File | New | MEC testing
///   OK
/// Verify that this file was created:
///   unnamed.mec.tsv
///
/// THE TESTS BELOW ARE NOT IMPLEMENTED YET!
///
/// File | New | Census
/// Census | Edit case defaults
///   Comments: "idiosyncrasyZ"
///   Corporation name: "ABC Inc."
///   OK
///   Yes (apply to all)
/// Census | Edit cell
///   Insured name: John Brown
///   OK
/// Census | Add cell
/// File | Save as "ABC" [used in output file names]
/// Use this census for the tests below.
///
/// Census | Run case
/// Verify that these files were created:
///   ABC.John_Brown.000000001.monthly_trace.tsv
///   ABC.000000002.monthly_trace.tsv
/// ...and delete them both now.
///
/// Census | Print case to spreadsheet
/// Verify that these files were created:
///   ABC.John_Brown.000000001.monthly_trace.tsv
///   ABC.000000002.monthly_trace.tsv
///   ABC.cns.tsv
/// ...and delete all three now.
///
/// Census | Print roster to spreadsheet
/// Verify that these files were created:
///   ABC.John_Brown.000000001.monthly_trace.tsv
///   ABC.000000002.monthly_trace.tsv
//    ABC.cns.roster.tsv
/// ...and delete all three now.
///
/// Census | Edit case defaults
///   Comments: replace contents with "idiosyncrasy_spreadsheet"
///   OK
///   Yes (apply to all)
/// Census | Print case to PDF
/// Verify that this file was created:
///   values.tsv
///
/// select the "John Brown" cell
/// Census | Run cell
/// Illustration | Copy full illustration data [Ctrl-D]
/// Verify that this file was created:
///   ABC.John_Brown.000000001.ill.tsv
/// and that its contents have been placed on the clipboard.
///
/// [These two functions:
///   IllustrationView::UponCopyFull()    [Ctrl-D]
///   IllustrationView::UponCopySummary() [Ctrl-C]
/// are very similar, the former being almost a superset of the
/// latter. Testing both seems unnecessary; and it would be less
/// convenient to test the latter because it does not create a file.
/// The (tested) Ctrl-D option is much likelier to be used.]
///
/// Finally, delete all files created directly or indirectly above
/// that happen not to have been deleted already.

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

        virtual wxString GetDefaultDescription() const
            {
            return "illustration properties dialog";
            }
    };

    // Create a new illustration with the special comment.

    // Double parentheses circumvent the most vexing parse.
    wx_test_new_illustration ill((enter_comment_in_illustration_dialog()));
    ill.close_discard_changes();

    // And check that this resulted in the creation of the expected file.
    LMI_ASSERT(unnamed_trace.exists());
}

/// Validate spreadsheet output for MEC testing.
///
/// Someday the spreadsheet tested here may be eliminated; then this
/// test should be expunged.
///
/// It might be worthwhile to add a test like this one for any similar
/// GPT spreadsheet that may be developed.

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
        ,wxExpectDismissableModal<MvcController>(wxID_OK).
            Describe("new MEC parameters dialog")
        );

    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    LMI_ASSERT(unnamed_output.exists());
}
