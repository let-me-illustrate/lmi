// Manage floating-point environment--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#if defined __MINGW32__
// No lmi code except this unit test uses this nonstandard function.
// MinGW declares it in <float.h> iff __STRICT_ANSI__ is not defined,
// but in practice that macro is always defined, and #undef'ing it
// is forbidden with gcc-12. The MinGW prototype is decorated with
// various macros, but those are liable to change over time, and they
// do not seem to be necessary.
unsigned int _control87(unsigned int newcw, unsigned int mask);
#endif // defined __MINGW32__

#include "fenv_guard.hpp"
#include "fenv_lmi.hpp"

#include "test_tools.hpp"

#include <bitset>
#include <cfenv>
#include <climits>                      // CHAR_BIT
#include <cmath>                        // rint()
#include <stdexcept>

std::bitset<CHAR_BIT * sizeof(std::uint32_t)> bits(std::uint32_t i)
{
    return std::bitset<CHAR_BIT * sizeof(std::uint32_t)>(i);
}

#if defined LMI_X87
#   define LMI_TEST_EQUAL_BITS(a,b)                       \
        if((a) == (b))                                    \
            lmi_test::record_success();                   \
        else                                              \
            {                                             \
            lmi_test::record_error();                     \
            lmi_test::error_stream() << "Unequal:";       \
            lmi_test::error_stream() << bits(a);          \
            lmi_test::error_stream() << bits(b);          \
            lmi_test::error_stream() << LMI_TEST_FLUSH;   \
            }                                             \

#endif // defined LMI_X87

int test_main(int, char*[])
{
#if defined LMI_X87
    std::uint16_t cw = 0x0000;

    LMI_TEST_EQUAL_BITS(0x037f, msvc_to_intel(0x0008001f));

    LMI_TEST_EQUAL_BITS(0x0008001f, intel_to_msvc(0x037f));

    cw = 0x037f;
    LMI_TEST_EQUAL_BITS(0x0008001f, intel_to_msvc(cw));
    LMI_TEST_EQUAL_BITS(cw, msvc_to_intel(0x0008001f));
    LMI_TEST_EQUAL_BITS(cw, msvc_to_intel(intel_to_msvc(cw)));

    cw = 0x027f;
    LMI_TEST_EQUAL_BITS(cw, msvc_to_intel(intel_to_msvc(cw)));

    cw = 0x037f;
    LMI_TEST_EQUAL_BITS(cw, msvc_to_intel(intel_to_msvc(cw)));

    // Most reserved bits should be left as initialized by FINIT...
    x87_control_word(0x0000);
    LMI_TEST_EQUAL_BITS(0x0040, x87_control_word());

    // ...but bit 12, the deprecated IC, remains settable.
    x87_control_word
        (static_cast<std::uint16_t>
            (~intel_control_word_parameters::settable
            )
        );

    LMI_TEST_EQUAL_BITS
        (intel_control_word_parameters::reserved_values
        ,~0x01000UL & x87_control_word()
        );

    // Make sure invalid PC values are forbidden. The implementation
    // traps this upon conversion between different control-word
    // types, but not otherwise--it guards against accidental misuse,
    // not fraud such as:
    //   e_x87_precision (0x01); // Poor practice at best.
    //   e_msvc_precision(0x03); // Unspecified behavior: C++98 7.2/9 .

    intel_control_word invalid_intel_control_word(0);
    invalid_intel_control_word.pc(e_x87_precision (0x01));
    LMI_TEST_THROW
        (msvc_control_word  msvc_error (invalid_intel_control_word)
        ,std::logic_error
        ,"Invalid fpu PC value."
        );

    msvc_control_word  invalid_msvc_control_word (0);
    invalid_msvc_control_word.pc (e_msvc_precision(0x03));
    LMI_TEST_THROW
        (intel_control_word intel_error(invalid_msvc_control_word )
        ,std::logic_error
        ,"Invalid fpu PC value."
        );

    x87_control_word(0x027f);
    LMI_TEST_EQUAL_BITS(0x027f, x87_control_word());

    x87_control_word(0x037f);
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());

    x87_control_word(default_x87_control_word());
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());

    fenv_initialize();
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());

#   if defined __MINGW32__
    // Test the C99 method, as extended by MinGW.
    LMI_TEST_EQUAL(0, std::fesetenv(FE_PC53_ENV));
    LMI_TEST_EQUAL_BITS(0x027f, x87_control_word());

    LMI_TEST_EQUAL(0, std::fesetenv(FE_PC64_ENV));
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());

    LMI_TEST_EQUAL(0, std::fesetenv(FE_DFL_ENV));
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());
#   endif // defined __MINGW32__

#   if defined _MCW_EM
    // Test the msvc rtl method.
    x87_control_word(0x0000);
    _control87(_MCW_EM,  _MCW_EM);
    _control87(_RC_NEAR, _MCW_RC);
    _control87(_PC_64,   _MCW_PC);
    LMI_TEST_EQUAL_BITS(0x037f, x87_control_word());
#   endif // defined _MCW_EM

    // Test precision and rounding control. These spotchecks are
    // complemented by the thorough generic tests below.

    fenv_initialize();
    fenv_precision(fe_dblprec);
    LMI_TEST_EQUAL_BITS(0x027f, x87_control_word());

    fenv_initialize();
    fenv_rounding(fe_towardzero);
    LMI_TEST_EQUAL_BITS(0x0f7f, x87_control_word());

#endif // defined LMI_X87

    // Test precision control iff supported.

#if defined LMI_X87
    fenv_precision  (fe_fltprec);
    LMI_TEST_EQUAL(fe_fltprec , fenv_precision());

    fenv_precision  (fe_dblprec);
    LMI_TEST_EQUAL(fe_dblprec , fenv_precision());

    fenv_precision  (fe_ldblprec);
    LMI_TEST_EQUAL(fe_ldblprec, fenv_precision());
#endif // defined LMI_X87

    // Test rounding control.

    // This would be a compile-time error:
//    fenv_rounding   (99999);

    fenv_rounding   (fe_tonearest);
    LMI_TEST_EQUAL(fe_tonearest , fenv_rounding());
    LMI_TEST_EQUAL(-2, std::rint(-2.5));
    LMI_TEST_EQUAL(-2, std::rint(-1.5));
    LMI_TEST_EQUAL( 2, std::rint( 1.5));
    LMI_TEST_EQUAL( 2, std::rint( 2.5));

    fenv_rounding   (fe_downward);
    LMI_TEST_EQUAL(fe_downward  , fenv_rounding());
    LMI_TEST_EQUAL(-3, std::rint(-2.5));
    LMI_TEST_EQUAL(-2, std::rint(-1.5));
    LMI_TEST_EQUAL( 1, std::rint( 1.5));
    LMI_TEST_EQUAL( 2, std::rint( 2.5));

    fenv_rounding   (fe_upward);
    LMI_TEST_EQUAL(fe_upward    , fenv_rounding());
    LMI_TEST_EQUAL(-2, std::rint(-2.5));
    LMI_TEST_EQUAL(-1, std::rint(-1.5));
    LMI_TEST_EQUAL( 2, std::rint( 1.5));
    LMI_TEST_EQUAL( 3, std::rint( 2.5));

    fenv_rounding   (fe_towardzero);
    LMI_TEST_EQUAL(fe_towardzero, fenv_rounding());
    LMI_TEST_EQUAL(-2, std::rint(-2.5));
    LMI_TEST_EQUAL(-1, std::rint(-1.5));
    LMI_TEST_EQUAL( 1, std::rint( 1.5));
    LMI_TEST_EQUAL( 2, std::rint( 2.5));

    fenv_initialize();
    LMI_TEST(fenv_validate());

#if defined LMI_X87
    std::cout
        << "Expect induced warnings exactly as predicted below,"
        << " but no test failure."
        << std::endl
        ;

    fenv_initialize();
    fenv_precision(fe_dblprec);
    LMI_TEST(fenv_validate(e_fenv_indulge_0x027f));
    LMI_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x007f' warning...\n" << std::endl;
    fenv_precision(fe_fltprec);
    LMI_TEST(!fenv_validate());
    std::cout << "...end of induced warning]." << std::endl;
    LMI_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x007f' warning...\n" << std::endl;
    fenv_precision(fe_fltprec);
    LMI_TEST(!fenv_validate(e_fenv_indulge_0x027f));
    std::cout << "...end of induced warning]." << std::endl;
    LMI_TEST(fenv_validate());

    LMI_TEST(0 == fenv_guard::instance_count());

    fenv_precision(fe_fltprec);
    {
    fenv_guard fg;
    LMI_TEST(1 == fenv_guard::instance_count());
    }
    LMI_TEST(0 == fenv_guard::instance_count());
    LMI_TEST(fenv_validate());

    std::cout << "\n[Expect an induced '0x027f' warning...\n" << std::endl;
    {
    fenv_guard fg;
    LMI_TEST(1 == fenv_guard::instance_count());
    fenv_precision(fe_dblprec);
    }
    LMI_TEST(0 == fenv_guard::instance_count());
    std::cout << "...end of induced warning]." << std::endl;
    LMI_TEST(fenv_validate());
#endif // defined LMI_X87

    return 0;
}
