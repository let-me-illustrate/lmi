// Shared-library callbacks: unit test.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: callback_test.cpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "callback.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

// The example used here is chosen for its testability alone, and not
// because it would actually be useful. See "Motivation" in the header
// for a more compelling use case.

// INELEGANT !! It would be more appropriate to factor this into a
// shared library and a separate application, but the present unit-
// testing setup doesn't make that as easy as one might wish, so
// for now it's factored into namespaces.

namespace shared_library
{
    typedef double(*adder_fp_type)(int,float,double);

    callback<adder_fp_type> adder_callback;

    void adder_initialize(adder_fp_type f)
    {
        adder_callback.initialize(f);
    }

    class adder
    {
      public:
        adder()
            {
            }

        double operator()(int x,float y,double z) const
            {
            return adder_callback()(x,y,z);
            }
    };
}

namespace application
{
    using namespace shared_library;
    double concrete_adder(int x, float y, double z)
    {
        return x + y + z;
    }

    void initialize_callback()
    {
        adder_initialize(concrete_adder);
    }

} // Unnamed namespace.

int test_main(int, char*[])
{
    application::initialize_callback();
    shared_library::adder()(2, 3.0f, 5.0);

    BOOST_TEST_EQUAL(10.0, shared_library::adder()(2, 3.0f, 5.0));

    return EXIT_SUCCESS;
}

