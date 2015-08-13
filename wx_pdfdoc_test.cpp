// wxPdfDocument library unit test.
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

// This unit test checks that linking with wxPdfDocument library and
// generating the simplest possible document with it works.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "platform_dependent.hpp"       // access()
#include "test_tools.hpp"

#include <wx/pdfdoc.h>

#include <wx/init.h>

int test_main(int argc, char* argv[])
{
    wxInitializer init(argc, argv);
    BOOST_TEST( init );

    wxPdfDocument pdf_doc(wxLANDSCAPE, "pt", wxPAPER_LETTER);
    pdf_doc.AddPage();
    pdf_doc.SetFont("Helvetica", "", 16);
    pdf_doc.Text(20, 20, "Hello PDF!");

    char const* const p = "/tmp/eraseme.pdf";
    pdf_doc.SaveAsFile(p);
    BOOST_TEST_EQUAL(0, access(p, R_OK));
    BOOST_TEST(0 == std::remove(p));

    return 0;
}
