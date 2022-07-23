// Convenient base classes that use CRTP--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2022 Gregory W. Chicares.
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

#include "crtp_base.hpp"

#include "test_tools.hpp"

#include <type_traits>

class X : private lmi::uncopyable<X> {};

/// Test uncopyability (and unmovability) of lmi::uncopyable derivatives.
///
/// Unlike most lmi unit tests, the conditions tested are all
/// ascertainable at compile time.

void test_uncopyable()
{
    static_assert( std::is_default_constructible_v <X>);
    static_assert( std::is_destructible_v          <X>);
    static_assert(!std::is_copy_constructible_v    <X>);
    static_assert(!std::is_move_constructible_v    <X>);
    static_assert(!std::is_copy_assignable_v       <X>);
    static_assert(!std::is_move_assignable_v       <X>);
}

int test_main(int, char*[])
{
    test_uncopyable();
    return 0;
}
