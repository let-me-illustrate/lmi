// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: test_tools.hpp,v 1.5 2005-12-15 15:13:49 chicares Exp $

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

#include <cstdlib> // EXIT_SUCCESS, EXIT_FAILURE
#include <exception>
#include <ostream>
#include <string>
#include <typeinfo>

// These are macros for the same reason that 'assert' is.

#define BOOST_TEST_FLUSH              \
       "\n[file "  << __FILE__        \
    << ", line " << __LINE__ << "]\n" \
    << std::flush                     \

#define BOOST_TEST(exp)             \
    if(exp)                         \
        {                           \
        lmi_test::record_success(); \
        }                           \
    else                            \
        {                           \
        lmi_test::record_error();   \
        lmi_test::error_stream()    \
            << (exp)                \
            << BOOST_TEST_FLUSH     \
            ;                       \
        }                           \

// Relational macros require their arguments to be streamable.

#define BOOST_TEST_EQUAL(a,b) \
    BOOST_TEST_RELATION(a,==,b)  \

#define BOOST_TEST_UNEQUAL(a,b) \
    BOOST_TEST_RELATION(a,!=,b)  \

#define BOOST_TEST_RELATION(a,op,b)                      \
    if((a) op (b))                                       \
        lmi_test::record_success();                      \
    else                                                 \
        {                                                \
        lmi_test::record_error();                        \
        lmi_test::error_stream()                         \
            << "  '" << (a) << "' "#op" '" << (b) << "'" \
            << BOOST_TEST_FLUSH                          \
            ;                                            \
        }                                                \

/// Make sure 'expression' throws the anticipated exception. Signal an
/// error if no exception is thrown. Otherwise, compare the exception
/// actually thrown against the one anticipated: deem them equivalent
/// iff both
///  - their types match exactly, and
///  - either
///    - 'WHAT' is empty, or
///    - 'WHAT' exactly matches the actual exception's what(), or
///    - 'WHAT' matches the actual exception's what() up to but not
///      including the first newline (because some lmi exceptions add
///      a newline and the file name and line number).
///
/// TODO ?? Probably the first element of the triple condition should
/// be removed, and tests that rely on it strengthened.
///
/// COMPILER !! The borland compiler complains:
///   std::out_of_range: position beyond end of string in function:
///   basic_string::compare(size_t,size_t,const basic_string&) const
///   index: -1 is greater than max_index: [size of string]
/// but that complaint seems incorrect: the second argument is allowed
/// to be npos, and only an invalid first argument can cause this
/// exception, but the first argument here is always zero, which is
/// always permissible. See C++98 21.3.6.8/3 and 21.3.1/4 and cf.
/// Josuttis, TC++SL, 11.3.4 .

#define BOOST_TEST_THROW(expression,TYPE,WHAT)          \
    try                                                 \
        {                                               \
        expression;                                     \
        lmi_test::error_stream()                        \
            << "Expression '"                           \
            << #expression                              \
            << "' failed to throw expected exception '" \
            << #TYPE                                    \
            << "'"                                      \
            << BOOST_TEST_FLUSH                         \
            ;                                           \
        lmi_test::record_error();                       \
        }                                               \
    catch(std::exception const& e)                      \
        {                                               \
        if                                              \
            (   typeid(e)                               \
            !=  typeid(TYPE)                            \
            )                                           \
            {                                           \
            lmi_test::error_stream()                    \
                << "Caught exception of type\n    '"    \
                << typeid(e).name()                     \
                << "'\n  when type\n    '"              \
                << typeid(TYPE).name()                  \
                << "'\n  was expected."                 \
                << BOOST_TEST_FLUSH                     \
                ;                                       \
            lmi_test::record_error();                   \
            }                                           \
        else if                                         \
            (   std::string(WHAT).size()                \
            &&  (   std::string((e).what())             \
                !=  std::string(WHAT)                   \
                )                                       \
            &&  0 != std::string((e).what()).compare    \
                    (0                                  \
                    ,std::string((e).what()).find('\n') \
                    ,std::string(WHAT)                  \
                    )                                   \
            )                                           \
            {                                           \
            lmi_test::error_stream()                    \
                << "Caught exception\n    '"            \
                << (e).what()                           \
                << "'\n  when\n    '"                   \
                << (WHAT)                               \
                << "'\n  was expected."                 \
                << BOOST_TEST_FLUSH                     \
                ;                                       \
            lmi_test::record_error();                   \
            }                                           \
        else                                            \
            {                                           \
            lmi_test::record_success();                 \
            }                                           \
        }                                               \

#define INVOKE_BOOST_TEST(exp,file,line)  \
    if(!(exp))                            \
        {                                 \
        lmi_test::record_error();         \
        lmi_test::error_stream()          \
            << "\n**** test failed: "     \
            << (exp)                      \
            << "\n[invoked from "         \
            << "file " << (file) << ", "  \
            << "line: " << (line)         \
            << "]"                        \
            << BOOST_TEST_FLUSH           \
            ;                             \
        }                                 \
    else                                  \
        lmi_test::record_success();       \

#define INVOKE_BOOST_TEST_EQUAL(a,b,file,line) \
    INVOKE_BOOST_TEST_RELATION(a,==,b,file,line)  \

#define INVOKE_BOOST_TEST_UNEQUAL(a,b,file,line) \
    INVOKE_BOOST_TEST_RELATION(a,!=,b,file,line)  \

#define INVOKE_BOOST_TEST_RELATION(a,op,b,file,line)     \
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
            << BOOST_TEST_FLUSH                          \
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
    // Rationale: An alternative implementation of BOOST_TEST_THROW
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

    std::ostream& error_stream();

    void record_success();
    // Effect: increment test_tools_successes counter.

    void record_error();
    // Effect: increment test_tools_errors counter.
} // Namespace lmi_test.

// For convenience, allow the user to request inclusion of lower-level layers.
#ifdef BOOST_INCLUDE_MAIN
#   include "cpp_main.cpp"
#   include "test_main.cpp"
#endif // BOOST_INCLUDE_MAIN

// Deprecated macros.
//
// Macros BOOST_ERROR, BOOST_CRITICAL_ERROR, and BOOST_CRITICAL_TEST
// are provided only for backward compatibility, not because they're
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

#define BOOST_CRITICAL_TEST(exp)                      \
    if(exp)                                           \
        {                                             \
        lmi_test::record_success();                   \
        }                                             \
    else                                              \
        {                                             \
        lmi_test::record_error();                     \
        lmi_test::error_stream()                      \
            << (exp)                                  \
            << BOOST_TEST_FLUSH                       \
            ;                                         \
        throw lmi_test::test::test_tools_exception(); \
        }                                             \

#define BOOST_ERROR(exp)          \
        lmi_test::record_error(); \
        lmi_test::error_stream()  \
            << (exp)              \
            << BOOST_TEST_FLUSH   \

#define BOOST_CRITICAL_ERROR(exp)                    \
        lmi_test::record_error();                    \
        lmi_test::error_stream()                     \
            << (exp)                                 \
            << BOOST_TEST_FLUSH                      \
            ;                                        \
        throw lmi_test::test::test_tools_exception() \

// Revision History
//  2005-12-15 GWC Include <cstdlib> for EXIT_SUCCESS and EXIT_FAILURE.
//  2005-11-23 GWC Improve exception-testing macro.
//  2005-02-28 GWC Fix defective typeid comparison.
//  2005-01-29 GWC Move #included '.cpp' files after declarations that
//    were otherwise redundant, resolving a gcc warning.
//  2005-01-09 GWC Update email address
//  2004-10-20 GWC Add macros to test equality and exceptions, and to
//    invoke tests through subfunctions. Add error_stream() to improve
//    versatility; remove former error-reporting functions. Revise
//    documentation. Rename namespace.
//  2004-10-05 GWC Report number of successful tests.
//  2004-10-04 GWC Add record_error().
//  2004-05-06 GWC Remove carriage returns.
//  2004-05-05 GWC Extract original library from boost-1.23.0, remove
//    dependencies on other parts of boost, and adapt to lmi conventions.
//  26 Feb 01  Numerous changes suggested during formal review. (Beman)
//   7 Feb 01  #include <boost/test/test_main.cpp> if requested. (Beman)
//  22 Jan 01  Remove all header dependencies. (Beman)
//   3 Dec 00  Remove function definitions. (Ed Brey)
//   5 Nov 00  Initial boost version (Beman Dawes)

#endif // test_tools_hpp

