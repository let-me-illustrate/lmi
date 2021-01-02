// Dereference a non-null pointer, optionally downcasting it--unit test.
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

#include "pchfile.hpp"

#include "safely_dereference_as.hpp"

#include "test_tools.hpp"

struct B
{
    virtual ~B() = default;
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
    diagnostic0 = "Cannot dereference null pointer of type '";
    diagnostic0 += lmi::TypeInfo(typeid(D*)).Name();
    diagnostic0 += "'.";

    D* null_pointer = nullptr;
    BOOST_TEST_THROW
        (safely_dereference_as<D>(null_pointer)
        ,std::runtime_error
        ,diagnostic0
        );

    std::string diagnostic1;
    diagnostic1 = "Cannot cast pointer of type '";
    diagnostic1 += lmi::TypeInfo(typeid(B*)).Name();
    diagnostic1 += "' to type '";
    diagnostic1 += lmi::TypeInfo(typeid(D*)).Name();
    diagnostic1 += "'.";

    BOOST_TEST_THROW
        (safely_dereference_as<D>(&b)
        ,std::runtime_error
        ,diagnostic1
        );

    return 0;
}
