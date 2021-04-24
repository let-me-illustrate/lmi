// Functions not in C++98 but available in a platform-dependent way.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef platform_dependent_hpp
#define platform_dependent_hpp

#include "config.hpp"

#if defined LMI_POSIX
#   include <stdio.h>                   // fileno()
#   include <stdlib.h>                  // putenv()
#   include <string.h>                  // strdup()
#   include <strings.h>                 // strcasecmp()
#   include <unistd.h>                  // access(), R_OK, chdir()
#elif defined LMI_MSW
    // The MinGW port of gcc to msw prototypes what lmi needs in these
    // headers. Other compilers for the msw platform might not.
#   include <direct.h>                  // chdir()
#   include <io.h>                      // access(), R_OK
#   include <stdio.h>                   // fileno()
#   include <stdlib.h>                  // putenv()
#   include <string.h>                  // _wcsdup(), strcasecmp(), strdup()
#else // Unknown platform.
    // It seems too fragile to give the prototypes here:
    //   extern "C" int access(char const*, int);
    //   extern "C" int chdir(char const*);
    // especially when they may vary by platform:
    //   extern "C" int putenv(char*);       // [posix]
    //   extern "C" int putenv(char const*); // [msw]
    // and worse to define R_OK:
    //   #define R_OK some_unknowable_value
    // so just complain.
#   error Unknown platform. Consider contributing support.
#endif // Unknown platform.

// Formerly, this function
//   extern "C" int getch();
// was provided for above, thus:
//   #include <curses.h> // getch()
// for posix, and similarly for msw. However, the 'curses' header
// pollutes the global namespace intolerably with ill-named macros:
//   https://lists.nongnu.org/archive/html/lmi/2008-06/msg00045.html
// and therefore a prototype for getch() is instead provided by other
// means, locally, wherever it's needed. As of 2017-04, getch() is no
// longer used; this paragraph is kept lest it be reintroduced.

#endif // platform_dependent_hpp
