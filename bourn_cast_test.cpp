// Numeric stinted cast, across whose bourn no value is returned--unit test.
//
// Copyright (C) 2017 Gregory W. Chicares.
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
#include <type_traits>                  // std::conditional

/// Test trivial casts between identical types.

template<typename T>
void test_same(char const* file, int line)
{
    using limits = std::numeric_limits<T>;
    T upper = limits::max();
    T lower = limits::lowest();
    INVOKE_BOOST_TEST_EQUAL(upper, bourn_cast<T>(upper), file, line);
    INVOKE_BOOST_TEST_EQUAL(T( 1), bourn_cast<T>(T( 1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(T( 0), bourn_cast<T>(T( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(lower, bourn_cast<T>(lower), file, line);
    if(limits::is_signed)
        {
        INVOKE_BOOST_TEST_EQUAL(T(-1), bourn_cast<T>(T(-1)), file, line);
        }
}

/// Test casts involving two possibly different signednesses.

template<bool SignedFrom, bool SignedTo>
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
    INVOKE_BOOST_TEST_EQUAL(CTo( 0), bourn_cast<CTo>(CFrom( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo( 1), bourn_cast<CTo>(CFrom( 1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo(99), bourn_cast<CTo>(CFrom(99)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo_max, bourn_cast<CTo>(CFrom_max), file, line);

    // Both int.
    INVOKE_BOOST_TEST_EQUAL(ITo( 0), bourn_cast<ITo>(IFrom( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo( 1), bourn_cast<ITo>(IFrom( 1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo(99), bourn_cast<ITo>(IFrom(99)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo_max, bourn_cast<ITo>(IFrom_max), file, line);

    // Both long long.
    INVOKE_BOOST_TEST_EQUAL(LTo( 0), bourn_cast<LTo>(LFrom( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(LTo( 1), bourn_cast<LTo>(LFrom( 1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(LTo(99), bourn_cast<LTo>(LFrom(99)), file, line);
    INVOKE_BOOST_TEST_EQUAL(LTo_max, bourn_cast<LTo>(LFrom_max), file, line);

    // To wider than From.
    INVOKE_BOOST_TEST_EQUAL(CTo_max, bourn_cast<ITo>(CFrom_max), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo_max, bourn_cast<ITo>(CFrom_max), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo_max, bourn_cast<LTo>(IFrom_max), file, line);

    // From wider than To.
    INVOKE_BOOST_TEST_EQUAL(CTo( 0), bourn_cast<CTo>(IFrom( 0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo( 1), bourn_cast<CTo>(LFrom( 1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo(99), bourn_cast<CTo>(LFrom(99)), file, line);

    if(!SignedFrom || !SignedTo) return;

    CFrom CFrom_min = SCHAR_MIN;
    IFrom IFrom_min = INT_MIN;
    LFrom LFrom_min = LLONG_MIN;

    CTo   CTo_min   = SCHAR_MIN;
    ITo   ITo_min   = INT_MIN;
    LTo   LTo_min   = LLONG_MIN;

    // SCHAR_MIN must be <= -127, so -9 must be representable.

    // Both char.
    INVOKE_BOOST_TEST_EQUAL(CTo(-1), bourn_cast<CTo>(CFrom(-1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(CFrom(-9)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo_min, bourn_cast<CTo>(CFrom_min), file, line);

    // Both int.
    INVOKE_BOOST_TEST_EQUAL(ITo(-1), bourn_cast<ITo>(IFrom(-1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo(-9), bourn_cast<ITo>(IFrom(-9)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo_min, bourn_cast<ITo>(IFrom_min), file, line);

    // Both long long.
    INVOKE_BOOST_TEST_EQUAL(LTo(-1), bourn_cast<LTo>(LFrom(-1)), file, line);
    INVOKE_BOOST_TEST_EQUAL(LTo(-9), bourn_cast<LTo>(LFrom(-9)), file, line);
    INVOKE_BOOST_TEST_EQUAL(LTo_min, bourn_cast<LTo>(LFrom_min), file, line);

    // To wider than From.
    INVOKE_BOOST_TEST_EQUAL(CTo_min, bourn_cast<ITo>(CFrom_min), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo_min, bourn_cast<ITo>(CFrom_min), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo_min, bourn_cast<LTo>(IFrom_min), file, line);

    // From wider than To.
    INVOKE_BOOST_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(IFrom(-9)), file, line);
    INVOKE_BOOST_TEST_EQUAL(CTo(-9), bourn_cast<CTo>(LFrom(-9)), file, line);
    INVOKE_BOOST_TEST_EQUAL(ITo(-9), bourn_cast<ITo>(LFrom(-9)), file, line);
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

    static_assert(  to_traits::is_iec559, "");
    static_assert(from_traits::is_iec559, "");

    // std::isnormal values representable in any IEC559 'arithmetic
    // format' (i.e., excluding the binary16 'interchange format').

    From const largenum = nonstd::power(From(2), 100);
    From const smallnum = From(1) / largenum;
    INVOKE_BOOST_TEST_EQUAL(largenum, bourn_cast<To>(largenum), file, line);
    INVOKE_BOOST_TEST_EQUAL(smallnum, bourn_cast<To>(smallnum), file, line);

    // Normal min, max, and lowest.

    From const from_min = from_traits::min();
    From const from_max = from_traits::max();
    From const from_low = from_traits::lowest();

    if(from_traits::digits10 <= to_traits::digits10) // Widening or same.
        {
        INVOKE_BOOST_TEST_EQUAL(from_min, bourn_cast<To>(from_min), file, line);
        INVOKE_BOOST_TEST_EQUAL(from_max, bourn_cast<To>(from_max), file, line);
        INVOKE_BOOST_TEST_EQUAL(from_low, bourn_cast<To>(from_low), file, line);
        }
    else // Narrowing.
        {
        INVOKE_BOOST_TEST_EQUAL(To(0), bourn_cast<To>(from_min), file, line);
        BOOST_TEST_THROW
            (bourn_cast<To>(from_max)
            ,std::runtime_error
            ,"Cast would transgress upper limit."
            );
        BOOST_TEST_THROW
            (bourn_cast<To>(from_low)
            ,std::runtime_error
            ,"Cast would transgress lower limit."
            );
        }

    // Signed zeros.

    INVOKE_BOOST_TEST_EQUAL(0.0, bourn_cast<To>( From(0)), file, line);
    INVOKE_BOOST_TEST_EQUAL(0.0, bourn_cast<To>(-From(0)), file, line);
    INVOKE_BOOST_TEST(!std::signbit(bourn_cast<To>( From(0))), file, line);
    INVOKE_BOOST_TEST( std::signbit(bourn_cast<To>(-From(0))), file, line);

    // Infinities.

    To   const   to_inf =   to_traits::infinity();
    From const from_inf = from_traits::infinity();
#if !defined TEST_BOOST_CAST_INSTEAD
    INVOKE_BOOST_TEST( std::isinf(bourn_cast<To>( from_inf)), file, line);
    INVOKE_BOOST_TEST( std::isinf(bourn_cast<To>(-from_inf)), file, line);
    INVOKE_BOOST_TEST(!std::signbit(bourn_cast<To>( from_inf)), file, line);
    INVOKE_BOOST_TEST( std::signbit(bourn_cast<To>(-from_inf)), file, line);
    INVOKE_BOOST_TEST_EQUAL( to_inf, bourn_cast<To>( from_inf), file, line);
    INVOKE_BOOST_TEST_EQUAL(-to_inf, bourn_cast<To>(-from_inf), file, line);
#else  // defined TEST_BOOST_CAST_INSTEAD
    // Boost doesn't allow conversion of infinities to a narrower
    // floating type, presumably because is presumptively permits
    // conversion to the same or a wider floating type.
    if(from_traits::digits10 <= to_traits::digits10) // Widening or same.
        {
        INVOKE_BOOST_TEST_EQUAL( to_inf, bourn_cast<To>( from_inf), file, line);
        INVOKE_BOOST_TEST_EQUAL(-to_inf, bourn_cast<To>(-from_inf), file, line);
        }
    else
        {
        INVOKE_BOOST_TEST(false, file, line); // This should be allowed.
        BOOST_TEST_THROW
            (bourn_cast<To>( from_traits::infinity())
            ,std::runtime_error
            ,"Cast would transgress upper limit."
            );
        BOOST_TEST_THROW
            (bourn_cast<To>(-from_traits::infinity())
            ,std::runtime_error
            ,"Cast would transgress lower limit."
            );
        }
#endif // defined TEST_BOOST_CAST_INSTEAD

    // NaNs.

    From const from_qnan = from_traits::quiet_NaN();
    INVOKE_BOOST_TEST(std::isnan(bourn_cast<To>(from_qnan)), file, line);
}

/// Test boost::numeric_cast anomalies reported here:
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00127.html
/// and confirmed here:
///   http://lists.nongnu.org/archive/html/lmi/2017-03/msg00128.html

void test_boost_anomalies()
{
    using double_limits = std::numeric_limits<double>;

    // IEEE 754-2008 [5.8, conversion to integer]: "When a NaN or infinite
    // operand cannot be represented in the destination format and this
    // cannot otherwise be indicated, the invalid operation exception shall
    // be signaled."
    BOOST_TEST_THROW
        (bourn_cast<int>(double_limits::quiet_NaN())
        ,std::runtime_error
        ,"Cannot cast NaN to integral."
        );

    // IEEE 754-2008 [6.1]: "Operations on infinite operands are usually
    // exact and therefore signal no exceptions, including ... conversion of
    // an infinity into the same infinity in another format."
    try
        {
        bourn_cast<long double>(double_limits::infinity());
        // That worked, so this should too...
        bourn_cast<float>(double_limits::infinity());
        // ...because all infinities are convertible.
        BOOST_TEST(true);
        }
    catch(...) {BOOST_TEST(false);}

    try
        {
        bourn_cast<int>(INT_MIN);
        bourn_cast<int>((double)INT_MIN);
        // That worked, so this should too...
        bourn_cast<int>((float)INT_MIN);
        // ...because INT_MIN = an exact power of 2.
        BOOST_TEST(true);
        }
    catch(...) {BOOST_TEST(false);}

    try
        {
        bourn_cast<long long int>((long double)LLONG_MIN);
        // That worked, so this should too...
        bourn_cast<long long int>((float)LLONG_MIN);
        // ...because LLONG_MIN = an exact power of 2.
        BOOST_TEST(true);
        }
    catch(...) {BOOST_TEST(false);}

    try
        {
        bourn_cast<long long int>((long double)LLONG_MIN);
        // That worked, so this should too...
        bourn_cast<long long int>((double)LLONG_MIN);
        // ...because LLONG_MIN = an exact power of 2.
        BOOST_TEST(true);
        }
    catch(...) {BOOST_TEST(false);}
}

/// Speed test: convert one million times, using static_cast.

template<typename To, typename From>
void mete_static()
{
    enum {N = 1000000};
    using from_traits = std::numeric_limits<From>;
    static_assert(from_traits::is_specialized, "");
    static_assert(N < from_traits::max(), "");
    volatile To z(0);
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
    static_assert(from_traits::is_specialized, "");
    static_assert(N < from_traits::max(), "");
    volatile To z(0);
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
//  BOOST_TEST(-1 < 1U);
    // This test succeeds: -1 is less than 1, as expected.
    BOOST_TEST(-1 < bourn_cast<int>(1U));
    // This test throws: instead of converting a negative value to
    // unsigned, bourn_cast throws an exception.
//  BOOST_TEST(bourn_cast<unsigned int>(-1) < 1U);

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

    BOOST_TEST_EQUAL(true , bourn_cast<bool>((signed char)(1)));
    BOOST_TEST_EQUAL(false, bourn_cast<bool>((signed char)(0)));

    BOOST_TEST_THROW
        (bourn_cast<bool>((signed char)(2))
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    BOOST_TEST_THROW
        (bourn_cast<bool>((signed char)(-1))
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

    // Attempt forbidden conversion from negative to unsigned.

    BOOST_TEST_THROW
        (bourn_cast<unsigned char>(std::numeric_limits<signed char>::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    BOOST_TEST_THROW
        (bourn_cast<unsigned int >(std::numeric_limits<signed int >::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Still forbidden even if unsigned type is wider.
    BOOST_TEST_THROW
        (bourn_cast<unsigned long>(std::numeric_limits<signed char>::lowest())
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Still forbidden even if value is only "slightly" negative.
    BOOST_TEST_THROW
        (bourn_cast<unsigned long>(-1)
        ,std::runtime_error
        ,"Cannot cast negative to unsigned."
        );

    // Transgress lower limit. It is not possible to write a unit test
    // that is guaranteed to throw this particular exception, because
    // the present bourn_cast<>() implementation tests first for
    // attempted conversion of a negative value to an unsigned type.

#if LLONG_MIN < SCHAR_MIN
    BOOST_TEST_THROW
        (bourn_cast<signed char>(LLONG_MIN)
        ,std::runtime_error
        ,"Cast would transgress lower limit."
        );
#endif // LLONG_MIN < SCHAR_MIN

    // Transgress upper limit.

#if UCHAR_MAX < ULLONG_MAX
    BOOST_TEST_THROW
        (bourn_cast<unsigned char>(ULLONG_MAX)
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );
#endif // UCHAR_MAX < ULLONG_MAX

    BOOST_TEST_THROW
        (bourn_cast<signed char>(std::numeric_limits<unsigned char>::max())
        ,std::runtime_error
        ,"Cast would transgress upper limit."
        );

    BOOST_TEST_THROW
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

