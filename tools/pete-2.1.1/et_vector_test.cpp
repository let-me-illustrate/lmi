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

// $Id: et_vector_test.cpp,v 1.2 2008-09-06 00:22:34 chicares Exp $

#include "et_vector.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>

int main()
{
    std::vector<double> v0;

    v0.push_back(0.0);
    v0.push_back(1.1);
    v0.push_back(2.2);

    v0 *= v0;

    std::ostream_iterator<double> osi(std::cout, "\n");
    std::copy(v0.begin(), v0.end(), osi);

    // Demonstrate a peteCast() anomaly.
    std::vector<int> v1(v0.size());
    // This succeeds:
    peteCast(int(0), v0);
    // Cannot convert UnaryReturn<double, OpCast<int> > to int:
    assign(v1, peteCast(int(0), v0));
    std::copy(v1.begin(), v1.end(), osi);
}

