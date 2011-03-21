// Handlers for exceptional situations.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

// $Id$

#ifndef handle_exceptions_hpp
#define handle_exceptions_hpp

#include "config.hpp"

#include "alert.hpp" // hobsons_choice_exception, safely_show_message()

#include <cstdlib>   // std::exit()
#include <exception>

/// This function, of type std::terminate_handler, is intended to be
/// used as the argument of std::set_terminate().

inline void lmi_terminate_handler()
{
    safely_show_message
        ("Abnormal-termination handler called. Please report this problem."
        );
    std::exit(EXIT_FAILURE);
}

/// Handle an uncaught exception, showing a description if available
/// readily through what(), with the following idiomatic usage:
///   catch(...) {report_exception();}
/// which makes it possible to write generic exception-handling code
/// once and only once. See:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/7ac8db2c59c34103
///
/// It may seem like a good idea to test std::uncaught_exception()
/// right before the try block, as recommended here:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/ec0ef69dd3949955
///   "Before the try-block in report_exception, query
///   std::uncaught_exception() to determine if an exception is
///   active. If it is not, throw std::logic_error or some exception
//    that you know that your framework will catch."
/// but actually that's invalid--see:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/aa7ce713ee90c044
///   "The only problem with uncaught_exception is that it doesn't
///   tell you when you're in a catch(...) { ... throw; } block"
///
/// Show no message when hobsons_choice_exception is caught. It's
/// thrown only when
///  - an appropriate message was just shown, and then
///  - the safe default action (throwing this exception) was accepted,
/// in which case it's pointless to repeat the same message.
///
/// See
//   http://article.gmane.org/gmane.comp.gnu.mingw.user/18355
//     [2005-12-16T09:20:33Z from Greg Chicares]
/// for a grave gcc problem with msw dlls, which is fixed
//   http://article.gmane.org/gmane.comp.gnu.mingw.user/18594
//     [2006-01-10T22:00:24Z from Danny Smith]
/// in MinGW gcc-3.4.5; it is avoided altogether by writing this
/// function inline, so this code works with earlier versions, too.

inline void report_exception()
{
    try
        {
        throw;
        }
    catch(hobsons_choice_exception const&)
        {
        }
    catch(std::exception const& e)
        {
        safely_show_message(e.what());
        }
    catch(...)
        {
        safely_show_message("Unknown error");
        }
}

#endif // handle_exceptions_hpp

