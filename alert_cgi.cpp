// Alert messages for cgi-bin interface.
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

void alarum_alert(std::string const& s)
{
    throw std::runtime_error(s);
}

void safe_message_alert(char const* message)
{
    safely_show_on_stderr(message);
}
