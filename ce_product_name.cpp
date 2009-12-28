// A value-Constrained Enumeration for product names.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ce_product_name.cpp,v 1.14 2008-12-27 02:56:37 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ce_product_name.hpp"

#include "alert.hpp"
#include "facets.hpp"
#include "product_names.hpp"

#include <algorithm>

ce_product_name::ce_product_name()
    :mc_enum_base(product_names().size())
    ,value_(default_product_name())
{}

ce_product_name::ce_product_name(std::string const& s)
    :mc_enum_base(product_names().size())
    ,value_(product_names()[ordinal(s)])
{}

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

std::size_t ce_product_name::cardinality() const
{
    return product_names().size();
}

std::size_t ce_product_name::ordinal() const
{
    return ordinal(value_);
}

std::size_t ce_product_name::ordinal(std::string const& s)
{
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

std::vector<std::string> const& ce_product_name::product_names()
{
    return ::product_names();
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

/// No product is ever proscribed.

void ce_product_name::enforce_proscription()
{}

