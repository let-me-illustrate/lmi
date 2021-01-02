// Alert messages for command-line interface.
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

#include "pchfile.hpp"

#include "alert.hpp"

#include <cstdio>                       // getchar()
#include <iostream>
#include <stdexcept>

namespace
{
bool volatile ensure_setup = set_alert_functions
    (status_alert
    ,warning_alert
    ,hobsons_choice_alert
    ,alarum_alert
    ,safe_message_alert
    );

bool continue_anyway()
{
    int c;
  ask:
    c = std::getchar();
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
    // TODO ?? If it seems desirable to offer a choice, then this
    // should be made a configurable option. This behavior is
    // certainly a poor choice for applications that should run
    // unattended, such as servers or regression tests.
    //
    static bool const volatile offer_hobsons_choice = false;
    if(offer_hobsons_choice)
        {
        std::cerr << s << '\n' << hobsons_prompt() << std::endl;
        if(continue_anyway())
            {
            throw hobsons_choice_exception();
            }
        }
    else
        {
        throw std::runtime_error(s);
        }
}

void alarum_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

void safe_message_alert(char const* message)
{
    safely_show_on_stderr(message);
}
