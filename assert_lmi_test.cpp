// Exception-throwing macro for lightweight assertions--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "assert_lmi.hpp"

#include "test_tools.hpp"

#define SINGLY_STRINGIFY(Z) #Z
#define DOUBLY_STRINGIFY(Z) SINGLY_STRINGIFY(Z)

#define LMI_LOCATION \
    "\n[file " __FILE__ ", line " DOUBLY_STRINGIFY(__LINE__) "]\n"

int test_main(int, char*[])
{
    LMI_ASSERT(true);

    bool const volatile not_true = false;
    BOOST_TEST_THROW
        (LMI_ASSERT(not_true)
        ,std::runtime_error
        ,"Assertion 'not_true' failed."
        );

    // The LMI_ASSERT macro is intended to "swallow the semicolon":
    //   https://gcc.gnu.org/onlinedocs/cpp/Swallowing-the-Semicolon.html
    if(not_true)
        LMI_ASSERT(not_true);
    else
        LMI_ASSERT(!not_true);

    // This use-case demonstrates why LMI_ASSERT_WITH_MSG's second
    // parameter is not token-pasted. This is a deliberate tradeoff,
    // with the consequence that this:
    //   LMI_ASSERT_WITH_MSG(1&1,1&1);
    // fails to compile.
    BOOST_TEST_THROW
        (LMI_ASSERT_WITH_MSG(not_true,"<" << not_true << ">")
        ,std::runtime_error
        ,"Assertion 'not_true' failed\n(<0>)." LMI_LOCATION
        );

    BOOST_TEST_THROW
        (LMI_ASSERT_EQUAL(not_true,true)
        ,std::runtime_error
        ,"Assertion '(not_true) == (true)' failed\n(expected 1 vs observed 0)." LMI_LOCATION
        );

    // It does seem wrong that this fails to compile:
    LMI_ASSERT_EQUAL(1&1,1&1);

    return 0;
}

