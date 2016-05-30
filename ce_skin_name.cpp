// A value-Constrained Enumeration for skin names.
//
// Copyright (C) 2016 Gregory W. Chicares.
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

#include "ce_skin_name.hpp"

#include "alert.hpp"
#include "facets.hpp"
#include "global_settings.hpp"
#include "path_utility.hpp"             // fs::path inserter

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <cstring>

namespace
{
std::string const& default_skin_name()
{
    static std::string const default_name("default");
    return default_name;
}

std::vector<std::string> fetch_skin_names()
{
    fs::path path(global_settings::instance().data_directory());
    std::vector<std::string> names;
    fs::directory_iterator i(path);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        if(is_directory(*i) || ".xrc" != fs::extension(*i))
            {
            continue;
            }
        std::string const name(i->leaf());
        // Skin files are expected to be called "skin_something.xrc" with the
        // exception of "skin.xrc".
        static char const* skin_prefix = "skin";
        static std::size_t const skin_prefix_len = std::strlen(skin_prefix);
        if(name.compare(0, skin_prefix_len, skin_prefix) != 0)
            {
            continue;
            }
        if(fs::basename(*i).length() > skin_prefix_len && name[skin_prefix_len] != '_')
            {
            continue;
            }

        names.push_back(name);
        }

    if(names.empty())
        {
        fatal_error()
            << "Data directory '"
            << path
            << "' contains no skin files."
            << LMI_FLUSH
            ;
        }

    return names;
}
} // Unnamed namespace.

ce_skin_name::ce_skin_name()
    :mc_enum_base(skin_names().size())
    ,value_(default_skin_name())
{}

ce_skin_name::ce_skin_name(std::string const& s)
    :mc_enum_base(skin_names().size())
    ,value_(skin_names()[ordinal(s)])
{}

ce_skin_name& ce_skin_name::operator=(std::string const& s)
{
    value_ = skin_names()[ordinal(s)];
    return *this;
}

bool ce_skin_name::operator==(ce_skin_name const& z) const
{
    return z.value_ == value_;
}

bool ce_skin_name::operator==(std::string const& s) const
{
    return s == str();
}

std::size_t ce_skin_name::ordinal(std::string const& s)
{
    std::size_t v =
            std::find
                (skin_names().begin()
                ,skin_names().end()
                ,s
                )
        -   skin_names().begin()
        ;
    if(v == skin_names().size())
        {
        fatal_error()
            << "Value '"
            << s
            << "' invalid for type '"
            << "ce_skin_name"
            << "'."
            << LMI_FLUSH
            ;
        }
    return v;
}

std::vector<std::string> const& ce_skin_name::all_strings() const
{
    return skin_names();
}

std::size_t ce_skin_name::cardinality() const
{
    return skin_names().size();
}

/// No skin is ever proscribed.

void ce_skin_name::enforce_proscription()
{}

std::size_t ce_skin_name::ordinal() const
{
    return ordinal(value_);
}

std::string ce_skin_name::str(int j) const
{
    return skin_names()[j];
}

std::string ce_skin_name::str() const
{
    return value_;
}

std::string ce_skin_name::value() const
{
    return value_;
}

std::vector<std::string> const& ce_skin_name::skin_names()
{
    static std::vector<std::string> const names(fetch_skin_names());
    return names;
}

std::istream& ce_skin_name::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    std::string s;
    is >> s;
    operator=(s);
    is.imbue(old_locale);
    return is;
}

std::ostream& ce_skin_name::write(std::ostream& os) const
{
    return os << str();
}

