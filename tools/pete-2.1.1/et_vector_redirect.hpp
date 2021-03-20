// PETE with std::vector.
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

// This header would seem to belong in lmi's main source directory,
// but instead it resides in a subdirectory due to the weird structure
// of PETE. Study these old discussions:
//    https://lists.nongnu.org/archive/html/lmi/2008-09/msg00021.html
//    https://lists.nongnu.org/archive/html/lmi/2008-11/msg00008.html
// before attempting to "fix" this.
//
// Improved: Moved 'et_vector.hpp' to lmi's main source directory.
// Stripped almost everything out of the present header, and renamed
// it; now it serves only to work around the PETE weirdness mentioned
// above, by redirecting the inclusion of PETE headers.

#ifndef et_vector_redirect_hpp
#define et_vector_redirect_hpp

#if 0
#include "config.hpp"
#endif // 0

#include "PETE/PETE.h"

#include <vector>

// Include "et_vector_operators.hpp" last because it's generated
// automatically and doesn't include all the headers it needs.

// gcc's '-Weffc++' flags user-defined boolean AND and OR operators
// because they cannot implement short-circuit evaluation. Although
// Meyers's "Never overload &&, ||, or ," is a generally sound rule
// for scalar classes, it doesn't make sense for containers, where
// short-circuit evaluation is not possible anyway. Presumably this
// warning would be issued for std::valarray::operator||() and &&(),
// except that they're standard. Similarly, it would issued here for
// PETE's || and && operators, except for the pragma.

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Weffc++"
#endif // defined __GNUC__
#include "et_vector_operators.hpp"
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__

#endif // et_vector_redirect_hpp
