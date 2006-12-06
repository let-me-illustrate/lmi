// Get startup path from argv[0] if available: unit test.
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

// $Id: argv0_test.cpp,v 1.7 2006-12-06 16:23:17 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "argv0.hpp"

#include "test_tools.hpp"

int test_main(int, char* argv[])
{
    // Initialize default name-checking policy to 'native'--see:
    //   http://lists.boost.org/MailArchives/boost/msg47862.php
    // This shouldn't be necessary, but is for MSYS's bash, which
    // translates argv[0] to something like 'c:\foo\bar'. This
    // initialization may fail if this test program is linked with a
    // boost dll--see the documentation for initialize_filesystem().
    //
    fs::path::default_name_check(fs::native);

    fs::startup_path(argv[0]);

    fs::initial_path();

    BOOST_TEST_EQUAL(fs::startup_path().string(), fs::initial_path().string());

    return EXIT_SUCCESS;
}

