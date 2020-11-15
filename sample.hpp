// Parameters for a company's entire portfolio of products.
//
// Copyright (C) 2020 Gregory W. Chicares.
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

#ifndef sample_hpp
#define sample_hpp

#include "config.hpp"

// For now, this file contains only an enumeration, but someday it may
// include other information that applies to an entire portfolio.

/// For the fictional Superior Life Insurance Company of Superior, WI.

namespace superior
{
/// Enumerate lingo strings.
///
/// This is deliberately defined with enum-key 'enum' rather than
/// 'enum class' or 'enum struct'. Because it is defined inside a
/// namespace, with an enum-base, it is the same as an 'enum class'
/// except that its enumerators decay to int as nature intended.
/// Feature comparison:
///
///   this enum  enum class   desirable properties
///   ---------  ----------   --------------------
///      yes        yes       avoids polluting global namespace
///      yes        yes       specifies underlying type
///      yes         no       implicitly converts to int
///
/// Enumerator zero is reserved for an empty string because zero is
/// the default value for database entities.

enum lingo : int
    {empty_string = 0
    ,policy_form_term
    ,policy_form
    ,policy_form_KS_KY
    };
} // namespace superior

#endif // sample_hpp
