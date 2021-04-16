// Numeric stinted cast, across whose bourn no value is returned--unit test.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

//#define TEST_BOOST_CAST_INSTEAD
#if defined TEST_BOOST_CAST_INSTEAD
#   include <boost/cast.hpp>
template<typename To, typename From>
inline To bourn_cast(From from)
{
    try
        {
        return boost::numeric_cast<To>(from);
        }
    catch(boost::numeric::positive_overflow)
        {
        throw std::runtime_error("Cast would transgress upper limit.");
        }
    catch(boost::numeric::negative_overflow)
        {
        using   to_traits = std::numeric_limits<To  >;
        using from_traits = std::numeric_limits<From>;
        if(from_traits::is_integer && !to_traits::is_signed && from < 0)
            throw std::runtime_error("Cannot cast negative to unsigned.");
        else
            throw std::runtime_error("Cast would transgress lower limit.");
        }
}
#else  // !defined TEST_BOOST_CAST_INSTEAD
#   include "bourn_cast.hpp"
#endif // !defined TEST_BOOST_CAST_INSTEAD

#include "miscellany.hpp"               // stifle_warning_for_unused_variable()
#include "stl_extensions.hpp"           // nonstd::power()
#include "test_tools.hpp"
#include "timer.hpp"

#include <climits>                      // INT_MIN, LLONG_MIN, SCHAR_MIN

/// Test trivial casts between identical types.

template<typename T>
void test_same(char const* file, int line)
{
    using traits = std::numeric_limits<T>;
    T upper = traits::max();
    T lower = traits::lowest();
    INVOKE_LMI_TEST(upper == bourn_cast<T>(upper), file, line);
    INVOKE_LMI_TEST(T( 1) == bourn_cast<T>(T( 1)), file, line);
    INVOKE_LMI_TEST(T( 0) == bourn_cast<T>(T( 0)), file, line);
    INVOKE_LMI_TEST(lower == bourn_cast<T>(lower), file, line);

    if(traits::is_signed)
        {
        INVOKE_LMI_TEST(T(-1) == bourn_cast<T>(T(-1)), file, line);
        }

    // Test whether integer limits are correctly calculated by this
    // std::scalbln() technique, so that it can be relied upon in
    // the bourn_cast floating-to-integral implementation. This
    // demonstration has been tested with 32- and 64-bit gcc, with
    // an 80-bit long double type whose 64-bit mantissa suffices to
    // test the limits of every integral type up to 64 digits exactly
    // because it can distinguish +/-(2^64) from +/-(2^64 - 1).
    if(traits::is_integer)
        {
        long double const x = std::scalbln(1.0l, traits::digits);
        long double const max = x - 1;
        long double const min = traits::is_signed ? -x : 0;
        INVOKE_LMI_TEST(traits::max() == max, file, line);
        INVOKE_LMI_TEST(traits::min() == min, file, line);
        T imax = bourn_cast<T>(max);
        T imin = bourn_cast<T>(min);
        INVOKE_LMI_TEST(traits::max() == imax, file, line);
        INVOKE_LMI_TEST(traits::min() == imin, file, line);
        }
}

/// Test casts involving two possibly different signednesses.

template<bool SignedTo, bool SignedFrom>
void test_signednesses(char const* file, int line)
{
    using CS = signed char;
    using IS = signed int;
    using LS = signed long long int;

    using CU = unsigned char;
    using IU = unsigned int;
    using LU = unsigned long long int;

    using CFrom = typename std::conditional<SignedFrom, CS, CU>::type;
    using IFrom = typename std::conditional<SignedFrom, IS, IU>::type;
    using LFrom = typename std::conditional<SignedFrom, LS, LU>::type;

    using CTo   = typename std::conditional<SignedTo  , CS, CU>::type;
    using ITo   = typename std::conditional<SignedTo  , IS, IU>::type;
    using LTo   = typename std::conditional<SignedTo  , LS, LU>::type;

    // For any pair of corresponding signed and unsigned integral
    // types, the maximum signed value is interconvertible. Thus,
    // corresponding elements of these two triplets have the same
    // value--and indeed the same bit representation--but different
    // types.

    CFrom CFrom_max = SCHAR_MAX;
    IFrom IFrom_max = INT_MAX;
    LFrom LFrom_max = LLONG_MAX;

    CTo   CTo_max   = SCHAR_MAX;
    ITo   ITo_max   = INT_MAX;
    LTo   LTo_max   = LLONG_MAX;

    // SCHAR_MAX must be at least 127, so 99 must be representable.

    // Both char.
    INVOKE_LMI_TEST_EQUAL(CTo( 0), bourn_cast<CTo>(CFrom( 0)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo( 1), bourn_cast<CTo>(CFrom( 1)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo(99), bourn_cast<CTo>(CFrom(99)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo_max, bourn_cast<CTo>(CFrom_max), file, line);

    // Both int.
    INVOKE_LMI_TEST_EQUAL(ITo( 0), bourn_cast<ITo>(IFrom( 0)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo( 1), bourn_cast<ITo>(IFrom( 1)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo(99), bourn_cast<ITo>(IFrom(99)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo_max, bourn_cast<ITo>(IFrom_max), file, line);

    // Both long long.
    INVOKE_LMI_TEST_EQUAL(LTo( 0), bourn_cast<LTo>(LFrom( 0)), file, line);
    INVOKE_LMI_TEST_EQUAL(LTo( 1), bourn_cast<LTo>(LFrom( 1)), file, line);
    INVOKE_LMI_TEST_EQUAL(LTo(99), bourn_cast<LTo>(LFrom(99)), file, line);
    INVOKE_LMI_TEST_EQUAL(LTo_max, bourn_cast<LTo>(LFrom_max), file, line);

    // To wider than From.
    INVOKE_LMI_TEST_EQUAL(CTo_max, bourn_cast<ITo>(CFrom_max), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo_max, bourn_cast<ITo>(CFrom_max), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo_max, bourn_cast<LTo>(IFrom_max), file, line);

    // From wider than To.
    INVOKE_LMI_TEST_EQUAL(CTo( 0), bourn_cast<CTo>(IFrom( 0)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo( 1), bourn_cast<CTo>(LFrom( 1)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo(99), bourn_cast<CTo>(LFrom(99)), file, line);

    if(!SignedFrom || !SignedTo) return;

    CFrom CFrom_min = SCHAR_MIN;
    IFrom IFrom_min = INT_MIN;
    LFrom LFrom_min = LLONG_MIN;

    CTo   CTo_min   = SCHAR_MIN;
    ITo   ITo_min   = INT_MIN;
    LTo   LTo_min   = LLONG_MIN;

    // SCHAR_MIN must be <= -127, so -9 must be representable.

    // Both char.
    INVOKE_LMI_TEST_EQUAL(CTo(-1), bourn_cast<CTo>(CFrom(-1)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(CFrom(-9)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo_min, bourn_cast<CTo>(CFrom_min), file, line);

    // Both int.
    INVOKE_LMI_TEST_EQUAL(ITo(-1), bourn_cast<ITo>(IFrom(-1)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo(-9), bourn_cast<ITo>(IFrom(-9)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo_min, bourn_cast<ITo>(IFrom_min), file, line);

    // Both long long.
    INVOKE_LMI_TEST_EQUAL(LTo(-1), bourn_cast<LTo>(LFrom(-1)), file, line);
    INVOKE_LMI_TEST_EQUAL(LTo(-9), bourn_cast<LTo>(LFrom(-9)), file, line);
    INVOKE_LMI_TEST_EQUAL(LTo_min, bourn_cast<LTo>(LFrom_min), file, line);

    // To wider than From.
    INVOKE_LMI_TEST_EQUAL(CTo_min, bourn_cast<ITo>(CFrom_min), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo_min, bourn_cast<ITo>(CFrom_min), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo_min, bourn_cast<LTo>(IFrom_min), file, line);

    // From wider than To.
    INVOKE_LMI_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(IFrom(-9)), file, line);
    INVOKE_LMI_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(LFrom(-9)), file, line);
    INVOKE_LMI_TEST_EQUAL(ITo(-9), bourn_cast<ITo>(LFrom(-9)), file, line);
}

/// Test floating-point conversions [conv.double].
///
/// Calling this for every combination of {float, double, long double}
/// means that any commutative test is performed twice, but making the
/// code more complex to avoid that is a poor idea because this entire
/// unit test takes only about a microsecond to run.

template<typename To, typename From>
void test_floating_conversions(char const* file, int line)
{
    using   to_traits = std::numeric_limits<To  >;
    using from_traits = std::numeric_limits<From>;

    static_assert(  to_traits::is_iec559);
    static_assert(from_traits::is_iec559);

    // std::isnormal values representable in any IEC559 'arithmetic
    // format' (i.e., excluding the binary16 'interchange format').

    From const largenum = nonstd::power(From(2), 100);
    From const smallnum = From(1) / largenum;
    INVOKE_LMI_TEST_EQUAL(largenum, bourn_cast<To>(largenum), file, line);
    INVOKE_LMI_TEST_EQUAL(smallnum, bourn_cast<To>(smallnum), file, line);

    // Normal min, max, and lowest.

    From const from_min = from_traits::min();
    From const from_max = from_traits::max();
    From const from_low = from_traits::lowest();

    if(from_traits::digits10 <= to_traits::digits10) // Widening or same.
        {
        INVOKE_LMI_TEST_EQUAL(from_min, bourn_cast<To>(from_min), file, line);
        INVOKE_LMI_TEST_EQUAL(from_max, bourn_cast<To>(from_max), file, line);
        INVOKE_LMI_TEST_EQUAL(from_low, bourn_cast<To>(from_low), file, line);
        }
    else // Narrowing.
        {
        INVOKE_LMI_TEST_EQUAL(To(0), bourn_cast<To>(from_min), file, line);
        LMI_TEST_THROW
            (bourn_cast<To>(from_max)
            ,std::runtime_error
            ,"Cast would transgress upper limit."
            );
        LMI_TEST_THROW
            (bourn_cast<To>(from_low)
            ,std::runtime_error
            ,"Cast would transgress lower limit."
            );
        }

    // Signed zeros.

    INVOKE_LMI_TEST_EQUAL(0.0, bourn_cast<To>( From(0)), file, line);
    INVOKE_LMI_TEST_EQUAL(0.0, bourn_cast<To>(-From(0)), file, line);
    INVOKE_LMI_TEST(!std::signbit(bourn_cast<To>( From(0))), file, line);
    INVOKE_LMI_TEST( std::signbit(bourn_cast<To>(-From(0))), file, line);

    // Infinities.

    To   const   to_inf =   to_traits::infinity();
    From const from_inf = from_traits::infinity();
#if !defined TEST_BOOST_CAST_INSTEAD
    INVOKE_LMI_TEST( std::isinf(bourn_cast<To>( from_inf)), file, line);
    INVOKE_LMI_TEST( std::isinf(bourn_cast<To>(-from_inf)), file, line);
    INVOKE_LMI_TEST(!std::signbit(bourn_cast<To>( from_inf)), file, line);
    INVOKE_LMI_TEST( std::signbit(bourn_cast<To>(-from_inf)), file, line);
    INVOKE_LMI_TEST_EQUAL( to_inf, bourn_cast<To>( from_inf), file, line);
    INVOKE_LMI_TEST_EQUAL(-to_inf, bourn_cast<To>(-from_inf), file, line);
#else  // defined TEST_BOOST_CAST_INSTEAD
    // Boost allows conversion of infinities to the same type or a
    // wider floating type, but not to a narrower type--presumably
    // because infinities are outside the [lowest(), max()] range but
    // non-narrowing conversions are presumptively allowed regardless
    // of value.
    if(from_traits::digits10 <= to_traits::digits10) // Widening or same.
        {
        INVOKE_LMI_TEST_EQUAL( to_inf, bourn_cast<To>( from_inf), file, line);
        INVOKE_LMI_TEST_EQUAL(-to_inf, bourn_cast<To>(-from_inf), file, line);
        }
    else
        {
        LMI_TEST_THROW
            (bourn_cast<To>( from_traits::infinity())
            ,std::runtime_error
            ,"This cast should have succeeded."
            );
        LMI_TEST_THROW
            (bourn_cast<To>(-from_traits::infinity())
            ,std::runtime_error
            ,"This cast should have succeeded."
            );
        }
#endif // defined TEST_BOOST_CAST_INSTEAD

    // NaNs.

    From const from_qnan = from_traits::quiet_NaN();
    INVOKE_LMI_TEST(std::isnan(bourn_cast<To>(from_qnan)), file, line);
}

/// Test conversions between integral and floating types [conv.fpint].

template<typename I, typename F>
void test_conv_fpint(char const* file, int line)
{
    using i_traits = std::numeric_limits<I>;
    using f_traits = std::numeric_limits<F>;

    static_assert(i_traits::is_integer);
    static_assert(f_traits::is_iec559);

    // Make sure 'digits' comparisons below are valid.
    static_assert(2 == i_traits::radix);
    static_assert(2 == f_traits::radix);

    // Integral to floating and back.

    I const i_hi = i_traits::max();
    F const f_i_hi = bourn_cast<F>(i_hi);

    if(i_traits::digits <= f_traits::digits)
        {
        INVOKE_LMI_TEST_EQUAL(i_hi, bourn_cast<I>(f_i_hi), file, line);
        }
    else
        {
        LMI_TEST_THROW
            (bourn_cast<I>(f_i_hi)
            ,std::runtime_error
            ,"Cast would transgress upper limit."
            );
        }

    I const i_lo = i_traits::lowest();
    F const f_i_lo = bourn_cast<F>(i_lo);

#if !defined TEST_BOOST_CAST_INSTEAD
    INVOKE_LMI_TEST_EQUAL(i_lo, bourn_cast<I>(f_i_lo), file, line);
#else  // defined TEST_BOOST_CAST_INSTEAD
    // boost::numeric_cast throws on conversions:
    // -9223372036854775808.0f --> 64-bit signed int
    //          -2147483648.0f --> 32-bit signed int
    // -9223372036854775808.0  --> 64-bit signed int
    if(!i_traits::is_signed || i_traits::digits <= f_traits::digits)
        {
        INVOKE_LMI_TEST_EQUAL(i_lo, bourn_cast<I>(f_i_lo), file, line);
        }
    else
        {
        LMI_TEST_THROW
            (bourn_cast<I>(f_i_lo)
            ,std::runtime_error
            ,"This cast should have succeeded."
            );
        }
#endif // defined TEST_BOOST_CAST_INSTEAD

    // Floating to integral.

    // Widening: generally not possible with standard arithmetic
    // types because long long int is 64 bits wide in practice, and
    // the exponent for float (IEEE 754 binary32) is in [-126, +127].

    // Narrowing.

    // An integer-valued floating-point number has no fractional part
    // to truncate, so converting it to an integral type wide enough
    // to represent it preserves value.
    INVOKE_LMI_TEST_EQUAL(I(3), bourn_cast<I>(F(3)), file, line);

    // From positive zero.
    INVOKE_LMI_TEST_EQUAL(I(0), bourn_cast<I>(+F(0)), file, line);

    // From negative zero. Interestingly, this negative value is
    // properly convertible to an unsigned integral type.
    INVOKE_LMI_TEST_EQUAL(I(0), bourn_cast<I>(-F(0)), file, line);

    // Out of bounds.

    // Floating-point lowest and highest values are not necessarily
    // outside the range of all integral types, but they almost
    // certainly are for standard types.
    LMI_TEST_THROW
        (bourn_cast<I>(f_traits::max())
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );
    LMI_TEST_THROW
        (bourn_cast<I>(f_traits::lowest())
        ,std::runtime_error
        ,   (i_traits::is_signed
            ? "Cast would transgress lower limit."
            : "Cannot cast negative to unsigned."
            )
        );

    // From +inf.
    LMI_TEST_THROW
        (bourn_cast<I>(+f_traits::infinity())
        ,std::runtime_error
        ,"Cannot cast infinite to integral."
        );

    // From -inf.
    LMI_TEST_THROW
        (bourn_cast<I>(-f_traits::infinity())
        ,std::runtime_error
        ,"Cannot cast infinite to integral."
        );

    // Otherwise disallowed.

    // Truncating.

#if !defined TEST_BOOST_CAST_INSTEAD
    LMI_TEST_THROW
        (bourn_cast<I>(F(3.14))
        ,std::runtime_error
        ,lmi_test::what_regex("^Cast.*would not preserve value\\.$")
        );
#else  // defined TEST_BOOST_CAST_INSTEAD
    // boost::numeric cast truncates whereas bourn_cast throws; both
    // are deliberate design decisions.
    INVOKE_LMI_TEST_EQUAL(3, bourn_cast<I>(F(3.14)), file, line);
#endif // defined TEST_BOOST_CAST_INSTEAD

    // From NaN.
    LMI_TEST_THROW
        (bourn_cast<I>(f_traits::quiet_NaN())
        ,std::runtime_error
        ,"Cannot cast NaN to integral."
        );
}

/// Test conversions between wide integral and narrow floating types.

void test_m64_neighborhood()
{
    using ull_traits = std::numeric_limits<unsigned long long int>;
    if(64 != ull_traits::digits)
        {
        std::cout
            << "test_m64_neighborhood() not run because"
            << "\nunsigned long long int is not a 64-bit type."
            << std::endl
            ;
        return;
        }

    // ULLONG_MAX must be at least 2^64 - 1 [C99 E/1], the 64th
    // Mersenne number, M64. Converting that number between types
    // float (IEEE 754 binary32) and unsigned long long int is
    // interesting because
    //   (2^64 - 1)ULL = 18446744073709551615 = M64     = 2^64 - 1
    //   (2^64 - 1)f   = 18446744073709551616 = M64 + 1 = 2^64
    // Cast either to the type of the other, and they compare equal,
    // at least with gcc-4.9.2, although casting 2^64 to a 64-bit
    // unsigned integer is UB.

    unsigned long long int const ull_max = ull_traits::max();
#if defined __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif // defined __clang__
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif // defined __GNUC__
    float const f_ull_max = ull_max;
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
#if defined __clang__
#   pragma clang diagnostic pop
#endif // defined __clang__
    LMI_TEST(f_ull_max == static_cast<float>(ull_max));
    // Suppressed because behavior is undefined:
    // LMI_TEST(ull_max == static_cast<unsigned long long int>(f_ull_max));

    // However, unlike static_cast, bourn_cast refuses to cast 2^64
    // to a 64-bit integer, because it is out of range and therefore
    // would constitute UB.

    LMI_TEST_EQUAL(f_ull_max, bourn_cast<float>(ull_max));
    LMI_TEST_THROW
        (bourn_cast<unsigned long long int>(f_ull_max)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    // To show that this case is not unique, test a value that is
    // lower by two.

    unsigned long long int const ull_hi = ull_traits::max() - 2; // 2^64 - 3

    float const f_ull_hi = bourn_cast<float>(ull_hi);
    LMI_TEST_THROW
        (bourn_cast<unsigned long long int>(f_ull_hi)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    // The same outcome is observed with a value that is lower by
    // about half a trillion units.

    double const d_2_64 = nonstd::power(2.0, 64);
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif // defined __GNUC__
    double const d_interesting = 0.5 * (d_2_64 + std::nextafterf(d_2_64, 0));
    unsigned long long int const ull_interesting = d_interesting;
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__
    float const f_interesting = bourn_cast<float>(ull_interesting);
    LMI_TEST_THROW
        (bourn_cast<unsigned long long int>(f_interesting)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );
    float const f_uninteresting = bourn_cast<float>(ull_interesting - 1ULL);
    bourn_cast<unsigned long long int>(f_uninteresting);

    // A similar cast must fail for IEEE 754 binary64, because its 53
    // mantissa bits cannot represent a value this close to 2^64.

    double const d_ull_hi = bourn_cast<double>(ull_hi);
    LMI_TEST_THROW
        (bourn_cast<unsigned long long int>(d_ull_hi)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    // However, the same cast succeeds when the floating-point type
    // has at least as much precision as the integral type.

    using ld_traits = std::numeric_limits<long double>;
    if(ull_traits::digits <= ld_traits::digits)
        {
        long double const ld_ull_hi = bourn_cast<long double>(ull_hi);
        LMI_TEST_EQUAL(ull_hi, bourn_cast<unsigned long long int>(ld_ull_hi));
        }

    // These circumstances for gcc on i686 or x86_64:
    //   64 = long double mantissa bits
    //   63 = signed long long int non-sign bits
    // clamor for a unit test. The extra bit in the significand lets
    // the floating type represent exact integers one greater in
    // magnitude than the integral type's limits. Adding one to the
    // maximum
    //   9223372036854775807 = 2^63 - 1 (sll_max below)
    // gives
    //   9223372036854775808 = 2^63     (ld_sll_too_high below)
    // reminiscent of 2^64 and its successor above. A more interesting
    // case is the integral minimum, which is an exact power of two in
    // magnitude:
    //   -9223372036854775808 = -2^63       (sll_min below)
    //   -9223372036854775809 = -(2^63 + 1) (ld_sll_too_low below)

    using sll_traits = std::numeric_limits<signed long long int>;
    if(sll_traits::digits < ld_traits::digits)
        {
        signed long long int const sll_max = sll_traits::max();

        long double const ld_sll_max = bourn_cast<long double>(sll_max);
        LMI_TEST_EQUAL(sll_max, bourn_cast<signed long long int>(ld_sll_max));

        long double const ld_sll_too_high = ld_sll_max + 1;
        LMI_TEST_THROW
            (bourn_cast<signed long long int>(ld_sll_too_high)
            ,std::runtime_error
            ,"Cast would transgress upper limit."
            );

        signed long long int const sll_min = sll_traits::min();

        long double const ld_sll_min = bourn_cast<long double>(sll_min);
        LMI_TEST_EQUAL(sll_min, bourn_cast<signed long long int>(ld_sll_min));

        long double const ld_sll_too_low = ld_sll_min - 1;
        LMI_TEST_THROW
            (bourn_cast<signed long long int>(ld_sll_too_low)
            ,std::runtime_error
            ,"Cast would transgress lower limit."
            );
        }
}

/// Test boost::numeric_cast anomalies reported here:
///   https://lists.nongnu.org/archive/html/lmi/2017-03/msg00127.html
/// All these tests fail with boost-1.33.1 and gcc-4.9 '-O2' on
///   x86_64-linux-gnu
///   i686-linux-gnu
///   i686-w64-mingw32
/// and also with boost-1.62 and x86_64-linux-gnu as confirmed here:
///   https://lists.nongnu.org/archive/html/lmi/2017-03/msg00128.html

void test_boost_anomalies()
{
    using double_traits = std::numeric_limits<double>;

    // IEEE 754-2008 [5.8, conversion to integer]: "When a NaN or infinite
    // operand cannot be represented in the destination format and this
    // cannot otherwise be indicated, the invalid operation exception shall
    // be signaled."
    LMI_TEST_THROW
        (bourn_cast<int>(double_traits::quiet_NaN())
        ,std::runtime_error
        ,"Cannot cast NaN to integral."
        );

    // IEEE 754-2008 [6.1]: "Operations on infinite operands are usually
    // exact and therefore signal no exceptions, including ... conversion of
    // an infinity into the same infinity in another format."
    try
        {
        bourn_cast<long double>(double_traits::infinity());
        // That worked, so this should too...
        bourn_cast<float>(double_traits::infinity());
        // ...because all infinities are convertible.
        LMI_TEST(true);
        }
    catch(...) {LMI_TEST(false);}

    try
        {
        bourn_cast<int>(INT_MIN);
        bourn_cast<int>((double)INT_MIN);
        // That worked, so this should too...
        bourn_cast<int>((float)INT_MIN);
        // ...because INT_MIN = an exact power of 2.
        LMI_TEST(true);
        }
    catch(...) {LMI_TEST(false);}

    try
        {
        bourn_cast<long long int>((long double)LLONG_MIN);
        // That worked, so this should too...
        bourn_cast<long long int>((float)LLONG_MIN);
        // ...because LLONG_MIN = an exact power of 2.
        LMI_TEST(true);
        }
    catch(...) {LMI_TEST(false);}

    try
        {
        bourn_cast<long long int>((long double)LLONG_MIN);
        // That worked, so this should too...
        bourn_cast<long long int>((double)LLONG_MIN);
        // ...because LLONG_MIN = an exact power of 2.
        LMI_TEST(true);
        }
    catch(...) {LMI_TEST(false);}
}

/// Speed test: convert one million times, using static_cast.

template<typename To, typename From>
void mete_static()
{
    enum {N = 1000000};
    using from_traits = std::numeric_limits<From>;
    static_assert(from_traits::is_specialized);
    static_assert(N < from_traits::max());
    To volatile z(0);
    for(From j = 0; j < N; ++j)
        {
        z = static_cast<To>(j);
        }
    stifle_warning_for_unused_variable(z);
}

/// Speed test: convert one million times, using bourn_cast.

template<typename To, typename From>
void mete_bourn()
{
    enum {N = 1000000};
    using from_traits = std::numeric_limits<From>;
    static_assert(from_traits::is_specialized);
    static_assert(N < from_traits::max());
    To volatile z(0);
    for(From j = 0; j < N; ++j)
        {
        z = bourn_cast<To>(j);
        }
    stifle_warning_for_unused_variable(z);
}

void assay_speed()
{
    using D  =            double;
    using F  =             float;
    using SL =   signed long int;
    using UL = unsigned long int;
    std::cout
        << "\n  Speed tests (Double, Float, Signed, Unsigned):"
        << "\n"
        << "\n  static_cast<U>(S): " << TimeAnAliquot(mete_static<UL,SL>)
        << "\n   bourn_cast<U>(S): " << TimeAnAliquot(mete_bourn <UL,SL>)
        << "\n   bourn_cast<S>(U): " << TimeAnAliquot(mete_bourn <SL,UL>)
        << "\n"
        << "\n  static_cast<D>(U): " << TimeAnAliquot(mete_static<D,UL>)
        << "\n   bourn_cast<D>(U): " << TimeAnAliquot(mete_bourn <D,UL>)
        << "\n"
        << "\n  static_cast<U>(D): " << TimeAnAliquot(mete_static<UL,D>)
        << "\n   bourn_cast<U>(D): " << TimeAnAliquot(mete_bourn <UL,D>)
        << "\n   bourn_cast<S>(D): " << TimeAnAliquot(mete_bourn <SL,D>)
        << "\n"
        << "\n  static_cast<F>(D): " << TimeAnAliquot(mete_static<F,D>)
        << "\n   bourn_cast<F>(D): " << TimeAnAliquot(mete_bourn <F,D>)
        << "\n   bourn_cast<D>(F): " << TimeAnAliquot(mete_bourn <D,F>)
        << std::endl
        ;
}

int test_main(int, char*[])
{
    // Motivating case. This test fails: -1 really isn't less than 1U.
    // (The test is suppressed to avoid a compiler warning.)
//  LMI_TEST(-1 < 1U);
    // This test succeeds: -1 is less than 1, as expected.
    LMI_TEST(-1 < bourn_cast<int>(1U));
    // This test throws: instead of converting a negative value to
    // unsigned, bourn_cast throws an exception.
//  LMI_TEST(bourn_cast<unsigned int>(-1) < 1U);

    // Trivially cast to same type.

    test_same<bool>                  (__FILE__, __LINE__);
    test_same<char>                  (__FILE__, __LINE__);
    test_same<signed char>           (__FILE__, __LINE__);
    test_same<unsigned char>         (__FILE__, __LINE__);
    test_same<char16_t>              (__FILE__, __LINE__);
    test_same<char32_t>              (__FILE__, __LINE__);
    test_same<wchar_t>               (__FILE__, __LINE__);
    test_same<short int>             (__FILE__, __LINE__);
    test_same<int>                   (__FILE__, __LINE__);
    test_same<long int>              (__FILE__, __LINE__);
    test_same<long long int>         (__FILE__, __LINE__);
    test_same<unsigned short int>    (__FILE__, __LINE__);
    test_same<unsigned int>          (__FILE__, __LINE__);
    test_same<unsigned long int>     (__FILE__, __LINE__);
    test_same<unsigned long long int>(__FILE__, __LINE__);
    test_same<float>                 (__FILE__, __LINE__);
    test_same<double>                (__FILE__, __LINE__);
    test_same<long double>           (__FILE__, __LINE__);

    // Cast between bool and int. C++11 [18.3.2.7/3] specifies that
    // std::numeric_limits<bool>is_signed is false, so the types
    // {bool, signed char} must have opposite signedness and different
    // [lowest(), max()] ranges. Therefore, the tests in this block
    // are guaranteed to cover such diversity, even on a machine where
    // unsigned char and unsigned long long int are synonyms.

    LMI_TEST_EQUAL(true , bourn_cast<bool>(static_cast<signed char>(1)));
    LMI_TEST_EQUAL(false, bourn_cast<bool>(static_cast<signed char>(0)));

    LMI_TEST_THROW
        (bourn_cast<bool>(static_cast<signed char>(2))
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    LMI_TEST_THROW
        (bourn_cast<bool>(static_cast<signed char>(-1))
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Cast from signed to unsigned.

    test_signednesses<true,false>(__FILE__, __LINE__);

    // Cast from unsigned to signed.

    test_signednesses<false,true>(__FILE__, __LINE__);

    // Cast from signed to signed.

    test_signednesses<true,true>(__FILE__, __LINE__);

    // Cast from unsigned to unsigned.

    test_signednesses<false,false>(__FILE__, __LINE__);

    // Cast between floating types.

    test_floating_conversions<float      , float      >(__FILE__, __LINE__);
    test_floating_conversions<float      , double     >(__FILE__, __LINE__);
    test_floating_conversions<float      , long double>(__FILE__, __LINE__);
    test_floating_conversions<double     , float      >(__FILE__, __LINE__);
    test_floating_conversions<double     , double     >(__FILE__, __LINE__);
    test_floating_conversions<double     , long double>(__FILE__, __LINE__);
    test_floating_conversions<long double, float      >(__FILE__, __LINE__);
    test_floating_conversions<long double, double     >(__FILE__, __LINE__);
    test_floating_conversions<long double, long double>(__FILE__, __LINE__);

    // Cast between floating and integral types.

    test_conv_fpint<unsigned long long int,       float>(__FILE__, __LINE__);
    test_conv_fpint<  signed long long int,       float>(__FILE__, __LINE__);
    test_conv_fpint<unsigned           int,       float>(__FILE__, __LINE__);
    test_conv_fpint<  signed           int,       float>(__FILE__, __LINE__);
    test_conv_fpint<unsigned          char,       float>(__FILE__, __LINE__);
    test_conv_fpint<  signed          char,       float>(__FILE__, __LINE__);

    test_conv_fpint<unsigned long long int,      double>(__FILE__, __LINE__);
    test_conv_fpint<  signed long long int,      double>(__FILE__, __LINE__);
    test_conv_fpint<unsigned           int,      double>(__FILE__, __LINE__);
    test_conv_fpint<  signed           int,      double>(__FILE__, __LINE__);
    test_conv_fpint<unsigned          char,      double>(__FILE__, __LINE__);
    test_conv_fpint<  signed          char,      double>(__FILE__, __LINE__);

    test_conv_fpint<unsigned long long int, long double>(__FILE__, __LINE__);
    test_conv_fpint<  signed long long int, long double>(__FILE__, __LINE__);
    test_conv_fpint<unsigned           int, long double>(__FILE__, __LINE__);
    test_conv_fpint<  signed           int, long double>(__FILE__, __LINE__);
    test_conv_fpint<unsigned          char, long double>(__FILE__, __LINE__);
    test_conv_fpint<  signed          char, long double>(__FILE__, __LINE__);

    // Test a peculiarly ill-conditioned range.

    test_m64_neighborhood();

    // Attempt forbidden conversion from negative to unsigned.

    LMI_TEST_THROW
        (bourn_cast<unsigned char>(std::numeric_limits<signed char>::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    LMI_TEST_THROW
        (bourn_cast<unsigned int >(std::numeric_limits<signed int >::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Still forbidden even if unsigned type is wider than signed type.
    LMI_TEST_THROW
        (bourn_cast<unsigned long int>(std::numeric_limits<signed char>::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Still forbidden even if value is only "slightly" negative.
    LMI_TEST_THROW
        (bourn_cast<unsigned long int>(-1)
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Transgress lower limit. It is not possible to write a unit test
    // that is guaranteed to throw this particular exception, because
    // the present bourn_cast<>() implementation tests first for
    // attempted conversion of a negative value to an unsigned type.

#if LLONG_MIN < SCHAR_MIN
    LMI_TEST_THROW
        (bourn_cast<signed char>(LLONG_MIN)
        ,std::runtime_error
        ,"Cast would transgress lower limit."
        );
#endif // LLONG_MIN < SCHAR_MIN

    // Transgress upper limit.

#if UCHAR_MAX < ULLONG_MAX
    LMI_TEST_THROW
        (bourn_cast<unsigned char>(ULLONG_MAX)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );
#endif // UCHAR_MAX < ULLONG_MAX

    LMI_TEST_THROW
        (bourn_cast<signed char>(std::numeric_limits<unsigned char>::max())
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    LMI_TEST_THROW
        (bourn_cast<signed int >(std::numeric_limits<unsigned int >::max())
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    // Test boost::numeric_cast anomalies.

    test_boost_anomalies();

    // Time representative casts.

    assay_speed();

    return EXIT_SUCCESS;
}
