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

// $Id: catch_exceptions.hpp,v 1.1.1.1 2004-05-15 19:58:07 chicares Exp $

// This is a derived work based on Beman Dawes's boost test library
// that bears the following copyright and license statement:
// [Beman Dawes's copyright and license statement begins]
// (C) Copyright Beman Dawes 1995-2001. Permission to copy, use, modify, sell
// and distribute this software is granted provided this copyright notice
// appears in all copies. This software is provided "as is" without express or
// implied warranty, and with no claim as to its suitability for any purpose.

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

// boost/catch_exceptions.hpp -----------------------------------------------//

#ifndef catch_exceptions_hpp
#define catch_exceptions_hpp

#include "config.hpp"

#include "exit_codes.hpp"

#include <string>             // for string
#include <new>                // for bad_alloc
#include <typeinfo>           // for bad_cast, bad_typeid
#include <exception>          // for exception, bad_exception
#include <ostream>            // for ostream
#include <stdexcept>          // for std exception hierarchy

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.

namespace boost_modified
{

  namespace detail
  {
    // A separate reporting function was requested during formal review.
    inline void report_exception
        (std::ostream& os
        ,char const*   name
        ,char const*   info
        )
      {os << "\n** uncaught exception: " << name << " " << info << std::endl;}
  }

  // catch_exceptions  ------------------------------------------------------//

    // A function object to run the 'main' function.
    template<class Generator>
    int catch_exceptions
        (Generator     function_object
        ,std::ostream& out
        ,std::ostream& err
        )
    {
        // Quiet compiler warnings.
        int result = 0;
        // Avoid setting result for each exception type.
        bool exception_thrown = true;

        try
            {
            result = function_object();
            exception_thrown = false;
            }

    // As a result of hard experience with strangely interleaved output
    // under some compilers, there is a lot of use of endl in the code below
    // where a simple '\n' might appear to do.

    // The rules for catch & arguments are a bit different from function
    // arguments (ISO 15.3 paragraphs 18 & 19). Apparently const isn't
    // required, but it doesn't hurt and some programmers ask for it.

        catch(char const* ex )
            {detail::report_exception(out, "", ex);}
        catch(std::string const& ex )
            {detail::report_exception(out, "", ex.c_str());}

        // std:: exceptions
        catch(std::bad_alloc const& ex)
            {detail::report_exception( out, "std::bad_alloc:", ex.what());}

#if !defined(__BORLANDC__) || __BORLANDC__ > 0x0551
        catch(std::bad_cast const& ex)
            {detail::report_exception(out, "std::bad_cast:", ex.what());}
        catch(std::bad_typeid const& ex)
            {detail::report_exception(out, "std::bad_typeid:", ex.what());}
#else // Old borland compiler.
        catch(std::bad_cast const& ex)
            {detail::report_exception(out, "std::bad_cast", "");}
        catch(std::bad_typeid const& ex)
            {detail::report_exception(out, "std::bad_typeid", "");}
#endif // Old borland compiler.

        catch(const std::bad_exception & ex)
            {detail::report_exception(out, "std::bad_exception:", ex.what());}
        catch(const std::domain_error & ex)
            {detail::report_exception(out, "std::domain_error:", ex.what());}
        catch(const std::invalid_argument & ex)
            {detail::report_exception(out, "std::invalid_argument:", ex.what());}
        catch(const std::length_error & ex)
            {detail::report_exception(out, "std::length_error:", ex.what());}
        catch(const std::out_of_range & ex)
            {detail::report_exception(out, "std::out_of_range:", ex.what());}
        catch(const std::range_error & ex)
            {detail::report_exception(out, "std::range_error:", ex.what());}
        catch(const std::overflow_error & ex)
            {detail::report_exception(out, "std::overflow_error:", ex.what());}
        catch(const std::underflow_error & ex)
            {detail::report_exception(out, "std::underflow_error:", ex.what());}
        catch(const std::logic_error & ex)
            {detail::report_exception(out, "std::logic_error:", ex.what());}
        catch(const std::runtime_error & ex)
            {detail::report_exception(out, "std::runtime_error:", ex.what());}
        catch(const std::exception & ex)
            {detail::report_exception(out, "std::exception:", ex.what());}

        catch(...)
            {detail::report_exception(out, "unknown exception", "");}

        if(exception_thrown) result = boost_modified::exit_exception_failure;

        if(result != 0 && result != exit_success)
            {
            out
                << std::endl
                << "**** returning with error code " << result
                << std::endl
                ;
            err
                << "**********  errors detected; see stdout for details  ***********"
                << std::endl
                ;
            }
        else
            {
            out << std::flush << "no errors detected" << std::endl;
            }

        return result;
    } // catch_exceptions()

} // boost_modified

// Revision History
//  2004-05-06 GWC removed carriage returns.
//  2004-05-05 GWC extracted original library from boost-1.23.0,
//    removed dependencies on other parts of boost, and adapted it to
//    lmi conventions.
//  13 Jun 01 report_exception() made inline. (John Maddock, Jesse Jones)
//  26 Feb 01 Numerous changes suggested during formal review. (Beman)
//  25 Jan 01 catch_exceptions.hpp code factored out of cpp_main.cpp.
//  22 Jan 01 Remove test_tools dependencies to reduce coupling.
//   5 Nov 00 Initial boost version (Beman Dawes)

#endif // catch_exceptions_hpp

