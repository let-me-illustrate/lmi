// Manage floating-point environment.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "fenv_lmi.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "miscellany.hpp"               // stifle_unused_warning()

#include <cfenv>
#include <iomanip>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>

/// Manage the floating-point environment, generally using compiler-
/// and platform-specific techniques.
///
/// SOMEDAY !! The facilities added in C99's <fenv.h> could handle
/// some of this work, but not all: notably, they don't address
/// hardware precision. Some C99 implementations are written here, but
/// commented out, in case they are useful for some new architecture
/// or compiler someday. They have not been tested with any compiler
/// that supports C99 fully, and in particular
///   #pragma STDC FENV_ACCESS ON
/// must be added to avoid undefined behavior. Furthermore, return
/// codes of these C99 functions are not checked; perhaps one day C++
/// will be extended to provide cover functions that throw exceptions
/// on failure instead.
///
/// C99 <fenv.h> facilities are not used here in the cases for which
/// they are sufficient, in order to maintain consistency with cases
/// for which they are not. Another reason for this design decision
/// is type safety: for instance,
///   std::fesetround(int);
/// accepts any integer, whereas
///   void fenv_rounding(e_ieee754_rounding rounding_mode)
/// accepts only the arguments we allow.

namespace floating_point_environment {} // doxygen workaround.

void fenv_initialize()
{
#if defined LMI_X87
    x87_control_word(default_x87_control_word());
#else  // !defined LMI_X87
    LMI_ASSERT(0 == std::fesetround(FE_TONEAREST));
    // Standard C++ provides no way to set hardware precision.
    // Here is an example of a C99 7.6/9 extension that controls
    // hardware precision for MinGW32:
    //   std::fesetenv(FE_PC64_ENV);
#endif // !defined LMI_X87
}

e_ieee754_precision fenv_precision()
{
#if defined LMI_X87
    e_x87_precision pc = intel_control_word(x87_control_word()).pc();
    return
          (x87_fe_fltprec  == pc) ? fe_fltprec
        : (x87_fe_dblprec  == pc) ? fe_dblprec
        : (x87_fe_ldblprec == pc) ? fe_ldblprec
        : throw std::runtime_error("Failed to determine hardware precision.")
        ;
#else  // !defined LMI_X87
    // Assume that any reasonable platform other than x87 uses
    // double-precision floating point by default.
    return fe_dblprec;
#endif // !defined LMI_X87
}

void fenv_precision(e_ieee754_precision precision_mode)
{
#if defined LMI_X87
    e_x87_precision pc =
          (fe_fltprec  == precision_mode) ? x87_fe_fltprec
        : (fe_dblprec  == precision_mode) ? x87_fe_dblprec
        : (fe_ldblprec == precision_mode) ? x87_fe_ldblprec
        : throw std::runtime_error("Failed to set hardware precision.")
        ;
    intel_control_word control_word(x87_control_word());
    control_word.pc(pc);
    x87_control_word(control_word.cw());
#else  // !defined LMI_X87
    stifle_unused_warning(precision_mode);
    throw std::logic_error("Unable to set hardware precision.");
#endif // !defined LMI_X87
}

e_ieee754_rounding fenv_rounding()
{
#if defined LMI_X87
    e_x87_rounding rc = intel_control_word(x87_control_word()).rc();
    return
          (x87_fe_tonearest  == rc) ? fe_tonearest
        : (x87_fe_downward   == rc) ? fe_downward
        : (x87_fe_upward     == rc) ? fe_upward
        : (x87_fe_towardzero == rc) ? fe_towardzero
        : throw std::runtime_error("Failed to determine rounding mode.")
        ;
#else  // !defined LMI_X87
    int z = std::fegetround();
    LMI_ASSERT(0 <= z); // Returns negative on failure [C99 7.6.3.1/3].
    return
          (FE_TONEAREST  == z) ? fe_tonearest
        : (FE_DOWNWARD   == z) ? fe_downward
        : (FE_UPWARD     == z) ? fe_upward
        : (FE_TOWARDZERO == z) ? fe_towardzero
        : throw std::runtime_error("Failed to determine rounding mode.")
        ;
#endif // !defined LMI_X87
}

void fenv_rounding(e_ieee754_rounding rounding_mode)
{
#if defined LMI_X87
    e_x87_rounding rc =
          (fe_tonearest  == rounding_mode) ? x87_fe_tonearest
        : (fe_downward   == rounding_mode) ? x87_fe_downward
        : (fe_upward     == rounding_mode) ? x87_fe_upward
        : (fe_towardzero == rounding_mode) ? x87_fe_towardzero
        : throw std::runtime_error("Failed to set rounding mode.")
        ;
    intel_control_word control_word(x87_control_word());
    control_word.rc(rc);
    x87_control_word(control_word.cw());
#else  // !defined LMI_X87
    int z =
          (fe_tonearest  == rounding_mode) ? FE_TONEAREST
        : (fe_downward   == rounding_mode) ? FE_DOWNWARD
        : (fe_upward     == rounding_mode) ? FE_UPWARD
        : (fe_towardzero == rounding_mode) ? FE_TOWARDZERO
        : throw std::runtime_error("Failed to set rounding mode.")
        ;
    LMI_ASSERT(0 == std::fesetround(z));
#endif // !defined LMI_X87
}

bool fenv_is_valid()
{
#if defined LMI_X87
    return default_x87_control_word() == x87_control_word();
#else  // !defined LMI_X87
    // SOMEDAY !! Consider using glibc's fegetexcept() here to test
    // which exceptions are actually enabled. std::fetestexcept() is
    // useless for this purpose because it tests the status word
    // rather than the control word--see:
    //   https://lists.nongnu.org/archive/html/lmi/2017-01/msg00024.html
    return FE_TONEAREST == std::fegetround();
#endif // !defined LMI_X87
}

namespace
{
std::string fenv_explain_invalid_control_word()
{
    std::ostringstream oss;
#if defined LMI_X87
    oss
        << "The floating-point control word was unexpectedly '"
        << std::hex << std::internal << std::showbase << std::setfill('0')
        << std::setw(6) << x87_control_word()
        << "'.\n"
        ;
#else  // !defined LMI_X87
    // SOMEDAY !! Consider using glibc's fegetexcept() here to show
    // which exceptions are actually enabled. std::fetestexcept() is
    // useless for this purpose because it tests the status word
    // rather than the control word--see:
    //   https://lists.nongnu.org/archive/html/lmi/2017-01/msg00024.html
    oss
        << "The floating-point environment unexpectedly changed."
        << "\nThe rounding mode is " << std::fegetround()
//      << " and the exception bitmask is " << fegetexcept()
        << ".\n"
        ;
#endif // !defined LMI_X87
    oss
        << "\nProbably some other program changed this crucial setting."
        << "\nIt has been reset correctly. Rerun any illustration that"
        << "\nwas being run when this message appeared, because it may"
        << "\nbe incorrect."
        << "\n"
        ;
    return oss.str();
}
} // Unnamed namespace.

/// Test floating-point control word; if invalid, reset and complain.
///
/// Grant an indulgence if specified by the argument. Motivation: on
/// the msw platform, some dlls rudely change the control word to
/// 0x027f, the default value used for ms applications. That's the
/// control word most often reported by users, and it isn't as
/// disastrous as some others would be (e.g., borland-built dlls
/// would change exception masks, potentially causing application
/// crashes), though extensive testing has demonstrated that it could
/// still cause numerical results of this program to vary unacceptably
/// from one run to the next. Close investigation has shown that the
/// control word is changed during GUI activity--e.g., when a dll for
/// a particular control is loaded, bringing in a cascade of third-
/// party dlls that have installed system-wide hooks--and that can be
/// indulged as long as critical calculations are guarded without any
/// indulgence.
///
/// Postcondition: control word is valid.

bool fenv_validate(enum_fenv_indulgence indulgence)
{
#if defined LMI_X87
    if
        (   e_fenv_indulge_0x027f == indulgence
        &&  e_fenv_indulge_0x027f == x87_control_word()
        )
        {
        fenv_initialize();
        }
#else  // !defined LMI_X87
    stifle_unused_warning(indulgence);
#endif // !defined LMI_X87

    bool okay = fenv_is_valid();

    if(!okay)
        {
        std::string s(fenv_explain_invalid_control_word());
        fenv_initialize();
        warning() << s << LMI_FLUSH;
        }

    return okay;
}
