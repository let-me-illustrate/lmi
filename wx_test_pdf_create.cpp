// Test printing census and illustration documents to PDF.
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
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"
#include "wx_test_output.hpp"

#include <wx/docview.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <boost/filesystem/operations.hpp>

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

// Return the suffix used for the FO files created by printing the census.
std::string fo_suffix(int n)
{
    return wxString::Format(".%09d", n).ToStdString();
}

// Specialized version of output_file_existence_checker for the output PDF
// files: it takes just the base name of the file, without neither the
// directory part nor the .pdf extension, in its ctor and also takes care of
// deleting the .fo.xml created as a side effect of PDF generation when the PDF
// file itself is removed.
class output_pdf_existence_checker :public output_file_existence_checker
{
  public:
    explicit output_pdf_existence_checker(std::string const& base_name)
        :output_file_existence_checker
            (make_full_print_path(base_name + ".pdf")
            )
        ,fo_xml_path_
            (make_full_print_path(base_name + ".fo.xml")
            )
        {
        // We do not remove .fo.xml file here, this is unnecessary as we don't
        // particularly care whether it exists or not because we never check
        // for its existence.
        }

    ~output_pdf_existence_checker()
        {
        // Do remove the .fo.xml file to avoid littering the print directory
        // with the files generated during the test run.
        try
            {
            fs::remove(fo_xml_path_);
            }
        catch(...)
            {
            }
        }

  private:
    // Return the full path in the print directory for the file with the given
    // leaf name.
    static fs::path make_full_print_path(std::string const& leaf)
        {
        fs::path p(configurable_settings::instance().print_directory());
        p /= leaf;
        return p;
        }

    fs::path fo_xml_path_;
};

} // Unnamed namespace.

/// Test printing an illustration document to PDF.
///
/// Run these commands:
///   File | New | Illustration
///   File | Print to PDF
/// and verify that the PDF file was created; then erase it.

LMI_WX_TEST_CASE(pdf_illustration)
{
    // Create a new illustration.
    wx_test_new_illustration ill;

    // Ensure that the output file doesn't exist in the first place.
    output_pdf_existence_checker output_pdf(get_current_document_name());

    wxUIActionSimulator ui;
    ui.Char('i', wxMOD_CONTROL);    // "File | Print to PDF"
    wxYield();

    // Close the illustration, we don't need it any more.
    ill.close();

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
        composite_pdf(name + ".composite" + fo_suffix(0));

    output_pdf_existence_checker
        cell_pdf(name + fo_suffix(1));

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

