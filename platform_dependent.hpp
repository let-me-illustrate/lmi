// Platform-dependent functions not in C++98.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: platform_dependent.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef platform_dependent_hpp
#define platform_dependent_hpp

#include "config.hpp"

#if defined LMI_POSIX
#   include <curses.h> // getch()
#   include <stdlib.h> // putenv()
#   include <unistd.h> // access(), R_OK, chdir()
#elif defined LMI_MSW
    // The MinGW port of gcc to msw prototypes what lmi needs in these
    // headers. Other compilers for the msw platform might not.
#   include <conio.h>  // getch()
#   include <direct.h> // chdir()
#   include <io.h>     // access(), R_OK
#   include <stdlib.h> // putenv()
#else // Unknown platform.
    // It seems too fragile to give the prototypes here:
    //   extern "C" int access(char const*, int);
    //   extern "C" int getch();
    //   extern "C" int chdir(char const*);
    //   extern "C" int putenv(char const*);
    // and worse to define R_OK
    //   #define R_OK some_unknowable_value
    // so just complain.
#   error "Unknown platform. Consider contributing support."
#endif // Unknown platform.

#endif // platform_dependent_hpp

