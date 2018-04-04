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

#include "pchfile_wx.hpp"

#include "pdf_writer_wx.hpp"

#include "alert.hpp"                    // safely_show_message()
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "html.hpp"

#include <wx/filesys.h>
#include <wx/html/htmlcell.h>

#include <exception>                    // uncaught_exceptions()
#include <limits>

namespace
{

// These margins are arbitrary and can be changed to conform to subjective
// preferences.
constexpr int horz_margin = 18;
constexpr int vert_margin = 24;

wxPrintData make_print_data
    (wxString const&    output_filename
    ,wxPrintOrientation orientation
    )
{
    wxPrintData print_data;
    print_data.SetPaperId(wxPAPER_LETTER);
    print_data.SetFilename(output_filename);
    print_data.SetOrientation(orientation);
    return print_data;
}

} // Unnamed namespace.

pdf_writer_wx::pdf_writer_wx
    (wxString const&    output_filename
    ,wxPrintOrientation orientation
    ,std::array<int, 7> const* html_font_sizes
    )
    :print_data_        {make_print_data(output_filename, orientation)}
    ,pdf_dc_            {print_data_}
    ,html_parser_       {nullptr}
    ,total_page_size_   {pdf_dc_.GetSize()}
{
    // Ensure that the output is independent of the current display resolution:
    // it seems that this is only the case with the PDF map mode and wxDC mode
    // different from wxMM_TEXT.
    pdf_dc_.SetMapModeStyle(wxPDF_MAPMODESTYLE_PDF);

    // For simplicity, use points for everything: font sizers are expressed in
    // them anyhow, so it's convenient to use them for everything else too.
    pdf_dc_.SetMapMode(wxMM_POINTS);

    pdf_dc_.StartDoc(wxString()); // Argument is not used.
    pdf_dc_.GetPdfDocument()->SetCompression
        (
        !contains(global_settings::instance().pyx(), "uncompressed_pdf")
        );
    pdf_dc_.StartPage();

    // Use a standard PDF Helvetica font (without embedding any custom fonts in
    // the generated file, the only other realistic choice is Times New Roman).
    pdf_dc_.SetFont
        (wxFontInfo
            (html_font_sizes
                ? html_font_sizes->at(2)
                : 8
            )
            .Family(wxFONTFAMILY_SWISS)
            .FaceName("Helvetica")
        );

    // Create an HTML parser to allow easily adding HTML contents to the output.
    html_parser_.SetDC(&pdf_dc_);
    if(html_font_sizes)
        {
        html_parser_.SetFonts
            ("Helvetica"
            ,"Courier"
            ,html_font_sizes->data()
            );
        }
    else
        {
        html_parser_.SetStandardFonts
            (pdf_dc_.GetFont().GetPointSize()
            ,"Helvetica"
            ,"Courier"
            );
        }

    // Create the virtual file system object for loading images referenced from
    // HTML and interpret relative paths from the data directory.
    html_vfs_.reset(new wxFileSystem());
    html_vfs_->ChangePathTo
        (global_settings::instance().data_directory().string()
        ,true /* argument is a directory, not file path */
        );
    html_parser_.SetFS(html_vfs_.get());
}

wxDC& pdf_writer_wx::dc()
{
    LMI_ASSERT(!save_has_been_called_);
    return pdf_dc_;
}

/// Output an image at the given scale into the PDF.
///
/// The scale specifies how many times the image should be shrunk:
/// scale > 1 makes the image smaller, while scale < 1 makes it larger.
///
/// Updates pos_y by increasing it by the height of the specified
/// image at the given scale.

void pdf_writer_wx::output_image
    (wxImage const&               image
    ,char const*                  image_name
    ,double                       scale
    ,int                          x
    ,int*                         pos_y
    ,oenum_render_or_only_measure output_mode
    )
{
    LMI_ASSERT(!save_has_been_called_);

    int const y = wxRound(image.GetHeight() / scale);

    switch(output_mode)
        {
        case oe_render:
            {
            // Use wxPdfDocument API directly as wxDC doesn't provide a way to
            // set the image scale at PDF level and also because passing via
            // wxDC wastefully converts wxImage to wxBitmap only to convert it
            // back to wxImage when embedding it into the PDF.
            wxPdfDocument* const pdf_doc = pdf_dc_.GetPdfDocument();
            LMI_ASSERT(pdf_doc);

            pdf_doc->SetImageScale(scale);
            pdf_doc->Image(image_name, image, x, *pos_y);
            pdf_doc->SetImageScale(1);
            }
            break;
        case oe_only_measure:
            // Do nothing.
            break;
        }

    *pos_y += y;
}

/// Render, or just pretend rendering in order to measure it, the given HTML
/// contents at the specified position wrapping it at the given width.
/// Return the height of the output (using this width).

int pdf_writer_wx::output_html
    (int                          x
    ,int                          y
    ,int                          width
    ,html::text&&                 html
    ,oenum_render_or_only_measure output_mode
    )
{
    LMI_ASSERT(!save_has_been_called_);

    // We don't really want to change the font, but to preserve the current DC
    // font which is changed by rendering the HTML contents.
    wxDCFontChanger preserve_font(pdf_dc_, wxFont());

    auto const html_str = wxString::FromUTF8(std::move(html).as_html());
    std::unique_ptr<wxHtmlContainerCell> const cell
        (static_cast<wxHtmlContainerCell*>(html_parser_.Parse(html_str))
        );
    LMI_ASSERT(cell);

    cell->Layout(width);
    switch(output_mode)
        {
        case oe_render:
            {
            wxHtmlRenderingInfo rendering_info;
            cell->Draw
                (pdf_dc_
                ,x
                ,y
                ,0
                ,std::numeric_limits<int>::max()
                ,rendering_info
                );
            }
            break;
        case oe_only_measure:
            // Do nothing.
            break;
        }

    return cell->GetHeight();
}

int pdf_writer_wx::get_horz_margin() const
{
    return horz_margin;
}

int pdf_writer_wx::get_vert_margin() const
{
    return vert_margin;
}

int pdf_writer_wx::get_page_width()  const
{
    return total_page_size_.x - 2 * horz_margin;
}

int pdf_writer_wx::get_total_width() const
{
    return total_page_size_.x;
}

int pdf_writer_wx::get_page_height() const
{
    return total_page_size_.y - 2 * vert_margin;
}

int pdf_writer_wx::get_page_bottom() const
{
    return total_page_size_.y - vert_margin;
}

/// Save the PDF to the output file name specified in the ctor.

void pdf_writer_wx::save()
{
    LMI_ASSERT(!save_has_been_called_);
    pdf_dc_.EndDoc();
    save_has_been_called_ = true;
}

/// Dtor: validates that save() has been called.
///
/// Canonically, std::uncaught_exceptions() would be called in each
/// ctor, and its result there compared to its result here. Instead,
/// for simplicity it's called only here--in effect, presuming that
/// no object of this class is created by another object's dtor. At
/// worst, this simplification would result in displaying a warning
/// that wouldn't otherwise be shown, in a situation so weird that
/// a warning would be appropriate.

pdf_writer_wx::~pdf_writer_wx()
{
    if(!std::uncaught_exceptions() && !save_has_been_called_)
        {
        safely_show_message("Please report this: save() not called for PDF.");
        }
}
