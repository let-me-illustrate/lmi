// Manage floating-point environment.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef fenv_lmi_hpp
#define fenv_lmi_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#if defined LMI_X86
#   include "fenv_lmi_x86.hpp"
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform.
#endif // Unknown compiler or platform.

// SOMEDAY !! Revisit suppressed __STDC_IEC_559__ support. See:
//   http://lists.nongnu.org/archive/html/lmi/2008-06/msg00033.html
#if defined LMI_IEC_559
// In case the C++ compiler supports C99 7.6 facilities, assume that
// it defines __STDC_IEC_559__ (except that MinGW supports some such
// facilities but defines no such macro), and puts prototypes in
// <fenv.h> but not in namespace std.
#   include <fenv.h>
#   if defined __GNUC__ && LMI_GCC_VERSION <= 40300
// As of 2007-07-05, the gcc manual here:
//   http://gcc.gnu.org/onlinedocs/gcc/Floating-point-implementation.html
// which "corresponds to GCC version 4.3.0" says "This pragma is not
// implemented".
#   else  // Pragma STDC FENV_ACCESS implemented.
#       pragma STDC FENV_ACCESS ON
#   endif // Pragma STDC FENV_ACCESS implemented.
#endif // defined LMI_IEC_559

/// These functions manage the floating-point environment.
///
///   void fenv_initialize();
/// Initialize floating-point environment to lmi default settings.
///
///   e_ieee754_rounding fenv_rounding();
///   void fenv_rounding(e_ieee754_rounding);
/// The rounding-mode functions are similar to C99 fe[gs]etround().
/// Because this is C++, not C, no 'get-' and 'set-' lexemes are
/// needed to simulate overloading, and exceptions obviate return
/// codes (although it's not clear how such a function could ever fail
/// with a valid argument, and invalid arguments can be disallowed by
/// C++'s type system).
///
///   e_ieee754_precision fenv_precision();
///   void fenv_precision(e_ieee754_precision);
/// The precision functions similarly resemble GNU/Linux functions
/// fe[gs]etprecision().
///
///   bool LMI_SO fenv_is_valid()
/// If current floating-point environment matches lmi default, then
/// return 'true'; else return 'false'.
///
///   bool LMI_SO fenv_validate();
/// Make sure current floating-point environment matches lmi default.
/// If it doesn't, then reset it to lmi default, display a message,
/// and return 'false'; else return 'true'.
///
/// Postcondition: floating-point environment matches lmi default.
///
/// Throws: nothing, unless displaying the error message fails, e.g.
/// due to lack of memory. Failure is indicated by a return code
/// rather than an exception, so that this function can be called at
/// program exit, outside any try block, without abending, which is
/// crucial for GUI frameworks that don't handle exceptions
/// gracefully during normal termination.

namespace floating_point_environment {} // doxygen workaround.

enum enum_fenv_indulgence
    {e_fenv_indulge_nothing = 0
    ,e_fenv_indulge_0x027f  = 0x027f
    };

void LMI_SO fenv_initialize();

e_ieee754_precision LMI_SO fenv_precision();
void                LMI_SO fenv_precision(e_ieee754_precision);

e_ieee754_rounding LMI_SO fenv_rounding();
void               LMI_SO fenv_rounding(e_ieee754_rounding);

bool LMI_SO fenv_is_valid();
bool LMI_SO fenv_validate(enum_fenv_indulgence = e_fenv_indulge_nothing);

#endif // fenv_lmi_hpp

