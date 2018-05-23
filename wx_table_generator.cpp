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

#include <algorithm>                    // max()

// Is this a struct only because we want its members to be publicly
// accessible? But their values can also be changed by clients, and
// isn't that undesirable?

// Under what circumstances might columns be hidden, centered, or clipped?
//
// General answer:
//  In principle, all of those are independent. In practice, inelastic
// columns are centered and elastic columns are clipped and only the
// former can be hidden. But I don't think this low level class should impose
// such high level constraints on its use, which is why it doesn't do it.
//
// - is_hidden()
//
// All potential data are passed for every row; is_hidden() suppresses
// any column that needs to be filtered out.
//
// - needs_clipping()
//
// And what sort of columns need to be clipped? As currently implemented,
// this function is equivalent to is_elastic()--i.e., only
// the "Participant" column on group quotes?
// Answer: Currently, yes, as it's the only elastic column.
//
// Does this all boil down to...
//  - left-align and clip the group-quote "Participant" column
//  - center all other group-quote columns
//  - ignore all these accessors for illustration PDFs
// ?
// Answer: yes.

//  - is_hidden(): A hidden column is present in the data passed into
//    this class, but is to be suppressed so that it doesn't appear in
//    the output at all.
//
//  - is_elastic(): An elastic column has no innate fixed or preferred
//    width. After all inelastic columns have claimed their required
//    widths, any remaining width available is prorated among elastic
//    columns, which therefore may be wider than their widest contents
//    or narrower than their narrowest. As a consequence, elastic
//    columns must be clipped if necessary.
//
//  - needs_clipping(): Indicate whether column contents need to be
//    clipped when outputting it. Only elastic columns are clippable:
//    for example, personal names are of unbounded length and might
//    even exceed the total page width. Inelastic columns, OTOH, have
//    fixed minimum widths and must never be clipped.

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

    std::string const& col_header()     const {return col_header_;}
    int                col_width()      const {return col_width_;}
    oenum_h_align      alignment()      const {return alignment_;}
    bool               is_hidden()      const {return is_hidden_;}
    bool               is_elastic()     const {return is_elastic_;}
    bool               needs_clipping() const {return is_elastic();}

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

int wx_table_generator::column_margin() const
{
    return column_margin_;
}

std::vector<wx_table_generator::column_info> const& wx_table_generator::all_columns() const
{
    return all_columns_;
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
            header_font_setter.Set(get_header_font());
            }

        wxCoord w, h, lh;
        dc_.GetMultiLineTextExtent(z.header, &w, &h, &lh, &dc_.GetFont());
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
                width = std::max(w, dc_.GetTextExtent(z.widest_text).x);
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

/// Return the font used for the headers.

wxFont wx_table_generator::get_header_font() const
{
    return dc_.GetFont().Bold();
}

// Horizontal and vertical separators are considered to be drawn in
// interlinear space, so they are not counted when calculating the
// positions of textual elements.

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
    int x = left_margin_;
    for(std::size_t col = 0; col < column; ++col)
        {
        x += all_columns().at(col).col_width();
        }

    return x;
}

/// Return the height of a single table row.

int wx_table_generator::row_height() const
{
    return row_height_;
}

/// Return the rectangle containing the cell area.

wxRect wx_table_generator::cell_rect(std::size_t column, int y)
{
    return wxRect
        (do_get_cell_x(column)
        ,y
        ,all_columns().at(column).col_width()
        ,row_height_
        );
}

/// Return the rectangle adjusted for the text contents of the cell: it is
/// more narrow than the full cell rectangle to leave margins around the
/// text and its vertical position is adjusted so that it can be directly
/// passed to wxDC::DrawLabel().

wxRect wx_table_generator::text_rect(std::size_t column, int y)
{
    wxRect z = cell_rect(column, y).Deflate(dc_.GetCharWidth(), 0);
    z.Offset(0, char_height_);
    return z;
}

// class members used, mutably or immutably:
//
// const    total_width_
// mutable  column_margin_
// mutable  all_columns_
//   i.e. std::vector<column_info> all_columns_;
// mutable  column_info elements
//   the only column_info function member called is is_hidden()
//   the only column_info data member modified is col_width_
//
// meanings (written before each variable, as in header documentation):
//
    // ctor parameter:
    // max table width (page width minus horizontal page margins)
// const    total_width_
    // spacing on both left and right of column
    // initialized in ctor to # pixels in one em: (dc_.GetTextExtent("M").x)
    // changed in this function and nowhere else
// mutable  column_margin_
    // std::vector<column_info>
// mutable  all_columns_

/// Compute column widths.

void wx_table_generator::compute_column_widths()
{
    // Number of non-hidden columns.
    int n_columns = 0;

    // Number of non-hidden elastic columns.
    //
    // In practice, only the "Participant" column on group quotes has
    // this property.
    //
    // The rationale for this property is that, once adequate width
    // has been allocated to each column, any excess width left over
    // is to be distributed among such elastic columns only:
    // i.e., they (and only they) are to be "expanded".
    int n_expand = 0;

    // Total width of all non-hidden inelastic columns.
    // The width of each inelastic column reflects:
    //  - a mask like "999,999" (ideally, there would instead be a
    //    quasi-global data structure mapping symbolic column names
    //    to their corresponding headers and maximal widths)
    //  - the header width
    //  - the bilateral margins that have already been added
    // The margins may be slightly reduced by this function to make
    // everything fit when it otherwise wouldn't.
    int total_inelastic_width = 0;

    for(auto const& i : all_columns())
        {
// Instead of retaining hidden columns, and explicitly skipping them
// here and repeatedly later, why not just remove them from the vector?
        if(i.is_hidden())
            {
            continue;
            }

        n_columns++;

        if(i.is_elastic())
            {
            n_expand++;
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
        if(!n_expand)
            {
// Also calculate the number of pixels by which it overflows for each column
            // We need to round up in division here to be sure that all columns
            // fit into the available width.
            auto const overflow_per_column =
                (overflow + n_columns - 1) / n_columns;
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
                auto underflow = overflow_per_column * n_columns - overflow;

                for(auto& i : all_columns_)
                    {
                    if(i.is_hidden())
                        {
                        continue;
                        }

                    i.col_width_ -= overflow_per_column;

                    if(0 < underflow)
                        {
                        i.col_width_++;
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
            << "Not enough space for all " << n_columns << " columns."
            << "\nPrintable width is " << total_width_ << " points."
            << "\nData alone require " << total_inelastic_width - 2 * column_margin() * n_columns
            << " points without any margins for legibility."
            << "\nColumn margins of " << column_margin() << " points on both sides"
            << " would take up " << 2 * column_margin() * n_columns << " additional points."
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

    // Lay out elastic columns in whatever space is left over
    // after accounting for all inelastic columns.
    //
    // If there's more than enough space for them, then expand them
    // to consume all available space.
    //
    // If there isn't enough space for their headers and contents,
    // then clip them. Motivation: the archetypal elastic
    // column is a personal name, which has practically unlimited
    // width. On a group premium quote, numeric columns must never
    // be truncated, but truncating one extremely long personal name
    // is preferable to failing to produce any quote at all. It would
    // of course be possible to take the header of such a column as
    // its minimal width, but that would be a useless refinement in
    // the problem domain. In the most extreme conceivable case, all
    // inelastic columns would fit, but there would be not a single
    // pixel available for elastic columns and they would all
    // in effect be dropped; again, in the problem domain, that would
    // actually be preferable to failing to produce any output.
    if(n_expand)
        {
        int const per_expand
            = (total_width_ - total_inelastic_width + n_expand - 1) / n_expand;

        for(auto& i : all_columns_)
            {
            if(i.is_hidden())
                {
                continue;
                }

            if(i.is_elastic())
                {
                i.col_width_ = per_expand;
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

    int const y_text = pos_y + char_height_;
    pos_y += row_height_;

    if(draw_separators_)
        {
        do_output_vert_separator(pos_x, y_top, pos_y);
        }

    std::size_t const n_columns = all_columns().size();
    for(std::size_t col = 0; col < n_columns; ++col)
        {
        column_info const& ci = all_columns().at(col);
        if(ci.is_hidden())
            {
            continue;
            }

        std::string const& s = values[col];
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
                    x_text += (ci.col_width() - dc_.GetTextExtent(s).x) / 2;
                    }
                    break;
                case oe_right:
                    {
                    x_text += ci.col_width() - dc_.GetTextExtent(s).x;
                    }
                    break;
                }

            if(ci.needs_clipping())
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

/// Output a vertical separator line before the given column. Notice that
/// the column index here may be equal to the number of columns in order to
/// output a separator after the last column.

void wx_table_generator::output_vert_separator
    (std::size_t before_column
    ,int         y
    )
{
    LMI_ASSERT(before_column <= all_columns().size());

    do_output_vert_separator
        (do_get_cell_x(before_column)
        ,y
        ,y + row_height_
        );
}

/// Output a horizontal separator line across the specified columns,
/// using the usual C++ close/open interval convention.

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

    int const x1 = do_get_cell_x(begin_column);

    int x2 = x1;
    for(std::size_t col = begin_column; col < end_column; ++col)
        {
        x2 += all_columns().at(col).col_width();
        }

    do_output_horz_separator(x1, x2, y);
}

// Return the amount of vertical space taken by separator lines in the
// table headers.
int wx_table_generator::get_separator_line_height() const
{
    // This is completely arbitrary and chosen just because it seems to
    // look well.
    return row_height() / 2;
}

/// Render the headers at the given position and update it.

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
        header_font_setter.Set(get_header_font());
        // The distance from the font's descender line to its ascender
        // line must not exceed the distance between lines.
        LMI_ASSERT(dc_.GetCharHeight() <= row_height());
        // do_output_single_row(), called below, uses a cached char_height_
        // that is assumed not to differ from the bold GetCharHeight().
        LMI_ASSERT(dc_.GetCharHeight() == char_height_);
        }

    // Split headers in single lines and fill up the entire columns*lines 2D
    // matrix, using empty strings for the headers with less than the maximal
    // number of lines.
    std::size_t const n_columns = all_columns().size();
    std::vector<std::string> headers_by_line(max_header_lines_ * n_columns);
    for(std::size_t col = 0; col < n_columns; ++col)
        {
        column_info const& ci = all_columns().at(col);
        if(ci.is_hidden())
            {
            continue;
            }

        std::vector<std::string> const lines(split_into_lines(ci.col_header()));

        // Fill the elements from the bottom line to the top one, so that a
        // single line header is shown on the last line.
        std::size_t const first_line = max_header_lines_ - lines.size();
        for(std::size_t line = 0; line < lines.size(); ++line)
            {
            headers_by_line.at
                ((first_line + line) * n_columns + col
                ) = lines.at(line);
            }
        }

    // And output all lines of all column headers.
    int y_top = pos_y;
    int x = 0;
    for(std::size_t line = 0; line < max_header_lines_; ++line)
        {
        std::vector<std::string> const nth_line
            (headers_by_line.begin() +      line  * n_columns
            ,headers_by_line.begin() + (1 + line) * n_columns
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

/// Render a super-header, i.e. a header spanning over several columns. The
/// columns range is specified as a close/open interval, as usual in C++.
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
    rect.width += do_get_cell_x(end_column) - do_get_cell_x(begin_column + 1);

    for(auto const& line : lines)
        {
        dc_.DrawLabel(line, rect, wxALIGN_CENTER_HORIZONTAL);

        rect.y += row_height_;
        pos_y  += row_height_;
        }

    LMI_ASSERT(anticipated_pos_y == pos_y);
}

/// Render a row with the given values at the given position and update it.
/// The values here can be single-line only and there must be exactly the
/// same number of them as the number of columns.

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

/// Render a single highlighted (by shading its background) cell with the
/// given string displayed in it (always centered).

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
