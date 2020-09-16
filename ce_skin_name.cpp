// A value-Constrained Enumeration for skin names.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ce_skin_name.hpp"

#include "alert.hpp"
#include "bourn_cast.hpp"
#include "contains.hpp"
#include "facets.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"               // begins_with()
#include "path.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "ssize_lmi.hpp"

#include <algorithm>                    // find()

namespace
{
std::vector<std::string> fetch_skin_names()
{
    fs::path path(global_settings::instance().data_directory());
    std::vector<std::string> names;
    for(auto const& i : fs::directory_iterator(path))
        {
        if(i.is_directory())
            {
            continue;
            }
        auto const current_path{i.path()};
        if(".xrc" != current_path.extension())
            {
            continue;
            }
        std::string const name(current_path.filename().string());
        if(!begins_with(name, "skin"))
            {
            continue;
            }
        names.push_back(name);
        }

    if(names.empty())
        {
        alarum()
            << "Data directory '"
            << path
            << "' contains no skin files."
            << LMI_FLUSH
            ;
        }

    return names;
}

std::vector<std::string> const& skin_names()
{
    static std::vector<std::string> const names(fetch_skin_names());
    return names;
}

/// Default skin is 'skin.xrc' if that file exists,
/// else the first skin file found.

std::string const& default_skin_name()
{
    static std::string const default_name =
        contains(skin_names(), "skin.xrc")
        ? std::string("skin.xrc")
        : skin_names().front()
        ;
    return default_name;
}
} // Unnamed namespace.

ce_skin_name::ce_skin_name()
    :mc_enum_base (lmi::ssize(skin_names()))
    ,value_       {default_skin_name()}
{}

ce_skin_name::ce_skin_name(std::string const& s)
    :mc_enum_base (lmi::ssize(skin_names()))
    ,value_       {skin_names()[ordinal(s)]}
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

int ce_skin_name::ordinal(std::string const& s)
{
    auto v = bourn_cast<int>
        ( std::find
            (skin_names().begin()
            ,skin_names().end()
            ,s
            )
        - skin_names().begin()
        );
    if(v == lmi::ssize(skin_names()))
        {
        alarum()
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

int ce_skin_name::cardinality() const
{
    return lmi::ssize(skin_names());
}

/// No skin is ever proscribed.

void ce_skin_name::enforce_proscription()
{}

int ce_skin_name::ordinal() const
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

/// DWISOTT.
///
/// Calls operator=(std::string const&), which throws if the value
/// read from the stream is invalid.

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
