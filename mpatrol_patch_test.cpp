// Patch for mpatrol-1.4.8: unit test.
//
// Copyright (C) 2003, 2005, 2006 Gregory W. Chicares.
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

// $Id: mpatrol_patch_test.cpp,v 1.3 2006-12-06 16:23:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "test_tools.hpp"

#include <cstdlib>
#include <cstring>

// References:
//   http://groups.yahoo.com/group/mpatrol/message/796
//   http://sf.net/tracker/index.php?func=detail&aid=1112376&group_id=19456&atid=319456

void test_zero_byte_moves()
{
    void* a = std::malloc(1);
    void* b = std::malloc(1);
    std::free(a);
    std::free(b);
    volatile std::size_t zero(0);
    std::memcpy (a, b, zero);
    std::memmove(a, b, zero);
}

void test_failure_0()
{
    char* p = (char*) std::malloc(16);
    std::memset(p - 1, 0, 18);
    std::free(p);
    p[8] = '\0';
}

int test_main(int, char*[])
{
    test_zero_byte_moves();
    // Compiling with this test enabled demonstrates whether mpatrol
    // is working.
//    test_failure_0();

    return 0;
}

