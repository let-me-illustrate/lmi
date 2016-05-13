// Generate a table using wxDC.
//
// Copyright (C) 2015, 2016 Gregory W. Chicares.
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
#endif // __BORLANDC__

#include "wx_table_generator.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "miscellany.hpp"               // count_newlines(), split_into_lines()

namespace
{

/// Increase the first argument to the second one if it's smaller.

template<typename T>
void increase_to_if_smaller(T& first, T second)
{
    if(first < second)
        {
        first = second;
        }
}

} // Unnamed namespace.

wx_table_generator::wx_table_generator
    (wxDC& dc_
    ,int left_margin
    ,int total_width
    )
    :dc_(dc_)
    ,left_margin_(left_margin)
    ,total_width_(total_width)
    ,char_height_(dc_.GetCharHeight())
    ,row_height_((4 * char_height_ + 2) / 3) // Arbitrarily use 1.333 line spacing.
    ,has_column_widths_(false)
    ,max_header_lines_(1)
{
    // Set a pen with 0 width to get the thin lines and butt cap style for the
    // different segments drawn in do_output_values() to seamlessly combine
    // into a single line.
    wxPen pen(*wxBLACK, 0);
    pen.SetCap(wxCAP_ROUND);
    dc_.SetPen(pen);
}

void wx_table_generator::add_column
    (std::string const& header
    ,std::string const& widest_text
    )
{
    wxDCFontChanger set_header_font(dc_, get_header_font());

    // Set width to the special value of 0 for the variable width columns.
    int width = widest_text.empty() ? 0 : dc_.GetTextExtent(widest_text).x;

    // Keep track of the maximal number of lines in a header as this determines
    // the number of lines used for all of them. This is one plus the number of
    // newlines in the anticipated case where there is no newline character at
    // the beginning or end of the header's string representation.
    increase_to_if_smaller(max_header_lines_, 1u + count_newlines(header));

    // Also increase the column width to be sufficiently wide to fit
    // this header line if it has fixed width.
    if(0 != width)
        {
        increase_to_if_smaller(width, dc_.GetMultiLineTextExtent(header).x);

        // Add roughly 1 em margins on both sides.
        width += dc_.GetTextExtent("MM").x;
        }

    columns_.push_back(column_info(header, width));
}

wxFont wx_table_generator::get_header_font() const
{
    return dc_.GetFont().Bold();
}

void wx_table_generator::do_output_horz_separator(int x1, int x2, int y)
{
    dc_.DrawLine(x1, y, x2, y);
}

void wx_table_generator::do_output_vert_separator(int x, int y1, int y2)
{
    // TODO: add a possibility to have a thick border between the columns.
    dc_.DrawLine(x, y1, x, y2);
}

int wx_table_generator::do_get_cell_x(std::size_t column)
{
    do_compute_column_widths_if_necessary();

    int x = left_margin_;
    for(std::size_t col = 0; col < column; ++col)
        {
        x += columns_.at(col).width_;
        }

    return x;
}

wxRect wx_table_generator::cell_rect(std::size_t column, int y)
{
    LMI_ASSERT(column < columns_.size());

    // Note: call do_get_cell_x() here and not from the wxRect ctor arguments
    // list to ensure that the column width is initialized before it is used
    // below.
    int const x = do_get_cell_x(column);

    return wxRect(x, y, columns_.at(column).width_, row_height_);
}

void wx_table_generator::do_compute_column_widths_if_necessary()
{
    if(has_column_widths_)
        {
        return;
        }

    int num_expand = 0;
    int total_fixed = 0;

    typedef std::vector<column_info>::const_iterator cici;
    for(cici i = columns_.begin(); i != columns_.end(); ++i)
        {
        if(0 == i->width_)
            {
            num_expand++;
            }
        else
            {
            total_fixed += i->width_;
            }
        }

    if(total_width_ < total_fixed)
        {
        warning() << "Not enough space for all fixed columns." << LMI_FLUSH;
        return;
        }

    if(num_expand)
        {
        int const per_expand
            = (total_width_ - total_fixed + num_expand - 1)/num_expand;

        typedef std::vector<column_info>::iterator cii;
        for(cii i = columns_.begin(); i != columns_.end(); ++i)
            {
            if(0 == i->width_)
                {
                i->width_ = per_expand;
                }
            }
        }

    has_column_widths_ = true;
}

void wx_table_generator::do_output_values
    (int& x
    ,int& y
    ,std::string const* values
    )
{
    int const y_top = y;

    int const y_text = y + char_height_;
    y += row_height_;

    do_output_vert_separator(x, y_top, y);

    std::size_t const num_columns = columns_.size();
    for(std::size_t col = 0; col < num_columns; ++col)
        {
        int const width = columns_.at(col).width_;

        std::string const& s = values[col];
        if(!s.empty())
            {
            int x_text = x;
            if(columns_.at(col).is_centered_)
                {
                // Centre the text for the columns configured to do it.
                x_text += (width - dc_.GetTextExtent(s).x) / 2;
                }
            else
                {
                // Otherwise just offset it by ~1 em.
                x_text += dc_.GetTextExtent("M").x;
                }

            dc_.DrawText(s, x_text, y_text);
            }
        x += width;
        do_output_vert_separator(x, y_top, y);
        }
}

void wx_table_generator::output_vert_separator
    (std::size_t before_column
    ,int y
    )
{
    LMI_ASSERT(before_column <= columns_.size());

    do_output_vert_separator
        (do_get_cell_x(before_column)
        ,y
        ,y + row_height_
        );
}

void wx_table_generator::output_horz_separator
    (std::size_t begin_column
    ,std::size_t end_column
    ,int y
    )
{
    LMI_ASSERT(begin_column < end_column);
    LMI_ASSERT(end_column <= columns_.size());

    do_compute_column_widths_if_necessary();

    int const x1 = do_get_cell_x(begin_column);

    int x2 = x1;
    for(std::size_t col = begin_column; col < end_column; ++col)
        {
        x2 += columns_.at(col).width_;
        }

    do_output_horz_separator(x1, x2, y);
}

void wx_table_generator::output_header(int* pos_y)
{
    do_compute_column_widths_if_necessary();

    wxDCFontChanger set_header_font(dc_, get_header_font());

    // Split headers in single lines and fill up the entire columns*lines 2D
    // matrix, using empty strings for the headers with less than the maximal
    // number of lines.
    std::size_t const num_columns = columns_.size();
    std::vector<std::string> headers_by_line(max_header_lines_ * num_columns);
    for(std::size_t col = 0; col < num_columns; ++col)
        {
        column_info const& ci = columns_.at(col);
        std::vector<std::string> const lines(split_into_lines(ci.header_));

        // Fill the elements from the bottom line to the top one, so that a
        // single line header is shown on the last line.
        std::size_t const first_line = max_header_lines_ - lines.size();
        for(std::size_t line = 0; line < lines.size(); ++line)
            {
            headers_by_line.at
                ((first_line + line) * num_columns + col
                ) = lines.at(line);
            }
        }

    // And output all lines of all column headers.
    int y_top = *pos_y;
    int x = 0;
    for(std::size_t line = 0; line < max_header_lines_; ++line)
        {
        x = left_margin_;
        do_output_values
            (x
            ,*pos_y
            ,&headers_by_line.at(line * num_columns)
            );
        }

    // Finally draw the separators above and (a double one) below them.
    do_output_horz_separator(left_margin_, x,  y_top    );
    do_output_horz_separator(left_margin_, x, *pos_y - 1);
    do_output_horz_separator(left_margin_, x, *pos_y    );
}

void wx_table_generator::output_row
    (int* pos_y
    ,std::string const* values
    )
{
    int x = left_margin_;
    do_output_values(x, *pos_y, values);

    do_output_horz_separator(left_margin_, x, *pos_y);
}

