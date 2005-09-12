// A value-Constrained Enumeration for product names.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: ce_product_name.cpp,v 1.3 2005-09-12 01:32:19 chicares Exp $

#include "ce_product_name.hpp"

#include "alert.hpp"
#include "facets.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iterator>

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

ce_product_name::ce_product_name()
    :mc_enum_base(product_names().size())
    ,value_(product_names().front())
{
}

ce_product_name::ce_product_name(std::string const& s)
    :mc_enum_base(product_names().size())
    ,value_(product_names()[ordinal(s)])
{
}

ce_product_name& ce_product_name::operator=(std::string const& s)
{
    value_ = product_names()[ordinal(s)];
    return *this;
}

bool ce_product_name::operator==(ce_product_name const& z) const
{
    return z.value_ == value_;
}

bool ce_product_name::operator==(std::string const& s) const
{
    return s == str();
}

std::istream& ce_product_name::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    std::string s;
    is >> s;
    operator=(s);
    is.imbue(old_locale);
    return is;
}

std::ostream& ce_product_name::write(std::ostream& os) const
{
    return os << str();
}

std::size_t ce_product_name::allowed_ordinal() const
{
    return ordinal();
}

std::size_t ce_product_name::cardinality() const
{
    return product_names().size();
}

std::string ce_product_name::str(int j) const
{
    return product_names()[j];
}

std::string ce_product_name::str() const
{
    return value_;
}

std::string ce_product_name::value() const
{
    return value_;
}

std::size_t ce_product_name::ordinal() const
{
    return ordinal(value_);
}

std::size_t ce_product_name::ordinal(std::string const& s) const
{
// TODO ?? Shouldn't this be ptrdiff_t?
    std::size_t v =
            std::find
                (product_names().begin()
                ,product_names().end()
                ,s
                )
        -   product_names().begin()
        ;
    if(v == product_names().size())
        {
        fatal_error()
            << "Value '"
            << s
            << "' invalid for type '"
            << "ce_product_name"
            << "'."
            << LMI_FLUSH
            ;
        }
    return v;
}

std::vector<std::string> const& ce_product_name::product_names()const
{
    static std::vector<std::string> names(fetch_product_names());
    return names;
}

