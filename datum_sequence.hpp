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

// The remainder of this initial version is just a copy of
// 'datum_string.hpp' with 's/datum_string/datum_sequence/g'.

#ifndef datum_sequence_hpp
#define datum_sequence_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include "value_cast.hpp"

#include <boost/operators.hpp>

#include <string>

// Implicitly-declared special member functions do the right thing.

class datum_sequence
    :public datum_base
    ,private boost::equality_comparable<datum_sequence,datum_sequence>
{
  public:
    datum_sequence();
    explicit datum_sequence(std::string const&);
    virtual ~datum_sequence();

    datum_sequence& operator=(std::string const&);

    std::string const& value() const;

    // datum_base required implementation.
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

  private:
    std::string value_;
};

bool operator==(datum_sequence const&, datum_sequence const&);

template<>
inline datum_sequence value_cast<datum_sequence,std::string>
    (std::string const& from)
{
    return datum_sequence(from);
}

template<>
inline std::string value_cast<std::string,datum_sequence>
    (datum_sequence const& from)
{
    return from.value();
}

#endif // datum_sequence_hpp

