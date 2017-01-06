// Rounding--unit test.
//
// Copyright (C) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "round_to.hpp"

#include "fenv_lmi.hpp"
#include "test_tools.hpp"

#include <algorithm>                    // std::max()
#include <cstddef>                      // std::size_t
#include <ios>
#include <iostream>
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
  // For gcc with x87, sizeof(long double) == 12, but only
  // ten bytes are significant--the other two are padding.
  std::size_t size_of_T = sizeof(T);
#if defined __GNUC__ && defined LMI_X87
  if(12 == size_of_T)
    size_of_T = 10;
#endif // defined __GNUC__ && defined LMI_X87
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

template<typename RealType>
bool test_one_case
    (RealType       unrounded
    ,RealType       expected
    ,int            decimals
    ,rounding_style style
    )
{
    round_to<RealType> const f(decimals, style);
    RealType observed = f(unrounded);

    max_prec_real abs_error = std::fabs(observed - expected);
    // Nonstandardly define relative error in terms of
    // o(bserved) and e(xpected) as
    //   |(o-e)/e| if e nonzero, else
    //   |(o-e)/o| if o nonzero, else
    //   zero
    // in order to avoid division by zero.
    max_prec_real rel_error(0.0);
    if(max_prec_real(0.0) != expected)
        {
        rel_error = std::fabs
            (
              (observed - max_prec_real(expected))
            / expected
            );
        }
    else if(max_prec_real(0.0) != observed)
        {
        rel_error = std::fabs
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

    // If the decimal scaling factor is not unity, then either it or
    // its reciprocal has no exact finite binary representation. Such
    // scaling erodes the best available accuracy by a factor of
    //   +/- (1 + machine epsilon)
    // so it is not guaranteed that
    //   X == 1.0eN * X * 1.0e-N
    // for nonzero N. It would be nice if
    //   X == 1.0e0 * X * 1.0e-0
    // but C++ doesn't guarantee that. C99 (F.8.2) guarantees that an
    // implementation may make that transformation, assuming that
    // X is not a signaling NaN, although it doesn't require that the
    // identity hold at runtime unless __STDC_IEC_559__ is defined.
    if(0 != decimals)
        {
        // 'tolerance' is of the maximum-precision floating-point
        // type so that it can more closely represent this quantity for
        // types with less precision, without letting the cross-product
        // term epsilon**2 vanish.
        //
        // TODO ?? Shouldn't one epsilon here be epsilon of
        // max-precision-real type?
        tolerance =
               (1.0 + std::numeric_limits<RealType>::epsilon())
             * (1.0 + std::numeric_limits<RealType>::epsilon())
// TODO ??        * (1.0 + std::numeric_limits<max_prec_real>::epsilon())
             - 1.0
             ;
        }
#ifdef LMI_COMO_WITH_MINGW
    // COMPILER !! This looks like a como porting defect: with mingw
    // as the underlying C compiler, a long double should occupy
    // twelve bytes, ten significant and two for padding.
    if(8 == sizeof(long double))
        {
        tolerance = std::max
            (tolerance
            ,2.0L * (max_prec_real)std::numeric_limits<double>::epsilon()
            );
        }
#endif // defined LMI_COMO_WITH_MINGW
    bool error_is_within_tolerance = rel_error <= tolerance;

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
            << "Rounding   "     << get_name_of_float_type<RealType>()
            << unrounded
            << "\n  to "         << decimals << " decimals"
            << "\n  with style " << get_name_of_style(style)
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
    (int            decimals
    ,rounding_style style
    ,long double    unrounded
    ,long double    expected
    )
{
    long double factor = detail::perform_pow(10.0L, -decimals);
    long double u = unrounded * factor;
    long double e = expected  * factor;
    BOOST_TEST((test_one_case(static_cast<float      >(u), static_cast<float      >(e), decimals, style)));
    BOOST_TEST((test_one_case(static_cast<double     >(u), static_cast<double     >(e), decimals, style)));
    BOOST_TEST((test_one_case(static_cast<long double>(u), static_cast<long double>(e), decimals, style)));
}

// Test rounding to various numbers of decimal places.
void test_various_decimals
    (rounding_style style
    ,long double    unrounded
    ,long double    expected
    )
{
    test_various_float_types( 15, style, unrounded, expected);
    test_various_float_types(  3, style, unrounded, expected);
    test_various_float_types(  0, style, unrounded, expected);
    test_various_float_types( -2, style, unrounded, expected);
    test_various_float_types(-10, style, unrounded, expected);
}

// Test rounding to zero decimals with each rounding style.
void test_various_styles
    (long double    unrounded
    ,long double    expected
    )
{
    test_various_float_types(0, r_toward_zero, unrounded, expected);
    test_various_float_types(0, r_to_nearest,  unrounded, expected);
    test_various_float_types(0, r_upward,      unrounded, expected);
    test_various_float_types(0, r_downward,    unrounded, expected);
    test_various_float_types(0, r_not_at_all,  unrounded, expected);
}

void test_rounding()
{
    // The first several blocks of tests use values with no more than
    // six significant decimal digits, six being a natural value for
    //   std::numeric_limits<float>::digits10
    // on an IEC 60559 machine.

    // Test rounding to nearest, deferring halfway cases.

    test_various_decimals(r_to_nearest, -0.999999L, -1.0L);
    test_various_decimals(r_to_nearest, -0.500001L, -1.0L);
//  test_various_decimals(r_to_nearest, -0.500000L, -0.0L); // Deferred.
    test_various_decimals(r_to_nearest, -0.499999L, -0.0L);
    test_various_decimals(r_to_nearest, -0.000001L, -0.0L);
    test_various_decimals(r_to_nearest, -0.000000L, -0.0L);
    test_various_decimals(r_to_nearest,  0.000000L,  0.0L);
    test_various_decimals(r_to_nearest,  0.000001L,  0.0L);
    test_various_decimals(r_to_nearest,  0.499999L,  0.0L);
//  test_various_decimals(r_to_nearest,  0.500000L,  0.0L); // Deferred.
    test_various_decimals(r_to_nearest,  0.500001L,  1.0L);
    test_various_decimals(r_to_nearest,  0.999999L,  1.0L);

    // Rounding to nearest: make sure halfway cases are rounded to even
    // with no decimal scaling. If the input were scaled by a decimal
    // factor other than unity, then results might not accord with
    // expectations based on the real number system, since e.g.
    // .0005L * 1000L may not exactly equal one-half.

    test_various_float_types(0, r_to_nearest, -4.5L, -4.0L);
    test_various_float_types(0, r_to_nearest, -3.5L, -4.0L);
    test_various_float_types(0, r_to_nearest, -2.5L, -2.0L);
    test_various_float_types(0, r_to_nearest, -1.5L, -2.0L);
    test_various_float_types(0, r_to_nearest, -0.5L, -0.0L);
    test_various_float_types(0, r_to_nearest,  0.5L,  0.0L);
    test_various_float_types(0, r_to_nearest,  1.5L,  2.0L);
    test_various_float_types(0, r_to_nearest,  2.5L,  2.0L);
    test_various_float_types(0, r_to_nearest,  3.5L,  4.0L);
    test_various_float_types(0, r_to_nearest,  4.5L,  4.0L);

    // Test rounding toward zero.

    test_various_decimals(r_toward_zero, -0.999999L, -0.0L);
    test_various_decimals(r_toward_zero, -0.500001L, -0.0L);
    test_various_decimals(r_toward_zero, -0.500000L, -0.0L);
    test_various_decimals(r_toward_zero, -0.499999L, -0.0L);
    test_various_decimals(r_toward_zero, -0.000001L, -0.0L);
    test_various_decimals(r_toward_zero,  0.000000L,  0.0L);
    test_various_decimals(r_toward_zero,  0.000001L,  0.0L);
    test_various_decimals(r_toward_zero,  0.499999L,  0.0L);
    test_various_decimals(r_toward_zero,  0.500000L,  0.0L);
    test_various_decimals(r_toward_zero,  0.500001L,  0.0L);
    test_various_decimals(r_toward_zero,  0.999999L,  0.0L);

    // Test rounding toward infinity.

    test_various_decimals(r_upward, -0.999999L, -0.0L);
    test_various_decimals(r_upward, -0.500001L, -0.0L);
    test_various_decimals(r_upward, -0.500000L, -0.0L);
    test_various_decimals(r_upward, -0.499999L, -0.0L);
    test_various_decimals(r_upward, -0.000001L, -0.0L);
    test_various_decimals(r_upward,  0.000000L,  0.0L);
    test_various_decimals(r_upward,  0.000001L,  1.0L);
    test_various_decimals(r_upward,  0.499999L,  1.0L);
    test_various_decimals(r_upward,  0.500000L,  1.0L);
    test_various_decimals(r_upward,  0.500001L,  1.0L);
    test_various_decimals(r_upward,  0.999999L,  1.0L);

    // Test rounding toward negative infinity.

    test_various_decimals(r_downward, -0.999999L, -1.0L);
    test_various_decimals(r_downward, -0.500001L, -1.0L);
    test_various_decimals(r_downward, -0.500000L, -1.0L);
    test_various_decimals(r_downward, -0.499999L, -1.0L);
    test_various_decimals(r_downward, -0.000001L, -1.0L);
    test_various_decimals(r_downward,  0.000000L,  0.0L);
    test_various_decimals(r_downward,  0.000001L,  0.0L);
    test_various_decimals(r_downward,  0.499999L,  0.0L);
    test_various_decimals(r_downward,  0.500000L,  0.0L);
    test_various_decimals(r_downward,  0.500001L,  0.0L);
    test_various_decimals(r_downward,  0.999999L,  0.0L);

    // Test style r_not_at_all, which should just return its argument
    // to within a tolerance of (1 + epsilon)^2 - 1 .

    test_various_decimals(r_not_at_all, -0.999999L, -0.999999L);
    test_various_decimals(r_not_at_all, -0.500001L, -0.500001L);
    test_various_decimals(r_not_at_all, -0.500000L, -0.500000L);
    test_various_decimals(r_not_at_all, -0.499999L, -0.499999L);
    test_various_decimals(r_not_at_all, -0.000001L, -0.000001L);
    test_various_decimals(r_not_at_all,  0.000000L,  0.000000L);
    test_various_decimals(r_not_at_all,  0.000001L,  0.000001L);
    test_various_decimals(r_not_at_all,  0.499999L,  0.499999L);
    test_various_decimals(r_not_at_all,  0.500000L,  0.500000L);
    test_various_decimals(r_not_at_all,  0.500001L,  0.500001L);
    test_various_decimals(r_not_at_all,  0.999999L,  0.999999L);

    // The next block of tests uses values in the open interval
    //   (-1E29, +1E29)
    // so that scaling by 1E10 yields a representable value for a float
    // on an IEC 60559 machine that uses the natural implementation
    // such that
    //   std::numeric_limits<float>::max_exponent10
    // is 38. We test with scaling factors both within and outside the
    // range (1E-10, 1E10).

    // Rounding an already-rounded integral value shouldn't change its
    // representation. Note however that some of these integers are
    // deliberately outside the range that can be exactly represented,
    // at least on a machine where
    //   std::numeric_limits<some-float-type)::digits10
    // is less than twenty-eight.

    test_various_styles(-1234567890123456789012345678.0L, -1234567890123456789012345678.0L);
    test_various_styles(-9876543200000000000000000000.0L, -9876543200000000000000000000.0L);
    test_various_styles(-1000000000000000000000000000.0L, -1000000000000000000000000000.0L);
    test_various_styles(-100001.0L, -100001.0L);
    test_various_styles(-2.0L, -2.0L);
    test_various_styles(-1.0L, -1.0L);
    test_various_styles(-0.0L, -0.0L);
    test_various_styles( 0.0L,  0.0L);
    test_various_styles( 1.0L,  1.0L);
    test_various_styles( 2.0L,  2.0L);
    test_various_styles( 100001.0L,  100001.0L);
    test_various_styles( 1000000000000000000000000000.0L,  1000000000000000000000000000.0L);
    test_various_styles( 9876543200000000000000000000.0L,  9876543200000000000000000000.0L);
    test_various_styles( 1234567890123456789012345678.0L,  1234567890123456789012345678.0L);

    // Test some numbers that are representable in all IEEE formats,
    // but not necessarily as exact integers because they have
    // FLT_DIG or DBL_DIG significant digits (and mantissae chosen
    // so that no more digits are accurately representable).
    test_various_float_types(0, r_to_nearest, 987654.321L, 987654.0L);
    test_various_float_types(0, r_to_nearest, 987654321098765.375L,  987654321098765.0L);
    test_various_float_types(0, r_to_nearest, 987654321098765.500L,  987654321098766.0L);

    // Note: This test
//  test_various_float_types(0, r_to_nearest, 987654321098765.4321L,  987654321098765.0L);
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

    // It is anticipated that a rounding functor will typically be
    // created once and used many times, like this:
    round_to<double> const RoundToNearestHundredth(2, r_to_nearest);
    double sample_value = 1.0;
    for(int j = 0; j < 1000; j++)
        {
        sample_value *= 1.05;
        sample_value = RoundToNearestHundredth(sample_value);
        }
    // Most of this test suite is by its nature not typical in that sense.

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

    // Test default constructor.
    round_to<double> const round_erroneously;
    BOOST_TEST_THROW
        (round_erroneously(2.7)
        ,std::logic_error
        ,"Erroneous rounding function."
        );

    // Test copy constructor and copy assignment operator.
    round_to<double> const round0(2, r_to_nearest);
    BOOST_TEST(2 == round0.decimals());
    BOOST_TEST(r_to_nearest == round0.style());

    round_to<double> round1(round0);
    BOOST_TEST(2 == round1.decimals());
    BOOST_TEST(r_to_nearest == round1.style());

    round1 = round_to<double>(3, r_toward_zero);
    BOOST_TEST(3 == round1.decimals());
    BOOST_TEST(r_toward_zero == round1.style());

    round1 = round0;
    BOOST_TEST(2 == round1.decimals());
    BOOST_TEST(r_to_nearest == round1.style());

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

