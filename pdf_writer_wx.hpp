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

#include "assert_lmi.hpp"

#include <wx/html/winpars.h>

#include <wx/pdfdc.h>

#include <memory>                       // std::unique_ptr

enum enum_output_mode
    {e_output_normal
    ,e_output_measure_only
    };

class pdf_writer_wx
{
  public:
    pdf_writer_wx
        (wxString const&    output_filename
        ,wxPrintOrientation orientation
        );

    pdf_writer_wx(pdf_writer_wx const&) = delete;
    pdf_writer_wx& operator=(pdf_writer_wx const&) = delete;

    ~pdf_writer_wx();

    // High level functions which should be preferably used if possible.
    int output_html
        (int x
        ,int y
        ,int width
        ,wxString const& html
        ,enum_output_mode output_mode = e_output_normal
        );

    void output_image
        (wxImage const&   image
        ,char const*      image_name
        ,double           scale
        ,int              x
        ,int*             pos_y
        ,enum_output_mode output_mode = e_output_normal
        );

    // Accessors allowing to use lower level wxDC API directly.
    wxDC& dc() { return pdf_dc_; }

    // Page metrics: the page width and height are the size of the page region
    // reserved for the normal contents, excluding horizontal and vertical
    // margins. Total width and height include the margins.
    int get_horz_margin() const;
    int get_vert_margin() const;
    int get_page_width()  const;
    int get_total_width() const;
    int get_page_bottom() const;

  private:
    wxPrintData print_data_;
    wxPdfDC pdf_dc_;
    wxHtmlWinParser html_parser_;

    wxSize const total_page_size_;
};

#endif // pdf_writer_wx_hpp
