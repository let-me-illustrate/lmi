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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "wx_test_case.hpp"

#include <wx/testing.h>
#include <wx/uiaction.h>

#include <boost/filesystem/operations.hpp>

LMI_WX_TEST_CASE(pdf_illustration)
{
    // Create a new illustration.
    wxUIActionSimulator ui;
    ui.Char('n', wxMOD_CONTROL);    // "File|New"
    ui.Char('i');                   // "Illustration"

    wxTEST_DIALOG(wxYield(), wxExpectAny(wxID_OK));

    // Get the name used for it: it depends on the tests that had been ran
    // previously, so get it from the document itself.
    wxDocManager const* const docm = wxDocManager::GetDocumentManager();
    LMI_ASSERT(docm);
    wxDocument const* const doc = docm->GetCurrentDocument();
    LMI_ASSERT(doc);
    wxString const name = doc->GetUserReadableName();

    // Ensure that the output file doesn't exist in the first place.
    fs::path pdf_path(configurable_settings::instance().print_directory());
    pdf_path /= name.ToStdString() + ".pdf";
    fs::remove(pdf_path);

    // Launch the PDF creation as side effect of previewing it.
    ui.Char('v', wxMOD_CONTROL);    // "File|Print preview"
    wxYield();

    // Close the illustration, we don't need it any more.
    ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    wxYield();

    // Finally check for the expected output file existence.
    LMI_ASSERT(fs::exists(pdf_path));

    // Don't remove it here, the PDF file is still opened in the PDF reader and
    // can't be removed before it is closed.
}
