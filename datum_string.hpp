// String input class for wx data-transfer framework.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: datum_string.hpp,v 1.9 2008-12-27 02:56:39 chicares Exp $

#ifndef datum_string_hpp
#define datum_string_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include "value_cast.hpp"

#include <boost/operators.hpp>

#include <string>

// Implicitly-declared special member functions do the right thing.

class datum_string
    :public datum_base
    ,private boost::equality_comparable<datum_string,datum_string>
{
  public:
    datum_string();
    explicit datum_string(std::string const&);
    virtual ~datum_string();

    datum_string& operator=(std::string const&);

    std::string const& value() const;

    // datum_base required implementation.
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

  private:
    std::string value_;
};

bool operator==(datum_string const&, datum_string const&);

template<>
inline datum_string value_cast<datum_string,std::string>
    (std::string const& from)
{
    return datum_string(from);
}

template<>
inline std::string value_cast<std::string,datum_string>
    (datum_string const& from)
{
    return from.value();
}

#endif // datum_string_hpp

