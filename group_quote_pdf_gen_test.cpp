// Group premium quote PDF generation code unit test.
//
// Copyright (C) 2015 Gregory W. Chicares.
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

#include "force_linking.hpp"
#include "global_settings.hpp"
#include "illustrator.hpp"
#include "multiple_cell_document.hpp"
#include "path_utility.hpp"
#include "test_tools.hpp"

#include <wx/app.h>
#include <wx/image.h>
#include <wx/init.h>
#include <wx/html/forcelnk.h>

// PDF generator uses wxHtmlParser, so ensure that wxHTML is linked in.
FORCE_WXHTML_MODULES()

LMI_FORCE_LINKING_EX_SITU(group_quote_pdf_generator_wx)

// Although this is a console application, it uses GUI wxWidgets functionality
// via wxPdfDocument used to produce the group premiums PDF output, so it needs
// to have a full GUI application class.
wxIMPLEMENT_APP_NO_MAIN(wxApp);

int test_main(int argc, char* argv[])
{
    wxInitializer init(argc, argv);
    LMI_TEST( init );

    wxInitAllImageHandlers();

    // Skip system authentication.
    global_settings::instance().set_regression_testing(true);

    illustrator ill(static_cast<mcenum_emission>(mce_emit_group_quote | mce_emit_timings));
    ill("sample.cns");

    // TODO: remove sample.cns.quote.pdf

    return 0;
}
