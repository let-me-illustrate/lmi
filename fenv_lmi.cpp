// Manage floating-point environment.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "fenv_lmi.hpp"

#include "alert.hpp"
#include "miscellany.hpp"               // stifle_warning_for_unused_variable()

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
///   fesetround(int);
/// accepts any integer, whereas
///   void fenv_rounding(e_ieee754_rounding rounding_mode)
/// accepts only the arguments we allow.

namespace floating_point_environment {} // doxygen workaround.

void fenv_initialize()
{
#if defined LMI_X87
    x87_control_word(default_x87_control_word());
#else  // !defined LMI_X87
    fenv_t save_env;
    feholdexcept(&save_env);
    fesetround(FE_TONEAREST);

    // We currently don't set hardware precision in this case as this is an
    // x87-specific feature.
#endif // !defined LMI_X87
}

#if defined LMI_X87
e_ieee754_precision fenv_precision()
{
#if defined __BORLANDC__
    unsigned short int pc = (unsigned short int)(MCW_PC) & x87_control_word();
    return
          (PC_24 == pc) ? fe_fltprec
        : (PC_53 == pc) ? fe_dblprec
        : (PC_64 == pc) ? fe_ldblprec
        : throw std::runtime_error("Failed to determine hardware precision.")
        ;
#else  // !defined __BORLANDC__
    e_x87_precision pc = intel_control_word(x87_control_word()).pc();
    return
          (x87_fe_fltprec  == pc) ? fe_fltprec
        : (x87_fe_dblprec  == pc) ? fe_dblprec
        : (x87_fe_ldblprec == pc) ? fe_ldblprec
        : throw std::runtime_error("Failed to determine hardware precision.")
        ;
#endif // !defined __BORLANDC__
}

void fenv_precision(e_ieee754_precision precision_mode)
{
#if defined __BORLANDC__
    unsigned short int z =
          (fe_fltprec  == precision_mode) ? (unsigned short int)(PC_24)
        : (fe_dblprec  == precision_mode) ? (unsigned short int)(PC_53)
        : (fe_ldblprec == precision_mode) ? (unsigned short int)(PC_64)
        : throw std::runtime_error("Failed to set hardware precision.")
        ;
    _control87(z, MCW_PC);
#else  // !defined __BORLANDC__
    e_x87_precision pc =
          (fe_fltprec  == precision_mode) ? x87_fe_fltprec
        : (fe_dblprec  == precision_mode) ? x87_fe_dblprec
        : (fe_ldblprec == precision_mode) ? x87_fe_ldblprec
        : throw std::runtime_error("Failed to set hardware precision.")
        ;
    intel_control_word control_word(x87_control_word());
    control_word.pc(pc);
    x87_control_word(control_word.cw());
#endif // !defined __BORLANDC__
}
#endif // defined LMI_X87

e_ieee754_rounding fenv_rounding()
{
#if defined LMI_X87
#if defined __BORLANDC__
    unsigned short int rc = (unsigned short int)(MCW_RC) & x87_control_word();
    return
          (RC_NEAR == rc) ? fe_tonearest
        : (RC_DOWN == rc) ? fe_downward
        : (RC_UP   == rc) ? fe_upward
        : (RC_CHOP == rc) ? fe_towardzero
        : throw std::runtime_error("Failed to determine rounding mode.")
        ;
#else  // !defined __BORLANDC__
    e_x87_rounding rc = intel_control_word(x87_control_word()).rc();
    return
          (x87_fe_tonearest  == rc) ? fe_tonearest
        : (x87_fe_downward   == rc) ? fe_downward
        : (x87_fe_upward     == rc) ? fe_upward
        : (x87_fe_towardzero == rc) ? fe_towardzero
        : throw std::runtime_error("Failed to determine rounding mode.")
        ;
#endif // !defined __BORLANDC__
#else  // !defined LMI_X87
    int z = fegetround();
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
#if defined __BORLANDC__
    unsigned short int z =
          (fe_tonearest  == rounding_mode) ? (unsigned short int)(RC_NEAR)
        : (fe_downward   == rounding_mode) ? (unsigned short int)(RC_DOWN)
        : (fe_upward     == rounding_mode) ? (unsigned short int)(RC_UP)
        : (fe_towardzero == rounding_mode) ? (unsigned short int)(RC_CHOP)
        : throw std::runtime_error("Failed to set rounding mode.")
        ;
    _control87(z, MCW_RC);
#else  // !defined __BORLANDC__
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
#endif // !defined __BORLANDC__
#else  // !defined LMI_X87
    int z =
          (fe_tonearest  == rounding_mode) ? FE_TONEAREST
        : (fe_downward   == rounding_mode) ? FE_DOWNWARD
        : (fe_upward     == rounding_mode) ? FE_UPWARD
        : (fe_towardzero == rounding_mode) ? FE_TOWARDZERO
        : throw std::runtime_error("Failed to set rounding mode.")
        ;
    fesetround(z);
#endif // !defined LMI_X87
}

namespace
{
bool check_fpenv_and_explain_if_invalid(std::string& explanation)
{
    std::ostringstream oss;

#if defined LMI_X87
    if(default_x87_control_word() != x87_control_word())
        {
        oss
            << "The floating-point control word was unexpectedly '"
            << std::hex << std::internal << std::showbase << std::setfill('0')
            << std::setw(6) << x87_control_word()
            << "'.\n"
            ;
        }
#else  // !defined LMI_X87
    if(fenv_rounding() != fe_tonearest)
        {
        oss
            << "The rounding mode was unexpectedly "
            << fenv_rounding()
            << " instead of the expected "
            << fe_tonearest
            << ".\n"
            ;
        }

    int const enabled_fp_exceptions = fetestexcept(FE_ALL_EXCEPT);
    if(enabled_fp_exceptions != 0)
        {
        oss
            << "Floating point exceptions corresponding to the bitmask '"
            << std::hex << std::internal << std::showbase << std::setfill('0')
            << std::setw(8) << enabled_fp_exceptions
            << "' have been found to be unexpectedly enabled.\n"
            ;
        }
#endif // !defined LMI_X87

    if(!oss.str().empty())
        {
        oss
            << "\nProbably some other program changed this crucial setting."
            << "\nIt has been reset correctly. Rerun any illustration that"
            << "\nwas being run when this message appeared, because it may"
            << "\nbe incorrect."
            << "\n"
            ;
        explanation = oss.str();
        return false;
        }

    return true;
}
} // Unnamed namespace.

bool fenv_is_valid()
{
    std::string explanation;
    return check_fpenv_and_explain_if_invalid(explanation);
}

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
    stifle_warning_for_unused_variable(indulgence);
#endif // !defined LMI_X87

    std::string explanation;
    bool okay = check_fpenv_and_explain_if_invalid(explanation);

    if(!okay)
        {
        fenv_initialize();
        warning() << explanation << LMI_FLUSH;
        }

    return okay;
}

