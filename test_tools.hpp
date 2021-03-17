// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// This is a derived work based on Beman Dawes's boost test library
// that bears the following copyright and license statement:
// [Beman Dawes's copyright and license statement begins]
// (C) Copyright Beman Dawes 2000. Permission to copy, use, modify, sell
// and distribute this software is granted provided this copyright notice
// appears in all copies. This software is provided "as is" without express or
// implied warranty, and with no claim as to its suitability for any purpose.
//
// See http://www.boost.org for updates, documentation, and revision history.
// [end Beman Dawes's copyright and license statement]
//
// Gregory W. Chicares modified it in 2004 to make it independent of
// the boost directory structure, and in any other ways indicated
// below, and in any later years shown above. Any defect in it should
// not reflect on Beman Dawes's reputation.

// The original boost.org test library upon which this derived work is
// based was later replaced by a very different library. That new test
// library has more features but is less transparent; it is not
// strictly compatible with tests written for the original library;
// and, for boost-1.31.0 at least, it didn't work out of the box with
// the latest como compiler. The extra features don't seem to be worth
// the cost.
//
// It seems unwise to mix boost versions, and better to extract this
// library from boost-1.23.0 and remove its dependencies on other
// parts of boost, which is easy to do because, as Beman Dawes says:
// Header dependencies are deliberately restricted to reduce coupling.

// This file may be included as a header file, or may be compiled and placed
// in a library for traditional linking. It is unusual for non-template
// non-inline implementation code to be used as a header file, but the user
// may elect to do so because header-only implementation requires no library
// build support. (Suggested by Ed Brey)

// test tools header  -------------------------------------------------------//

#ifndef test_tools_hpp
#define test_tools_hpp

#include "config.hpp"

#include "unwind.hpp"                   // scoped_unwind_toggler

#include <cstdlib>                      // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>
#include <ostream>
#include <string>
#include <typeinfo>

// These are macros for the same reason that 'assert' is.

#define LMI_TEST_FLUSH                \
       "\n[file "  << __FILE__        \
    << ", line " << __LINE__ << "]\n" \
    << std::flush                     \

#define LMI_TEST(exp)               \
    if(exp)                         \
        {                           \
        lmi_test::record_success(); \
        }                           \
    else                            \
        {                           \
        lmi_test::record_error();   \
        lmi_test::error_stream()    \
            << (exp)                \
            << LMI_TEST_FLUSH       \
            ;                       \
        }                           \

// Relational macros require their arguments to be streamable.

#define LMI_TEST_EQUAL(a,b)   \
    LMI_TEST_RELATION(a,==,b) \

#define LMI_TEST_UNEQUAL(a,b) \
    LMI_TEST_RELATION(a,!=,b) \

#define LMI_TEST_RELATION(a,op,b)                        \
    if((a) op (b))                                       \
        lmi_test::record_success();                      \
    else                                                 \
        {                                                \
        lmi_test::record_error();                        \
        lmi_test::error_stream()                         \
            << "  '" << (a) << "' "#op" '" << (b) << "'" \
            << LMI_TEST_FLUSH                            \
            ;                                            \
        }                                                \

namespace lmi_test
{
/// Judge whether what() matches macro argument WHAT acceptably.
///
/// Intended to be called only by LMI_TEST_THROW(). Arguments:
///   - observed: the what() string actually thrown;
///   - expected: macro argument 'WHAT', the string anticipated.
///
/// Deem the arguments equivalent iff either:
///   - 'WHAT' is empty, indicating that the actual exception's what()
///      is to be disregarded (not that it is expected to be empty);
///   - 'WHAT' exactly matches the actual exception's what(); or
///   - 'WHAT' matches the actual exception's what() up to but not
///      including any lmi exception-location string. (Some lmi
///      exceptions add a newline and the file name and line number,
///      always beginning "\n["--cf. LMI_FLUSH--which sequence is
///      assumed otherwise not to occur in what().)

inline bool whats_what(std::string const& observed, std::string const& expected)
{
    return
           expected.empty()
        || observed == expected
        || 0 == observed.compare(0, observed.find("\n["), expected)
        ;
}

class what_regex;

bool whats_what(std::string const& observed, what_regex const& expected);
} // namespace lmi_test

/// Make sure 'expression' throws the anticipated exception. Signal an
/// error if no exception is thrown. Otherwise, compare the exception
/// actually thrown against the one anticipated: deem them equivalent
/// iff both
///  - their types match exactly, and
///  - lmi_test::whats_what() deems the observed what() equivalent to
///    macro argument WHAT.
///
/// Don't display a backtrace--it would be superfluous clutter here.

#define LMI_TEST_THROW(expression,TYPE,WHAT)                  \
    {                                                         \
    scoped_unwind_toggler meaningless_name;                   \
    try                                                       \
        {                                                     \
        expression;                                           \
        lmi_test::error_stream()                              \
            << "Expression '"                                 \
            << #expression                                    \
            << "' failed to throw expected exception '"       \
            << #TYPE                                          \
            << "'"                                            \
            << LMI_TEST_FLUSH                                 \
            ;                                                 \
        lmi_test::record_error();                             \
        }                                                     \
    catch(std::exception const& e)                            \
        {                                                     \
        if(typeid(e) != typeid(TYPE))                         \
            {                                                 \
            lmi_test::error_stream()                          \
                << "Caught exception of type\n    '"          \
                << typeid(e).name()                           \
                << "'\n  when type\n    '"                    \
                << typeid(TYPE).name()                        \
                << "'\n  was expected."                       \
                << LMI_TEST_FLUSH                             \
                ;                                             \
            lmi_test::record_error();                         \
            }                                                 \
        else if(!lmi_test::whats_what((e.what()), (WHAT)))    \
            {                                                 \
            lmi_test::error_stream()                          \
                << "Caught exception\n    '"                  \
                << (e).what()                                 \
                << "'\n  when\n    '"                         \
                << (WHAT)                                     \
                << "'\n  was expected."                       \
                << LMI_TEST_FLUSH                             \
                ;                                             \
            lmi_test::record_error();                         \
            }                                                 \
        else                                                  \
            {                                                 \
            lmi_test::record_success();                       \
            }                                                 \
        }                                                     \
    }                                                         \

#define INVOKE_LMI_TEST(exp,file,line)      \
    if(!(exp))                              \
        {                                   \
        lmi_test::record_error();           \
        lmi_test::error_stream()            \
            << lmi_test::error_prefix       \
            << "test failed: "              \
            << (exp)                        \
            << "\n[invoked from "           \
            << "file " << (file) << ", "    \
            << "line: " << (line)           \
            << "]"                          \
            << LMI_TEST_FLUSH               \
            ;                               \
        }                                   \
    else                                    \
        lmi_test::record_success();         \

#define INVOKE_LMI_TEST_EQUAL(a,b,file,line)   \
    INVOKE_LMI_TEST_RELATION(a,==,b,file,line) \

#define INVOKE_LMI_TEST_UNEQUAL(a,b,file,line) \
    INVOKE_LMI_TEST_RELATION(a,!=,b,file,line) \

#define INVOKE_LMI_TEST_RELATION(a,op,b,file,line)       \
    if((a) op (b))                                       \
        lmi_test::record_success();                      \
    else                                                 \
        {                                                \
        lmi_test::record_error();                        \
        lmi_test::error_stream()                         \
            << "  '" << (a) << "' "#op" '" << (b) << "'" \
            << "\n[invoked from "                        \
            << "file " << (file) << ", "                 \
            << "line: " << (line)                        \
            << "]"                                       \
            << LMI_TEST_FLUSH                            \
            ;                                            \
        }                                                \

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.
namespace lmi_test
{
    // The original implementation had only a few testing macros, with
    // simple error reporting that was sensibly factored out of the
    // macros and into a single function (with a trivial variant that
    // added termination semantics). Its rationale declared:
    //   "space is more important than speed since error functions get
    //   called relatively infrequently."
    //
    // This implementation has a greater number of more specialized
    // testing macros that give more detailed error reports. Instead
    // of adding more reporting functions, it presents a std::ostream
    // for versatility and clarity.
    //
    // Rationale: An alternative implementation of LMI_TEST_THROW
    // was written to determine how much work could be delegated to
    // functions without reporting less information. Because the
    // 'expression' parameter can be a simple semicolon, it must be a
    // macro parameter. Therefore, the try-block that encloses it must
    // be written in the macro, and, consequently, so must the
    // associated catch-clause. Only the reporting can be delegated to
    // functions. Three such functions were written, because three
    // different types of errors can occur. Understanding this design
    // required looking back and forth between an irreducibly complex
    // macro in one file and three function implementations in another
    // file, and writing the function invocations in the macro took
    // about as much space as inlining the code.
    //
    // OTOH, comparing the expected to the observed what() string is
    // simpler in an auxiliary function: that function, whats_what(),
    // is more readable than the inlined macro code it replaced, and
    // easier to overload.

    std::ostream& error_stream();

    // Increment test_tools_successes counter.
    void record_success();

    // Increment test_tools_errors counter.
    void record_error();
} // namespace lmi_test

// For convenience, include lower-level layers. (The original boost
// library did this only if BOOST_INCLUDE_MAIN had been defined.)

#include "cpp_main.cpp"
#include "test_main.cpp"

// Withdrawn macros.
//
// Macros BOOST_ERROR, BOOST_CRITICAL_ERROR, and BOOST_CRITICAL_TEST,
// formerly provided, have been withdrawn because they were never
// really useful. In boost-1.23.0, except for the testing framework's
// own unit tests, BOOST_ERROR and BOOST_CRITICAL_ERROR are used only
// once each and only in a single file, in the idiom
//   if(condition) {macro;}
// where
//   BOOST_TEST(condition);
// or
//   BOOST_CRITICAL_TEST(condition);
// would be more natural. BOOST_CRITICAL_TEST is used more often in
// boost-1.23.0, but, except for the testing framework's own unit
// tests, it occurs only in files whose authors don't use BOOST_TEST,
// where it seems that BOOST_TEST would be better. Probably BOOST_TEST
// is always better: if the success of a particular test is thought to
// be a precondition for success of all later tests, it's not clearly
// a good idea to skip the later tests--if any of them succeed, that's
// interesting.

#endif // test_tools_hpp
