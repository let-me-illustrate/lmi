// Get startup path from argv[0] if available: unit test.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: argv0_test.cpp,v 1.1 2005-03-11 03:19:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "argv0.hpp"

#include <iostream>
#include <ostream>

int main(int, char* argv[])
{
// TODO ?? In this message
//   http://lists.boost.org/MailArchives/boost/msg47862.php
// the author of the boost filesystem library recommends
//   fs::path path(argv[0], fs::native);
// instead--why do otherwise here?

#ifndef __WXMSW__
    fs::startup_path(argv[0]);
#else // __WXMSW__ defined.
    fs::startup_path(__argv[0]);
#endif // __WXMSW__ defined.

    fs::initial_path();

// TODO ?? Test these paths against expected values instead of just
// writing them to stdout.

    std::cout << "Paths:\n";

    std::cout << fs::initial_path().string() << " = initial_path\n";

    std::cout << fs::startup_path().string() << " = startup_path\n";

    fs::path path0(fs::startup_path());

    std::cout << path0.string() << std::endl;
    std::cout << path0.native_directory_string() << std::endl;

    std::cout << "Done.\n";
}

