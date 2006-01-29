// Overloaded operator new--unit test.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: wx_new_test.cpp,v 1.4 2006-01-29 13:52:01 chicares Exp $

// This unit test proves little, but including it in the unit-test
// suite ensures that it'll be compiled with stronger warning options
// than wx would permit.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

// The '.cpp' file is deliberately included here instead of the header
// because it was probably already compiled for inclusion in a dll,
// resulting in an object that wouldn't necessarily work here.
#include "wx_new.cpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

int test_main(int, char*[])
{
    int* p0 = new int;
    delete p0;

    int* p1 = new(wx) int;
    delete p1;

    return 0;
}

