// Range-checked type.
//
// Copyright (C) 2001, 2002, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: xrange.cpp,v 1.3 2007-01-27 00:00:52 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xrange.hpp"

#include <sstream>

// This function could have been written inline in the header; the
// only reason to write it out-of-line here is to make it easier to
// set a breakpoint on it.
std::string xrange_error_message
    (std::string const& bad_value
    ,std::string const& typeid_name
    ,std::string const& minimum
    ,std::string const& maximum
    )
{
    std::ostringstream error;
    error
        << bad_value
        << " out of range for type "
        << typeid_name
        << " (must be between "
        << minimum
        << " and "
        << maximum
        << ")."
        ;
    return error.str();
}

