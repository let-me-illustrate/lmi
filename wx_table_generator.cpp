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
#include "math_functions.hpp"           // outward_quotient()
#include "miscellany.hpp"               // count_newlines(), split_into_lines()

#include <algorithm>                    // max()

// Default size of various characters for illustrations and group quotes:
//   'M' 7pt; 'N' 6pt; '1' 4pt; '9' 4pt; ',' 2pt

// Is this a struct only because we want its members to be publicly
// accessible? But their values can also be changed by clients, and
// isn't that undesirable?

// Elasticity and clipping
//
// Most columns are inelastic: they have a fixed minimum width and
// are not clipped lest crucial information (e.g., part of a number)
// be lost. The archetypal elastic column is a personal name, whose
// width is practically unlimited and might even exceed the total page
// width; it is better to truncate one extremely long personal name
// than to present an error message and produce no report at all.
//
// An ideal report generator might call GetTextExtent() on every row
// of data to determine a column's ideal width, but this one favors
// speed by setting a presumptive maximum width for each column.
// Therefore, it treats a personal-name column as having no natural
// width at all. Its minimum width might be set equal to its header
// width, but such a refinement is needless in the problem domain. In
// the most extreme case, all inelastic columns would fit, but there
// would be not a single pixel available for elastic columns, which
// would all in effect be dropped; again, in the problem domain, that
// would actually be preferable to failing to produce any output.
//
// Therefore, elastic columns are clipped, and inelastic ones are not.
// All other column properties are independent, and specified by
// arguments, but clipping depends on the elasticity argument. It is
// distinguished only because clipping is a distinct layout operation.
//
//  - is_hidden(): Data for every row of all potential columns are
//    passed into this class; hidden columns are suppressed so that
//    they don't appear in the output at all.
//
//  - is_elastic(): An elastic column has no innate fixed or preferred
//    width. After all inelastic columns have claimed their required
//    widths, any remaining width available is prorated among elastic
//    columns, which therefore may be wider than their widest contents
//    or narrower than their narrowest. As a consequence, elastic
//    columns are clipped--vide supra.
//
//  - is_clipped(): A clipped column is truncated to fit its allotted
//    space. Only elastic columns are clipped--vide supra.

class wx_table_generator::column_info
{
  public:
    column_info
        (std::string      const& header
        ,int                     width
        ,oenum_h_align    const  alignment
        ,oenum_visibility const  visibility
        ,oenum_elasticity const  elasticity
        )
        :col_header_ (header)
        ,col_width_  (width)
        ,alignment_  (alignment)
        ,is_hidden_  (oe_hidden  == visibility)
        ,is_elastic_ (oe_elastic == elasticity)
        {
        }

    std::string const& col_header() const {return col_header_;}
    int                col_width()  const {return col_width_;}
    oenum_h_align      alignment()  const {return alignment_;}
    bool               is_hidden()  const {return is_hidden_;}
    bool               is_elastic() const {return is_elastic_;}
    bool               is_clipped() const {return is_elastic();}

  private:
    std::string   const col_header_;

  public: // but dubiously so
    // Width in pixels. Because the wxPdfDC uses wxMM_POINTS, each
    // pixel is one point = 1/72 inch.
    //
    // Modified directly by wx_table_generator code, hence not const.
    int col_width_;

  private:
    oenum_h_align const alignment_;
    bool          const is_hidden_;
    bool          const is_elastic_;
};

wx_table_generator::wx_table_generator
    (group_quote_style_tag                 // tag not referenced
    ,std::vector<column_parameters> const& vc
    ,wxDC&                                 dc
    ,int                                   left_margin
    ,int                                   total_width
    )
    :dc_               (dc)
    ,left_margin_      (left_margin)
    ,total_width_      (total_width)
    ,char_height_      (dc_.GetCharHeight())
    // Arbitrarily use 1.333 line spacing.
    ,row_height_       ((4 * char_height_ + 2) / 3)
    ,column_margin_    (dc_.GetTextExtent("M").x)
    ,max_header_lines_ (1)
    ,draw_separators_  (true)
    ,use_bold_headers_ (true)
{
    for(auto const& i : vc)
        {
        enroll_column(i);
        }
    compute_column_widths();

    // Set a pen with zero width to make grid lines thin,
    // and round cap style so that they combine seamlessly.
    wxPen pen(*wxBLACK, 0);
    pen.SetCap(wxCAP_ROUND);
    dc_.SetPen(pen);
}

wx_table_generator::wx_table_generator
    (illustration_style_tag                // tag not referenced
    ,std::vector<column_parameters> const& vc
    ,wxDC&                                 dc
    ,int                                   left_margin
    ,int                                   total_width
    )
    :dc_               (dc)
    ,left_margin_      (left_margin)
    ,total_width_      (total_width)
    ,char_height_      (dc_.GetCharHeight())
    ,row_height_       (char_height_)
    ,column_margin_    (dc_.GetTextExtent("M").x)
    ,max_header_lines_ (1)
    ,draw_separators_  (false)
    ,use_bold_headers_ (false)
{
    for(auto const& i : vc)
        {
        enroll_column(i);
        }
    compute_column_widths();

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
    std::size_t const number_of_columns = all_columns().size();
    std::vector<std::string> headers_by_line(max_header_lines_ * number_of_columns);
    for(std::size_t i = 0; i < number_of_columns; ++i)
        {
        column_info const& ci = all_columns().at(i);
        if(ci.is_hidden())
            {
            continue;
            }

        std::vector<std::string> const lines(split_into_lines(ci.col_header()));

        // Fill the elements from the bottom line to the top one, so that a
        // single line header is shown on the last line.
        std::size_t const first_line = max_header_lines_ - lines.size();
        for(std::size_t j = 0; j < lines.size(); ++j)
            {
            headers_by_line.at
                ((first_line + j) * number_of_columns + i
                ) = lines.at(j);
            }
        }

    // And output all lines of all column headers.
    int y_top = pos_y;
    int x = 0;
    for(std::size_t i = 0; i < max_header_lines_; ++i)
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
        ,std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode
        )
{
    std::vector<std::string> const lines(split_into_lines(header));
    int const anticipated_pos_y = pos_y + row_height() * lines.size();

    switch(output_mode)
        {
        case oe_render:
            break;
        case oe_only_measure:
            pos_y = anticipated_pos_y;
            return;
        }

    // We don't have a function for getting the rectangle of a span of columns,
    // but we can reuse the existing text_rect() if we just increase its width
    // by the width of all the extra (i.e. not counting the starting one)
    // columns in this span.
    auto rect = text_rect(begin_column, pos_y);
    rect.width += cell_pos_x(end_column) - cell_pos_x(begin_column + 1);

    for(auto const& i : lines)
        {
        dc_.DrawLabel(i, rect, wxALIGN_CENTER_HORIZONTAL);
        rect.y += row_height_;
        pos_y  += row_height_;
        }

    LMI_ASSERT(anticipated_pos_y == pos_y);
}

/// Shade the background of a single cell; center the given contents.

void wx_table_generator::output_highlighted_cell
    (std::size_t        column
    ,int                y
    ,std::string const& value
    )
{
    if(all_columns().at(column).is_hidden())
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

/// Render a single row with the given values at the given position.
///
/// The cardinality of the 'values' argument must equal the total
/// number of columns passed to enroll_column().

void wx_table_generator::output_row
    (int&                           pos_y
    ,std::vector<std::string> const values
    )
{
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

void wx_table_generator::output_vert_separator
    (std::size_t before_column
    ,int         y
    )
{
    LMI_ASSERT(before_column <= all_columns().size());

    do_output_vert_separator(cell_pos_x(before_column), y, y + row_height_);
}

/// Output a horizontal separator line across the specified columns.
///
/// The column range is specified as [begin, end), as is usual in C++.

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
    LMI_ASSERT(end_column <= all_columns().size());

    int const x1 = cell_pos_x(begin_column);

    int x2 = x1;
    for(std::size_t i = begin_column; i < end_column; ++i)
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

/// Rectangle corresponding to a cell's text contents.
///
/// This is narrower than the full cell rectangle to leave a small
/// margin. Its vertical position is adjusted to center the text vertically.

wxRect wx_table_generator::text_rect(std::size_t column, int y) const
{
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
/// All data for every row and every potential column are passed into
/// this class, even for "hidden" columns that are to be suppressed so
/// that they don't appear in the output at all. This approach trades
/// extra complexity here for a uniform data representation elsewhere.
///
/// Notice that column headers may be multiline strings.
///
/// Design alternative: this could be written as a nonmember function,
/// by passing the DC and the header font as arguments.

void wx_table_generator::enroll_column(column_parameters const& z)
{
    // A hidden column's width must be initialized to zero, because
    // other member functions calculate total width by accumulating
    // the widths of all columns, whether hidden or not.
    //
    // An elastic column's width must be initialized to zero, because
    // compute_column_widths() skips setting it when there's no room
    // for any elastic column.
    int width = 0;
    if(oe_shown == z.visibility)
        {
        wxDCFontChanger header_font_setter(dc_);
        if(use_bold_headers_)
            {
            header_font_setter.Set(header_font());
            }

        wxCoord w, h, lh;
        dc().GetMultiLineTextExtent(z.header, &w, &h, &lh, &dc().GetFont());
        LMI_ASSERT(0 != lh);
        LMI_ASSERT(0 == h % lh);
// Temporarily assert that this does the same as the code it replaced:
LMI_ASSERT(h / lh == int(1u + count_newlines(z.header)));
// Check it again because of the unfortunate mixed-mode arithmetic:
LMI_ASSERT(std::size_t(h / lh) == 1u + count_newlines(z.header));
        // Store number of lines used by tallest unhidden header:
        // output_headers() uses it to write all headers as a block.
        max_header_lines_ = std::max(max_header_lines_, std::size_t(h / lh));

        switch(z.elasticity)
            {
            case oe_inelastic:
                {
                // Greater of header width and 'widest_text' width.
                width = std::max(w, dc().GetTextExtent(z.widest_text).x);
                // PDF !! Reconsider whether margin should be added here,
                // because compute_column_widths() may need to remove it.
                width += 2 * column_margin();
                }
                break;
            case oe_elastic:
                {
                ; // Do nothing: 'width' already initialized to zero.
                }
                break;
            }
        }

    all_columns_.push_back
        (column_info
            (z.header, width, z.alignment, z.visibility, z.elasticity)
        );
}

// class members used, mutably or immutably:
//
// const    total_width_    max table width (page width - page margins)
// mutable  column_margin_  spacing on both left and right of column
// mutable  all_columns_    std::vector<column_info>
//   column_info::col_width_ is the only member changed
//
// column_margin_ and col_width_ are modified here and nowhere else

/// Compute column widths.
///
/// First, allocate adequate width to each inelastic column; then
/// distribute any excess width left over among elastic columns.
///
/// The width of each inelastic column reflects:
///  - a mask like "999,999" (ideally, there would instead be a
///    quasi-global data structure mapping symbolic column names
///    to their corresponding headers and maximal widths)
///  - the header width
///  - the bilateral margins that have already been added
/// The margins may be slightly reduced by this function to make
/// everything fit when it otherwise wouldn't.

void wx_table_generator::compute_column_widths()
{
    // Number of non-hidden columns.
    int number_of_columns = 0;

    // Number of non-hidden elastic columns.
    int number_of_elastic_columns = 0;

    // Total width of all non-hidden inelastic columns.
    int total_inelastic_width = 0;

    for(auto const& i : all_columns())
        {
// Instead of retaining hidden columns, and explicitly skipping them
// here and repeatedly later, why not just remove them from the vector?
        if(i.is_hidden())
            {
            continue;
            }

        ++number_of_columns;

        if(i.is_elastic())
            {
            ++number_of_elastic_columns;
            }
        else
            {
            total_inelastic_width += i.col_width();
            }
        }

    if(total_width_ < total_inelastic_width)
        {
        // The inelastic columns don't all fit with their original
        // one-em presumptive bilateral margins. Try to make them fit
        // by reducing the margins slightly.
        //
        // The number of pixels that would need to be removed is:
        auto const overflow = total_inelastic_width - total_width_;

        // Because inelastic columns take more than the available
        // horizontal space, there's no room to fit any elastic
        // columns, so the column-fitting problem is overconstrained.
        // Therefore, don't even try reducing margins if there are any
        // elastic columns.
        if(!number_of_elastic_columns)
            {
// Also calculate the number of pixels by which it overflows for each column
            // We need to round up in division here to be sure that all columns
            // fit into the available width.
            auto const overflow_per_column = outward_quotient
                (overflow
                ,number_of_columns
                );
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
// Answer: Yes--the integers count points, not ems or characters, and
// typographers wouldn't use any finer unit for this task.
            if(overflow_per_column <= 2 * column_margin())
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
                auto underflow = overflow_per_column * number_of_columns - overflow;

                for(auto& i : all_columns_)
                    {
                    if(i.is_hidden())
                        {
                        continue;
                        }

                    i.col_width_ -= overflow_per_column;

                    if(0 < underflow)
                        {
                        ++i.col_width_;
                        --underflow;
                        }
                    }

                column_margin_ -= (overflow_per_column + 1) / 2;

                // We condensed the columns enough to make them fit, so no need
                // for the warning and we don't have any elastic columns, so
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

        warning()
            << "Not enough space for all " << number_of_columns << " columns."
            << "\nPrintable width is " << total_width_ << " points."
            << "\nData alone require " << total_inelastic_width - 2 * column_margin() * number_of_columns
            << " points without any margins for legibility."
            << "\nColumn margins of " << column_margin() << " points on both sides"
            << " would take up " << 2 * column_margin() * number_of_columns << " additional points."
            << LMI_FLUSH
            ;
        return;
        }

    // Lay out elastic columns in whatever space is left over after
    // accounting for all inelastic columns. Clip to make them fit.
    //
    // If there's more than enough space for them, then expand them
    // to consume all available space.
    if(number_of_elastic_columns)
        {
        int const width_of_each_elastic_column = outward_quotient
            (total_width_ - total_inelastic_width
            ,number_of_elastic_columns
            );

        for(auto& i : all_columns_)
            {
            if(i.is_hidden())
                {
                continue;
                }

            if(i.is_elastic())
                {
                i.col_width_ = width_of_each_elastic_column;
                }
            }
        }
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

    std::size_t const number_of_columns = all_columns().size();
    for(std::size_t i = 0; i < number_of_columns; ++i)
        {
        column_info const& ci = all_columns().at(i);
        if(ci.is_hidden())
            {
            continue;
            }

        std::string const& s = values[i];
        if(!s.empty())
            {
            int x_text = pos_x;

            switch(ci.alignment())
                {
                case oe_left:
                    {
                    // PDF !! 'x_text += 0;' here would parallel the other
                    // cases. The implicit assumption here is that alignment
                    // is oe_left iff elasticity is oe_elastic; col_width()
                    // has been augmented by twice the margin for oe_inelastic
                    // columns only, and this adjustment compensates for that.
                    x_text += column_margin();
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

            if(ci.is_clipped())
                {
                // It is assumed that the width of the "Participant" column
                // on a group quote was initially zero, and then was expanded
                // by some positive amount, and then incremented by one times
                // the margin (not two times the margin as for other columns,
                // because this column has only a left-hand unlateral margin).
                // Make sure that any failure in this chain of assumptions
                // doesn't result in (undefined) negative clipping.
                LMI_ASSERT(0 <= ci.col_width() - column_margin());
                wxDCClipper clip
                    (dc_
                    ,wxRect
                        {wxPoint{pos_x, y_top}
                        ,wxSize{ci.col_width() - column_margin(), row_height_}
                        }
                    );
                dc_.DrawText(s, x_text, y_text);
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
    // PDF !! add a possibility to have a thick border between the columns.
    dc_.DrawLine(x, y1, x, y2);
}

void wx_table_generator::do_output_horz_separator(int x1, int x2, int y)
{
    dc_.DrawLine(x1, y, x2, y);
}

int wx_table_generator::cell_pos_x(std::size_t column) const
{
    int x = left_margin_;
    for(std::size_t i = 0; i < column; ++i)
        {
        x += all_columns().at(i).col_width();
        }

    return x;
}

/// Rectangle corresponding to a cell.

wxRect wx_table_generator::cell_rect(std::size_t column, int y) const
{
    return wxRect
        (cell_pos_x(column)
        ,y
        ,all_columns().at(column).col_width()
        ,row_height_
        );
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

int wx_table_generator::column_margin() const
{
    return column_margin_;
}

std::vector<wx_table_generator::column_info> const& wx_table_generator::all_columns() const
{
    return all_columns_;
}
