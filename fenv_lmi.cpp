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

// $Id: fenv_lmi.cpp,v 1.2 2005-04-11 23:53:25 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "fenv_lmi.hpp"

#include <sstream>
#include <stdexcept>

#ifdef __STDC_IEC_559__
    // In case the C++ compiler supports C99 7.6 facilities, assume
    // that it sets __STDC_IEC_559__ and puts prototypes in <fenv.h>
    // but not in namespace std.
#   if \
        defined __GNUC__ && __GNUC__ <= 3 \
        defined __GNUC__ && __GNUC__ == 4 && __GNUC_MINOR__ <= 1
        // As of 2005-04-08, the gcc manual here
        // http://gcc.gnu.org/onlinedocs/gcc/Floating-point-implementation.html
        // which "corresponds to GCC version 4.1.0" says "This pragma
        // is not implemented".
#   else // !(defined __GNUC__ && __GNUC__ <= 3)
#       include <fenv.h>
#       pragma STDC FENV_ACCESS ON
#   endif // !(defined __GNUC__ && __GNUC__ <= 3)
#endif // __STDC_IEC_559__

#if defined __BORLANDC__ || defined _MSC_VER
#   include <float.h> // Nonstandard floating-point hardware control.
#endif // defined __BORLANDC__ || defined _MSC_VER

#include <iomanip>

// Implementation note: the ms implementation of _control87(),
// strangely enough, takes an argument that is not identical to the
// hardware control word.
//   http://groups.google.com/groups?selm=34775BB8.E10BA020%40tc.umn.edu
// Instead of writing
//   _control87(0x08001f,  0xffffffff);
// which would be correct for ms but incorrect for borland and perhaps
// for some other compilers, it is better to use these ms macros
//   _MCW_EM _MCW_RC _MCW_PC
// which are not defined by borland or presumably by other compilers
// that use the intel control word as an argument.

void initialize_fpu()
{
#if defined __GNUC__ && defined LMI_X86
    volatile unsigned short int control_word = 0x037f;
    asm volatile("fldcw %0" : : "m" (control_word));
#elif defined __BORLANDC__
    _control87(0x037f, 0xffff);
#elif defined _MSC_VER
    // Test _MSC_VER last because some non-ms compilers or libraries
    // define it.
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

void validate_fenv()
{
    volatile unsigned short int control_word = 0x0;
#if defined __GNUC__ && defined LMI_X86
    asm volatile("fstcw %0" : : "m" (control_word));
    bool okay = 0x037f == control_word;
#elif defined __BORLANDC__
    control_word = static_cast<unsigned short int>(_control87(0, 0));
    bool okay = 0x037f == control_word;
#elif defined _MSC_VER
    control_word = static_cast<unsigned short int>(_control87(0, 0));
    bool okay = (_MCW_EM | _MCW_RC | _MCW_PC) == control_word;
#else // Unknown compiler or platform.
#   error Unknown compiler or platform. Please contribute an implementation.
#endif // Unknown compiler or platform.
    if(!okay)
        {
        std::ostringstream error;
        error
            << "The floating-point control word is unexpectedly "
            << std::hex << control_word
            << ". This is a problem: results may be invalid. "
            << "Probably some other program changed this crucial "
            << "setting."
            ;
        throw error.str();
        }
}

