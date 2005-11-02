// Functions not in C++98 but available in a platform-dependent way.
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

// $Id: platform_dependent.hpp,v 1.7 2005-11-02 17:23:28 chicares Exp $

#ifndef platform_dependent_hpp
#define platform_dependent_hpp

#include "config.hpp"

// When compiling with '-ansi' or certain '-std=' options, gcc defines
// macro __STRICT_ANSI__. Standard headers like <stdlib.h> may provide
// prototypes for nonstandard functions like putenv() only if that
// macro is not defined, in order to conform to C99 4/7. [Consider
// the following strictly-conforming program (C99 4/6):
//
//   #include <stdlib.h>
//   void foo(void);
//   int main() {foo(); return 0;}
//
// and replace 'foo' with 'putenv', which latter name the language
// standard clearly does not reserve.]
//
// However, ISO 9945:2002 (POSIX) declares putenv() in <stdlib.h>. In
// light of the present header's raison d'�tre, it is sensible to
// suspend the operation of the __STRICT_ANSI__ mechanism, taking care
// to restore it after including other headers here.
//
// Some of the functions made accessible here provide capabilities
// that are useful but absent from the standard language: for example,
// it is difficult to implement cgi-bin without putenv(). Some others,
// like strdup(), should be avoided in general, but are required by
// wx. Still others, like expm1(), are in C99 but not C++98, and the
// way their prototypes are guarded varies by platform.

#if defined __GNUC__ && defined __STRICT_ANSI__
#   define LMI_GNUC_STRICT_ANSI
#   undef __STRICT_ANSI__
#endif // defined __GNUC__ && defined __STRICT_ANSI__

#if defined LMI_POSIX
#   include <curses.h>  // getch()
#   include <stdio.h>   // fileno()
#   include <stdlib.h>  // putenv()
#   include <string.h>  // strdup()
#   include <strings.h> // strcasecmp()
#   include <unistd.h>  // access(), R_OK, chdir()
#elif defined LMI_MSW
    // The MinGW port of gcc to msw prototypes what lmi needs in these
    // headers. Other compilers for the msw platform might not.
#   include <conio.h>   // getch()
#   include <direct.h>  // chdir()
#   include <io.h>      // access(), R_OK
#   include <stdio.h>   // fileno()
#   include <stdlib.h>  // putenv()
#   include <string.h>  // strdup()
#   ifdef __BORLANDC__
#       define R_OK 04
#   endif // __BORLANDC__
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

// GNU/Linux (but not MinGW) requires including certain headers while
// __STRICT_ANSI__ is not defined in order to get prototypes for
// certain functions, for C++ with '-std=c++98':
//   math.h:  expm1() and log1p()
// Use the C instead of the C++ system header so that the present file
// can be included in C as well as C++ translation units, which is
// temporarily useful until 'expm1.c' can be removed.

#include <math.h>

// Although gcc may once have defined __STRICT_ANSI__ differently:
//   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=3199
// its intended value now is 1:
//   http://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html

#if defined LMI_GNUC_STRICT_ANSI
#   define __STRICT_ANSI__ 1
#endif // defined LMI_GNUC_STRICT_ANSI

#undef LMI_GNUC_STRICT_ANSI

#endif // platform_dependent_hpp

