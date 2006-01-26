// List of available products.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: product_names.cpp,v 1.1 2006-01-26 07:11:01 chicares Exp $

#include "product_names.hpp"

#include "alert.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"

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
        if(is_directory(*i) || ".pol" != fs::extension(*i))
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
            << path.string()
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
        (   product_names().end()
        !=  std::find(product_names().begin(), product_names().end(), "sample")
        )
        ? std::string("sample")
        : product_names().front()
        ;
    return default_name;
}

