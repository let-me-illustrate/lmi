// Unit-test framework based on Beman Dawes's boost library.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include <exception>                    // exception, bad_exception
#include <new>                          // bad_alloc
#include <ostream>
#include <stdexcept>
#include <string>
#include <typeinfo>                     // bad_cast, bad_typeid

// GWC changed namespace 'boost' to prevent any conflict with code in
// a later version of boost.

namespace lmi_test
{

  namespace detail
  {
    // A separate reporting function was requested during formal review.
    inline void herald_exception
        (std::ostream& os
        ,char const*   name
        ,char const*   info
        )
      {
      os
          << "\n" << default_error_prefix
          << "uncaught exception: " << name << " " << info
          << std::endl
          ;
      }
  } // namespace detail

  // catch_exceptions  ------------------------------------------------------//

    // A function object to run the 'main' function.
    template<typename Generator>
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

        catch(char const*& e)
            {detail::herald_exception(out, ""                      , e);}
        catch(std::string const& e)
            {detail::herald_exception(out, ""                      ,e.c_str());}

        // std:: exceptions
        catch(std::bad_alloc const& e)
            {detail::herald_exception(out, "std::bad_alloc:"       , e.what());}
        catch(std::bad_cast const& e)
            {detail::herald_exception(out, "std::bad_cast:"        , e.what());}
        catch(std::bad_typeid const& e)
            {detail::herald_exception(out, "std::bad_typeid:"      , e.what());}
        catch(std::bad_exception const& e)
            {detail::herald_exception(out, "std::bad_exception:"   , e.what());}
        catch(std::domain_error const& e)
            {detail::herald_exception(out, "std::domain_error:"    , e.what());}
        catch(std::invalid_argument const& e)
            {detail::herald_exception(out, "std::invalid_argument:", e.what());}
        catch(std::length_error const& e)
            {detail::herald_exception(out, "std::length_error:"    , e.what());}
        catch(std::out_of_range const& e)
            {detail::herald_exception(out, "std::out_of_range:"    , e.what());}
        catch(std::range_error const& e)
            {detail::herald_exception(out, "std::range_error:"     , e.what());}
        catch(std::overflow_error const& e)
            {detail::herald_exception(out, "std::overflow_error:"  , e.what());}
        catch(std::underflow_error const& e)
            {detail::herald_exception(out, "std::underflow_error:" , e.what());}
        catch(std::logic_error const& e)
            {detail::herald_exception(out, "std::logic_error:"     , e.what());}
        catch(std::runtime_error const& e)
            {detail::herald_exception(out, "std::runtime_error:"   , e.what());}
        catch(std::exception const& e)
            {detail::herald_exception(out, "std::exception:"       , e.what());}

        catch(...)
            {detail::herald_exception(out, "unknown exception"     , "");}

        if(exception_thrown) result = lmi_test::exit_exception_failure;

        if(result != 0 && result != exit_success)
            {
            out
                << std::endl
                << default_error_prefix
                << "returning with error code " << result
                << std::endl
                ;
            err
                << default_error_prefix << default_error_prefix // Yes, twice.
                << "errors detected; see stdout for details"
                << std::endl
                ;
            }
        else
            {
            out
                << std::flush
                << success_prefix
                << "no errors detected"
                << std::endl
                ;
            }

        return result;
    } // catch_exceptions()

} // namespace lmi_test

#endif // catch_exceptions_hpp
