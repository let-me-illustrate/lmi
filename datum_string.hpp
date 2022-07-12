// String input class for wx data-transfer framework.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef datum_string_hpp
#define datum_string_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include "value_cast.hpp"

#include <string>

class datum_string
    :public datum_base
{
  public:
    datum_string() = default;
    explicit datum_string(std::string const&);

    datum_string(datum_string const&) = default;
    datum_string(datum_string&&) = default;
    datum_string& operator=(datum_string const&) = default;
    datum_string& operator=(datum_string&&) = default;
    ~datum_string() override = default;

    datum_string& operator=(std::string const&);

    std::string const& value() const;

    // datum_base required implementation.
    std::istream& read (std::istream&) override;
    std::ostream& write(std::ostream&) const override;

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
