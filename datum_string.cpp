// String input class for wx data-transfer framework.
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

#include "datum_string.hpp"

#include "facets.hpp"

#include <istream>
#include <ostream>

datum_string::datum_string(std::string const& value)
    :value_ {value}
{
}

datum_string& datum_string::operator=(std::string const& s)
{
    value_ = s;
    return *this;
}

std::string const& datum_string::value() const
{
    return value_;
}

std::istream& datum_string::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    is >> value_;
    is.imbue(old_locale);
    return is;
}

std::ostream& datum_string::write(std::ostream& os) const
{
    return os << value();
}

bool operator==(datum_string const& lhs, datum_string const& rhs)
{
    return lhs.value() == rhs.value();
}
