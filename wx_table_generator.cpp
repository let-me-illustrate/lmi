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

//   is_centered_ is a member variable, initialized in the ctor
//   is_hidden() is a member function, whose return value is dynamic
// Should these really be implemented in those two different ways?
// Wouldn't it be better to treat is_hidden() the same as is_centered_?
//
// Is this a struct only because we want its members to be publicly
// accessible? But their values can also be changed by clients, and
// isn't that undesirable?
//
// In wx_table_generator::do_output_single_row():
//   if(align_right_)
//   if(ci.is_centered_)
// it seems that right-alignment is a quasi-global, while
// center-alignment is a column_info data member. Historically, this
// evolved because right-alignment was recently added to support
// illustrations, while center-alignment was already used for group
// quotes. But when code is complex for "historical reasons", it's
// natural to ask whether it ought to be refactored for uniformity.

// Under what circumstances might columns be hidden, centered, or clipped?
//
// General answer:
//  In principle, all of those are independent. In practice, fixed width
// columns are centered and variable width columns are clipped and only the
// former can be hidden. But I don't think this low level class should impose
// such high level constraints on its use, which is why it doesn't do it.
//
// - is_hidden()
//
// Apparently used only for group premium quotes, e.g.:
//
//             case e_col_total_face_amount:
//                 if(!has_suppl_amount)
//                     // Leave the header empty to hide this column.
//                     break;
//                 // Fall through
//             ...
//                 header = cd.header_;
//
// Some columns are conditionally hidden by should_show_column():
//
//     // May be overridden to return false if the given column shouldn't be shown
//     // for the specific ledger values (currently used to exclude individual
//     // columns from composite illustrations).
//     virtual bool should_show_column(Ledger const& ledger, int column) const
//
// but that technique seems to be orthogonal to is_hidden() and used
// only for illustration PDFs.
// --No, it's not orthogonal, should_show_column() is used to decide whether
// the column label should be left empty, making the column hidden.
//
//  - is_centered()
//
// This seems to be used only in one place:
//
//     if(ci.is_centered())
//         {
//         // Centre the text for the columns configured to do it.
//         x_text += (width - dc_.GetTextExtent(s).x) / 2;
//         }
//
// What exactly does it mean for a column to be "centered"? I think this
// is a different concept than using "center" alignment for cells in a
// spreadsheet column, which would give, e.g.:
//     1
//   11111
// --No, it's exactly the same concept.
// In spreadsheet terminology, almost all our columns are numeric, and our
// numeric columns are right-aligned. But the function is documented thus:
//
//     // Return true if this column should be centered, rather than
//     // left-aligned. Notice that this is ignored for globally right-aligned
//     // tables.
//
// Is it then the case that:
//  - for illustration PDFs, all columns are right-aligned, and
//  - is_centered is used only for group quotes, where it really does
//    mean the same thing as "center" alignment in a spreadsheet
// ?
// Answer: yes.
//  This is indeed not as lucid as I'd like, but the alternative would to
// modify the PDF quotes code to align all the columns explicitly, which I
// preferred not to do as part of illustrations work. Maybe now, that this is
// merged, it's indeed worth changing this.
//  OTOH, unlike a spreadsheet, this class doesn't have any notion of numeric
// or text values, so its align_right_ member is still useful to globally
// configure all columns to be right-aligned. Perhaps we could just add a
// similar align_centre() method and call it from the group PDF quotes code
// and continue to handle the variable width columns specially by
// left-aligning them in any case?
//
// Apparently is_centered() always returns true (but is ignored)
// for illustrations, and this comment inside the function body
// applies to group quotes only:
//
//     // Fixed width columns are centered by default, variable width ones
//     // are not as long strings look better with the default left
//     // alignment.
//
// What sort of columns are not centered?
// Answer: Variable width ones (only used by PDF quotes).
//
// - needs_clipping()
//
// And what sort of columns need to be clipped? As currently implemented,
// this function is the logical negation of is_centered(), so only columns
// that are not centered need clipping--but what columns are those? Only
// the "Participant" column on group quotes?
// Answer: Currently, yes, as it's the only variable width column.
//
// Does this all boil down to...
//  - left-align and clip the group-quote "Participant" column
//  - center all other group-quote columns
//  - ignore all these accessors for illustration PDFs
// ?
// Answer: yes.
//
// And what does 'is_variable_width_' mean? As implemented, it means
// that the constructor's 'width' argument was zero at the time of
// construction. Is that in effect just another way of identifying
// the "Participant" column?
// Answer:
//  No, as with "centered" above, it really means what it says: a variable
// width column is one whose width is not fixed, i.e. not defined by the
// widest string that can appear in it, but takes all the available space
// remaining from the other, fixed width columns (in principle, there can be
// more than one variable width column, even if currently this is not the
// case).
// The fundamental distinction is really
// between fixed and variable width columns: the latter ones are always
// left-aligned and need to be clipped, while the former ones are either
// centered or right-aligned (if align_right_ is true) and not clipped.
// And I think things are reasonably simple seen from this point of view and
// this is how you're supposed to see them, because it's how this class is
// used, while the various accessors discussed above are just its
// implementation details.

//  - is_hidden(): A column with empty header is considered to be
//    suppressed and doesn't appear in the output at all.
//
//  - is_centered(): Indicate whether column should be centered,
//    rather than left-aligned. Ignored for globally right-aligned
//    tables. [Fixed width columns are centered by default, variable
//    width ones are not as long strings look better with the default
//    left alignment.]
//
//  - is_variable_width():
//
//  - needs_clipping(): Indicate whether column contents need to be
//    clipped when outputting it. [Variable width columns can have
//    practically unlimited length and hence overflow into the next
//    column or even beyond and must be clipped to prevent this from
//    happening. Fixed width columns are not supposed to overflow
//    anyhow, so clipping them is unnecessary.]

class wx_table_generator::column_info
{
  public:
    column_info(std::string const& header, int width)
        :col_header_       (header)
        ,col_width_        (width)
        ,is_variable_width_(0 == width)
        {
        }

    bool is_hidden()         const {return col_header().empty();}
    bool is_centered()       const {return !is_variable_width_;}
    bool is_variable_width() const {return  is_variable_width_;}
    bool needs_clipping()    const {return  is_variable_width_;}

    std::string const& col_header() const {return col_header_;}
    int col_width()                 const {return col_width_;}

  private:
    std::string const col_header_;

  public: // but dubiously so
    // Width in pixels. Because the wxPdfDC uses wxMM_POINTS, each
    // pixel is one point = 1/72 inch.
    //
    // Modified directly by wx_table_generator code, hence not const.
    int col_width_;

  private:
    bool const is_variable_width_;
};

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
    ,align_right_      (false)
{
    for(auto const& i : vc)
        {
        enroll_column(i.header, i.widest_text);
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
    // For the nonce, columns are centered by default because
    // that's what group quotes need; this flag forces right
    // alignment for illustrations.
    ,align_right_      (true)
{
    for(auto const& i : vc)
        {
        enroll_column(i.header, i.widest_text);
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
/// The total number of columns thus enrolled determines the cardinality
/// of the 'values' argument in output_row() calls.
///
/// Providing an empty header suppresses the column display, while still
/// taking it into account in output_row(), providing a convenient way to
/// hide a single column without changing the data representation.
///
/// Each column must either have a fixed width, specified as the width of
/// the longest text that may appear in this column, or be expandable
/// meaning that the rest of the page width is allocated to it which will
/// be the case if widest_text is empty.
///
/// Notice that column headers may be multiline strings.

void wx_table_generator::enroll_column
    (std::string const& header
    ,std::string const& widest_text
    )
{
    // If a column's header is empty, then it is to be hidden--and its
    // width must be initialized to zero, because other member functions
    // calculate total width by accumulating the widths of all columns,
    // whether hidden or not.
    int width = 0;
    if(!header.empty())
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
            width += 2 * column_margin();
            }
        }

    all_columns_.push_back(column_info(header, width));
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

    for(auto const& i : all_columns())
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
            total_fixed += i.col_width();
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
                auto underflow = overflow_per_column * num_columns - overflow;

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
            << "Not enough space for all " << num_columns << " columns."
            << "\nPrintable width is " << total_width_ << " points."
            << "\nData alone require " << total_fixed - 2 * column_margin() * num_columns
            << " points without any margins for legibility."
            << "\nColumn margins of " << column_margin() << " points on both sides"
            << " would take up " << 2 * column_margin() * num_columns << " additional points."
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

        for(auto& i : all_columns_)
            {
            if(i.is_hidden())
                {
                continue;
                }

            if(i.is_variable_width())
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

    std::size_t const num_columns = all_columns().size();
    for(std::size_t col = 0; col < num_columns; ++col)
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

            if(align_right_)
                {
                x_text += ci.col_width() - dc_.GetTextExtent(s).x;
                }
            else
                {
                if(ci.is_centered())
                    {
                    // Centre the text for the columns configured to do it.
                    x_text += (ci.col_width() - dc_.GetTextExtent(s).x) / 2;
                    }
                else
                    {
                    x_text += column_margin();
                    }
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

void wx_table_generator::output_header
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
    std::size_t const num_columns = all_columns().size();
    std::vector<std::string> headers_by_line(max_header_lines_ * num_columns);
    for(std::size_t col = 0; col < num_columns; ++col)
        {
        column_info const& ci = all_columns().at(col);
        std::vector<std::string> const lines(split_into_lines(ci.col_header()));

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
    int y_top = pos_y;
    int x = 0;
    for(std::size_t line = 0; line < max_header_lines_; ++line)
        {
        std::vector<std::string> const nth_line
            (headers_by_line.begin() +      line  * num_columns
            ,headers_by_line.begin() + (1 + line) * num_columns
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
