// PDF generation helpers.
//
// Copyright (C) 2017 Gregory W. Chicares.
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

#ifndef pdf_writer_wx_hpp
#define pdf_writer_wx_hpp

#include "config.hpp"

#include <wx/html/winpars.h>

#include <wx/pdfdc.h>

class pdf_writer_wx
{
  public:
    pdf_writer_wx
        (wxString const&    output_filename
        ,wxPrintOrientation orientation
        );

    ~pdf_writer_wx();

    wxPdfDC& dc() { return pdf_dc_; }

    wxHtmlWinParser& html_parser() { return html_parser_; }

  private:
    wxPrintData print_data_;
    wxPdfDC pdf_dc_;
    wxHtmlWinParser html_parser_;
};

#endif // pdf_writer_wx_hpp
