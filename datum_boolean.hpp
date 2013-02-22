// Boolean input class for wx data-transfer framework.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#ifndef datum_boolean_hpp
#define datum_boolean_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include <boost/operators.hpp>

// Implicitly-declared special member functions do the right thing.

class datum_boolean
    :public datum_base
    ,private boost::equality_comparable<datum_boolean,datum_boolean>
{
  public:
    explicit datum_boolean(bool = true);
    virtual ~datum_boolean();

    datum_boolean& operator=(bool);

    bool value() const;

    // datum_base required implementation.
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

  private:
    bool value_;
};

bool operator==(datum_boolean const&, datum_boolean const&);

#endif // datum_boolean_hpp

