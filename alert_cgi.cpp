// Alert messages for cgi-bin interface.
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

// $Id: alert_cgi.cpp,v 1.2 2005-05-26 22:01:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

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

    // With cgicc, the most natural way to treat any alert that must
    // be brought to the user's attention is to throw an exception.
    void throw_alert(std::string const& s)
    {
    std::cerr << s << std::endl;
    throw std::runtime_error(s);
    }
} // Unnamed namespace.

void status_alert(std::string const&)
{
    ; // Do nothing.
}

void warning_alert(std::string const& s)
{
    throw_alert(s);
}

void hobsons_choice_alert(std::string const& s)
{
    throw_alert(s);
}

void fatal_error_alert(std::string const& s)
{
    throw_alert(s);
}

