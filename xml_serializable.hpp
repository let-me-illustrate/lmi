// Mixin for xml serialization.
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

#ifndef xml_serializable_hpp
#define xml_serializable_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <list>
#include <map>
#include <string>

/// Derive from this mixin class to use its xml serialization.
///
/// Implicitly-declared special member functions do the right thing.

template<typename T>
class LMI_SO xml_serializable
{
  public:
    virtual ~xml_serializable();

    void read (xml::element const&);
    void write(xml::element&) const;

  private:
    virtual int         class_version() const = 0;
    virtual std::string xml_root_name() const = 0;
    virtual bool        is_detritus(std::string const&) const = 0;
    virtual std::string redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,std::string const& value
        ) const = 0;
    virtual void        redintegrate_ex_post
        (int                                file_version
        ,std::map<std::string, std::string> detritus_map
        ,std::list<std::string>             residuary_names
        ) = 0;
    virtual void        redintegrate_ad_terminum() = 0;
};

template<typename T>
inline xml::element const& operator>>(xml::element const& x, xml_serializable<T>& z)
{
    z.read(x);
    return x;
}

template<typename T>
inline xml::element& operator<<(xml::element& x, xml_serializable<T> const& z)
{
    z.write(x);
    return x;
}

#endif // xml_serializable_hpp

