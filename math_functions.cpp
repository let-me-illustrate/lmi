// Miscellaneous mathematical operations.
//
// Copyright (C) 2022 Gregory W. Chicares.
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

#include "math_functions.hpp"

#include "fdlibm.hpp"                   // fdlibm_expm1(), fdlibm_log1p()

// expm1() and log1p()
//
// Production uses only the 'double' overloads, which forward to
// fdlibm implementations, which are substantially the same as
// glibc's. Motivation: to use identical code for all x86_64
// architectures, especially for MinGW-w64, whose implementations
// are x87 code.
//
// For 'float' and 'long double', simply forward to the C RTL.

namespace lmi
{
      float expm1(      float z) {return std::expm1(z);}
      float log1p(      float z) {return std::log1p(z);}
     double expm1(     double z) {return fdlibm_expm1(z);}
     double log1p(     double z) {return fdlibm_log1p(z);}
long double expm1(long double z) {return std::expm1(z);}
long double log1p(long double z) {return std::log1p(z);}
} // namespace lmi
