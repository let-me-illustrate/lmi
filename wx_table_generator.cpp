// Generate a table using wxDC.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "wx_table_generator.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "miscellany.hpp"               // count_newlines(), split_into_lines()
#include "ssize_lmi.hpp"

#include <algorithm>                    // max()

// Default size of various characters for illustrations and group quotes:
//   'M' 7pt; 'N' 6pt; '1' 4pt; '9' 4pt; ',' 2pt
// Widths are in pixels. Because the wxPdfDC uses wxMM_POINTS, each pixel
// is one point = 1/72 inch.

wx_table_generator::wx_table_generator
    (group_quote_style_tag                 // tag not referenced
    ,std::vector<column_parameters> const& vc
    ,std::vector<int>               const& indices
    ,wxDC&                                 dc
    ,int                                   left_margin
    ,int                                   total_width
    )
    :indices_          {indices}
    ,dc_               {dc}
    ,left_margin_      {left_margin}
    ,total_width_      {total_width}
    ,char_height_      {dc_.GetCharHeight()}
    // Arbitrarily use 1.333 line spacing.
    ,row_height_       {(4 * char_height_ + 2) / 3}
    ,one_em_           {dc_.GetTextExtent("M").x}
    ,max_header_lines_ {1}
    ,draw_separators_  {true}
    ,use_bold_headers_ {true}
{
    for(auto const& i : vc)
        {
        enroll_column(i);
        }
    // Ideally this would be '&thinsp;' instead of '&puncsp;'.
    int const one_puncsp = dc_.GetTextExtent(".").x;
    std::vector<int> const w = set_column_widths
        (all_columns_
        ,total_width_
        ,2 * one_em_
        ,one_puncsp
        );

    std::vector<table_column_info> resized_columns;
    for(int j = 0; j < lmi::ssize(all_columns()); ++j)
        {
        resized_columns.emplace_back
            (all_columns_[j].col_header()
            ,w           [j]
            ,all_columns_[j].alignment()
            ,all_columns_[j].is_elastic() ? oe_elastic : oe_inelastic
            );
        }
    all_columns_.swap(resized_columns);

    // Set a pen with zero width to make grid lines thin,
    // and round cap style so that they combine seamlessly.
    wxPen pen(*wxBLACK, 0);
    pen.SetCap(wxCAP_ROUND);
    dc_.SetPen(pen);
}

wx_table_generator::wx_table_generator
    (illustration_style_tag                // tag not referenced
    ,std::vector<column_parameters> const& vc
    ,std::vector<int>               const& indices
    ,wxDC&                                 dc
    ,int                                   left_margin
    ,int                                   total_width
    )
    :indices_          {indices}
    ,dc_               {dc}
    ,left_margin_      {left_margin}
    ,total_width_      {total_width}
    ,char_height_      {dc_.GetCharHeight()}
    ,row_height_       {char_height_}
    ,one_em_           {dc_.GetTextExtent("M").x}
    ,max_header_lines_ {1}
    ,draw_separators_  {false}
    ,use_bold_headers_ {false}
{
    for(auto const& i : vc)
        {
        enroll_column(i);
        }
    // Ideally this would be '&thinsp;' instead of '&puncsp;'.
    int const one_puncsp = dc_.GetTextExtent(".").x;
    std::vector<int> const w = set_column_widths
        (all_columns_
        ,total_width_
        ,2 * one_em_
        ,one_puncsp
        );

    std::vector<table_column_info> resized_columns;
    for(int j = 0; j < lmi::ssize(all_columns()); ++j)
        {
        resized_columns.emplace_back
            (all_columns_[j].col_header()
            ,w           [j]
            ,all_columns_[j].alignment()
            ,all_columns_[j].is_elastic() ? oe_elastic : oe_inelastic
            );
        }
    all_columns_.swap(resized_columns);

    dc_.SetPen(illustration_rule_color);
}

wx_table_generator::wx_table_generator(wx_table_generator const&) = default;

wx_table_generator::~wx_table_generator() = default;

/// Render headers at the given position.

void wx_table_generator::output_headers
    (int&                         pos_y
    ,oenum_render_or_only_measure output_mode
    )
{
    int const anticipated_pos_y =
          pos_y
        + draw_separators_
        + row_height() * max_header_lines_
        ;

    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            pos_y = anticipated_pos_y;
            return;
        }

    wxDCFontChanger header_font_setter(dc_);
    if(use_bold_headers_)
        {
        header_font_setter.Set(header_font());
        // The distance from the font's descender line to its ascender
        // line must not exceed the distance between lines.
        LMI_ASSERT(dc().GetCharHeight() <= row_height());
        // do_output_single_row(), called below, uses a cached char_height_
        // that is assumed not to differ from the bold GetCharHeight().
        LMI_ASSERT(dc().GetCharHeight() == char_height_);
        }

    // Split headers in single lines and fill up the entire columns*lines 2D
    // matrix, using empty strings for the headers with less than the maximal
    // number of lines.
    int const number_of_columns = lmi::ssize(all_columns());
    std::vector<std::string> headers_by_line(max_header_lines_ * number_of_columns);
    for(int i = 0; i < number_of_columns; ++i)
        {
        // Fill the elements from the bottom line to the top one, so that a
        // single line header is shown on the last line.
        table_column_info const& ci = all_columns().at(i);
        std::vector<std::string> const lines(split_into_lines(ci.col_header()));
        int const first_line = max_header_lines_ - lmi::ssize(lines);
        for(int j = 0; j < lmi::ssize(lines); ++j)
            {
            headers_by_line.at
                ((first_line + j) * number_of_columns + i
                ) = lines.at(j);
            }
        }

    // And output all lines of all column headers.
    int y_top = pos_y;
    int x = 0;
    for(int i = 0; i < max_header_lines_; ++i)
        {
        std::vector<std::string> const nth_line
            (headers_by_line.begin() +      i  * number_of_columns
            ,headers_by_line.begin() + (1 + i) * number_of_columns
            );
        x = left_margin_;
        do_output_single_row(x, pos_y, nth_line);
        }

    // Finally draw the separators above and (a double one) below them.
    if(draw_separators_)
        {
        do_output_horz_separator(left_margin_, x, y_top);
        do_output_horz_separator(left_margin_, x, pos_y); ++pos_y;
        do_output_horz_separator(left_margin_, x, pos_y);
        }

    LMI_ASSERT(anticipated_pos_y == pos_y);
}

/// Render a super-header, i.e. a header spanning over several columns.
///
/// The column range is specified as [begin, end), as is usual in C++.
/// The header string may be multiline, just as with normal headers.

void wx_table_generator::output_super_header
        (std::string const&           header
        ,int                          a_begin_column
        ,int                          a_end_column
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        )
{
    int begin_column = indices_[a_begin_column];
    int end_column   = indices_[a_end_column];

    std::vector<std::string> const lines(split_into_lines(header));
    int const anticipated_pos_y = pos_y + row_height() * lmi::ssize(lines);

    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            pos_y = anticipated_pos_y;
            return;
        }

    auto rect = cell_rect(begin_column, end_column, pos_y);

    for(auto const& i : lines)
        {
        if(rect.width < dc().GetTextExtent(i).x)
            {
            alarum()
                << "Superheader width ("
                << dc().GetTextExtent(i).x
                << ") exceeds available space ("
                << rect.width
                << "): text is '" << i << "'."
                << LMI_FLUSH
                ;
            }
        dc_.DrawLabel(i, rect, wxALIGN_CENTER_HORIZONTAL);
        rect.y += row_height_;
        pos_y  += row_height_;
        }

    LMI_ASSERT(anticipated_pos_y == pos_y);
}

/// Shade the background of a single cell; center the given contents.

void wx_table_generator::output_highlighted_cell
    (int                a_column
    ,int                y
    ,std::string const& value
    )
{
    int column = indices_[a_column];
    LMI_ASSERT(column < lmi::ssize(all_columns()));

    {
    wxDCPenChanger set_transparent_pen(dc_, *wxTRANSPARENT_PEN);
    wxDCBrushChanger set_grey_brush(dc_, *wxLIGHT_GREY_BRUSH);
    dc_.DrawRectangle(cell_rect(column, y));
    }

    dc_.DrawLabel(value, text_rect(column, y), wxALIGN_CENTER_HORIZONTAL);

    do_output_vert_separator(cell_pos_x(column), y, y + row_height_);
}

/// Render a single row with the given values at the given position.
///
/// Asserted precondition: the cardinality of the 'values' argument
/// equals the total number of columns passed to enroll_column(),
/// i.e., the cardinality of all_columns().

void wx_table_generator::output_row
    (int&                           pos_y
    ,std::vector<std::string> const values
    )
{
    LMI_ASSERT(values.size() == all_columns().size());
    int x = left_margin_;
    do_output_single_row(x, pos_y, values);

    if(draw_separators_)
        {
        do_output_horz_separator(left_margin_, x, pos_y);
        }
}

/// Output a vertical separator line before the given column.
///
/// If the column index equals the number of columns, output a
/// separator after the last column.

void wx_table_generator::output_vert_separator(int a_before_column, int y)
{
    int before_column = indices_[a_before_column];

    LMI_ASSERT(before_column <= lmi::ssize(all_columns()));

    do_output_vert_separator(cell_pos_x(before_column), y, y + row_height_);
}

/// Output a horizontal separator line across the specified columns.
///
/// The column range is specified as [begin, end), as is usual in C++.

void wx_table_generator::output_horz_separator
    (int                          a_begin_column
    ,int                          a_end_column
    ,int                          y
    ,oenum_render_or_only_measure output_mode
    )
{
    int begin_column = indices_[a_begin_column];
    int end_column   = indices_[a_end_column];

    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            return;
        }

    LMI_ASSERT(begin_column < end_column);
    LMI_ASSERT(end_column <= lmi::ssize(all_columns()));

    int const x1 = cell_pos_x(begin_column);

    int x2 = x1;
    for(int i = begin_column; i < end_column; ++i)
        {
        x2 += all_columns().at(i).col_width();
        }

    do_output_horz_separator(x1, x2, y);
}

/// Height of a single table row.

int wx_table_generator::row_height() const
{
    return row_height_;
}

/// Amount of vertical space taken by a separator line in table headers.

int wx_table_generator::separator_line_height() const
{
    // This is completely arbitrary and chosen just because it seems to
    // look well.
    return row_height() / 2;
}

wxRect wx_table_generator::external_text_rect(int a_column, int y) const
{
    int column = indices_[a_column];
    return text_rect(column, y);
}

/// Rectangle corresponding to a cell's text contents.
///
/// This is narrower than the full cell rectangle to leave a small
/// margin. Its vertical position is adjusted to center the text vertically.

wxRect wx_table_generator::text_rect(int column, int y) const
{
    LMI_ASSERT(column < lmi::ssize(all_columns()));
    wxRect z = cell_rect(column, y).Deflate(dc().GetCharWidth(), 0);
    z.Offset(0, (row_height_ - char_height_)/2);
    return z;
}

/// Indicate an intention to include a column by storing its metadata.
///
/// Sets max_header_lines_.
///
/// The total number of columns thus enrolled determines the cardinality
/// of the 'values' argument in output_row() calls.
///
/// Column headers may be multiline strings.
///
/// Postconditions:
/// - An elastic column's width is initialized to zero. (If there's
///   enough room to display it, set_column_widths() resets its width
///   appropriately.)
///
/// Design alternative: this could be written as a nonmember function,
/// by passing the DC and the header font as arguments.

void wx_table_generator::enroll_column(column_parameters const& z)
{
    int width = 0;

    wxDCFontChanger header_font_setter(dc_);
    if(use_bold_headers_)
        {
        header_font_setter.Set(header_font());
        }

    wxCoord w, h, lh;
    dc().GetMultiLineTextExtent(z.header, &w, &h, &lh, &dc().GetFont());
    LMI_ASSERT(0 != lh);
    LMI_ASSERT(0 == h % lh);
    LMI_ASSERT(h / lh == int(1u + count_newlines(z.header)));
    // Store number of lines used by tallest header:
    // output_headers() uses it to write all headers as a block.
    max_header_lines_ = std::max(max_header_lines_, h / lh);

    switch(z.elasticity)
        {
        case oe_inelastic:
            {
            // Greater of header width and 'widest_text' width.
            width = std::max(w, dc().GetTextExtent(z.widest_text).x);
            }
            break;
        case oe_elastic:
            {
            ; // Do nothing: 'width' already initialized to zero.
            }
            break;
        }

    all_columns_.emplace_back(z.header, width, z.alignment, z.elasticity);
}

void wx_table_generator::do_output_single_row
    (int&                            pos_x
    ,int&                            pos_y
    ,std::vector<std::string> const& values
    )
{
    int const y_top = pos_y;

    int const y_text = text_rect(0, pos_y).y;
    pos_y += row_height_;

    if(draw_separators_)
        {
        do_output_vert_separator(pos_x, y_top, pos_y);
        }

    int const number_of_columns = lmi::ssize(all_columns());
    for(int i = 0; i < number_of_columns; ++i)
        {
        table_column_info const& ci = all_columns().at(i);
        std::string const& s = values[i];
        if(!s.empty() && 0 != ci.col_width())
            {
            int x_text = pos_x;

            switch(ci.alignment())
                {
                case oe_left:
                    {
                    x_text += 0;
                    }
                    break;
                case oe_center:
                    {
                    x_text += (ci.col_width() - dc().GetTextExtent(s).x) / 2;
                    }
                    break;
                case oe_right:
                    {
                    x_text += ci.col_width() - dc().GetTextExtent(s).x;
                    }
                    break;
                }

            if(ci.is_clipped() && 0 <= ci.col_width() - one_em_)
                {
                // Write clipped text with bilateral column margins:
                //  - aligned left, indented 1em for a left margin; and
                //  - clipped on the right to width minus a 1em margin.
                wxDCClipper clip
                    (dc_
                    ,wxRect
                        {wxPoint{pos_x, y_top}
                        ,wxSize{ci.col_width() - one_em_, row_height_}
                        }
                    );
                dc_.DrawText(s, x_text + one_em_, y_text);
                }
            else
                {
                dc_.DrawText(s, x_text, y_text);
                }
            }
        pos_x += ci.col_width();
        if(draw_separators_)
            {
            do_output_vert_separator(pos_x, y_top, pos_y);
            }
        }
}

// Horizontal and vertical separators are considered to be drawn in
// interlinear space, so they are not counted when calculating the
// positions of textual elements.

void wx_table_generator::do_output_vert_separator(int x, int y1, int y2)
{
    dc_.DrawLine(x, y1, x, y2);
}

void wx_table_generator::do_output_horz_separator(int x1, int x2, int y)
{
    dc_.DrawLine(x1, y, x2, y);
}

int wx_table_generator::cell_pos_x(int column) const
{
    LMI_ASSERT(column <= lmi::ssize(all_columns()));
    int x = left_margin_;
    for(int i = 0; i < column; ++i)
        {
        x += all_columns().at(i).col_width();
        }

    return x;
}

/// Rectangle corresponding to a cell.

wxRect wx_table_generator::cell_rect(int column, int y) const
{
    return cell_rect(column, 1 + column, y);
}

/// Rectangle corresponding to a horizontal range of cells.

wxRect wx_table_generator::cell_rect(int begin_column, int end_column, int y) const
{
    LMI_ASSERT(begin_column <= end_column);
    LMI_ASSERT(end_column <= lmi::ssize(all_columns()));

    int const x1 = cell_pos_x(begin_column);
    int x2 = x1;
    for(int i = begin_column; i < end_column; ++i)
        {
        x2 += all_columns().at(i).col_width();
        }

    return wxRect(x1, y, x2 - x1, row_height_);
}

/// Font used for headers.

wxFont wx_table_generator::header_font() const
{
    return dc().GetFont().Bold();
}

wxDC const& wx_table_generator::dc() const
{
    return dc_;
}

std::vector<table_column_info> const& wx_table_generator::all_columns() const
{
    return all_columns_;
}
