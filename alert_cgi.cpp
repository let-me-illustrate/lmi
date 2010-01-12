// Alert messages for cgi-bin interface.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#include <cstdio> // std::fputs()
#include <stdexcept>

namespace
{
    bool ensure_setup = set_alert_functions
        (status_alert
        ,warning_alert
        ,hobsons_choice_alert
        ,fatal_error_alert
        ,safe_message_alert
        );

} // Unnamed namespace.

void status_alert(std::string const&)
{
    ; // Do nothing.
}

// With cgicc, the most natural way to treat any alert that must be
// brought to the user's attention is to throw an exception and let
// a higher-level routine catch and display it. It might be desirable
// to write a log file, too.

void warning_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

void hobsons_choice_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

void fatal_error_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

void safe_message_alert(char const* message)
{
    std::fputs(message, stderr);
    std::fputc('\n', stderr);
    // Flush explicitly. C99 7.19.3/7 says only that stderr is
    // "not fully buffered", not that it is 'unbuffered'. See:
    //   http://article.gmane.org/gmane.comp.gnu.mingw.user/14358
    //     [2004-12-20T09:07:24Z from Danny Smith]
    //   http://article.gmane.org/gmane.comp.gnu.mingw.user/15063
    //     [2005-02-10T17:23:09Z from Greg Chicares]
    std::fflush(stderr);
}

