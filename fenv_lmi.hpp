// Manage floating-point environment.
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

// $Id: fenv_lmi.hpp,v 1.2 2005-01-29 02:47:41 chicares Exp $

// Manage the floating-point environment, using C99 7.6 facilities
// where available. Otherwise, use compiler- and platform-specific
// techniques where they are known: only intel hardware is fully
// supported for now.

// This header is named "fenv_lmi.hpp" because it seems likely that
// other libraries or compilers will use the obvious name "fenv.hpp".

#ifndef fenv_lmi_hpp
#define fenv_lmi_hpp

#include "config.hpp"

#ifdef __STDC_IEC_559__
    // In case the C++ compiler supports C99 7.6 facilities, assume
    // that it sets __STDC_IEC_559__ and puts prototypes in <fenv.h>
    // but not in namespace std.
#   pragma STDC FENV_ACCESS ON
#   include <fenv.h>
#endif // __STDC_IEC_559__

#if defined __BORLANDC__ || defined _MSC_VER
#   include <float.h> // Nonstandard floating-point hardware control.
#endif // defined __BORLANDC__ || defined _MSC_VER

inline void initialize_fpu()
{
#if defined __GNUC__ && defined _X86_
    volatile unsigned short int control_word = 0x037f;
    asm volatile("fldcw %0" : : "m" (control_word));
#elif defined __BORLANDC__
    _control87(0x037f, 0xffff);
#elif defined _MSC_VER
    // Test _MSC_VER last because some non-ms compilers or libraries
    // define it.
    //
    // The ms implementation of _control87(), strangely enough, takes
    // an argument that is not identical to the hardware control word.
    //   http://groups.google.com/groups?selm=34775BB8.E10BA020%40tc.umn.edu
    // Instead of writing
    //   _control87(0x08001f,  0xffffffff);
    // which would be correct for ms but incorrect for borland and
    // perhaps for some other compilers, it is better to use these ms
    // macros, which are not defined by borland or presumably by other
    // compilers that use the intel control word as an argument.
    //
    _control87(_MCW_EM,  _MCW_EM);
    _control87(_RC_NEAR, _MCW_RC);
    _control87(_PC_64,   _MCW_PC);
#else // Unknown compiler or platform.
#   error Unknown compiler or platform. Please contribute an implementation.
#endif // Unknown compiler or platform.

// TODO ?? Duplicative and unclear. The code above should be unneeded
// for a platform like MinGW that does everything in a standard way.
// Need a unit test to prove that the code below works correctly.

    // The facilities offered by C99's <fenv.h> are useful, but not
    // sufficient: they require no standard facility to set hardware
    // precision, although 7.6/9 provides for extensions like mingw's
    // FE_PC64_ENV.
#ifdef __STDC_IEC_559__
#   ifndef __MINGW32__
    // Hardware precision not set.
    fenv_t save_env;
    feholdexcept(&save_env);
    fesetround(FE_TONEAREST);
#   else // __MINGW32__
    fesetenv(FE_PC64_ENV);
#   endif // __MINGW32__
#endif // __STDC_IEC_559__
}

#endif // fenv_lmi_hpp

