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

// Rule of Zero: all special members are defaulted.

class B0 : private lmi::abstract_base<B0> {};
class B1 : private lmi::abstract_base<B1> {};
class C : private B0, private B1 {};

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#endif // defined __GNUC__
class D final : private C
{
  private:
    void concrete_if_not_pure() override {}
};
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__

/// Test abstract-xor-final hierarchy.

void test_abstract_or_final()
{
    static_assert( std::is_abstract_v <B0>);
    static_assert( std::is_abstract_v <B1>);
    static_assert( std::is_abstract_v <C>); // abstract by inheritance
    static_assert(!std::is_abstract_v <D>);

    static_assert(!std::is_final_v    <B0>);
    static_assert(!std::is_final_v    <B1>);
    static_assert(!std::is_final_v    <C>);
    static_assert( std::is_final_v    <D>);

    static_assert(!std::is_default_constructible_v <B0>);
    static_assert(!std::is_default_constructible_v <B1>);

    static_assert(!std::is_default_constructible_v <C>);
    static_assert( std::is_destructible_v          <C>);
    static_assert(!std::is_copy_constructible_v    <C>);
    static_assert(!std::is_move_constructible_v    <C>);
    static_assert( std::is_copy_assignable_v       <C>);
    static_assert( std::is_move_assignable_v       <C>);

    static_assert( std::is_default_constructible_v <D>);
    static_assert( std::is_destructible_v          <D>);
    static_assert( std::is_copy_constructible_v    <D>);
    static_assert( std::is_move_constructible_v    <D>);
    static_assert( std::is_copy_assignable_v       <D>);
    static_assert( std::is_move_assignable_v       <D>);

    D da;                 // D()
    D db(da);             // D(D const&)
    db = da;              // D& operator=(D const&)
    D dd {std::move(da)}; // D(D&&)
    db = std::move(da);   // D& operator=(D&&)
}

int test_main(int, char*[])
{
    test_uncopyable();
    test_abstract_or_final();
    return 0;
}
