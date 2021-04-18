// Reformed std::size() returning a signed integer: unit test.
//
// Copyright (C) 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ssize_lmi.hpp"

#include "bourn_cast.hpp"
#include "rtti_lmi.hpp"
#include "test_tools.hpp"

#include <array>
#include <climits>                      // CHAR_MAX
#include <cstring>                      // strlen()
#include <iterator>                     // size()
#include <string>
#include <vector>

/// Experimental investigation of array-bound deduction.
///
/// An array-bound parameter can be deduced as a signed integer: the
/// deduced value need not be unsigned. See C++17 (N4659) [17.8.2.1]
/// (i.e., [temp.deduc.call]):
///
/// | template<class T, int N> void h(T const(&)[N]);
/// | h({1,2,3}); // T deduced to int, N deduced to 3
///
/// However, the array bound is of type std::size_t [17.8.2.5]
/// (i.e., [temp.deduct.type]):
///
/// | The type of N in the type T[N] is std::size_t.
///
/// Presumably deduction works as if:
///
///   // explicit type of 'n' is signed char
///   // deduced value of 'n' is an ICE
///   // initialization is an error if the conversion is narrowing
///   //
///   template<typename T, signed char n>
///   constexpr signed char foo(T const(&)[n])
///   {
///       n = signed char({ICE_deduced_for_n});
///   }
///
/// and the compiler should emit a diagnostic if an array with more
/// than SCHAR_MAX elements is passed. Because the standard does not
/// specify this precisely, it seems best to use a deduced type for
/// array-bound template parameters and convert their values to the
/// desired return type using a facility such as bourn_cast or a
/// braced-init-list that ensures value preservation.
///
/// In the sample implementation above, 'const' on the unnamed array
/// parameter means nothing, and is omitted in the examples below; it
/// is specified in the header tested here to document that const
/// arrays are handled correctly. Similarly, 'constexpr' serves only
/// a documentary purpose, because an ICE is already a compile-time
/// constant.
///
/// Conclusions drawn from the experiments below.
///
/// With i686-w64-mingw32-gcc-7.3, evidently the non-type parameter
/// is deduced to have a value that is always an ICE of type int,
/// not type std::size_t. It is then progressively converted to the
/// integral type specified in the template-parameter-list, and then
/// to the return type, eliciting a diagnostic if the value cannot be
/// preserved. Comments beginning 'error:' are extracted from actual
/// gcc diagnostics.
///
/// Using bourn_cast<>() in the implementation turns an informative
/// compile-time diagnostic into a runtime exception, so that's just
/// a poor idea. Using a braced-init-list in the body of the function
/// template gives the most useful diagnostic, so f0g() seems best, at
/// least with this particular compiler. The template-parameter-list
/// might specify std::size_t for the non-type parameter, especially
/// in light of the [temp.deduct.type] quote above, but it seems even
/// better to use auto, again as in f0g(), and to specify the return
/// type of lmi::ssize() as lmi::ssize_t (which is the signed analogue
/// of std::size_t for 32-bit architectures, but not for LP64, e.g.).

namespace experimental
{
// deduce char, return int
// here gcc further reports:
// | In substitution of 'template<class T, char n>
// | int experimental::f0a(T (&)[n])
// | [with T = const float; char n = '\37777777600']'
// a web search for that octal constant finds references to
//   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=77573
// | bogus wide string literals in diagnostics
// so that extra information is not actually very helpful
template<typename T, char n>
int f0a(T(&)[n]) // error: overflow in constant expression
{
    return bourn_cast<int>(n);
}

// deduce int, return char; not constexpr
template<typename T, int n>
char f0b(T(&)[n])
{
    return bourn_cast<char>(n);
}

// deduce int, return char; constexpr (makes no difference)
template<typename T, int n>
constexpr char f0c(T(&)[n])
{
    return bourn_cast<char>(n);
}

#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wbraced-scalar-init"
#endif // defined LMI_CLANG

// deduce int, return char; braced-init-list
template<typename T, int n>
char f0d(T(&)[n])
{
    return {n}; // error: narrowing conversion of '128' from 'int' to 'char'
}

// deduce short int, return char; braced-init-list
template<typename T, short int n>
char f0e(T(&)[n])
{
    return {n}; // error: narrowing conversion of '128' from 'short int'
}

// deduce std::size_t, return char; braced-init-list
template<typename T, std::size_t n>
char f0f(T(&)[n])
{
    return {n}; // error: narrowing conversion of '128' from 'unsigned int'
}

#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG

// deduce auto, return char; braced-init-list
// auto is deduced to int, not to std::size_t
template<typename T, auto n>
char f0g(T(&)[n])
{
    return {n}; // error: narrowing conversion of '128' from 'int' to 'char'
}

// deduce auto, return deduced type
// auto is deduced to int, not to std::size_t
template<typename T, auto n>
auto f0h(T(&)[n])
{
    std::cout << "Expect something like 'int' to be printed:" << std::endl;
    std::cout << lmi::TypeInfo(typeid(n)) << std::endl; // prints "int"
//  return {n}; // error: returning initializer list
    return n;
}

void test_array_bound_deduction()
{
    std::size_t const array_size = std::size_t(1) + CHAR_MAX;
    float const array[array_size] {};
//  f0a(array); // compile-time error
//  f0b(array); // run-time error
//  f0c(array); // run-time error
//  f0d(array); // compile-time error
//  f0e(array); // compile-time error
//  f0f(array); // compile-time error
//  f0g(array); // compile-time error
    f0h(array);
    lmi::ssize(array);
}
} // namespace experimental

void test_various_containers()
{
    char const c[2] = {'0'};
    LMI_TEST_EQUAL(lmi::ssize(c), 2);
    LMI_TEST_EQUAL(lmi::ssize(c), bourn_cast<int>(std::size(c)));

    std::array<int,3> const a{1, 2};
    LMI_TEST_EQUAL(lmi::ssize(a), 3);
    LMI_TEST_EQUAL(lmi::ssize(a), bourn_cast<int>(std::size(a)));

    std::vector<int> const v(5);
    LMI_TEST_EQUAL(lmi::ssize(v), 5);
    LMI_TEST_EQUAL(lmi::ssize(v), bourn_cast<int>(std::size(v)));

    std::string const s("abcdefg");
    LMI_TEST_EQUAL(lmi::ssize(s), 7);
    LMI_TEST_EQUAL(lmi::ssize(s), bourn_cast<int>(std::size(s)));

    char const* p = "ABCDEFGHIJK";
    LMI_TEST_EQUAL(lmi::sstrlen(p), 11);
    LMI_TEST_EQUAL(lmi::sstrlen(p), bourn_cast<int>(std::strlen(p)));
}

int test_main(int, char*[])
{
    experimental::test_array_bound_deduction();
    test_various_containers();

    return 0;
}
