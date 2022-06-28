// Rounding--unit test.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// Unit tests for libmingwex round() and its kin. See:
//   http://sf.net/tracker/?func=detail&atid=302435&aid=1962656&group_id=2435
// All testcases reported to mingw.org are believed to be included here.
//
// This testsuite is adapted from 'round_to_test.cpp', and inherits
// its shortcomings, while perhaps retaining vestiges of code that
// are useful in the original but not here.
//
// Functions [ll,l]round[f,,l]() are not explicitly tested, though
// they ideally ought to be.

#include "pchfile.hpp"

#include "round_to.hpp"

#include "fenv_lmi.hpp"
#include "math_functions.hpp"           // relative_error()
#include "miscellany.hpp"               // floating_rep(), scoped_ios_format
#include "test_tools.hpp"

#include <algorithm>                    // max()
#include <cfloat>                       // DECIMAL_DIG
#include <cmath>                        // nextafter(), round()
#include <ios>
#include <iostream>
#include <limits>
#include <math.h>                       // ::round[,f,l]() and kin
#include <ostream>

// Print name of software rounding style for diagnostics.
char const* get_name_of_style(rounding_style style)
{
    return
          (r_toward_zero == style) ? "r_toward_zero"
        : (r_to_nearest  == style) ? "r_to_nearest"
        : (r_upward      == style) ? "r_upward"
        : (r_downward    == style) ? "r_downward"
        : (r_current     == style) ? "r_current"
        : (r_not_at_all  == style) ? "r_not_at_all"
        : "unrecognized"
        ;
}

// Print name of hardware rounding mode for diagnostics.
char const* get_name_of_hardware_rounding_mode(e_ieee754_rounding mode)
{
    return
          (fe_towardzero == mode) ? "toward zero"
        : (fe_tonearest  == mode) ? "to nearest"
        : (fe_upward     == mode) ? "upward"
        : (fe_downward   == mode) ? "downward"
        : "invalid"
        ;
}

// Print name of float type for diagnostics.
template<typename T>
char const* get_name_of_float_type()
{
    throw std::domain_error("Unknown float type.");
}

template<> char const* get_name_of_float_type<float>()
{
    return "(float)";
}

template<> char const* get_name_of_float_type<double>()
{
    return "(double)";
}

template<> char const* get_name_of_float_type<long double>()
{
    return "(long double)";
}

void set_hardware_rounding_mode(e_ieee754_rounding mode, bool synchronize)
{
    fenv_rounding(mode);

    if(synchronize)
        {
        // Synchronize software default rounding style with hardware
        // rounding mode.
        default_rounding_style() =
              (fe_towardzero == mode) ? r_toward_zero
            : (fe_tonearest  == mode) ? r_to_nearest
            : (fe_upward     == mode) ? r_upward
            : (fe_downward   == mode) ? r_downward
            : r_indeterminate // This should never be reached.
            ;
        if(r_indeterminate == default_rounding_style())
            {
            throw std::domain_error("Invalid hardware rounding mode.");
            }
        }
    else
        {
        // Don't synchronize software default rounding style with
        // hardware rounding mode; accordingly, set default style to
        // indeterminate.
        default_rounding_style() = r_indeterminate;
        }
}

/// Emulate std::round() in order to test it. This may seem somewhat
/// silly, because elsewhere a conforming C++11 implementation is
/// presumed. However, std::round() calls for extraordinary testing
/// because its MinGW-w64 CRT implementation is known to be defective,
/// so it is not to be assumed that its C++ overloads are flawless
/// (even though they probably come from an unmarred libstdc++).

template<typename T> T roundFDL(T) {throw std::domain_error("Unknown float type.");}
template<> float       roundFDL<float      >(float       t) {return roundf(t);}
template<> double      roundFDL<double     >(double      t) {return round (t);}
template<> long double roundFDL<long double>(long double t) {return roundl(t);}

// Arguments outside the range of RealType are blithely and silently
// cast to RealType here. This does no harm except for any confusion
// it would create for those who don't read this comment.

template<typename RealType>
bool test_one_case
    (RealType       unrounded
    ,RealType       expected
    )
{
    RealType observed = roundFDL(unrounded);
    // C++'s overloaded round should behave as if it calls C99's
    // round(), roundf(), or roundl() as appropriate.
    LMI_TEST_EQUAL(std::round(unrounded), observed);

    max_prec_real abs_error = std::fabs(observed - expected);
    max_prec_real rel_error = relative_error(observed, expected);

    // In general, we can't hope for the relative error to be less than
    // epsilon for the floating-point type being rounded. Suppose a
    // variable gets its value from a floating literal; 2.13.3/1 says
    //   "If the scaled value is in the range of representable values
    //   for its type, the result is the scaled value if representable,
    //   else the larger or smaller representable value nearest the
    //   scaled value, chosen in an implementation-defined manner."
    // The compiler might map a literal like .005 to some value at
    // compile time, but at run time, the result of even a simple store
    // operation may yield a different value depending on the rounding
    // direction, as can an expression like '5.0 / 1000.0'. C99 (but
    // not C++) requires evaluation of non-static floating-point
    // constants as if at runtime [F.7.4/1].
    max_prec_real tolerance = std::numeric_limits<RealType>::epsilon();

    // All tests pass even with a tolerance of zero, for
    //  - candidate libmingwex as 20080603Z, and
    //  - glibc for amd64, as reported here:
    //    https://lists.nongnu.org/archive/html/lmi/2008-06/msg00019.html
    // Code to support a more liberal tolerance is retained in case
    // it someday proves useful on some other platform.

//    bool error_is_within_tolerance = rel_error <= tolerance;
    bool error_is_within_tolerance = observed == expected;

    if(!error_is_within_tolerance)
        {
        scoped_ios_format meaningless_name(std::cout);
        std::cout << '\n';
        std::cout.precision(DECIMAL_DIG);

        std::cout
            << "Rounding   " << get_name_of_float_type<RealType>()
            << unrounded
            ;
        std::cout << '\n';

        std::cout << "  input    " << floating_rep(unrounded) << '\n';
        std::cout << "  expected " << floating_rep(expected)  << '\n';
        std::cout << "  observed " << floating_rep(observed)  << '\n';

        std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);
        std::cout
            << "\n fixed:"
            << "\n  input     " << unrounded
            << "\n  expected  " << expected
            << "\n  observed  " << observed
            << "\n  abs error " << abs_error
            << "\n  rel error " << rel_error
            << "\n  tolerance " << tolerance
            ;

        std::cout.setf(std::ios_base::scientific, std::ios_base::floatfield);
        std::cout
            << "\n scientific:"
            << "\n  input     " << unrounded
            << "\n  expected  " << expected
            << "\n  observed  " << observed
            << "\n  abs error " << abs_error
            << "\n  rel error " << rel_error
            << "\n  tolerance " << tolerance
            ;

        std::cout << std::endl;
        }
    return error_is_within_tolerance;
}

void test_various_float_types
    (long double unrounded
    ,long double expected
    )
{
    LMI_TEST((test_one_case(static_cast<float >(unrounded), static_cast<float >(expected))));
    LMI_TEST((test_one_case(static_cast<double>(unrounded), static_cast<double>(expected))));
    LMI_TEST((test_one_case(/* long double */  (unrounded), /* long double */  (expected))));
}

// C99 7.12.9.6: round "to the nearest integer value in floating-point
// format, rounding halfway cases away from zero".

void test_rounding()
{
    // The first several blocks of tests use values with no more than
    // six significant decimal digits, six being a natural value for
    //   std::numeric_limits<float>::digits10
    // on an IEC 60559 machine.

    // Test rounding in (-1.0, 1.0).

    test_various_float_types(-0.999999L, -1.0L);
    test_various_float_types(-0.500001L, -1.0L);
    test_various_float_types(-0.500000L, -1.0L); // Away from zero.
    test_various_float_types(-0.499999L, -0.0L);
    test_various_float_types(-0.000001L, -0.0L);
    test_various_float_types(-0.000000L, -0.0L);
    test_various_float_types( 0.000000L,  0.0L);
    test_various_float_types( 0.000001L,  0.0L);
    test_various_float_types( 0.499999L,  0.0L);
    test_various_float_types( 0.500000L,  1.0L); // Away from zero.
    test_various_float_types( 0.500001L,  1.0L);
    test_various_float_types( 0.999999L,  1.0L);

    // Rounding to nearest: make sure halfway cases are rounded away
    // from zero.

    test_various_float_types(-4.5L, -5.0L);
    test_various_float_types(-3.5L, -4.0L);
    test_various_float_types(-2.5L, -3.0L);
    test_various_float_types(-1.5L, -2.0L);
    test_various_float_types(-0.5L, -1.0L);
    test_various_float_types( 0.5L,  1.0L);
    test_various_float_types( 1.5L,  2.0L);
    test_various_float_types( 2.5L,  3.0L);
    test_various_float_types( 3.5L,  4.0L);
    test_various_float_types( 4.5L,  5.0L);

    // The next block of tests uses values in the open interval
    //   (-1E29, +1E29)
    // which is well within the range of a float on an IEC 60559 machine
    // that uses the natural implementation such that
    //   std::numeric_limits<float>::max_exponent10
    // is 38.

    // Rounding an already-rounded integral value shouldn't change its
    // representation. Note however that some of these integers are
    // deliberately outside the range that can be exactly represented,
    // at least on a machine where
    //   std::numeric_limits<some-float-type)::digits10
    // is less than twenty-eight.

    test_various_float_types(-1234567890123456789012345678.0L, -1234567890123456789012345678.0L);
    test_various_float_types(-9876543200000000000000000000.0L, -9876543200000000000000000000.0L);
    test_various_float_types(-1000000000000000000000000000.0L, -1000000000000000000000000000.0L);
    test_various_float_types(-100001.0L, -100001.0L);
    test_various_float_types(-2.0L, -2.0L);
    test_various_float_types(-1.0L, -1.0L);
    test_various_float_types(-0.0L, -0.0L);
    test_various_float_types( 0.0L,  0.0L);
    test_various_float_types( 1.0L,  1.0L);
    test_various_float_types( 2.0L,  2.0L);
    test_various_float_types( 100001.0L,  100001.0L);
    test_various_float_types( 1000000000000000000000000000.0L,  1000000000000000000000000000.0L);
    test_various_float_types( 9876543200000000000000000000.0L,  9876543200000000000000000000.0L);
    test_various_float_types( 1234567890123456789012345678.0L,  1234567890123456789012345678.0L);

    // Test some numbers that are representable in all IEEE formats,
    // but not necessarily as exact integers because they have
    // FLT_DIG or DBL_DIG significant digits (and mantissae chosen
    // so that no more digits are accurately representable).
    test_various_float_types(987654.321L, 987654.0L);
    test_various_float_types(987654321098765.375L,  987654321098765.0L);
    test_various_float_types(987654321098765.500L,  987654321098766.0L);

    // Note: This test
//  test_various_float_types(987654321098765.4321L,  987654321098765.0L);
    // produces a 'failure' with type double on an intel machine using
    // ISO 60559 64-bit doubles (53-bit significand) when the rounding
    // direction is toward positive infinity. This is not a failure of
    // the algorithm, but rather an inherent limitation on precision.
    // The number to be rounded is not exactly representable as a double.
    // It is between 987654321098765.375 and 987654321098765.500, which
    // are exactly representable. The latter representation is chosen
    // only when the rounding direction is upward, and rounding it to
    // nearest correctly maps it to the next higher integer.
    //
    // Thus, the uncertainty due to [2.13.3/1] in the least significant
    // decimal digit of a number that is not exactly representable can
    // exceed four times epsilon. SOMEDAY !! It remains to establish
    // rigorous bounds, both overall and for each step.

    // The fifty-third Mersenne number was identified by the gnu octave
    // maintainers as a difficult case. See:
    //   http://article.gmane.org/gmane.comp.gnu.mingw.user/26299
    //     [2008-04-25T00:31:26Z from Tatsuro MATSUOKA]

    long double const M53 = 6361.0L * 69431.0L * 20394401.0L;

    test_various_float_types(      M53,  9007199254740991.0L);
    test_various_float_types(     -M53, -9007199254740991.0L);

    test_various_float_types( 1 +  M53,  9007199254740992.0L);
    test_various_float_types( 1 + -M53, -9007199254740990.0L);

    test_various_float_types(-1 +  M53,  9007199254740990.0L);
    test_various_float_types(-1 + -M53, -9007199254740992.0L);

    // Here is a similar testcase from François-Xavier Coudert,
    // who refers to:
    //   http://gcc.gnu.org/ml/gcc-patches/2006-10/msg00917.html
    // | 0.499999999999999944488848768742172978818416595458984375
    // | 4503599627370497.0
    // |
    // | They should be rounded to 0.0 and 4503599627370497.0, but
    // | simply adding 0.5 and truncating will round to 1.0 and
    // | 4503599627370498.
    //
    // The number above that's very close to one-half is in fact
    //   std::nextafter(0.5, 0.0)
    // both of which are ffffffffffffdf3f / 3fdfffffffffffff internally.

    test_various_float_types(4503599627370497.0L, 4503599627370497.0L);

    // This test:
//    test_various_float_types(std::nextafter(0.5L, 0.0L), 0.0L);
    // mustn't be run as such because, e.g., the value
    //   static_cast<float>(std::nextafter(0.5L, 0.0L))
    // need not be distinct from 0.0F.

    LMI_TEST((test_one_case(std::nextafterf(0.5F, 0.0F), 0.0F)));
    LMI_TEST((test_one_case(std::nextafter (0.5 , 0.0 ), 0.0 )));
    LMI_TEST((test_one_case(std::nextafterl(0.5L, 0.0L), 0.0L)));
}

int test_all_modes(bool synchronize)
{
    // As stated above, we'd like this to be true for all
    // floating-point types:
    //   X == 1.0e0 * X * 1.0e-0
    // But this is not generally true for long doubles with x86
    // compilers, which may use an 80-bit 'extended-real' format for
    // for long doubles, yet initialize the floating-point hardware
    // to use only a 53-bit mantissa--so initialize the hardware
    // explicitly.
    fenv_initialize();

    e_ieee754_rounding hardware_rounding_mode = fe_tonearest;
    set_hardware_rounding_mode(hardware_rounding_mode, synchronize);
    std::cout
        << "    hardware rounding mode: "
        << get_name_of_hardware_rounding_mode(hardware_rounding_mode)
        << std::endl
        ;
    test_rounding();

    hardware_rounding_mode = fe_downward;
    set_hardware_rounding_mode(hardware_rounding_mode, synchronize);
    std::cout
        << "    hardware rounding mode: "
        << get_name_of_hardware_rounding_mode(hardware_rounding_mode)
        << std::endl
        ;
    test_rounding();

    hardware_rounding_mode = fe_upward;
    set_hardware_rounding_mode(hardware_rounding_mode, synchronize);
    std::cout
        << "    hardware rounding mode: "
        << get_name_of_hardware_rounding_mode(hardware_rounding_mode)
        << std::endl
        ;
    test_rounding();

    hardware_rounding_mode = fe_towardzero;
    set_hardware_rounding_mode(hardware_rounding_mode, synchronize);
    std::cout
        << "    hardware rounding mode: "
        << get_name_of_hardware_rounding_mode(hardware_rounding_mode)
        << std::endl
        ;
    test_rounding();

    return 0;
}

int test_main(int, char*[])
{
    default_rounding_style() = r_indeterminate;

    // The software default rounding style and the hardware rounding
    // mode may be either synchronized or not, so test both ways.
    std::cout << "  Default style synchronized to hardware mode:\n";
    bool rc = test_all_modes(true);
    std::cout << "  Default style NOT synchronized to hardware mode:\n";
    // Use '+' rather than '||' to avoid short-circuit evaluation,
    // so that failure on one test doesn't prevent downstream tests
    // from being run.
    rc = rc + test_all_modes(false);
    return rc;
}
