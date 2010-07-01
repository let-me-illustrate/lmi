// Input-sequence class for wx data-transfer framework.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// The remainder of this initial version is just a copy of
// 'datum_string.cpp' with 's/datum_string/datum_sequence/g'.

#include "datum_sequence.hpp"

#include "facets.hpp"

datum_sequence::datum_sequence()
{
}

datum_sequence::datum_sequence(std::string const& value)
    :value_(value)
{
}

datum_sequence::~datum_sequence()
{
}

datum_sequence& datum_sequence::operator=(std::string const& s)
{
    value_ = s;
    return *this;
}

std::string const& datum_sequence::value() const
{
    return value_;
}

std::istream& datum_sequence::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    is >> value_;
    is.imbue(old_locale);
    return is;
}

std::ostream& datum_sequence::write(std::ostream& os) const
{
    return os << value();
}

bool operator==(datum_sequence const& lhs, datum_sequence const& rhs)
{
    return lhs.value() == rhs.value();
}

