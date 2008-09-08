// PETE with std::vector: unit test.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

// $Id: et_vector_test.cpp,v 1.4 2008-09-08 12:59:26 chicares Exp $

#include "et_vector.hpp"

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

int main()
{
    std::vector<double> v0;

    v0.push_back(0.0);
    v0.push_back(1.1);
    v0.push_back(2.2);

    v0 *= v0;

    show_vector(v0);

    // Test peteCast().
    std::vector<int> v1(v0.size());
    peteCast(int(0), v0);
    assign(v1, peteCast(int(0), v0));
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
}

