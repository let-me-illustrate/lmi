// String input class for wx data-transfer framework.
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

// $Id: datum_string.hpp,v 1.1 2005-03-11 03:09:22 chicares Exp $

#ifndef datum_string_hpp
#define datum_string_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include "value_cast.hpp"

#include <boost/operators.hpp>

// Implicitly-declared special member functions do the right thing.

class datum_string
    :public datum_base
    ,private boost::equality_comparable<datum_string,datum_string>
{
  public:
    datum_string();
    explicit datum_string(std::string const&);
    virtual ~datum_string();

    std::string const& value() const;

    // datum_base required implementation.
    virtual bool is_valid(std::string const&) const;
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

  private:
    std::string value_;
};

bool operator==(datum_string const&, datum_string const&);

template<>
inline datum_string value_cast<datum_string,std::string>
    (std::string from, datum_string)
{
    return datum_string(from);
}

template<>
inline std::string value_cast<std::string,datum_string>
    (datum_string from, std::string)
{
    return from.value();
}

#endif // datum_string_hpp

