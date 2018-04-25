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

#ifndef wx_table_generator_hpp
#define wx_table_generator_hpp

#include "config.hpp"

#include "oecumenic_enumerations.hpp"

#include <wx/dc.h>
#include <wx/font.h>

#include <cstddef>                      // size_t
#include <string>
#include <vector>

/// Simplifies outputting tabular data on wxDC.
///
/// To create a table, columns must be initialized first by calling
/// add_column() for each of them once. After this, output_header() and
/// output_row() can be called reusing the same pos_y argument which contains
/// the coordinate of the top of the header or row to output and is updated to
/// correspond to the value for the next row by these functions.

class wx_table_generator
{
  public:
    // The life time of the specified wxDC must be greater than the life time
    // of this object itself and nothing should be using it while this object
    // does (as it changes its attributes).
    //
    // The table has the given total width and starts at the left margin.
    wx_table_generator(wxDC& dc, int left_margin, int total_width);

    // Adds a column to the table. The total number of added columns determines
    // the cardinality of the 'values' argument in output_row() calls.
    //
    // Providing an empty header suppresses the column display, while still
    // taking it into account in output_row(), providing a convenient way to
    // hide a single column without changing the data representation.
    //
    // Each column must either have a fixed width, specified as the width of
    // the longest text that may appear in this column, or be expandable
    // meaning that the rest of the page width is allocated to it which will be
    // the case if widest_text is empty.
    //
    // Notice that column headers may be multiline strings.
    void add_column(std::string const& header, std::string const& widest_text);

    // Render the headers at the given position and update it.
    void output_header
        (int*                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    // Render a super-header, i.e. a header spanning over several columns. The
    // columns range is specified as a close/open interval, as usual in C++.
    // The header string may be multiline, just as with normal headers.
    void output_super_header
        (std::string const&           header
        ,std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int*                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    // Render a row with the given values at the given position and update it.
    // The values here can be single-line only and there must be exactly the
    // same number of them as the number of columns.
    void output_row(int* pos_y, std::string const* values);

    // Render a single highlighted (by shading its background) cell with the
    // given string displayed in it (always centered).
    void output_highlighted_cell
        (std::size_t        column
        ,int                y
        ,std::string const& value
        );

    // Return the number of columns.
    std::size_t columns_count() const {return columns_.size();}

    // Return the height of a single table row.
    int row_height() const {return row_height_;}

    // Return the rectangle containing the cell area.
    wxRect cell_rect(std::size_t column, int y);

    // Return the rectangle adjusted for the text contents of the cell: it is
    // more narrow than the full cell rectangle to leave margins around the
    // text and its vertical position is adjusted so that it can be directly
    // passed to wxDC::DrawLabel().
    wxRect text_rect(std::size_t column, int y);

    // Output a horizontal separator line across the specified columns,
    // using the usual C++ close/open interval convention.
    void output_horz_separator
        (std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int                          y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    // Output a vertical separator line before the given column. Notice that
    // the column index here may be equal to the number of columns in order to
    // output a separator after the last column.
    void output_vert_separator(std::size_t before_column, int y);

    // Use condensed style: don't draw separators between rows and make them
    // smaller.
    void use_condensed_style();

    // By default, columns are centered if they have fixed size or left-aligned
    // otherwise. By calling this method, this alignment auto-detection is
    // turned off and all columns are right-aligned.
    void align_right();

  private:
    // Return the font used for the headers.
    wxFont get_header_font() const;

    int do_get_cell_x(std::size_t column);

    void do_output_horz_separator(int x1, int x2, int y );
    void do_output_vert_separator(int x , int y1, int y2);

    void do_compute_column_widths();

    void do_output_values
        (int&               pos_x
        ,int&               pos_y
        ,std::string const* values
        );

    wxDC& dc_;

    int left_margin_;
    int total_width_;

    // These values could be recomputed, but cache them for performance.
    int const char_height_;
    int row_height_;
    int column_margin_;

    class column_info;
    std::vector<column_info> columns_;

    // Initially false, set to true after do_compute_column_widths()
    // has been called to make all column_info::width_ values valid.
    bool column_widths_already_computed_;

    // Maximal number of lines in any column header, initially 1 but can be
    // higher if multiline headers are used.
    std::size_t max_header_lines_;

    // If false, separator lines are not drawn automatically (they can still be
    // drawn by calling output_horz_separator() or output_vert_separator()
    // explicitly).
    bool draw_separators_ = true;

    // If true, headers are drawn in bold.
    bool use_bold_headers_ = true;

    // If true, force right alignment for all columns instead of centering them
    // automatically if they have fixed size.
    bool align_right_ = false;
};

//   is_centered_ is a member variable, initialized in the ctor
//   is_hidden() is a member function, whose return value is dynamic
// Should these really be implemented in those two different ways?
// Wouldn't it be better to treat is_hidden() the same as is_centered_?
//
// Is this a struct only because we want its members to be publicly
// accessible? But their values can also be changed by clients, and
// isn't that undesirable?
//
// In wx_table_generator::do_output_values():
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
// or text values, so its align_right() method is still useful to globally
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
// centered or right-aligned (if align_right() was called) and not clipped.
// And I think things are reasonably simple seen from this point of view and
// this is how you're supposed to see them, because it's how this class is
// used, while the various accessors discussed above are just its
// implementation details.

class wx_table_generator::column_info
{
  public:
    column_info(std::string const& header, int width)
        :header_(header)
        ,width_(width)
        ,is_variable_width_(width == 0)
        {
        }

    // A column with empty header is considered to be suppressed and
    // doesn't appear in the output at all.
    bool is_hidden() const { return header_.empty(); }

    // Return true if this column should be centered, rather than
    // left-aligned. Notice that this is ignored for globally right-aligned
    // tables.
    bool is_centered() const
    {
        // Fixed width columns are centered by default, variable width ones
        // are not as long strings look better with the default left
        // alignment.
        return !is_variable_width_;
    }

    bool is_variable_width() const
    {
        return is_variable_width_;
    }

    // Return true if the contents of this column needs to be clipped when
    // outputting it.
    bool needs_clipping() const
    {
        // Variable width columns can have practically unlimited length and
        // hence overflow into the next column or even beyond and must be
        // clipped to prevent this from happening. Fixed width columns are
        // not supposed to overflow anyhow, so clipping them is unnecessary.
        return is_variable_width_;
    }

    std::string const header_;

    // Width in pixels. Because the wxPdfDC uses wxMM_POINTS, each
    // pixel is one point = 1/72 inch.
    //
    // Modified directly by wx_table_generator code, hence not const.
    int width_;

  private:
    bool const is_variable_width_;
};

#endif // wx_table_generator_hpp
