// Generate a table using wxDC.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef wx_table_generator_hpp
#define wx_table_generator_hpp

#include "config.hpp"

#include "oecumenic_enumerations.hpp"
#include "report_table.hpp"             // table_column_info

#include <wx/dc.h>
#include <wx/font.h>

#include <string>
#include <vector>

/// Aggregate of per-column table-generator ctor arguments.

struct column_parameters
{
    std::string      const header;
    std::string      const widest_text;
    oenum_h_align    const alignment;
    oenum_elasticity const elasticity;
};

/// Specialized styles for first wx_table_generator ctor argument.
///
/// It would be possible to derive a distinct class for each use case,
/// but style differences are not great enough to warrant that.

class group_quote_style_tag  {};
class illustration_style_tag {};

// Color of rules and borders in illustrations.
wxColor const illustration_rule_color(0x00, 0x2f, 0x6c);

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
  public:
    static int const rows_per_group = 5;

    wx_table_generator
        (group_quote_style_tag
        ,std::vector<column_parameters> const& vc
        ,std::vector<int>               const& indices
        ,wxDC&                                 dc
        ,int                                   left_margin
        ,int                                   total_width
        );

    wx_table_generator
        (illustration_style_tag
        ,std::vector<column_parameters> const& vc
        ,std::vector<int>               const& indices
        ,wxDC&                                 dc
        ,int                                   left_margin
        ,int                                   total_width
        );

    wx_table_generator(wx_table_generator const&);

    ~wx_table_generator();

    void output_headers
        (int&                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_super_header
        (std::string const&           header
        ,int                          a_begin_column
        ,int                          a_end_column
        ,int&                         pos_y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    void output_highlighted_cell(int a_column, int y, std::string const& value);

    void output_row(int& pos_y, std::vector<std::string> const values);

    void output_vert_separator(int a_before_column, int y);
    void output_horz_separator
        (int                          a_begin_column
        ,int                          a_end_column
        ,int                          y
        ,oenum_render_or_only_measure output_mode = oe_render
        );

    int row_height() const;
    int separator_line_height() const;

    // Used only by group_quote_pdf_generator_wx::output_aggregate_values(),
    // in a context where something like output_highlighted_cell() should
    // probably be used instead. PDF !! revisit this later
    wxRect external_text_rect(int a_column, int y) const;

  private:
    void enroll_column(column_parameters const&);

    void do_output_single_row
        (int&                            pos_x
        ,int&                            pos_y
        ,std::vector<std::string> const& values
        );

    void do_output_vert_separator(int x , int y1, int y2);
    void do_output_horz_separator(int x1, int x2, int y );

    int cell_pos_x(int column) const;

    wxRect text_rect(int column, int y) const;
    wxRect cell_rect(int column, int y) const;
    wxRect cell_rect(int begin_column, int end_column, int y) const;

    wxFont header_font() const;

    // Const private accessors. Used in the implementation to
    // distinguish access from mutation.
    wxDC const& dc() const;
    std::vector<table_column_info> const& all_columns() const;

    std::vector<int> const indices_;

    wxDC& dc_;

    int const left_margin_;
    int const total_width_;

    // These values could be recomputed, but cache them for performance.
    int const char_height_;
    int const row_height_;
    int const one_em_;

    std::vector<table_column_info> all_columns_;

    // Maximal number of lines in any column header, initially 1 but can be
    // higher if multiline headers are used.
    int max_header_lines_;

    // If false, separator lines are not drawn automatically (they can still be
    // drawn by calling output_horz_separator() or output_vert_separator()
    // explicitly).
    bool const draw_separators_;

    // If true, headers are drawn in bold.
    bool const use_bold_headers_;
};

#endif // wx_table_generator_hpp
