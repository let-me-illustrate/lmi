// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef interpolate_string_hpp
#define interpolate_string_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <functional>                   // function
#include <string>

enum class interpolate_lookup_kind
    {variable
    ,section
    ,partial
    };

using lookup_function
    = std::function<std::string (std::string const&, interpolate_lookup_kind)>;

/// Interpolate string containing embedded variable references.
///
/// Return the input string after recursively replacing all {{variable}}
/// references in it with the value of the variable as returned by the provided
/// function. The syntax is a subset of Mustache templates with the following
/// features being supported:
///  - Recursive variable expansion for {{variable}}, i.e. -- unlike in
///    Mustache -- any {{...}} in the returned expansion are expanded again.
///  - Conditional expansion using {{#variable}}...{{/variable}}.
///  - Negated checks of the form {{^variable}}...{{/variable}}.
///  - Partials support, i.e. {{>filename}}.
///  - Comments of the form {{!this is ignored}}.
///
/// The following features are explicitly *not* supported:
///  - HTML escaping: this is done by a separate html::text class.
///  - Separate types: 0/1 is false/true, anything else is an error.
///  - Lists/section iteration (not needed yet).
///  - Lambdas: can't be implemented in non-dynamic languages such as C++.
///  - Changing delimiters: omitted for simplicity (to allow embedding literal
///    "{{" fragment into the returned string, create a pseudo-variable
///    expanding to these characters).
///
/// Throw if the lookup function throws, if the string uses invalid syntax or
/// if the maximum recursion level is exceeded.

LMI_SO std::string interpolate_string
    (char const* s
    ,lookup_function const& lookup
    );

#endif // interpolate_string_hpp
