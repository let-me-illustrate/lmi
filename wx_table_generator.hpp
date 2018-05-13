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

/// Aggregate of per-column table-generator ctor arguments.

struct column_parameters
{
    std::string header;
    std::string widest_text;
};

/// Simplifies outputting tabular data on wxDC.
///
/// Several member functions reuse the same pos_y argument (which is
/// initially the coordinate of the top of the header or row to output)
/// and update it to designate the next row to be written.
///
/// The life time of the specified wxDC must be greater than the life time
/// of this object itself and nothing should be using it while this object
/// does (as it changes its attributes).

class wx_table_generator
{
    class column_info;

  public:
    wx_table_generator
        (std::vector<column_parameters> const& vc
        ,wxDC&                                 dc
        ,int                                   left_margin
        ,int                                   total_width
        );

    wx_table_generator(wx_table_generator const&);

    ~wx_table_generator();

    void output_header
        (int*                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_super_header
        (std::string const&           header
        ,std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int*                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_row(int* pos_y, std::vector<std::string> const values);

    void output_highlighted_cell
        (std::size_t        column
        ,int                y
        ,std::string const& value
        );

    int row_height() const;

    wxRect cell_rect(std::size_t column, int y);
    wxRect text_rect(std::size_t column, int y);

    void output_horz_separator
        (std::size_t                  begin_column
        ,std::size_t                  end_column
        ,int                          y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_vert_separator(std::size_t before_column, int y);

    void use_condensed_style();
    void align_right();

  private:
    void enroll_column(std::string const& header, std::string const& widest_text);
    void compute_column_widths();

    wxFont get_header_font() const;

    int do_get_cell_x(std::size_t column);

    void do_output_horz_separator(int x1, int x2, int y );
    void do_output_vert_separator(int x , int y1, int y2);

    void do_output_single_row
        (int&                            pos_x
        ,int&                            pos_y
        ,std::vector<std::string> const& values
        );

    // Const private accessors. Used in the implementation to
    // distinguish access from mutation.
    int column_margin() const;
    std::vector<column_info> const& all_columns() const;

    wxDC& dc_;

    int left_margin_;
    int total_width_;

    // These values could be recomputed, but cache them for performance.
    int const char_height_;
    int row_height_;
    int column_margin_;

    std::vector<column_info> all_columns_;

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

#endif // wx_table_generator_hpp
