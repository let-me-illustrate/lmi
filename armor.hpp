// Handlers for exceptional situations.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: armor.hpp,v 1.1 2005-12-15 01:40:20 chicares Exp $

#ifndef armor_hpp
#define armor_hpp

#include "config.hpp"

#include "alert.hpp" // hobsons_choice_exception, safely_show_message()

#include <cstdlib>   // std::exit()
#include <exception>
#include <stdexcept>

/// This function, of type std::terminate_handler, is intended to be
/// used as the argument of std::set_terminate().

inline void lmi_terminate_handler()
{
    safely_show_message
        ("Abnormal-termination handler called. Please report this problem."
        );
    std::exit(EXIT_FAILURE);
}

/// Rethrow an uncaught exception, catch it, and show a description.
///
/// Purpose: Write generic exception-handling code once and only once.
///
/// Reference:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/7ac8db2c59c34103
///
/// Usage:
///   catch(...) {report_exception();}
///
/// Show no message when hobsons_choice_exception is caught. It's
/// thrown only when
///  - an appropriate message was just shown, and then
///  - the safe default action (throwing this exception) was accepted,
/// in which case it's pointless to repeat the same message.
///
/// Limitation: This idiom fails spectacularly when the rethrown
/// exception crosses a dll boundary on the msw platform. To work
/// around this limitation, use LMI_CATCH_AND_REPORT_EXCEPTION.

inline void report_exception()
{
    if(!std::uncaught_exception())
        {
        throw std::logic_error("Improper use of report_exception().");
        }
    try
        {
        throw;
        }
    catch(hobsons_choice_exception&)
        {
        }
    catch(std::exception& e)
        {
        safely_show_message(e.what());
        }
    catch(...)
        {
        safely_show_message("Unknown error");
        }
}

/// Workaround for the msw-dll limitation of report_exception(), q.v.
///
/// The last line eats a semicolon written after the macro invocation.

#define LMI_CATCH_AND_REPORT_EXCEPTION                               \
    catch(hobsons_choice_exception&)                                 \
        {                                                            \
        }                                                            \
    catch(std::exception& e)                                         \
        {                                                            \
        safely_show_message(e.what());                               \
        }                                                            \
    catch(...)                                                       \
        {                                                            \
        safely_show_message("Unknown error");                        \
        }                                                            \
    do {} while(0)

#endif // armor_hpp

