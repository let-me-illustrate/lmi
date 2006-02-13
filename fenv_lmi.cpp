// Manage floating-point environment.
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

// $Id: fenv_lmi.cpp,v 1.12 2006-02-13 05:30:27 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "fenv_lmi.hpp"

#include "alert.hpp"

#include <iomanip>
#include <sstream>

void fenv_initialize()
{
#ifdef LMI_X86
    x87_control_word(default_x87_control_word());
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.

#if 0
    // The facilities offered by C99's <fenv.h> are useful, but not
    // sufficient: they require no standard facility to set hardware
    // precision, although 7.6/9 provides for extensions like mingw's
    // FE_PC64_ENV. This block shows what could be accomplished in
    // standard C.
#   ifdef __STDC_IEC_559__
    fenv_t save_env;
    feholdexcept(&save_env);
    fesetround(FE_TONEAREST);
#   error Find a platform-specific way to set hardware precision if possible.
#   endif // __STDC_IEC_559__
#endif // 0
}

e_ieee754_precision fenv_precision()
{
#if defined LMI_X86
    return intel_control_word(x87_control_word()).pc();;
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.
}

void fenv_precision(e_ieee754_precision precision_mode)
{
#if defined LMI_X86
    intel_control_word control_word(x87_control_word());
    control_word.pc(precision_mode);
    x87_control_word(control_word.cw());
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.
}

e_ieee754_rounding fenv_rounding()
{
#ifdef __STDC_IEC_559__
    return fegetround(rounding_mode);
#elif defined LMI_X86
    return intel_control_word(x87_control_word()).rc();;
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.
}

// This C99 function call
//   fesetround(precision_mode);
// is nearly equivalent, except that it takes an int argument.
// It is not conditionally used here, because it seems unwise to
// weaken the type safety C++ affords merely to implement this
// one function when its kindred can't be implemented in C99.
//
void fenv_rounding(e_ieee754_rounding rounding_mode)
{
#if defined LMI_X86
    intel_control_word control_word(x87_control_word());
    control_word.rc(rounding_mode);
    x87_control_word(control_word.cw());
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.
}

bool fenv_validate()
{
#ifdef LMI_X86
    bool okay = default_x87_control_word() == x87_control_word();
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.

    if(!okay)
        {
        // Prefer this approach to fatal_error() because this function
        // is intended to be called just before the program closes, at
        // which time it may be unsafe to show messages by ordinary
        // means.
        std::ostringstream oss;
        oss
            << "The floating-point control word was unexpectedly '"
#ifdef LMI_X86
            << std::hex << x87_control_word()
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.
            << "'."
            << "\nProbably some other program changed this crucial setting."
            << "\nIt has been reset correctly. Rerun any illustration that"
            << "\nwas being run when this message appeared, because it may"
            << "\nbe incorrect."
            << "\n"
            ;
        fenv_initialize();
        safely_show_message(oss.str().c_str());
        }
    return okay;
}

