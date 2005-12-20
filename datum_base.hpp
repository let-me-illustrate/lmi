// Input data for wx data-transfer demonstration: base class.
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

// $Id: datum_base.hpp,v 1.4 2005-12-20 00:46:41 chicares Exp $

#ifndef datum_base_hpp
#define datum_base_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <iosfwd>
#include <string>

// Implicitly-declared special member functions do the right thing.

class LMI_EXPIMP datum_base
{
  public:
    datum_base();
    virtual ~datum_base();

    void enable(bool);
    bool is_enabled() const;

    virtual bool is_valid(std::string const&) const     = 0;

    virtual std::istream& read (std::istream& is)       = 0;
    virtual std::ostream& write(std::ostream& os) const = 0;

  private:
    bool enabled_;
};

// Templated streaming operators would necessitate member templates
// for read() and write(), which might suggest a different design.

inline std::istream& operator>>(std::istream& is, datum_base& z)
{
    return z.read(is);
}

inline std::ostream& operator<<(std::ostream& os, datum_base const& z)
{
    return z.write(os);
}

#endif // datum_base_hpp

