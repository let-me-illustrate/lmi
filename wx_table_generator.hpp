// Generate a table using wxDC.
//
// Copyright (C) 2015, 2016, 2017 Gregory W. Chicares.
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

#include <wx/dc.h>
#include <wx/font.h>

#include <cstddef>                      // size_t
#include <string>
#include <vector>

/// Simplifies outputting tabular data on wxDC.
///
/// To create a table, columns must be initialized first by calling
/// set_column() for each of them once. After this, output_header() and
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
    // the number of the expected value in output_row() calls.
    //
    // Providing an empty header suppresses the table display, while still
    // taking into account in output_row(), providing a convenient way to hide
    // a single column without changing the data representation.
    //
    // Each column must either have a fixed width, specified as the width of
    // the longest text that may appear in this column, or be expandable
    // meaning that the rest of the page width is allocated to it which will be
    // the case if widest_text is empty.
    // Notice that column headers may be multiline strings.
    void add_column(std::string const& header, std::string const& widest_text);

    // Render the headers at the given position and update it.
    void output_header(int* pos_y);

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
        (std::size_t begin_column
        ,std::size_t end_column
        ,int         y
        );

    // Output a vertical separator line before the given column. Notice that
    // the column index here may be equal to the number of columns in order to
    // output a separator after the last column.
    void output_vert_separator(std::size_t before_column, int y);

  private:
    // Return the font used for the headers.
    wxFont get_header_font() const;

    int do_get_cell_x(std::size_t column);

    void do_output_horz_separator(int x1, int x2, int y );
    void do_output_vert_separator(int x , int y1, int y2);

    void do_compute_column_widths_if_necessary();

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
    int const row_height_;

    struct column_info
    {
        column_info(std::string const& header, int width)
            :header_(header)
            ,width_(width)
            // Fixed width columns are centered by default, variable width ones
            // are not as long strings look better with the default left
            // alignment.
            ,is_centered_(width != 0)
            {
            }

        // A column with empty header is considered to be suppressed and
        // doesn't appear in the output at all.
        bool is_hidden() const { return header_.empty(); }

        std::string header_;
        int width_;
        bool is_centered_;
    };

    std::vector<column_info> columns_;

    // Initially false, set to true after do_compute_column_widths() call
    // meaning that all column_info::width_ values are now valid.
    bool has_column_widths_;

    // Maximal number of lines in any column header, initially 1 but can be
    // higher if multiline headers are used.
    std::size_t max_header_lines_;
};

#endif // wx_table_generator_hpp
