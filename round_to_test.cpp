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

#include "pchfile.hpp"

#include "round_to.hpp"

#include "bin_exp.hpp"
#include "currency.hpp"                 // currency::cents_digits
#include "fenv_lmi.hpp"
#include "miscellany.hpp"               // floating_rep(), scoped_ios_format
#include "test_tools.hpp"

#include <algorithm>                    // max()
#include <cfloat>                       // DECIMAL_DIG
#include <climits>                      // INT_MIN
#include <cmath>                        // fabs()
#include <ios>
#include <iostream>
#include <limits>
#include <ostream>
#include <stdexcept>

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

class round_to_test
{
  public:
    static void test();

  private:
    static void test_scaling();
    static void test_fundamentals();
    static void test_all_modes(bool synchronize);
    static void test_rounding();
    static void test_various_styles
        (long double    unrounded
        ,long double    expected
        );
    static void test_various_decimals
        (rounding_style style
        ,long double    unrounded
        ,long double    expected
        );
    static void test_various_float_types
        (int            decimals
        ,rounding_style style
        ,long double    unrounded
        ,long double    expected
        );

    template<typename RealType>
    static bool test_one_case
        (RealType       unrounded
        ,RealType       expected
        ,int            decimals
        ,rounding_style style
        );
};

template<typename RealType>
bool round_to_test::test_one_case
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
        // max-precision-real type, as shown in a comment?
        // But consider using std::nextafter instead of (1+epsilon).
        RealType const unity = 1;
        tolerance =
               (unity + std::numeric_limits<RealType>::epsilon())
             * (unity + std::numeric_limits<RealType>::epsilon())
//           * (unity + std::numeric_limits<max_prec_real>::epsilon())
             - unity
             ;
        }
#if defined LMI_COMO_WITH_MINGW
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
        scoped_ios_format meaningless_name(std::cout);
        std::cout << '\n';
        std::cout.precision(DECIMAL_DIG);

        std::cout
            << "Rounding   "     << get_name_of_float_type<RealType>()
            << unrounded
            << "\n  to "         << decimals << " decimals"
            << "\n  with style " << get_name_of_style(style)
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

void round_to_test::test_various_float_types
    (int            decimals
    ,rounding_style style
    ,long double    unrounded
    ,long double    expected
    )
{
    int const inverse_decimals = -decimals;
    // The intention is to avoid taking the reciprocal of a reciprocal,
    // but 'f0' and 'f1' appear to be equivalent, so perhaps that idea
    // is too precious.
    long double const f0 =        bin_exp(10.0L,  inverse_decimals);
    long double const f1 = 1.0L / bin_exp(10.0L, -inverse_decimals);
    LMI_TEST_EQUAL(f0, f1);
    long double const factor = (0 <= inverse_decimals) ? f0 : f1;
    long double const u = unrounded * factor;
    long double const e = expected  * factor;
    LMI_TEST((test_one_case(static_cast<float >(u), static_cast<float >(e), decimals, style)));
    LMI_TEST((test_one_case(static_cast<double>(u), static_cast<double>(e), decimals, style)));
    LMI_TEST((test_one_case(/* long double */  (u), /* long double */  (e), decimals, style)));
}

// Test rounding to various numbers of decimal places.
void round_to_test::test_various_decimals
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
void round_to_test::test_various_styles
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

void round_to_test::test_rounding()
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

void round_to_test::test_all_modes(bool synchronize)
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
    for(int j = 0; j < 1000; ++j)
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
}

void round_to_test::test_scaling()
{
    scoped_ios_format meaningless_name(std::cout);
    double const volatile d0 = 2.71828'18284'59045'23536;
    double const lo = nextafter(d0, -INFINITY);
    double const hi = nextafter(d0,  INFINITY);

    double const volatile d1 = (d0 * 1.0e8) / 1.0e8;
    double const volatile dreciprocal = 1.0 / 1.0e8;
    double const volatile d2 = (d0 * 1.0e8) * dreciprocal;

    double const volatile d3 = static_cast<double>((d0 * 1.0e8L) / 1.0e8L);
    long double const volatile lreciprocal = 1.0L / 1.0e8L;
    double const volatile d4 = static_cast<double>((d0 * 1.0e8L) * lreciprocal);

    std::cout.precision(DECIMAL_DIG);

    std::cout
        << lo << std::hexfloat << '\t' << lo << std::defaultfloat << " lo\n"
        << d0 << std::hexfloat << '\t' << d0 << std::defaultfloat << " d0\n"
        << hi << std::hexfloat << '\t' << hi << std::defaultfloat << " hi\n"
        << d1 << std::hexfloat << '\t' << d1 << std::defaultfloat << " d1\n"
        << d2 << std::hexfloat << '\t' << d2 << std::defaultfloat << " d2\n"
        << d3 << std::hexfloat << '\t' << d3 << std::defaultfloat << " d3\n"
        << d4 << std::hexfloat << '\t' << d4 << std::defaultfloat << " d4\n"
        ;

    std::cout << std::endl;
}

void round_to_test::test_fundamentals()
{
    default_rounding_style() = r_indeterminate;

    // Test default constructor.
    round_to<double> const round_erroneously;
    LMI_TEST_THROW
        (round_erroneously(2.7)
        ,std::logic_error
        ,"Erroneous rounding function."
        );

    // Test copy constructor and copy assignment operator.
    round_to<double> const round0(2, r_to_nearest);
    LMI_TEST(2 == round0.decimals());
    LMI_TEST(r_to_nearest == round0.style());

    round_to<double> round1(round0);
    LMI_TEST(2 == round1.decimals());
    LMI_TEST(r_to_nearest == round1.style());

    round1 = round_to<double>(3, r_toward_zero);
    LMI_TEST(3 == round1.decimals());
    LMI_TEST(r_toward_zero == round1.style());

    round1 = round0;
    LMI_TEST(2 == round1.decimals());
    LMI_TEST(r_to_nearest == round1.style());

    // Test rounding double to currency.
    currency c = round0.c(1.61803398875);
    LMI_TEST((1.62 - dblize(c)) < 1e-14);
    LMI_TEST_EQUAL(162, c.cents());
//  c *= 0.61803398875;
//  LMI_TEST_EQUAL(1, c);

    // Test a vector.
    std::vector<double> const v0 {3.1415926535, 2.718281828};
    std::vector<double> const v1 {round0(v0)};
    LMI_TEST_EQUAL(v0.size(), v1.size());
    LMI_TEST((3.14 - v1[0]) < 1e-14);
    LMI_TEST((2.72 - v1[1]) < 1e-14);

    // Try to provoke division by zero in ctor-initializer.
    //
    // bin_exp() negates a negative exponent, but negating INT_MIN
    // constitutes UB, so add one, plus currency::cents_digits because
    // of the interplay between classes currency and round_to.
    LMI_TEST_THROW
        (round_to<double>(1 + currency::cents_digits + INT_MIN, r_to_nearest)
        ,std::domain_error
        ,"Invalid number of decimals."
        );
}

void round_to_test::test()
{
    test_scaling();
    test_fundamentals();

    // The software default rounding style and the hardware rounding
    // mode may be either synchronized or not, so test both ways.
    std::cout << "  Default style synchronized to hardware mode:\n";
    test_all_modes(true);
    std::cout << "  Default style NOT synchronized to hardware mode:\n";
    test_all_modes(false);
}

int test_main(int, char*[])
{
    round_to_test::test();

    return EXIT_SUCCESS;
}
