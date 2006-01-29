// Boolean input class for wx data-transfer framework.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: datum_boolean.cpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "datum_boolean.hpp"

#include <istream>
#include <ostream>

datum_boolean::datum_boolean(bool value)
    :value_(value)
{
}

datum_boolean::~datum_boolean()
{
}

bool datum_boolean::is_valid(std::string const&) const
{
    return true;
}

bool datum_boolean::value() const
{
    return value_;
}

std::istream& datum_boolean::read(std::istream& is)
{
    return is >> value_;
}

std::ostream& datum_boolean::write(std::ostream& os) const
{
    return os << value();
}

bool operator==(datum_boolean const& lhs, datum_boolean const& rhs)
{
    return lhs.value() == rhs.value();
}

