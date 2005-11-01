// Alert messages for command-line interface.
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

// $Id: alert_cli.cpp,v 1.5 2005-11-01 04:59:36 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#include "platform_dependent.hpp" // getch()

#include <iostream>
#include <stdexcept>

namespace
{
    bool ensure_setup = set_alert_functions
        (status_alert
        ,warning_alert
        ,hobsons_choice_alert
        ,fatal_error_alert
        );

    // Not referenced for now, except in an ifdef'd-out block.
    bool continue_anyway()
    {
        int c;
    ask:
        c = getch();
        if('y' == c || 'Y' == c)
            {
            std::cout << std::endl;
            return true;
            }
        else if('n' == c || 'N' == c)
            {
            std::cout << std::endl;
            return false;
            }
        else
            {
            std::cerr << "\nPlease type 'y' or 'n'." << std::endl;
            }
    goto ask;
    }
} // Unnamed namespace.

void status_alert(std::string const&)
{
    ; // Do nothing.
}

void warning_alert(std::string const& s)
{
    std::cout << s << std::endl;
}

void hobsons_choice_alert(std::string const& s)
{
    throw std::runtime_error(s);
#if 0
    // If it seems desirable to offer a choice, the following tested
    // code can be used. That seems a poor choice for applications
    // that should run unattended, such as servers or regression
    // tests.
    //
    // TODO ?? This choice could be governed by
    //   configurable_settings::instance().offer_hobsons_choice()
    // if desired; otherwise, if this alternative behavior is never
    // useful, then this code should be removed.
    std::cerr << s << '\n' << hobsons_prompt() << std::endl;
    if(continue_anyway())
        {
        throw hobsons_choice_exception();
        }
#endif // 0
}

void fatal_error_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

