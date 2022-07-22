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

void test_uncopyable()
{
// If lmi provided unit tests that deliberately fail to compile, then
// this could be used:
//
// #include "uncopyable_lmi.hpp"
//
// class X : private lmi::uncopyable<X> {};
//
// int main()
// {
//     X x;
//     X y(x); // Error: cannot copy.
//     x = y;  // Error: cannot assign.
// }
}

int test_main(int, char*[])
{
    test_uncopyable();
    return 0;
}
