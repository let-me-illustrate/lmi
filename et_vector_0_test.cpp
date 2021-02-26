// PETE with std::vector: unit test.
//
// Copyright (C) 2008, 2010, 2016, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "et_vector.hpp"

#include "test_tools.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>

template <typename T>
void show_vector(std::vector<T> const& v)
{
    std::ostream_iterator<double> osi(std::cout, " ");
    std::copy(v.begin(), v.end(), osi);
    std::cout << std::endl;
}

int test_main(int, char*[])
{
    std::vector<double> v0;

    v0.push_back(0.0);
    v0.push_back(1.1);
    v0.push_back(2.2);

    v0 *= v0;

    show_vector(v0);

    // Test peteCast().
    std::vector<int> v1(v0.size());
    peteCast(int{}, v0);
    assign(v1, peteCast(int{}, v0));
    show_vector(v1);

    // Test std::unary_function.
    assign(v0, apply_unary(std::negate<double>(), v0));
    show_vector(v0);

    // Test std::binary_function.
    assign(v0, apply_binary(std::multiplies<double>(), -1.0, v0));
    show_vector(v0);
    assign(v0, sqrt(v0));
    show_vector(v0);
    assign(v0, apply_binary(std::multiplies<double>(), v0, -1.0));
    show_vector(v0);
    assign(v0, apply_binary(std::multiplies<double>(), v0, v0));
    show_vector(v0);
    assign(v0, apply_binary(std::plus<double>(), v0, 100.0));
    assign(v0, apply_binary(std::plus<double>(), 10000.0, v0));
    show_vector(v0);

    // Test Min() and Max().
    std::vector<double> v2 = {1.2, 2.3, 3.4, 7.7};
    std::vector<double> v3 = {1.9, 2.9, 3.9, 0.0};
    std::vector<double> v4(v2.size());
    assign(v4, Max(v2, v3));
    show_vector(v4);
    assign(v4, Min(v2, v3));
    show_vector(v4);

    std::cout << "Completed." << std::endl;

    return 0;
}
