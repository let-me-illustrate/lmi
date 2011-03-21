// Instruct the operating system to execute a command--unit test.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "system_command.hpp"

#include "miscellany.hpp"
#include "test_tools.hpp"

#include <cstdio> // std::remove()
#include <fstream>

int test_main(int, char*[])
{
    std::ofstream os("eraseme", ios_out_trunc_binary());
    os << "abc\n";
    os.close();

    system_command("grep --quiet abc eraseme");

    BOOST_TEST_THROW
        (system_command("grep --quiet xyz eraseme")
        ,std::runtime_error
        ,"Exit code 1 from command 'grep --quiet xyz eraseme'."
        );

    BOOST_TEST_THROW
        (system_command("xyzzy")
        ,std::runtime_error
        ,"Exit code 12345 from command 'xyzzy'."
        );

    std::remove("eraseme");

    return 0;
}

