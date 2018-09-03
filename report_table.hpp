// Platform-independent support for report tables.
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

#ifndef report_table_hpp
#define report_table_hpp

#include "config.hpp"

#include "oecumenic_enumerations.hpp"
#include "so_attributes.hpp"

#include <string>
#include <vector>

/// Elasticity and clipping
///
/// Most columns are inelastic: they have a fixed minimum width and
/// are not clipped lest crucial information (e.g., part of a number)
/// be lost. The archetypal elastic column is a personal name, whose
/// width is practically unlimited and might even exceed the total page
/// width; it is better to truncate one extremely long personal name
/// than to present an error message and produce no report at all.
///
/// An ideal report generator might call GetTextExtent() on every row
/// of data to determine a column's ideal width, but this one favors
/// speed by setting a presumptive maximum width for each column.
/// Therefore, it treats a personal-name column as having no natural
/// width at all. Its minimum width might be set equal to its header
/// width, but such a refinement is needless in the problem domain. In
/// the most extreme case, all inelastic columns would fit, but there
/// would be not a single pixel available for elastic columns, which
/// would all in effect be dropped; again, in the problem domain, that
/// would actually be preferable to failing to produce any output.
///
/// Therefore, elastic columns are clipped, and inelastic ones are not.
/// All other column properties are independent, and specified by
/// arguments, but clipping depends on the elasticity argument. It is
/// distinguished only because clipping is a distinct layout operation.
///
///  - is_elastic(): An elastic column has no innate fixed or preferred
///    width. After all inelastic columns have claimed their required
///    widths, any remaining width available is prorated among elastic
///    columns, which therefore may be wider than their widest contents
///    or narrower than their narrowest. As a consequence, elastic
///    columns are clipped--vide supra.
///
///  - is_clipped(): A clipped column is truncated to fit its allotted
///    space. Only elastic columns are clipped--vide supra.

class LMI_SO table_column_info
{
  public:
    table_column_info
        (std::string      const& header
        ,int                     width
        ,oenum_h_align    const  alignment
        ,oenum_elasticity const  elasticity
        )
        :col_header_ {header}
        ,col_width_  {width}
        ,alignment_  {alignment}
        ,is_elastic_ {oe_elastic == elasticity}
        {
        }

    std::string const& col_header() const {return col_header_;}
    int                col_width()  const {return col_width_;}
    oenum_h_align      alignment()  const {return alignment_;}
    bool               is_elastic() const {return is_elastic_;}
    bool               is_clipped() const {return is_elastic();}

  private:
    std::string   const col_header_;
    int           const col_width_;
    oenum_h_align const alignment_;
    bool          const is_elastic_;
};

std::vector<int> LMI_SO apportion(std::vector<int> const& votes, int seats);

std::vector<int> LMI_SO set_column_widths
    (std::vector<table_column_info> const& all_columns
    ,int                                   max_table_width
    ,int                                   desired_margin
    ,int                                   minimum_margin
    );

/// Display table rows in groups separated by blank lines.
///
/// Nomenclature:
///  - a 'line' is a printable zone of unit height;
///  - a 'row' is a series of data to be shown side by side.
/// With quinquennial spacing, the Morse alphabet is printed thus:
///
///   A   .-     line  0   row  0
///   B   -...   line  1   row  1
///   C   -.-.   line  2   row  2
///   D   -..    line  3   row  3
///   E   .      line  4   row  4
///   [blank]    line  5
///   F   ..-.   line  6   row  5
///   G   --.    line  7   row  6
///   ...
///   Z   --..   line 30   row 25
///
/// with a page length of 50 lines. With a page length of 25 lines,
/// the first page would end with
///   T   -      line 22   row 19
/// and the second page would be printed thus:
///
///   U   ..-    line  0   row 20
///   V   ...-   line  1   row 21
///   W   .--    line  2   row 22
///   X   -..-   line  3   row 23
///   Y   -.--   line  4   row 24
///   [blank]    line  5
///   Z   --..   line  6   row 25

class LMI_SO paginator
{
  public:
    paginator(int total_rows, int rows_per_group, int max_lines_per_page);

    int page_count() const;

  private:
    int const total_rows_;
    int const rows_per_group_;
    int const max_lines_per_page_;

    int page_count_;
};

#endif // report_table_hpp
