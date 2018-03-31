// PDF generation helpers.
//
// Copyright (C) 2017, 2018 Gregory W. Chicares.
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
#include "oecumenic_enumerations.hpp"

#include <wx/html/winpars.h>

#include <wx/pdfdc.h>

#include <array>
#include <memory>                       // unique_ptr

class wxFileSystem;

namespace html { class text; }

class pdf_writer_wx
{
  public:
    // Optional html_font_sizes array allows to override default font sizes for
    // the standard HTML3 fonts (1..7).
    pdf_writer_wx
        (wxString const&           output_filename
        ,wxPrintOrientation        orientation
        ,std::array<int, 7> const* html_font_sizes = nullptr
        );

    pdf_writer_wx(pdf_writer_wx const&) = delete;
    pdf_writer_wx& operator=(pdf_writer_wx const&) = delete;

    // Dtor checks if save() had been called, so don't forget to do it.
    ~pdf_writer_wx();

    // Save the PDF to the output file name specified in the ctor.
    //
    // This object becomes unusable after saving, i.e. no other methods can be
    // called on it. To help with preventing using any of them accidentally,
    // this method is rvalue-reference-qualified, meaning that calling
    // std::move() is required to call it.
    void save() &&;

    // High level functions which should be preferably used if possible.
    int output_html
        (int                          x
        ,int                          y
        ,int                          width
        ,html::text&&                 html
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_image
        (wxImage const&               image
        ,char const*                  image_name
        ,double                       scale
        ,int                          x
        ,int*                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    // Accessors allowing to use lower level wxDC API directly.
    wxDC& dc();

    // Page metrics: the page width and height are the size of the page region
    // reserved for the normal contents, excluding horizontal and vertical
    // margins. Total width and height include the margins.
    int get_horz_margin() const;
    int get_vert_margin() const;
    int get_page_width()  const;
    int get_total_width() const;
    int get_page_height() const;
    int get_page_bottom() const;

  private:
    wxPrintData print_data_;
    wxPdfDC pdf_dc_;

    // Order is potentially important here: html_parser_ uses html_vfs_, so
    // must be declared after it in order to be destroyed before it.
    std::unique_ptr<wxFileSystem> html_vfs_;
    wxHtmlWinParser html_parser_;

    wxSize const total_page_size_;

    // Set to true after save() was called.
    bool was_saved_{false};
};

#endif // pdf_writer_wx_hpp
