// Test creation and naming of spreadsheet output files.
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
#include "path.hpp"
#include "path_utility.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"
#include "wx_test_output_pdf.hpp"
#include "wx_utility.hpp"

#include <wx/ffile.h>
#include <wx/testing.h>
#include <wx/textctrl.h>
#include <wx/uiaction.h>

namespace
{

// Return the suffix for the spreadsheet files.
//
// This is just a shorter synonym for the existing function.
inline
std::string const& tsv_ext()
{
    return configurable_settings::instance().spreadsheet_file_extension();
}

// Return the suffix with the serial number appended to the file names.
std::string serial_suffix(int n)
{
    return wxString::Format(".%09d", n).ToStdString();
}

// Return the suffix for monthly trace files: this includes the serial part
// (with its leading period) and the spreadsheet extension.
std::string monthly_trace_suffix(int n)
{
    return serial_suffix(n) + ".monthly_trace" + tsv_ext();
}

// An expectation class expecting an MvcController dialog shown by the
// "Census|Edit case defaults" menu item and filling in its "Comments" field
// with the given string.
struct enter_comments_in_case_defaults_dialog
    :public wxExpectModalBase<MvcController>
{
    enter_comments_in_case_defaults_dialog(std::string const& comments)
        :comments_ {comments}
        {
        }

    int OnInvoked(MvcController* dialog) const override
        {
        dialog->Show();
        wxYield();

        wxWindow* const comments_window = wx_test_focus_controller_child
            (*dialog
            ,"Comments"
            );

        wxTextCtrl* const
            comments_text = dynamic_cast<wxTextCtrl*>(comments_window);
        LMI_ASSERT(comments_text);

        wxUIActionSimulator ui;

        // There could be an existing comment in this field, delete it first.
        // This does assume MSW-like key bindings.
        ui.Char(WXK_HOME);
        ui.Char(WXK_END, wxMOD_SHIFT);
        ui.Char(WXK_BACK);

        ui.Text(comments_.c_str());
        wxYield();

        LMI_ASSERT_EQUAL(comments_text->GetValue(), comments_);

        return wxID_OK;
        }

    wxString GetDefaultDescription() const override
        {
        return "case defaults dialog";
        }

    std::string const comments_;
};

// Prepare the census for testing using the given corporation and insured names.
void init_test_census
        (std::string const& corp_name
        ,std::string const& insured_name
        )
{
    wxUIActionSimulator ui;

    // Enter the special comment as well as the corporation name into the "case
    // defaults" dialog.
    struct change_corp_in_case_defaults_dialog
        :public enter_comments_in_case_defaults_dialog
    {
        change_corp_in_case_defaults_dialog(std::string const& corp_name)
            :enter_comments_in_case_defaults_dialog("idiosyncrasyZ")
            ,corp_name_                            {corp_name}
            {
            }

        int OnInvoked(MvcController* dialog) const override
            {
            enter_comments_in_case_defaults_dialog::OnInvoked(dialog);

            wx_test_focus_controller_child(*dialog, "CorporationName");

            wxUIActionSimulator ui;
            ui.Text((corp_name_ + " Inc.").c_str());
            wxYield();

            return wxID_OK;
            }

        std::string const corp_name_;
    };

    ui.Char('e', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Edit case defaults"
    wxTEST_DIALOG
        (wxYield()
        ,change_corp_in_case_defaults_dialog(corp_name)
        ,wxExpectModal<wxMessageDialog>(wxYES).
            Describe("message box asking whether to apply changes to all cells")
        );

    // Change the cell.
    struct change_name_in_cell_dialog
        :public wxExpectModalBase<MvcController>
    {
        change_name_in_cell_dialog(std::string const& insured_name)
            :insured_name_ {insured_name}
            {
            }

        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            wx_test_focus_controller_child(*dialog, "InsuredName");

            wxUIActionSimulator ui;
            ui.Text(insured_name_.c_str());
            wxYield();

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "census cell dialog";
            }

        std::string const& insured_name_;
    };

    ui.Char('e', wxMOD_CONTROL); // "Census|Edit cell"
    wxTEST_DIALOG
        (wxYield()
        ,change_name_in_cell_dialog(insured_name)
        );

    ui.Char('+', wxMOD_CONTROL); // "Census|Add cell"
    wxYield();
}

// Helper function for several operations producing monthly trace outputs: runs
// the specified function actually performing the operation in question and
// checks that doing this did create the expected files.
//
// The "what" argument describes the operation and is only used for diagnostic
// purposes if the test fails.
void do_validate_monthly_trace
        (std::string const& corp_name
        ,std::string const& insured_name
        ,void (*operation)()
        ,char const* what
        )
{
    fs::path p(configurable_settings::instance().print_directory());
    std::string const cell_trace_file
        (p.string() + "/" + corp_name + "." + insured_name + monthly_trace_suffix(1)
        );
    output_file_existence_checker output_cell_trace(cell_trace_file);

    std::string const census_trace_file
        (p.string() + "/" + corp_name + monthly_trace_suffix(2)
        );

    output_file_existence_checker output_census_trace(census_trace_file);

    (*operation)();

    LMI_ASSERT_WITH_MSG
        (output_cell_trace.exists()
        ,"file \"" << cell_trace_file << "\" after " << what
        );
    LMI_ASSERT_WITH_MSG
        (output_census_trace.exists()
        ,"file \"" << census_trace_file << "\" after " << what
        );
}

void validate_run_case_output
        (std::string const& corp_name
        ,std::string const& insured_name
        )
{
    // This struct is used only as a way to define a local perform() function
    // and keep all the logic of this test self-contained.
    struct run_case
        {
        static void perform()
            {
                wxUIActionSimulator ui;
                ui.Char('r', wxMOD_CONTROL | wxMOD_SHIFT); // "Census|Run case"
                wxYield();

                // Close the illustration opened by "Run case".
                ui.Char('l', wxMOD_CONTROL);    // "File|Close"
                wxYield();
            }
        };

    do_validate_monthly_trace
        (corp_name
        ,insured_name
        ,run_case::perform
        ,"run case"
        );
}

void validate_print_case_output
        (std::string const& corp_name
        ,std::string const& insured_name
        )
{
    fs::path p(configurable_settings::instance().print_directory());
    std::string const census_file((p / (corp_name + tsv_ext())).string());
    output_file_existence_checker output_census(census_file);

    struct print_case
        {
        static void perform()
            {
                wxUIActionSimulator ui;
                // "Census|Print case to spreadsheet"
                ui.Char('h', wxMOD_CONTROL | wxMOD_SHIFT);
                wxYield();
            }
        };

    do_validate_monthly_trace
        (corp_name
        ,insured_name
        ,print_case::perform
        ,"print case"
        );

    LMI_ASSERT_WITH_MSG
        (output_census.exists()
        ,"file \"" << census_file << "\" after print roster"
        );
}

void validate_print_roster_output
        (std::string const& corp_name
        ,std::string const& insured_name
        )
{
    fs::path p(configurable_settings::instance().print_directory());
    std::string const roster_file((p / (corp_name + ".roster" + tsv_ext())).string());
    output_file_existence_checker output_roster(roster_file);

    struct print_roster
        {
        static void perform()
            {
                wxUIActionSimulator ui;

                // "Census|Print group roster..."
                ui.Char('o', wxMOD_CONTROL | wxMOD_SHIFT);

                // Select "Print roster to spreadsheet" from the popup menu.
                ui.Char('o');

                wxYield();
            }
        };

    do_validate_monthly_trace
        (corp_name
        ,insured_name
        ,print_roster::perform
        ,"print roster"
        );

    LMI_ASSERT_WITH_MSG
        (output_roster.exists()
        ,"file \"" << roster_file << "\" after print roster"
        );
}

void validate_run_cell_and_copy_output
        (std::string const& corp_name
        ,std::string const& insured_name
        )
{
    fs::path p(configurable_settings::instance().print_directory());
    std::string const cell_trace_file
        (p.string() + "/" + corp_name + "." + insured_name + monthly_trace_suffix(1)
        );
    output_file_existence_checker output_cell_trace(cell_trace_file);

    wxUIActionSimulator ui;

    ui.Char(WXK_ESCAPE);              // Clear any selection.
    ui.Char(WXK_HOME, wxMOD_CONTROL); // Move focus to top row.

    ui.Char('r', wxMOD_CONTROL);      // "Census|Run cell"
    wxYield();

    LMI_ASSERT_WITH_MSG
        (output_cell_trace.exists()
        ,"file \"" << cell_trace_file << "\" after running the cell"
        );

    std::string const ill_data_file
        (corp_name + "." + insured_name + serial_suffix(1) + tsv_ext()
        );
    output_file_existence_checker output_ill_data(ill_data_file);

    ui.Char('d', wxMOD_CONTROL); // "Illustration|Copy full illustration data"
    wxYield();

    // Close the illustration window opened by running the cell in any case.
    ui.Char('l', wxMOD_CONTROL); // "File|Close"
    wxYield();

    LMI_ASSERT_WITH_MSG
        (output_ill_data.exists()
        ,"file \"" << ill_data_file << "\" after copying illustration data"
        );

    // Also check that the contents of the file was placed on clipboard.
    wxString contents;
    LMI_ASSERT(wxFFile(ill_data_file).ReadAll(&contents));

    LMI_ASSERT_EQUAL
        (wxString(ClipboardEx::GetText())
        ,contents
        );
}

} // Unnamed namespace.

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
///   ABC.tsv
/// ...and delete all three now.
///
/// Census | Print group roster... | Print roster to spreadsheet
/// Verify that these files were created:
///   ABC.John_Brown.000000001.monthly_trace.tsv
///   ABC.000000002.monthly_trace.tsv
///   ABC.roster.tsv
/// ...and delete all three now.
///
/// select the "John Brown" cell
/// Census | Run cell
/// Illustration | Copy full illustration data [Ctrl-D]
/// Verify that this file was created:
///   ABC.John_Brown.000000001.tsv
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
    // Build the path existence of which we're going to check and ensure that
    // it doesn't exist before the start of the test.
    fs::path p(configurable_settings::instance().print_directory());
    output_file_existence_checker
        unnamed_trace(p.string() + "/" + "unnamed.monthly_trace" + tsv_ext());

    struct enter_comment_in_illustration_dialog
        :public wxExpectModalBase<MvcController>
    {
        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            wx_test_focus_controller_child(*dialog, "Comments");

            wxUIActionSimulator ui;
            ui.Text("idiosyncrasyZ");
            wxYield();

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "illustration properties dialog";
            }
    };

    // Create a new illustration with the special comment.
    wx_test_new_illustration ill {enter_comment_in_illustration_dialog()};
    ill.close_discard_changes();

    // And check that this resulted in the creation of the expected file.
    LMI_ASSERT(unnamed_trace.exists());
}

LMI_WX_TEST_CASE(validate_output_census)
{
    // Arbitrary names used for testing.
    std::string const corp_name = "ABC";
    std::string const insured_name = "John Brown";

    // A variant of the insured name used to construct the file names.
    //
    // It's not really clear whether we should be using portable_filename()
    // here or just hardcode its result corresponding to the insured_name
    // value: the latter would have the advantage of catching any bugs in
    // portable_filename() itself, but arguably we're not testing this function
    // here and using it has the advantage of avoiding test breakages if this
    // function behaviour is intentionally changed in the future.
    std::string const insured_filename = portable_filename(insured_name);

    // Create, initialize and save the test census.
    wx_test_new_census census;

    init_test_census(corp_name, insured_name);

    std::string const
        census_file_name = get_test_file_path_for(corp_name + ".cns");
    output_file_existence_checker output_cns(census_file_name);

    wxUIActionSimulator ui;
    ui.Char('a', wxMOD_CONTROL); // "File|Save as"
    wxTEST_DIALOG
        (wxYield()
        ,wxExpectModal<wxFileDialog>(census_file_name).
            Describe("census save file dialog")
        );

    LMI_ASSERT(output_cns.exists());

    // Check that the expected output files are created by different operations.
    validate_run_case_output(corp_name, insured_filename);
    validate_print_case_output(corp_name, insured_filename);
    validate_print_roster_output(corp_name, insured_filename);

    validate_run_cell_and_copy_output(corp_name, insured_filename);

    census.close();
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

    // Test creation of the output file when opening a new MEC testing document.
    output_file_existence_checker unnamed_output("unnamed.mec" + tsv_ext());

    // We don't really care about this file existence, but it is created as a
    // side effect of running this test and we want to ensure it isn't left
    // over and just creating this object satisfies this requirement.
    output_file_existence_checker unnamed_xml("unnamed.mec.xml");

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
