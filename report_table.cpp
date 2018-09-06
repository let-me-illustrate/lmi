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

#include "pchfile.hpp"

#include "report_table.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "math_functions.hpp"           // outward_quotient()
#include "ssize_lmi.hpp"

#include <algorithm>                    // min()
#include <numeric>                      // accumulate()
#include <queue>
#include <utility>                      // pair

/// Apportion "seats" to "states" by their respective total "votes".
///
/// This algorithm is popularly associated with Alexander Hamilton,
/// who wrote: "as there would commonly be left ... an unapportioned
/// residue of the total number to be apportioned, it is of necessity
/// that that residue should be distributed among the several States
/// by some rule, and none more equal or defensible can be found than
/// that of giving a preference to the greatest remainders".
///
/// A fascinating geometric analysis is to be found in B.A. Bradberry,
/// "A Geometric View of Some Apportionment Paradoxes", 65 Mathematics
/// Magazine 1, 16 (1992).
///
/// If two elements of the 'votes' argument have the same value, then
/// any "residue" is arbitrarily apportioned to the earlier one first.
/// (Without such a rule, the result is indeterminate.) The present
/// implementation uses a priority queue, which is based on heapsort,
/// which is not a stable (order-preserving) algorithm--so, where the
/// priority of element j would naively be 'remainder' below ("giving
/// preference to the greatest remainders"), it subtracts a penalty
/// that increases with each successive element, yet is never so large
/// as to cross equivalence classes: notionally,
///   adjusted priority = remainder - double(j / cardinality)
/// (where the fraction is strictly less than unity, so the integer
/// part of the adjusted priority is still 'remainder'), although
/// actually it multiplies that expression by 'cardinality' in order
/// to avoid floating point.
///
/// Asserted postcondition: All seats are apportioned--i.e., the sum
/// of the returned vector equals the 'total_seats' argument--unless
/// the sum of the 'votes' argument is zero, in which case zero seats
/// are allocated.

std::vector<int> apportion(std::vector<int> const& votes, int total_seats)
{
    int const cardinality = lmi::ssize(votes);
    std::vector<int> seats(cardinality);
    int const total_votes = std::accumulate(votes.begin(), votes.end(), 0);
    if(0 == total_votes) return seats; // Avert division by zero.
    std::priority_queue<std::pair<int,int>> queue;
    for(int j = 0; j < cardinality; ++j)
        {
        seats[j]            = (votes[j] * total_seats) / total_votes;
        int const remainder = (votes[j] * total_seats) % total_votes;
        queue.push({cardinality * remainder - j, j});
        }
    int const dealt_seats = std::accumulate(seats.begin(), seats.end(), 0);
    for(int j = 0; j < total_seats - dealt_seats; ++j)
        {
        ++seats[queue.top().second];
        queue.pop();
        }
    LMI_ASSERT(std::accumulate(seats.begin(), seats.end(), 0) == total_seats);
    return seats;
}

/// Compute column widths.
///
/// First, allocate adequate width to each inelastic column; then
/// distribute any excess width left over among elastic columns.
///
/// Notes on arguments:
///   all_columns: the width of each inelastic column reflects:
///    - the header width, and
///    - a mask like "999,999" (ideally, there would instead be a
///      quasi-global data structure mapping symbolic column names
///      to their corresponding headers and maximal widths)
///   max_table_width: page width - page margins
///   desired_margin: maximum margin for each inelastic column
///   minimum_margin: minimum margin for every column

std::vector<int> set_column_widths
    (std::vector<table_column_info> const& all_columns
    ,int                                   max_table_width
    ,int                                   desired_margin
    ,int                                   minimum_margin
    )
{
    LMI_ASSERT(minimum_margin <= desired_margin);
    int const cardinality = lmi::ssize(all_columns);
    int data_width = 0;
    int n_columns_to_show = 0;
    for(int j = 0, cum_min_width = 0; j < cardinality; ++j)
        {
        cum_min_width += all_columns[j].col_width() + minimum_margin;
        if(cum_min_width <= max_table_width)
            {
            data_width += all_columns[j].col_width();
            ++n_columns_to_show;
            }
        else break;
        }

    if(0 == n_columns_to_show)
        {
        alarum() << "Not enough room for even the first column." << LMI_FLUSH;
        }

    // These two are boolean, but vector<bool> isn't a Container.
    std::vector<int> bool_inelastic(n_columns_to_show, false);
    std::vector<int> bool_elastic  (n_columns_to_show, false);
    for(int j = 0; j < n_columns_to_show; ++j)
        {
        if(all_columns[j].is_elastic()) {bool_elastic  [j] = true;}
        else                            {bool_inelastic[j] = true;}
        }

    int const residue = max_table_width - data_width;
    LMI_ASSERT(0 <= residue);

    // Apportion any residue among inelastic columns, up to the number
    // of such columns times the desired_margin argument.
    int const n_inelastic = std::accumulate(bool_inelastic.begin(), bool_inelastic.end(), 0);
    int const residue_inelastic = std::min(residue, n_inelastic * desired_margin);
    LMI_ASSERT(0 <= residue_inelastic);
    std::vector<int> const delta_inelastic = apportion(bool_inelastic, residue_inelastic);
    // That part of the residue should always be fully consumed.
    LMI_ASSERT(residue_inelastic ==  std::accumulate(delta_inelastic.begin(), delta_inelastic.end(), 0));

    // Apportion all remaining residue, if any, among elastic columns.
    int const residue_elastic = residue - residue_inelastic;
    LMI_ASSERT(0 <= residue_elastic);
    std::vector<int> const delta_elastic   = apportion(bool_elastic, residue_elastic);

    std::vector<int> w(cardinality);
    for(int j = 0; j < n_columns_to_show; ++j)
        {
        w[j] = all_columns[j].col_width() + delta_inelastic[j] + delta_elastic[j];
        }

    if(cardinality != n_columns_to_show)
        {
        warning()
            << "Printing only the first " << n_columns_to_show
            << " columns: not enough room for all " << cardinality << "."
            << std::flush
            ;
        }

    return w;
}

/// Preconditions: 0 <= total_rows && 0 < rows_per_group <= max_lines_per_page

paginator::paginator(int total_rows, int rows_per_group, int max_lines_per_page)
    :total_rows_         {total_rows}
    ,rows_per_group_     {rows_per_group}
    ,max_lines_per_page_ {max_lines_per_page}
    // "+ 1": blank-line separator after each group.
    ,lines_per_group_    {rows_per_group_ + 1}
    // "+ 1": no blank-line separator after the last group.
    ,groups_per_page_    {(max_lines_per_page_ + 1) / lines_per_group_}
    ,rows_per_page_      {rows_per_group_ * groups_per_page_}
    ,page_count_         {1}
{
    LMI_ASSERT(0 <= total_rows_);
    LMI_ASSERT(0 <  rows_per_group_                       );
    LMI_ASSERT(     rows_per_group_ <= max_lines_per_page_);

    // If there are zero rows of data, then one empty page is wanted.
    if(0 == total_rows_)
        {
        page_count_ = 1;
        return;
        }

    page_count_ = outward_quotient(total_rows_, rows_per_page_);

    // Avoid widowing a partial group on the last page, by moving it
    // to the preceding page if there's room.
    if(1 < page_count_)
        {
        auto const rows_on_last_page = total_rows_ - (page_count_ - 1) * rows_per_page_;
        auto const free_lines = max_lines_per_page_ - lines_per_group_ * groups_per_page_;
        LMI_ASSERT(free_lines < rows_per_group_);
        if(rows_on_last_page <= free_lines)
            {
            --page_count_;
            }
        }
}
