// Moderately secure system date validation--tells whether system has expired.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: security.cpp,v 1.9 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "security.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "global_settings.hpp"
#include "secure_date.hpp"

#include <cstdlib> // std::exit(), EXIT_FAILURE
#include <string>

void validate_security(bool do_validate)
{
    if(!do_validate)
        {
        return;
        }

    std::string diagnostic_message = secure_date::instance()->validate
        (calendar_date()
        ,global_settings::instance().data_directory()
        );

    if(!diagnostic_message.empty())
        {
        safely_show_message(diagnostic_message.c_str());
        std::exit(EXIT_FAILURE);
        }
}

