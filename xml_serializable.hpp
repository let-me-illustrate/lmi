// Mixin for xml serialization.
//
// Copyright (C) 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef xml_serializable_hpp
#define xml_serializable_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <boost/filesystem/path.hpp>

#include <list>
#include <map>
#include <string>

/// Type of a deserialized xml element.
///
/// Specialize this if the intended type is not interconvertible with
/// std::string.

template<typename T>
struct deserialized
{
    typedef std::string value_type;
};

/// Derive from this mixin class to use its xml serialization.
///
/// Implicitly-declared special member functions do the right thing.

template<typename T>
class LMI_SO xml_serializable
{
    typedef typename deserialized<T>::value_type value_type;

  public:
    virtual ~xml_serializable();

    void load(fs::path const&);
    void save(fs::path const&) const;

    void read (xml::element const&);
    void write(xml::element&) const;

  private:
    // Private non-virtuals.
    T      & t()      ;
    T const& t() const;
    void immit_members_into(xml::element&) const;

    // Class (T) identification.

    /// Backward-compatibility serial number of class T's xml version.
    virtual int                class_version() const = 0;
    /// Root tag (when T is saved as the root of a document).
    virtual std::string const& xml_root_name() const = 0;

    // Reading and writing.
    virtual value_type fetch_element
        (xml::element const& e
        ) const;
    virtual void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        );
    virtual void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const;
    virtual void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_leaf_name
        ) const;

    // Backward compatibility.
    virtual void handle_missing_version_attribute() const;
    virtual bool is_detritus(std::string const&) const;
    virtual void redintegrate_ex_ante
        (int                file_version
        ,std::string const& name
        ,value_type       & value
        ) const;
    virtual void redintegrate_ex_post
        (int                                     file_version
        ,std::map<std::string,value_type> const& detritus_map
        ,std::list<std::string>           const& residuary_names
        );
    virtual void redintegrate_ad_terminum();
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
