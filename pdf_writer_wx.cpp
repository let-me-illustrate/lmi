// PDF generation helpers.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pdf_writer_wx.hpp"

#include "alert.hpp"                    // safely_show_message()
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "html.hpp"

#include <wx/filesys.h>
#include <wx/html/htmlcell.h>
#include <wx/html/htmprint.h>

#include <exception>                    // uncaught_exceptions()

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

// Ensure that we call SetFonts() with consistent parameters both on
// wxHtmlWinParser and wxHtmlDCRenderer by using the same helper function to do
// it for both of them.
template <typename T>
void DoSetFonts(T& html_object, pdf_writer_wx::html_font_sizes const& font_sizes)
{
    html_object.SetFonts("Helvetica", "Courier", font_sizes.data());
}

// Helper ensuring that the font of the specified DC is restored on scope exit.
class dc_font_preserver
{
  public:
    explicit dc_font_preserver(wxDC& dc)
        :dc_   {dc}
        ,font_ {dc.GetFont()}
    {
    }

    ~dc_font_preserver()
    {
        dc_.SetFont(font_);
    }

    dc_font_preserver(dc_font_preserver const&) = delete;
    dc_font_preserver& operator=(dc_font_preserver const&) = delete;

  private:
    wxDC&        dc_;
    wxFont const font_;
};

} // Unnamed namespace.

pdf_writer_wx::pdf_writer_wx
    (wxString           const& output_filename
    ,wxPrintOrientation        orientation
    ,html_font_sizes    const& font_sizes
    )
    :print_data_      {make_print_data(output_filename, orientation)}
    ,pdf_dc_          {print_data_}
    ,html_parser_     {nullptr}
    ,html_font_sizes_ {font_sizes}
    ,total_page_size_ {pdf_dc_.GetSize()}
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
        (wxFontInfo(html_font_sizes_.at(2))
            .Family(wxFONTFAMILY_SWISS)
            .FaceName("Helvetica")
        );

    // Create the virtual file system object for loading images referenced from
    // HTML and interpret relative paths from the data directory.
    html_vfs_.reset(new wxFileSystem());
    html_vfs_->ChangePathTo
        (global_settings::instance().data_directory().string()
        ,true // argument is a directory, not file path
        );

    // Create an HTML parser to allow easily adding HTML contents to the output.
    initialize_html_parser(html_parser_);
}

/// Start a new page in the output PDF document.
///
/// This is equivalent to wxDC::EndPage() followed by wxDC::StartPage(), but
/// preferable to using these 2 functions directly, both because it's simpler
/// and because it's too easy to forget to call EndPage() otherwise, especially
/// as almost everything still works correctly even when it's not called --
/// except that the clipping region is not reset for the new page, which can
/// result in hard to diagnose problems.

void pdf_writer_wx::next_page()
{
    LMI_ASSERT(!save_has_been_called_);
    pdf_dc_.EndPage();
    pdf_dc_.StartPage();
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
    ,int&                         pos_y
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
            pdf_doc->Image(image_name, image, x, pos_y);
            pdf_doc->SetImageScale(1);
            }
            break;
        case oe_only_measure:
            // Do nothing.
            break;
        }

    pos_y += y;
}

/// Compute vertical page break positions needed when outputting the given HTML
/// contents into pages of the given height.
///
/// If the entire contents fits on a single page, the returned vector has a
/// single element equal to page_height. More generally, the size of the
/// returned vector is the number of pages needed for output.
///
/// Note that page_height is passed as parameter here because it can be smaller
/// than the value returned by our get_total_height() if headers or footers are
/// used. And page_width is used for consistency, even if currently it's always
/// the same as get_page_width().

std::vector<int> pdf_writer_wx::paginate_html
    (int                          page_width
    ,int                          page_height
    ,wxHtmlContainerCell&         cell
    )
{
    wxHtmlDCRenderer renderer;
    renderer.SetDC(&dc());
    renderer.SetSize(page_width, page_height);
    DoSetFonts(renderer, html_font_sizes_);

    renderer.SetHtmlCell(cell);

    std::vector<int> page_breaks;
    for(int pos = 0;;)
        {
        pos = renderer.FindNextPageBreak(pos);
        if(pos == wxNOT_FOUND)
            break;
        page_breaks.push_back(pos);
        }

    return page_breaks;
}

/// Render, or just pretend rendering in order to measure it, the given HTML
/// contents at the specified position wrapping it at the given width.
///
/// Return the height of the output (using this width).
///
/// Note the difference between "x" and "y" parameters, which specify the
/// position in the output DC, and "from" and "to" ones which contain the
/// starting and ending coordinates in the virtual view of the entire HTML
/// document: the HTML element at the position "from" will appear at "y".

int pdf_writer_wx::output_html
    (int                          x
    ,int                          y
    ,int                          width
    ,wxHtmlContainerCell&         cell
    ,int                          from
    ,int                          to
    ,oenum_render_or_only_measure output_mode
    )
{
    LMI_ASSERT(!save_has_been_called_);

    // Preserve the current font changed by rendering the HTML contents.
    dc_font_preserver font_preserver(pdf_dc_);

    cell.Layout(width);
    switch(output_mode)
        {
        case oe_render:
            {
            // Even though wxHtmlCell::Draw() omits drawing of the cells
            // entirely outside of the visible vertical range, we still need to
            // clip rendering to this range explicitly as a partially visible
            // cell could extend beyond the "to" boundary if we didn't do it.
            wxDCClipper clip(pdf_dc_, x, y, width, to - from);

            wxHtmlRenderingInfo rendering_info;

            // "Scroll" the cell upwards by "from" by subtracting it from the
            // vertical position.
            cell.Draw(pdf_dc_, x, y - from, y, y + to - from, rendering_info);
            }
            break;
        case oe_only_measure:
            // Do nothing.
            break;
        }

    return cell.GetHeight();
}

/// Convenient overload when rendering, or measuring, HTML text known to fit on
/// a single page.
///
/// In this case "from" and "to" parameters are not needed and we can take
/// html::text directly as it won't be used any more.

int pdf_writer_wx::output_html
    (int                          x
    ,int                          y
    ,int                          width
    ,html::text&&                 html
    ,oenum_render_or_only_measure output_mode
    )
{
    auto const cell{parse_html(std::move(html))};
    LMI_ASSERT(cell);

    return output_html(x, y, width, *cell, output_mode);
}

int pdf_writer_wx::output_html
    (int                          x
    ,int                          y
    ,int                          width
    ,wxHtmlContainerCell&         cell
    ,oenum_render_or_only_measure output_mode
    )
{
    int const height = output_html
        (x
        ,y
        ,width
        ,cell
        ,0
        ,get_total_height()
        ,output_mode
        );

    switch(output_mode)
        {
        case oe_render:
            // When rendering, all the text should have fit on this page,
            // otherwise this is not the right overload to use -- call
            // paginate_html() and the generic overload above instead.
            LMI_ASSERT(height <= get_total_height() - y);
            break;
        case oe_only_measure:
            // It's fine if the output doesn't fit when measuring it, as this
            // could be used to decide whether another page is needed or not,
            // so don't do anything here.
            break;
        }

    return height;
}

void pdf_writer_wx::initialize_html_parser(wxHtmlWinParser& html_parser)
{
    html_parser.SetDC(&pdf_dc_);
    DoSetFonts(html_parser, html_font_sizes_);

    html_parser.SetFS(html_vfs_.get());
}

std::unique_ptr<wxHtmlContainerCell> pdf_writer_wx::parse_html(html::text&& html)
{
    // Preserve the current font changed by parsing the HTML contents.
    dc_font_preserver font_preserver(pdf_dc_);

    return std::unique_ptr<wxHtmlContainerCell>
        (static_cast<wxHtmlContainerCell*>
            (html_parser_.Parse
                (wxString::FromUTF8
                    (std::move(html).as_html()
                    )
                )
            )
        );
}

/// Construct a self-contained HTML document from the given cell.
///
/// The function takes ownership of its argument and attaches it to the new,
/// empty, HTML document using the same parameters (i.e. fonts) as all the
/// other HTML created by output_html().

std::unique_ptr<wxHtmlContainerCell>
pdf_writer_wx::make_html_from(wxHtmlCell* cell)
{
    // Initializing wxHtmlWinParser changes the font of the DC, so
    // ensure that we preserve the original font.
    dc_font_preserver font_preserver(pdf_dc_);

    wxHtmlWinParser html_parser;
    initialize_html_parser(html_parser);
    html_parser.InitParser(wxString{});

    // Take ownership of the DOM containing just the initial colors and font.
    std::unique_ptr<wxHtmlContainerCell> document_cell
        {static_cast<wxHtmlContainerCell*>(html_parser.GetProduct())
        };

    // Give ownership of the cell to the new document.
    document_cell->InsertCell(cell);

    return document_cell;
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

int pdf_writer_wx::get_total_height() const
{
    return total_page_size_.y;
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
