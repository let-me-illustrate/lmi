// Print a matrix more or less as APL would.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef print_matrix_hpp
#define print_matrix_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <functional>
#include <iterator>                     // back_inserter()
#include <numeric>
#include <ostream>
#include <string>
#include <vector>

/// Print a matrix more or less as APL would.
///
/// Treat any dimension of length one as though it didn't exist.
///
/// Preserve decimal floating-point precision to the extent possible
/// while suppressing artifacts that would confuse the human reader.
/// For example, show 1.07 as "1.07", not "1.0700000000000001".
///
/// It is anticipated that the result will often be pasted into a
/// spreadsheet. Therefore, formatted data are delimited by tabs
/// (when not by newlines).
///
/// No effort is yet made to align data regularly.
///
/// Motivation: Entities in the lmi product database are essentially
/// seven-dimensional matrices, most axes being of length one. There
/// is no more natural way to print them than the way APL would.
///
/// Illustration of algorithm: Data are printed sequentially (as
/// though "ravelled", as one would say in APL); the only interesting
/// question is where to insert newlines. For a 2 by 3 by 5 matrix,
/// construct a vector of moduli {30, 15, 5} by reversed cumulative
/// multiplication. After the Nth datum (in index origin one), the
/// number of newlines to be written is clearly the inner product of
/// N modulo that vector. I have an expression of this in just a few
/// APL characters, which this character set is too small to contain.
///
/// Future directions: Consider adding labels for each hyperplane.
/// Consider returning a string instead of writing directly to a
/// std::ostream.

template<typename T>
void print_matrix
    (std::ostream&           os
    ,std::vector<T> const&   data
    ,std::vector<int> const& dimensions
    )
{
    std::vector<int> nonempty_dimensions;
    std::remove_copy
        (dimensions.begin()
        ,dimensions.end()
        ,std::back_inserter(nonempty_dimensions)
        ,1
        );

    std::vector<int> moduli(nonempty_dimensions.size());
    std::partial_sum
        (nonempty_dimensions.rbegin()
        ,nonempty_dimensions.rend()
        ,moduli.rbegin()
        ,std::multiplies<int>()
        );
    LMI_ASSERT(1 == lmi::ssize(data) || moduli.front() == lmi::ssize(data));

    for(int j = 0; j < lmi::ssize(data); ++j)
        {
        os << '\t' << value_cast<std::string>(data[j]);
        for(int k = 0; k < lmi::ssize(moduli); ++k)
            {
            if(0 == (1 + j) % moduli[k])
                {
                os << '\n';
                }
            }
        }
}

#endif // print_matrix_hpp
