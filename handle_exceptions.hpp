// Handlers for exceptional situations.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef handle_exceptions_hpp
#define handle_exceptions_hpp

#include "config.hpp"

#include "alert.hpp"                    // hobsons_choice_exception, safely_show_message()

#include <cstdlib>                      // exit()
#include <exception>
#include <stdexcept>

/// This function, of type std::terminate_handler, is intended to be
/// used as the argument of std::set_terminate().

[[noreturn]]
inline void lmi_terminate_handler()
{
    safely_show_message
        ("Abnormal-termination handler called. Please report this problem."
        );
    std::exit(EXIT_FAILURE);
}

/// An extraordinary exception designed to elude report_exception().
///
/// Motivating example: wx_test_exception derives from this class.
/// The production system (distributed to end users) is tested by
/// running its code under the supervision of the 'wx_test' program
/// (which is for developer use only)--in which case 'wx_test'
/// invokes the production code (replacing only its main() function)
/// and tests simulated GUI events. For this sole, exclusive purpose,
/// wx_test_exception is needed--and must pass through the production
/// code's exception traps as a neutrino through a fishing net. In any
/// other situation, it undermines the
///   catch(...) {report_exception();}
/// idiom, potentially causing great harm; to forestall that, a ctor
/// is declared here, but implemented only in the 'wx_test' code.
///
/// Implicitly-declared special member functions do the right thing.

class stealth_exception
    :public std::runtime_error
{
  public:
    explicit stealth_exception(std::string const& what_arg);
};

/// Handle an uncaught exception, showing a description if available
/// readily through what(), with the following idiomatic usage:
///   catch(...) {report_exception();}
/// which makes it possible to write generic exception-handling code
/// once and only once. See:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/7ac8db2c59c34103
///
/// Simply rethrow when stealth_exception is caught: see the comments
/// accompanying its declaration. To catch even stealth_exception and
/// classes derived from it--for instance, in a main() function, where
/// an untrapped exception would cause a crash--write:
///   catch(...) { try{report_exception();} catch(...){/*warning*/} }
///
/// Show no message when hobsons_choice_exception is caught--just
/// swallow it silently. It's thrown only when
///  - an appropriate message was just shown, and then
///  - the safe default action (throwing this exception) was accepted,
/// in which case it's pointless to repeat the same message.
///
/// It may seem like a good idea to test std::uncaught_exceptions()
/// right before the try block, as recommended here [these references
/// speak of the related facility deprecated by C++17]:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/ec0ef69dd3949955
///   "Before the try-block in report_exception, query
///   std::uncaught_exception() to determine if an exception is
///   active. If it is not, throw std::logic_error or some exception
///   that you know that your framework will catch."
/// but actually that's invalid--see:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/aa7ce713ee90c044
///   "The only problem with uncaught_exception is that it doesn't
///   tell you when you're in a catch(...) { ... throw; } block"
/// and see also:
///   https://lists.nongnu.org/archive/html/lmi/2018-03/msg00100.html
/// "uncaught_exception{,s}() would always return {false,0}" here.
///
/// See
///  http://article.gmane.org/gmane.comp.gnu.mingw.user/18355
///    [2005-12-16T09:20:33Z from Greg Chicares]
/// for a grave gcc problem with msw dlls, which is fixed
///  http://article.gmane.org/gmane.comp.gnu.mingw.user/18594
///    [2006-01-10T22:00:24Z from Danny Smith]
/// in MinGW gcc-3.4.5; it is avoided altogether by writing this
/// function inline, so this code works with earlier versions, too.

inline void report_exception()
{
    try
        {
        throw;
        }
    catch(stealth_exception const&)
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
