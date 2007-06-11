// Run a command against a file, respecting its extension--unit test.
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

// $Id: file_command_test.cpp,v 1.1 2007-06-11 22:11:06 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "file_command.hpp"

#include "test_tools.hpp"

int test_main(int, char*[])
{
    // This should throw a std::runtime_error saying it's not
    // yet implemented. At the moment, though, it throws a
    // std::logic_error, which indicates a problem.
    file_command()("", "");

    BOOST_TEST_THROW
        (file_command()("", "")
        ,std::runtime_error
        ,"Class 'file_command' not implemented for command-line interface."
        );

    return 0;
}

