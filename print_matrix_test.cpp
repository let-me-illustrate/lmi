// Print a matrix more or less as APL would--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "print_matrix.hpp"

#include "cso_table.hpp"
#include "test_tools.hpp"

namespace
{
/// 1980 CSO Ultimate ANB Male Unismoke

std::vector<double> const& sample_q()
{
    static std::vector<double> const q = cso_table
        (mce_1980cso
        ,oe_orthodox
        ,oe_age_nearest_birthday_ties_younger
        ,mce_male
        ,mce_unismoke
        );
    return q;
}
} // Unnamed namespace.

// TODO ?? Test the output instead of just displaying it.
int test_main(int, char*[])
{
    {
    std::vector<double> const& q(sample_q());
    std::vector<int> dimensions {10, 1, 1, 2, 5};
    print_matrix(std::cout, q, dimensions);
    std::cout << std::endl;
    }

    {
    std::vector<double> q(1, 2.718281828459045);
    std::vector<int> dimensions {1, 1, 1, 1};
    print_matrix(std::cout, q, dimensions);
    std::cout << std::endl;
    }

    {
    std::vector<std::string> q(1, "hello");
    std::vector<int> dimensions;
    print_matrix(std::cout, q, dimensions);
    std::cout << std::endl;
    }

    return 0;
}
