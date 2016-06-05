// Rounding--unit test.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#if defined __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // defined __BORLANDC__

#include "round_to.hpp"

#include "fenv_lmi.hpp"
#include "test_tools.hpp"

#include <algorithm>                    // std::max()
#include <cstddef>                      // std::size_t
#include <ios>
#include <iostream>
#include <math.h>                       // C99 round() and kin
#include <ostream>

#if defined LMI_IEC_559
    // In case the C++ compiler offers C99 fesetround(), assume that
    // it defines __STDC_IEC_559__, but doesn't support
    //   #pragma STDC FENV_ACCESS ON
    // in C++ mode. (I have no such compiler, so that assumption and
    // code that ought to use that pragma are untested.)
    enum e_ieee754_rounding
        {fe_tonearest  = FE_TONEAREST
        ,fe_downward   = FE_DOWNWARD
        ,fe_upward     = FE_UPWARD
        ,fe_towardzero = FE_TOWARDZERO
        };
#elif defined LMI_X86
    // "fenv_lmi_x86.hpp" provides the necessary values.
#else  // No known way to set rounding style.
#   error No known way to set rounding style.
#endif // No known way to set rounding style.

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
#if defined LMI_IEC_559
    fesetround(mode);
#elif defined LMI_X86
    fenv_rounding(mode);
#else // No known way to set hardware rounding mode.
    std::cerr
        << "\nCannot set floating-point hardware rounding mode.\n"
        << "Results may be invalid.\n"
        ;
#endif // No known way to set hardware rounding mode.

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

// print_hex_val() bears the following copyright and permission:
/* boost limits_test.cpp   test your <limits> file for important
 *
 * Copyright Jens Maurer 2000
 * Permission to use, copy, modify, sell, and distribute this software
 * is hereby granted without free [sic] provided that the above copyright notice
 * appears in all copies and that both that copyright notice and this
 * permission notice appear in supporting documentation,
 *
 * Jens Maurer makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 */
// GWC modified the code as noted below, e.g. to print in byte-reversed
// order as well, and made some trivial formatting changes.
template<typename T>
void print_hex_val(T t, char const* name)
{
  // GWC changed C cast to reinterpret_cast:
  unsigned char const* p = reinterpret_cast<unsigned char const*>(&t);
  std::cout << "hex value of " << name << " is: ";

// GWC modifications begin
  // My principal compiler, mingw gcc, has sizeof(long double) == 12,
  // but only ten bytes are significant; the other two are padding.
  std::size_t size_of_T = sizeof(T);
#if defined __GNUC__ && defined LMI_X86
  if(12 == size_of_T)
    size_of_T = 10;
#endif // defined __GNUC__ && defined LMI_X86
// GWC modifications end

  for (unsigned int i = 0; i < size_of_T; ++i) { // modified by GWC
    if(p[i] <= 0xF)
      std::cout << "0";
    // GWC changed C cast to static_cast:
    std::cout << std::hex << static_cast<int>(p[i]);
  }
// GWC modifications begin
  std::cout << " / ";
  for (int i = size_of_T - 1; 0 <= i; --i) {
    if(p[i] <= 0xF)
      std::cout << "0";
    std::cout << std::hex << static_cast<int>(p[i]);
  }
  std::cout << std::dec << std::endl;
// GWC modifications end
}

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

    max_prec_real abs_error = detail::perform_fabs(observed - expected);
    // Nonstandardly define relative error in terms of
    // o(bserved) and e(xpected) as
    //   |(o-e)/e| if e nonzero, else
    //   |(o-e)/o| if o nonzero, else
    //   zero
    // in order to avoid division by zero.
    max_prec_real rel_error(0.0);
    if(max_prec_real(0.0) != expected)
        {
        rel_error = detail::perform_fabs
            (
              (observed - max_prec_real(expected))
            / expected
            );
        }
    else if(max_prec_real(0.0) != observed)
        {
        rel_error = detail::perform_fabs
            (
              (observed - max_prec_real(expected))
            / observed
            );
        }

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
    //    http://lists.nongnu.org/archive/html/lmi/2008-06/msg00019.html
    // Code to support a more liberal tolerance is retained in case
    // it someday proves useful on some other platform.

//    bool error_is_within_tolerance = rel_error <= tolerance;
    bool error_is_within_tolerance = observed == expected;

    if(!error_is_within_tolerance)
        {
        std::cout << '\n';
        // Use enough precision to map the base-ten scientific
        // representation back to binary without loss of accuracy.
        // Cf. C99 5.2.4.2.2/8 (DECIMAL_DIG).
        int nbits = std::numeric_limits<RealType>::digits;
        std::streamsize old_precision = std::cout.precision
            (1 + static_cast<int>(std::ceil(std::log10(std::pow(2.0, nbits))))
            );
        std::ios_base::fmtflags old_flags = std::cout.flags();

        std::cout
            << "Rounding   " << get_name_of_float_type<RealType>()
            << unrounded
            ;
        std::cout << '\n';

        std::cout << "  ";
        print_hex_val(unrounded, "input   ");
        std::cout << "  ";
        print_hex_val(expected,  "expected");
        std::cout << "  ";
        print_hex_val(observed,  "observed");

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

        std::cout.setf(old_flags);
        std::cout.precision(old_precision);
        std::cout << std::endl;
        }
    return error_is_within_tolerance;
}

void test_various_float_types
    (long double unrounded
    ,long double expected
    )
{
    BOOST_TEST((test_one_case(static_cast<float      >(unrounded), static_cast<float      >(expected))));
    BOOST_TEST((test_one_case(static_cast<double     >(unrounded), static_cast<double     >(expected))));
    BOOST_TEST((test_one_case(static_cast<long double>(unrounded), static_cast<long double>(expected))));
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
    //   nextafter(0.5, 0.0)
    // both of which are ffffffffffffdf3f / 3fdfffffffffffff internally.

    test_various_float_types(4503599627370497.0L, 4503599627370497.0L);

    // This test:
//    test_various_float_types(nextafter(0.5L, 0.0L), 0.0L);
    // mustn't be run as such because, e.g., the value
    //   static_cast<float>(nextafter(0.5L, 0.0L))
    // need not be distinct from 0.0F.

    BOOST_TEST((test_one_case(nextafterf(0.5F, 0.0F), 0.0F)));
    BOOST_TEST((test_one_case(nextafter (0.5 , 0.0 ), 0.0 )));
    BOOST_TEST((test_one_case(nextafterl(0.5L, 0.0L), 0.0L)));
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

