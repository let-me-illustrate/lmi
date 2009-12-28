// Quiet NaN--unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: quiet_nan_test.cpp,v 1.6 2008-12-27 02:56:53 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "quiet_nan.hpp"

#include "test_tools.hpp"

#include <limits>

int test_main(int, char*[])
{
    if(std::numeric_limits<double>::has_quiet_NaN)
        {
        std::cerr << "has quiet NaN" << std::endl;
        }
    else
        {
        std::cerr << "lacks quiet NaN" << std::endl;
        }

    float       x = implausible_value<float>      ();
    double      y = implausible_value<double>     ();
    long double z = implausible_value<long double>();

    bool xx = x == x;
    BOOST_TEST(!xx);
    bool yy = y == y;
    BOOST_TEST(!yy);
    bool zz = z == z;
    BOOST_TEST(!zz);

    return 0;
}

