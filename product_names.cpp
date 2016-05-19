// List of available products.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "product_names.hpp"

#include "alert.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp" // fs::path inserter

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>

namespace
{
std::vector<std::string> fetch_product_names()
{
    fs::path path(global_settings::instance().data_directory());
    std::vector<std::string> names;
    fs::directory_iterator i(path);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        if(is_directory(*i) || ".policy" != fs::extension(*i))
            {
            continue;
            }
        std::string name(basename(*i));
        std::transform
            (name.begin()
            ,name.end()
            ,name.begin()
            ,lmi_tolower
            );
        names.push_back(name);
        }

    if(0 == names.size())
        {
        fatal_error()
            << "Data directory '"
            << path
            << "' contains no product files."
            << LMI_FLUSH
            ;
        }

    return names;
}
} // Unnamed namespace.

std::vector<std::string> const& product_names()
{
    static std::vector<std::string> const names(fetch_product_names());
    return names;
}

std::string const& default_product_name()
{
    static std::string const default_name =
        contains(product_names(), "sample")
        ? std::string("sample")
        : product_names().front()
        ;
    return default_name;
}

