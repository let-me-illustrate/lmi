// Exception-throwing macro for lightweight assertions--unit test.
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

// $Id: assert_lmi_test.cpp,v 1.2 2007-01-27 00:17:12 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "assert_lmi.hpp"

#include "test_tools.hpp"

int test_main(int, char*[])
{
    ASSERT_LMI(true);

    BOOST_TEST_THROW
        (ASSERT_LMI(false)
        ,std::runtime_error
        ,"Assertion 'false' failed."
        );

    return 0;
}

