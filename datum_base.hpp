// Input data for wx data-transfer demonstration: base class.
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

#ifndef datum_base_hpp
#define datum_base_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <iosfwd>

// Implicitly-declared special member functions do the right thing.

class LMI_SO datum_base
{
  public:
    datum_base() = default;

    datum_base(datum_base const&) = default;
    datum_base(datum_base&&) = default;
    datum_base& operator=(datum_base const&) = default;
    datum_base& operator=(datum_base&&) = default;
    virtual ~datum_base() = default;

    void enable(bool);
    bool is_enabled() const;

    virtual std::istream& read (std::istream&)       = 0;
    virtual std::ostream& write(std::ostream&) const = 0;

  private:
    bool enabled_ {true};
};

std::istream& operator>>(std::istream&, datum_base&);
std::ostream& operator<<(std::ostream&, datum_base const&);

#endif // datum_base_hpp
