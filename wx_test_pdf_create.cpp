// Test creating PDF output for census and illustration documents.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "wx_test_case.hpp"
#include "wx_test_new.hpp"

#include <wx/docview.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <boost/filesystem/operations.hpp>

namespace
{

// Get the name used for the last created document: it depends on the tests
// that had been ran previously, so get it from the document itself.
std::string get_current_document_name()
{
    wxDocManager const* const docm = wxDocManager::GetDocumentManager();
    LMI_ASSERT(docm);
    wxDocument const* const doc = docm->GetCurrentDocument();
    LMI_ASSERT(doc);

    return doc->GetUserReadableName().ToStdString();
}

// Build the path for the output PDF with the given base name.
fs::path make_pdf_path(std::string const& base_name)
{
    fs::path pdf_path(configurable_settings::instance().print_directory());
    pdf_path /= base_name + ".pdf";

    return pdf_path;
}

// Return the suffix used for the FO files created by printing the census.
std::string fo_suffix(int n)
{
    return wxString::Format(".%09d", n).ToStdString();
}

} // Unnamed namespace.

LMI_WX_TEST_CASE(pdf_illustration)
{
    // Create a new illustration.
    wx_test_new_illustration ill;

    // Ensure that the output file doesn't exist in the first place.
    fs::path const pdf_path(make_pdf_path(get_current_document_name()));
    fs::remove(pdf_path);

    // Launch the PDF creation as side effect of previewing it.
    wxUIActionSimulator ui;
    ui.Char('v', wxMOD_CONTROL);    // "File|Print preview"
    wxYield();

    // Close the illustration, we don't need it any more.
    ill.close();

    // Finally check for the expected output file existence.
    LMI_ASSERT(fs::exists(pdf_path));

    // Don't remove it here, the PDF file is still opened in the PDF reader and
    // can't be removed before it is closed.
}

LMI_WX_TEST_CASE(pdf_census)
{
    // Create a new census.
    wx_test_new_census census;

    // Add some cells to the census (it starts with one already).
    wxUIActionSimulator ui;
    static const int num_cells = 3;
    for(int n = 0; n < num_cells - 1; ++n)
        {
        ui.Char('+', wxMOD_CONTROL);    // "Census|Add cell".
        wxYield();
        }

    // Remove the expected output files to avoid false positives if they are
    // already present and not created by the test.
    std::string const name = get_current_document_name();

    fs::path const
        composite_pdf_path(make_pdf_path(name + ".composite" + fo_suffix(0)));
    fs::remove(composite_pdf_path);

    fs::path cell_pdf_paths[num_cells];
    for(int n = 0; n < num_cells; ++n)
        {
        cell_pdf_paths[n] = make_pdf_path(name + fo_suffix(n + 1));
        fs::remove(cell_pdf_paths[n]);
        }

    // Print the census to disk.
    ui.Char('k', wxMOD_CONTROL | wxMOD_SHIFT);  // "Census|Print case to disk"
    wxYield();

    // Close the census, we don't need it any more, and answer "No" to the
    // message box asking whether it should be saved.
    census.close_discard_changes();

    // Check the existence of the files and, unlike in the illustration case,
    // also delete them as they are not opened in any external viewer.
    LMI_ASSERT(fs::exists(composite_pdf_path));
    fs::remove(composite_pdf_path);
    for(int n = 0; n < num_cells; ++n)
        {
        LMI_ASSERT(fs::exists(cell_pdf_paths[n]));
        fs::remove(cell_pdf_paths[n]);
        }
}
