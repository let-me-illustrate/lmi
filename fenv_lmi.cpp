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

// $Id: fenv_lmi.cpp,v 1.9 2005-12-27 16:52:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "fenv_lmi.hpp"

#include "alert.hpp"

#include <iomanip>
#include <sstream>

#if defined __BORLANDC__ || defined _MSC_VER
#   include <float.h> // Nonstandard floating-point hardware control.
#endif // defined __BORLANDC__ || defined _MSC_VER

void initialize_fpu()
{
#ifdef __MINGW32__
    fesetenv(FE_PC64_ENV);
#elif defined __GNUC__ && defined LMI_X86
    volatile unsigned short int control_word = 0x037f;
    asm volatile("fldcw %0" : : "m" (control_word));
#elif defined __BORLANDC__
    _control87(0x037f, 0xffff);
#elif defined _MSC_VER
    // Test _MSC_VER last: some non-ms compilers or libraries define it.
    _control87(intel_to_msw(0x037f),  0x0ffffffff);
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform. Please contribute an implementation.
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

bool validate_fenv()
{
    volatile unsigned short int control_word = 0x0;
#if defined __GNUC__ && defined LMI_X86
    asm volatile("fstcw %0" : : "m" (control_word));
#elif defined __BORLANDC__
    control_word = static_cast<unsigned short int>(_control87(0, 0));
#elif defined _MSC_VER
    // Test _MSC_VER last: some non-ms compilers or libraries define it.
    control_word = msw_to_intel(_control87(0, 0));
#else // Unknown compiler or platform.
#   error Unknown compiler or platform. Please contribute an implementation.
#endif // Unknown compiler or platform.
    bool okay = 0x037f == control_word;
    if(!okay)
        {
        // Prefer this approach to fatal_error() because this function
        // is intended to be called just before the program closes, at
        // which time it may be unsafe to show messages by ordinary
        // means.
        std::ostringstream oss;
        oss
            << "The floating-point control word is unexpectedly '"
            << std::hex << control_word << "'.\n"
            << "Probably some other program changed this crucial setting.\n"
            << "This is a real problem: results may be invalid.\n"
            ;
        safely_show_message(oss.str().c_str());
        }
    return okay;
}

