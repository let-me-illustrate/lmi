// Manage floating-point environment--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

// Include this first, in order to get the prototype for nonstandard
// _control87() before any other header can include <float.h>.
// CYGWIN !! For cygwin with '-mno-cygwin', that doesn't work quite
// the same way as it does for MinGW:
//   http://cygwin.com/ml/cygwin/2005-03/msg00752.html
//   http://cygwin.com/ml/cygwin/2006-08/msg00521.html
// but that deficiency is easily worked around below, and no lmi code
// except this unit test uses that nonstandard function.
#if defined __MINGW32__
#   undef __STRICT_ANSI__
#   include <float.h>
#endif // defined __MINGW32__

#include "fenv_guard.hpp"
#include "fenv_lmi.hpp"

#include "test_tools.hpp"

#include <bitset>
#include <climits> // CHAR_BIT
#if defined LMI_IEC_559 || defined __MINGW32__
// Specify '|| defined __MINGW32__' to test MinGW extensions like FE_PC64_ENV.
#   include <fenv.h>
#endif // defined LMI_IEC_559
#include <math.h>  // C99 rint()
#include <stdexcept>

std::bitset<CHAR_BIT * sizeof(unsigned long int)> bits(unsigned long int i)
{
    return std::bitset<CHAR_BIT * sizeof(unsigned long int)>(i);
}

#define BOOST_TEST_EQUAL_BITS(a,b)                    \
    if((a) == (b))                                    \
        lmi_test::record_success();                   \
    else                                              \
        {                                             \
        lmi_test::record_error();                     \
        lmi_test::error_stream() << "Unequal:";       \
        lmi_test::error_stream() << bits(a);          \
        lmi_test::error_stream() << bits(b);          \
        lmi_test::error_stream() << BOOST_TEST_FLUSH; \
        }                                             \

int test_main(int, char*[])
{
#if defined __BORLANDC__
    // Skip many tests that this defective compiler can't handle.
#elif defined LMI_X86
    unsigned short int cw = 0x0000;

    BOOST_TEST_EQUAL_BITS(0x037f, msw_to_intel(0x0008001f));

    BOOST_TEST_EQUAL_BITS(0x0008001f, intel_to_msw(0x037f));

    cw = 0x037f;
    BOOST_TEST_EQUAL_BITS(0x0008001f, intel_to_msw(cw));
    BOOST_TEST_EQUAL_BITS(cw, msw_to_intel(0x0008001f));
    BOOST_TEST_EQUAL_BITS(cw, msw_to_intel(intel_to_msw(cw)));

    cw = 0x027f;
    BOOST_TEST_EQUAL_BITS(cw, msw_to_intel(intel_to_msw(cw)));

    cw = 0x037f;
    BOOST_TEST_EQUAL_BITS(cw, msw_to_intel(intel_to_msw(cw)));

    // Most reserved bits should be left as initialized by FINIT...
    x87_control_word(0x0000);
    BOOST_TEST_EQUAL_BITS(0x0040, x87_control_word());

    // ...but bit 12, the deprecated IC, remains settable.
    x87_control_word
        (static_cast<unsigned short int>
            (~intel_control_word_parameters::settable
            )
        );

    BOOST_TEST_EQUAL_BITS
        (intel_control_word_parameters::reserved_values
        ,~0x01000 & x87_control_word()
        );

    // Make sure invalid PC values are forbidden. The implementation
    // traps this upon conversion between different control-word
    // types, but not otherwise--it guards against accidental misuse,
    // not fraud such as:
    //   e_ieee754_precision(0x01); // Poor practice at best.
    //   e_msw_pc           (0x03); // Unspecified behavior: C++98 7.2/9 .

    intel_control_word invalid_intel_control_word(0);
    invalid_intel_control_word.pc(e_ieee754_precision(0x01));
    BOOST_TEST_THROW
        (msw_control_word   msw_error  (invalid_intel_control_word)
        ,std::logic_error
        ,"Invalid fpu PC value."
        );

    msw_control_word   invalid_msw_control_word  (0);
    invalid_msw_control_word.pc  (e_msw_pc  (0x03));
    BOOST_TEST_THROW
        (intel_control_word intel_error(invalid_msw_control_word  )
        ,std::logic_error
        ,"Invalid fpu PC value."
        );

    x87_control_word(0x027f);
    BOOST_TEST_EQUAL_BITS(0x027f, x87_control_word());

    x87_control_word(0x037f);
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());

    x87_control_word(default_x87_control_word());
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());

    fenv_initialize();
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());

#   if defined __MINGW32__
    // Test the C99 method, as extended by MinGW.
    fesetenv(FE_PC53_ENV);
    BOOST_TEST_EQUAL_BITS(0x027f, x87_control_word());

    fesetenv(FE_PC64_ENV);
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());

    fesetenv(FE_DFL_ENV);
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());
#   endif // defined __MINGW32__

#   if defined _MCW_EM
    // Test the ms C rtl method.
    x87_control_word(0x0000);
    _control87(_MCW_EM,  _MCW_EM);
    _control87(_RC_NEAR, _MCW_RC);
    _control87(_PC_64,   _MCW_PC);
    BOOST_TEST_EQUAL_BITS(0x037f, x87_control_word());
#   endif // defined _MCW_EM

    // Test precision and rounding control. These spotchecks are
    // complemented by the thorough generic tests below.

    fenv_initialize();
    fenv_precision(fe_dblprec);
    BOOST_TEST_EQUAL_BITS(0x027f, x87_control_word());

    fenv_initialize();
    fenv_rounding(fe_towardzero);
    BOOST_TEST_EQUAL_BITS(0x0f7f, x87_control_word());

#else  // Unknown platform.
    throw std::runtime_error("Unknown platform.");
#endif // Unknown platform.

    // Test precision control.

    fenv_precision  (fe_fltprec);
    BOOST_TEST_EQUAL(fe_fltprec , fenv_precision());

    fenv_precision  (fe_dblprec);
    BOOST_TEST_EQUAL(fe_dblprec , fenv_precision());

    fenv_precision  (fe_ldblprec);
    BOOST_TEST_EQUAL(fe_ldblprec, fenv_precision());

    // Test rounding control.

    // This would be a compile-time error:
//    fenv_rounding   (99999);

    fenv_rounding   (fe_tonearest);
    BOOST_TEST_EQUAL(fe_tonearest , fenv_rounding());
#if defined LMI_COMPILER_PROVIDES_RINT
    BOOST_TEST_EQUAL(-2, rint(-2.5));
    BOOST_TEST_EQUAL(-2, rint(-1.5));
    BOOST_TEST_EQUAL( 2, rint( 1.5));
    BOOST_TEST_EQUAL( 2, rint( 2.5));
#endif // defined LMI_COMPILER_PROVIDES_RINT

    fenv_rounding   (fe_downward);
    BOOST_TEST_EQUAL(fe_downward  , fenv_rounding());
#if defined LMI_COMPILER_PROVIDES_RINT
    BOOST_TEST_EQUAL(-3, rint(-2.5));
    BOOST_TEST_EQUAL(-2, rint(-1.5));
    BOOST_TEST_EQUAL( 1, rint( 1.5));
    BOOST_TEST_EQUAL( 2, rint( 2.5));
#endif // defined LMI_COMPILER_PROVIDES_RINT

    fenv_rounding   (fe_upward);
    BOOST_TEST_EQUAL(fe_upward    , fenv_rounding());
#if defined LMI_COMPILER_PROVIDES_RINT
    BOOST_TEST_EQUAL(-2, rint(-2.5));
    BOOST_TEST_EQUAL(-1, rint(-1.5));
    BOOST_TEST_EQUAL( 2, rint( 1.5));
    BOOST_TEST_EQUAL( 3, rint( 2.5));
#endif // defined LMI_COMPILER_PROVIDES_RINT

    fenv_rounding   (fe_towardzero);
    BOOST_TEST_EQUAL(fe_towardzero, fenv_rounding());
#if defined LMI_COMPILER_PROVIDES_RINT
    BOOST_TEST_EQUAL(-2, rint(-2.5));
    BOOST_TEST_EQUAL(-1, rint(-1.5));
    BOOST_TEST_EQUAL( 1, rint( 1.5));
    BOOST_TEST_EQUAL( 2, rint( 2.5));
#endif // defined LMI_COMPILER_PROVIDES_RINT

    std::cout
        << "Expect induced warnings exactly as predicted below,"
        << " but no test failure."
        << std::endl
        ;

    fenv_initialize();
    BOOST_TEST(fenv_validate());

    fenv_initialize();
    fenv_precision(fe_dblprec);
    BOOST_TEST(fenv_validate(e_fenv_indulge_0x027f));
    BOOST_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x007f' warning...\n" << std::endl;
    fenv_precision(fe_fltprec);
    BOOST_TEST(!fenv_validate());
    std::cout << "...end of induced warning]." << std::endl;
    BOOST_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x007f' warning...\n" << std::endl;
    fenv_precision(fe_fltprec);
    BOOST_TEST(!fenv_validate(e_fenv_indulge_0x027f));
    std::cout << "...end of induced warning]." << std::endl;
    BOOST_TEST(fenv_validate());

    BOOST_TEST(0 == fenv_guard::instance_count());

    fenv_precision(fe_fltprec);
    {
    fenv_guard fg;
    BOOST_TEST(1 == fenv_guard::instance_count());
    }
    BOOST_TEST(0 == fenv_guard::instance_count());
    BOOST_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x027f' warning...\n" << std::endl;
    {
    fenv_guard fg;
    BOOST_TEST(1 == fenv_guard::instance_count());
    fenv_precision(fe_dblprec);
    }
    BOOST_TEST(0 == fenv_guard::instance_count());
    std::cout << "...end of induced warning]." << std::endl;
    BOOST_TEST(fenv_validate());

    return 0;
}

