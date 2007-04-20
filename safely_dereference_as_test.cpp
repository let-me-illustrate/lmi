// Dereference a non-null pointer, optionally downcasting it--unit test.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: safely_dereference_as_test.cpp,v 1.2 2007-04-20 23:41:14 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "safely_dereference_as.hpp"

#include "test_tools.hpp"

struct B
{
    virtual ~B(){}
};

struct D
    :public B
{};

int test_main(int, char*[])
{
    B b;
    D d;

    // Motivating case.
    B* p = &d;
    BOOST_TEST_EQUAL(&d, &safely_dereference_as<D>(p));

    // Type need not be changed.
    BOOST_TEST_EQUAL(&b, &safely_dereference_as<B>(&b));
    BOOST_TEST_EQUAL(&d, &safely_dereference_as<D>(&d));

    // Upcasts are forbidden: this would be a compile-time error.
//    BOOST_TEST_EQUAL( p, &safely_dereference_as<B>(&d));

    std::string diagnostic0;
#if defined __GNUC__
#   if !(LMI_GCC_VERSION < 40000)
    diagnostic0 = "Cannot dereference null pointer of type 'D*'.";
#   else  // LMI_GCC_VERSION < 40000
    diagnostic0 = "Cannot dereference null pointer of type 'P1D'.";
#   endif // LMI_GCC_VERSION < 40000
#endif // defined __GNUC__

    D* null_pointer = 0;
    BOOST_TEST_THROW
        (safely_dereference_as<D>(null_pointer)
        ,std::runtime_error
        ,diagnostic0
        );

    std::string diagnostic1;
#if defined __GNUC__
#   if !(LMI_GCC_VERSION < 40000)
    diagnostic1 = "Cannot cast pointer of type 'B*' to type 'D*'.";
#   else  // LMI_GCC_VERSION < 40000
    diagnostic1 = "Cannot cast pointer of type 'P1B' to type 'P1D'.";
#   endif // LMI_GCC_VERSION < 40000
#endif // defined __GNUC__

    BOOST_TEST_THROW
        (safely_dereference_as<D>(&b)
        ,std::runtime_error
        ,diagnostic1
        );

    return 0;
}

