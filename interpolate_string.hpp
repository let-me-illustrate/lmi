// Interpolate string containing embedded variable references.
//
// Copyright (C) 2017, 2018 Gregory W. Chicares.
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

#ifndef interpolate_string_hpp
#define interpolate_string_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <functional>                   // function
#include <string>

enum class interpolate_lookup_kind
{
    variable,
    section,
    partial
};

using lookup_function
    = std::function<std::string (std::string const&, interpolate_lookup_kind)>;

/// Interpolate string containing embedded variable references.
///
/// Return the input string after replacing all {{variable}} references in it
/// with the value of the variable as returned by the provided function. The
/// syntax is a (strict) subset of Mustache templates, the following features
/// are supported:
///  - Simple variable expansion for {{variable}}.
///  - Conditional expansion using {{#variable}}...{{/variable}}.
///  - Negated checks of the form {{^variable}}...{{/variable}}.
///  - Partials support, i.e. {{>filename}}.
///
/// The following features are explicitly _not_ supported:
///  - HTML escaping: this is done by a separate html::text class.
///  - Separate types: 0/1 is false/true, anything else is an error.
///  - Lists/section iteration (not needed yet).
///  - Lambdas, comments, delimiter changes: omitted for simplicity.
///
/// To allow embedding literal "{{" fragment into the returned string, create a
/// pseudo-variable expanding to these characters as its expansion, there is no
/// built-in way to escape them.
///
/// Throw if the lookup function throws or if the string uses invalid syntax.

std::string LMI_SO interpolate_string
    (char const* s
    ,lookup_function const& lookup
    );

#endif // interpolate_string_hpp
