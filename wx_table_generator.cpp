// Generate a table using wxDC.
//
// Copyright (C) 2015, 2016, 2017, 2018 Gregory W. Chicares.
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
    (wxDC& dc
    ,int left_margin
    ,int total_width
    )
    :dc_(dc)
    ,left_margin_(left_margin)
    ,total_width_(total_width)
    ,char_height_(dc_.GetCharHeight())
    ,row_height_((4 * char_height_ + 2) / 3) // Arbitrarily use 1.333 line spacing.
    ,column_margin_(dc_.GetTextExtent("M").x)
    ,column_widths_already_computed_(false)
    ,max_header_lines_(1)
{
    // Set a pen with 0 width to get the thin lines and butt cap style for the
    // different segments drawn in do_output_values() to seamlessly combine
    // into a single line.
    wxPen pen(*wxBLACK, 0);
    pen.SetCap(wxCAP_ROUND);
    dc_.SetPen(pen);
}

void wx_table_generator::use_condensed_style()
{
    row_height_ = char_height_;
    draw_separators_ = false;
    use_bold_headers_ = false;
}

void wx_table_generator::align_right()
{
    align_right_ = true;
}

void wx_table_generator::add_column
    (std::string const& header
    ,std::string const& widest_text
    )
{
    LMI_ASSERT(!column_widths_already_computed_);

    // There is no need to care about the column width for hidden columns.
    int width;
    if(header.empty())
        {
        width = 0;
        }
    else
        {
        wxDCFontChanger header_font_setter(dc_);
        if(use_bold_headers_)
            {
            header_font_setter.Set(get_header_font());
            }

        // Set width to the special value of 0 for the variable width columns.
        width = widest_text.empty() ? 0 : dc_.GetTextExtent(widest_text).x;

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

            width += 2 * column_margin_;
            }
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
    // PDF !! add a possibility to have a thick border between the columns.
    dc_.DrawLine(x, y1, x, y2);
}

int wx_table_generator::do_get_cell_x(std::size_t column)
{
    do_compute_column_widths();

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

wxRect wx_table_generator::text_rect(std::size_t column, int y)
{
    wxRect text_rect = cell_rect(column, y).Deflate(dc_.GetCharWidth(), 0);
    text_rect.Offset(0, char_height_);
    return text_rect;
}

// class members used, mutably or immutably:
//
// const    total_width_
// mutable  column_widths_already_computed_
// mutable  column_margin_
// mutable  columns_
//   i.e. std::vector<column_info> columns_;
// mutable  column_info elements
//   the only column_info function member called is is_hidden()
//   the only column_info data member modified is width_
//
// meanings (written before each variable, as in header documentation):
//
    // ctor parameter:
    // max table width (page width minus horizontal page margins)
// const    total_width_
    // Used to prevent this function from being called more than once.
// mutable  column_widths_already_computed_
    // spacing on both left and right of column
    // initialized in ctor to # pixels in one em: (dc_.GetTextExtent("M").x)
    // changed in this function and nowhere else
// mutable  column_margin_
    // std::vector<column_info>
// mutable  columns_

/// Compute column widths.
///
/// This function must be called after the last time add_column() is
/// called, and before the first time that the column widths it sets
/// are used. It is assumed to be fairly expensive, so that it should
/// be called only once. There seems to be no simple way to impose
/// those synchronization requirements upon clients, so
///  - add_column() asserts that this function hasn't yet been called;
///  - this function exits early if it has already been called.
/// Thus, any violation of the first part of this contract is detected
/// at run time. It is hoped that this function is called in enough
/// places to fulfill the second part of the contract.

void wx_table_generator::do_compute_column_widths()
{
    if(column_widths_already_computed_) return;

    column_widths_already_computed_ = true;

    // Number of non-hidden columns.
    int num_columns = 0;

    // Number of non-hidden columns with variable width.
    //
    // In practice, only the "Participant" column on group quotes has
    // this property.
    //
    // The rationale for this property is that, once adequate width
    // has been allocated to each column, any excess width left over
    // is to be distributed among such "variable-width" columns only:
    // i.e., they (and only they) are to be "expanded".
    int num_expand = 0;

    // Total width of all non-hidden fixed-width columns.
    // The width of each fixed-width column reflects:
    //  - a mask like "999,999" (ideally, there would instead be a
    //    quasi-global data structure mapping symbolic column names
    //    to their corresponding headers and maximal widths)
    //  - the header width
    //  - the bilateral margins that have already been added
    // The margins may be slightly reduced by this function to make
    // everything fit when it otherwise wouldn't.
    int total_fixed = 0;

    for(auto const& i : columns_)
        {
// Instead of retaining hidden columns, and explicitly skipping them
// here and repeatedly later, why not just remove them from the vector?
        if(i.is_hidden())
            {
            continue;
            }

        num_columns++;

        if(i.is_variable_width())
            {
            num_expand++;
            }
        else
            {
            total_fixed += i.width_;
            }
        }

    if(total_width_ < total_fixed)
        {
        // The fixed-width columns don't all fit with their original
        // one-em presumptive bilateral margins. Try to make them fit
        // by reducing the margins slightly.
        //
        // The number of pixels that would need to be removed is:
        auto const overflow = total_fixed - total_width_;

        // Because fixed-width columns take more than the available
        // horizontal space, there's no room to fit any variable-width
        // columns, so the column-fitting problem is overconstrained.
        // Therefore, don't even try reducing margins if there are any
        // variable-width columns.
        if(!num_expand)
            {
// Also calculate the number of pixels by which it overflows for each column
            // We need to round up in division here to be sure that all columns
            // fit into the available width.
            auto const overflow_per_column =
                (overflow + num_columns - 1) / num_columns;
// Now determine whether reducing the margins will make the table fit.
// If that works, then do it; else don't do it, and print a warning.
//
// column_margin_ is the padding on each side of every column, so
// the number of pixels between columns, as the table was originally
// laid out, is two times column_margin_--which, as we just determined,
// was too generous, so we're going to try reducing it.
// Then this conditional compares
//   the number of pixels by which we must shrink each column, to
//   the number of pixels of padding between columns
// Reducing the padding is a workable strategy if the desired reduction
// is less than the padding.
//
// Is this as good as it can be, given that coordinates are integers?
            if(overflow_per_column <= 2 * column_margin_)
                {
                // We are going to reduce the total width by more than
                // necessary, in general, because of rounding up above, so
                // compensate for it by giving 1 extra pixel until we run out
                // of these "underflow" pixels.
// Defect: the number of pixels separating columns might now be zero.
// '9' is five PDF pixels wide; do we need, say, two pixels between columns?
//
// Suggestion: change the
//   overflow_per_column <= column_margin_
// condition to something like:
//    overflow_per_column <= column_margin_ - 4 // two pixels on each side
//    overflow_per_column <= column_margin_ - 2 // one pixel on each side
                auto underflow = overflow_per_column * num_columns - overflow;

                for(auto& i : columns_)
                    {
                    if(i.is_hidden())
                        {
                        continue;
                        }

                    i.width_ -= overflow_per_column;

                    if(0 < underflow)
                        {
                        i.width_++;
                        underflow--;
                        }
                    }

                column_margin_ -= (overflow_per_column + 1) / 2;

                // We condensed the columns enough to make them fit, so no need
                // for the warning and we don't have any expanding columns, so
                // we're done.
                return;
                }
// If overflow_per_column is 1, then column_margin_ -= 1
// "           "          "  2,   "        "           1
// "           "          "  3,   "        "           2
// "           "          "  4,   "        "           2
// The 'underflow' logic shrinks columns by the exact number of pixels
// to use up all the available width. But the column_margin_ reduction
// isn't exact due to truncation: when the margin is added (on both sides),
// is the total of all (margin+column+margin) widths lower than the maximum,
// so that this is just a small aesthetic issue, or is it too wide, so that
// not everything fits?
//
// Answer:
// This is an issue of aligning the column text, not of fitting, because the
// margin is used when positioning the text inside the column width. And the
// width is correct, so the worst that can happen here is that the text is
// offset by 0.5 pixels -- but, of course, if we rounded it down, it would be
// offset by 0.5 pixels in the other direction. So maybe we should write
//
//     column_margin_ -= overflow_per_column / 2;
//
// just because it's shorter and not necessarily worse (nor better).
            }

        // PDF !! Before release, consider showing less information here.
        warning()
            << "Not enough space for all " << num_columns << " columns."
            << "\nPrintable width is " << total_width_ << " points."
            << "\nData alone require " << total_fixed - 2 * column_margin_ * num_columns
            << " points without any margins for legibility."
            << "\nColumn margins of " << column_margin_ << " points on both sides"
            << " would take up " << 2 * column_margin_ * num_columns << " additional points."
            << "\nFor reference:"
            << "\n'M' is " << dc_.GetTextExtent("M").x << " points wide."
            << "\n'N' is " << dc_.GetTextExtent("N").x << " points wide."
            << "\n'1' is " << dc_.GetTextExtent("1").x << " points wide."
            << "\n'9' is " << dc_.GetTextExtent("9").x << " points wide."
            << "\n',' is " << dc_.GetTextExtent(",").x << " points wide."
            << LMI_FLUSH
            ;
        return;
        }

    // Lay out variable-width columns in whatever space is left over
    // after accounting for all fixed-width columns.
    //
    // If there's more than enough space for them, then expand them
    // to consume all available space.
    //
    // If there isn't enough space for their headers and contents,
    // then clip them. Motivation: the archetypal variable-width
    // column is a personal name, which has practically unlimited
    // width. On a group premium quote, numeric columns must never
    // be truncated, but truncating one extremely long personal name
    // is preferable to failing to produce any quote at all. It would
    // of course be possible to take the header of such a column as
    // its minimal width, but that would be a useless refinement in
    // the problem domain. In the most extreme conceivable case, all
    // fixed-width columns would fit, but there would be not a single
    // pixel available for variable-width columns and they would all
    // in effect be dropped; again, in the problem domain, that would
    // actually be preferable to failing to produce any output.
    if(num_expand)
        {
        int const per_expand
            = (total_width_ - total_fixed + num_expand - 1) / num_expand;

        for(auto& i : columns_)
            {
            if(i.is_hidden())
                {
                continue;
                }

            if(i.is_variable_width())
                {
                i.width_ = per_expand;
                }
            }
        }
}

void wx_table_generator::do_output_values
    (int&                            x
    ,int&                            y
    ,std::vector<std::string> const& values
    )
{
    int const y_top = y;

    int const y_text = y + char_height_;
    y += row_height_;

    if(draw_separators_)
        {
        do_output_vert_separator(x, y_top, y);
        }

    std::size_t const num_columns = columns_.size();
    for(std::size_t col = 0; col < num_columns; ++col)
        {
        column_info const& ci = columns_.at(col);
        if(ci.is_hidden())
            {
            continue;
            }

        int const width = ci.width_;

        std::string const& s = values[col];
        if(!s.empty())
            {
            int x_text = x;

            if(align_right_)
                {
                x_text += width - dc_.GetTextExtent(s).x;
                }
            else
                {
                if(ci.is_centered())
                    {
                    // Centre the text for the columns configured to do it.
                    x_text += (width - dc_.GetTextExtent(s).x) / 2;
                    }
                else
                    {
                    x_text += column_margin_;
                    }
                }

            // Tiny helper to avoid duplicating the same DrawText() call in
            // both branches of the "if" statement below. It might not be that
            // useful now, but could become so if this simple DrawText() gets
            // more complicated in the future.
            auto const do_output = [=]() { dc_.DrawText(s, x_text, y_text); };

            if(ci.needs_clipping())
                {
                wxDCClipper clip
                    (dc_
                    ,wxRect
                        {wxPoint{x, y_top}
                        ,wxSize{width - column_margin_, row_height_}
                        }
                    );

                do_output();
                }
            else
                {
                do_output();
                }
            }
        x += width;
        if(draw_separators_)
            {
            do_output_vert_separator(x, y_top, y);
            }
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
    (std::size_t                  begin_column
    ,std::size_t                  end_column
    ,int                          y
    ,oenum_render_or_only_measure output_mode
    )
{
    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            return;
        }

    LMI_ASSERT(begin_column < end_column);
    LMI_ASSERT(end_column <= columns_.size());

    do_compute_column_widths();

    int const x1 = do_get_cell_x(begin_column);

    int x2 = x1;
    for(std::size_t col = begin_column; col < end_column; ++col)
        {
        x2 += columns_.at(col).width_;
        }

    do_output_horz_separator(x1, x2, y);
}

void wx_table_generator::output_header
    (int* pos_y
    ,oenum_render_or_only_measure output_mode
    )
{
    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            *pos_y += max_header_lines_ * row_height_;
            return;
        }

    do_compute_column_widths();

    wxDCFontChanger header_font_setter(dc_);
    if(use_bold_headers_)
        {
        header_font_setter.Set(get_header_font());
        }

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
        std::vector<std::string> const nth_line
            (headers_by_line.begin() +      line  * num_columns
            ,headers_by_line.begin() + (1 + line) * num_columns
            );
        x = left_margin_;
        do_output_values(x, *pos_y, nth_line);
        }

    // Finally draw the separators above and (a double one) below them.
    if(draw_separators_)
        {
        do_output_horz_separator(left_margin_, x,  y_top    );
        do_output_horz_separator(left_margin_, x, *pos_y - 1);
        do_output_horz_separator(left_margin_, x, *pos_y    );
        }
}

void wx_table_generator::output_super_header
        (std::string const&           header
        ,std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int*                         pos_y
        ,oenum_render_or_only_measure output_mode
        )
{
    std::vector<std::string> const lines(split_into_lines(header));

    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            *pos_y += row_height_ * lines.size();
            return;
        }

    // We don't have a function for getting the rectangle of a span of columns,
    // but we can reuse the existing text_rect() if we just increase its width
    // by the width of all the extra (i.e. not counting the starting one)
    // columns in this span.
    auto rect = text_rect(begin_column, *pos_y);
    rect.width += do_get_cell_x(end_column) - do_get_cell_x(begin_column + 1);

    for(auto const& line : lines)
        {
        dc_.DrawLabel(line, rect, wxALIGN_CENTER_HORIZONTAL);

        rect.y += row_height_;
        *pos_y += row_height_;
        }
}

void wx_table_generator::output_row
    (int* pos_y
    ,std::vector<std::string> const values
    )
{
    int x = left_margin_;
    do_output_values(x, *pos_y, values);

    if(draw_separators_)
        {
        do_output_horz_separator(left_margin_, x, *pos_y);
        }
}

void wx_table_generator::output_highlighted_cell
    (std::size_t        column
    ,int                y
    ,std::string const& value
    )
{
    if(columns_.at(column).is_hidden())
        {
        return;
        }

    {
    wxDCPenChanger set_transparent_pen(dc_, *wxTRANSPARENT_PEN);
    wxDCBrushChanger set_grey_brush(dc_, *wxLIGHT_GREY_BRUSH);
    dc_.DrawRectangle(cell_rect(column, y));
    }

    dc_.DrawLabel(value, text_rect(column, y), wxALIGN_CENTER_HORIZONTAL);

    output_vert_separator(column, y);
}
