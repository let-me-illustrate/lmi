// Mixin for xml streaming operators.
//
// Copyright (C) 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef streamable_hpp
#define streamable_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <string>

/// TODO ?? Is this mixin class actually useful enough to perpetuate?

class LMI_SO streamable
{
  public:
    virtual ~streamable() = 0;

    virtual void read (xml::element const&) = 0;
    virtual void write(xml::element&) const = 0;

    virtual int class_version() const = 0;
    virtual std::string xml_root_name() const = 0;
};

inline xml::element const& operator>>(xml::element const& x, streamable& z)
{
    z.read(x);
    return x;
}

inline xml::element& operator<<(xml::element& x, streamable const& z)
{
    z.write(x);
    return x;
}

#endif // streamable_hpp

