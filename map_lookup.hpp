// Look up a value in a std::map.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef map_lookup_hpp
#define map_lookup_hpp

#include "config.hpp"

#include <map>
#include <sstream>
#include <stdexcept>

/// Retrieve a value from a std::map; throw if key not found.
///
/// The reason why this isn't called 'operator[]() const' is discussed
/// in the thread beginning here:
///   http://groups.google.com/group/comp.std.c++/msg/c1df492f65f29c5a

template<typename map_t>
typename map_t::mapped_type const& map_lookup
    (map_t const&                    map
    ,typename map_t::key_type const& key
    )
{
    typename map_t::const_iterator i = map.find(key);
    if(map.end() == i)
        {
        std::ostringstream oss;
        oss
            << "map_lookup: key '"
            << key
            << "' not found."
            ;
        throw std::runtime_error(oss.str());
        }

    return i->second;
}

#endif // map_lookup_hpp
