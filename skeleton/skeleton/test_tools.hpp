// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: test_tools.hpp,v 1.1.1.1 2004-05-15 19:59:18 chicares Exp $

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

// For convenience, allow the user to request inclusion of lower-level layers.
#ifdef BOOST_INCLUDE_MAIN
#   include "cpp_main.cpp"
#   include "test_main.cpp"
#endif // BOOST_INCLUDE_MAIN

// Header dependencies eliminated to reducing coupling.

// Macros (gasp!) ease use of reporting functions.

#define BOOST_TEST(exp) ((exp) ? static_cast<void>(0) : boost_modified::report_error(#exp,__FILE__,__LINE__))
// Effects: if (!exp) call report_error().

#define BOOST_CRITICAL_TEST(exp) ((exp) ? static_cast<void>(0) : boost_modified::report_critical_error(#exp,__FILE__,__LINE__))
// Effects: if (!exp) call report_critical_error().

#define BOOST_ERROR(msg) boost_modified::report_error((msg),__FILE__,__LINE__)

#define BOOST_CRITICAL_ERROR(msg) boost_modified::report_critical_error((msg),__FILE__,__LINE__)

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.
namespace boost_modified
{
    // Function implementations are not inline because it is better design to
    // decouple implementation, and because space is more important than speed
    // since error functions get called relatively infrequently.  Note that
    // separating implementatiion means that this header could be useful
    // without using the test_main.hpp header for a main() function,
    // and/or a different implementation could be supplied at link time.

    void report_error(char const* msg, char const* file, int line);
    // Effects: increment test_tools_error counter, write error message to cout.

    void report_critical_error(char const* msg, char const* file, int line);
    // Effects: report_error(msg,file,line), throw test_tools_exception.
}

// Revision History
//  2004-05-06 GWC removed carriage returns.
//  2004-05-05 GWC extracted original library from boost-1.23.0,
//    removed dependencies on other parts of boost, and adapted it to
//    lmi conventions.
//  26 Feb 01  Numerous changes suggested during formal review. (Beman)
//   7 Feb 01  #include <boost/test/test_main.cpp> if requested. (Beman)
//  22 Jan 01  Remove all header dependencies. (Beman)
//   3 Dec 00  Remove function definitions. (Ed Brey)
//   5 Nov 00  Initial boost version (Beman Dawes)

#endif // test_tools_hpp

