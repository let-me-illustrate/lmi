// Test printing census and illustration documents to PDF.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"
#include "wx_test_output_pdf.hpp"

#include <wx/docview.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

namespace
{

// Get the name used for the last created document: it depends on the tests
// that had been run previously, so get it from the document itself.
std::string get_current_document_name()
{
    wxDocManager const* const docm = wxDocManager::GetDocumentManager();
    LMI_ASSERT(docm);
    wxDocument const* const doc = docm->GetCurrentDocument();
    LMI_ASSERT(doc);

    return doc->GetUserReadableName().ToStdString();
}

// Return the suffix used for files created by printing the census.
std::string serial_suffix(int n)
{
    return wxString::Format(".%09d", n).ToStdString();
}

} // Unnamed namespace.

/// Test printing an illustration document to PDF.
///
/// Run these commands:
///   File | New | Illustration
///     [multiply specamt and pmts by 1000]
///     OK
///   File | Print to PDF
/// and verify that the PDF file was created; then erase it.
///
/// Design modified 2021-05-05: Formerly, lmi's default inputs
///   1,000,000 specamt
///      20,000 pmts
/// were simply multiplied by one thousand, to test ledger scaling
/// for enormous amounts. Due to a recent lmi change, the ratio of
/// those numbers would raise an alarum when used with a default
/// product for which
///   oe_min_single_premium_corr_mult == MinSinglePremiumType
/// To prevent that, changed the payment from $20M to $950M. Paying
/// $950M every year is of course absurd, but it's a good test of
/// ledger scaling.

LMI_WX_TEST_CASE(pdf_illustration)
{
    struct billion_dollar_illustration
        :public wxExpectModalBase<MvcController>
    {
        int OnInvoked(MvcController* dialog) const override
            {
            dialog->Show();
            wxYield();

            wx_test_focus_controller_child(*dialog, "SpecifiedAmount");

            wxUIActionSimulator ui;
            ui.Text("1000000000");
            wxYield();

            wx_test_focus_controller_child(*dialog, "Payment");

            ui.Text(" 950000000");
            wxYield();

            return wxID_OK;
            }

        wxString GetDefaultDescription() const override
            {
            return "edit cell dialog to test $1,000,000,000 scaling";
            }
    };

    // Create a new billion-dollar illustration.
    wx_test_new_illustration ill {billion_dollar_illustration()};

    // Ensure that the output file doesn't exist in the first place.
    output_pdf_existence_checker output_pdf(get_current_document_name());

    wxUIActionSimulator ui;
    ui.Char('i', wxMOD_CONTROL);    // "File | Print to PDF"
    wxYield();

    // Close the illustration, we don't need it any more.
    ill.close_discard_changes();

    // Finally check for the expected output file existence.
    LMI_ASSERT(output_pdf.exists());
}

/// Test printing a census document to PDF.
///
/// Run these commands:
///   File | New | Census
///   Census | Print case to PDF
/// and verify that two PDF files were created, one for the (single)
/// cell and one for the composite; then erase both files.
///
/// Once other tests have been specified and implemented, we may find
/// that this test is duplicative and consequently remove it.

LMI_WX_TEST_CASE(pdf_census)
{
    // Create a new census.
    wx_test_new_census census;

    // Remove the expected output files to avoid false positives if they are
    // already present and not created by the test.
    std::string const name = get_current_document_name();

    output_pdf_existence_checker
        composite_pdf(name + ".composite" + serial_suffix(0));

    output_pdf_existence_checker
        cell_pdf(name + serial_suffix(1));

    // Print the census to PDF.
    wxUIActionSimulator ui;
    ui.Char('i', wxMOD_CONTROL | wxMOD_SHIFT);  // "Census | Print case to PDF"
    wxYield();

    // Close the census, we don't need it any more.
    census.close();

    // Check the existence of the files, and then delete them.
    LMI_ASSERT(composite_pdf.exists());
    LMI_ASSERT(cell_pdf.exists());
}
