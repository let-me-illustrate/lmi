// Utilities for representing and generating HTML.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "html.hpp"

#include <cstring>                      // strlen()

namespace html
{

namespace attr
{

extern attribute const align        ("align");
extern attribute const cellpadding  ("cellpadding");
extern attribute const cellspacing  ("cellspacing");
extern attribute const colspan      ("colspan");
extern attribute const nowrap       ("nowrap");
extern attribute const size         ("size");
extern attribute const valign       ("valign");
extern attribute const width        ("width");

} // namespace attr

namespace tag
{

extern element      const b         ("b");
extern void_element const br        ("br");
extern element      const font      ("font");
extern element      const i         ("i");
extern element      const p         ("p");
extern element      const table     ("table");
extern element      const td        ("td");
extern element      const tr        ("tr");

} // namespace tag

std::string attribute::as_string() const
{
    std::string s(name_);
    if(!value_.empty())
        {
        s += "=";
        // PDF !! Escape quotes.
        s += value_;
        }
    return s;
}

namespace detail
{

std::string any_element::get_start() const
{
    std::string s("<");
    // Extra +1 for the space before attributes, even if it's not needed.
    s.reserve(1 + std::strlen(name_) + 1 + attributes_.length() + 1);
    s += name_;
    if(!attributes_.empty())
        {
        s += " ";
        s += attributes_;
        }
    s += ">";
    return s;
}

void any_element::update_attributes(attribute const& attr)
{
    if(attributes_.empty())
        {
        attributes_ = attr.as_string();
        }
    else
        {
        attributes_ += " ";
        attributes_ += attr.as_string();
        }
}

} // namespace detail

void element::update_contents(std::string&& contents)
{
    if(contents_.empty())
        {
        contents_ = std::move(contents);
        }
    else
        {
        contents_ += contents;
        }
}

element::operator text() const
{
    std::string s(get_start());
    s.reserve(s.length() + contents_.length() + 2 + std::strlen(name_) + 1);
    s += contents_;
    s += "</";
    s += name_;
    s += ">";

    return text::from_html(std::move(s));
}

} // namespace html
