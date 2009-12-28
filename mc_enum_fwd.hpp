// Forward declaration of Mapped Constrained Enumeration template.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: mc_enum_fwd.hpp,v 1.6 2008-12-27 02:56:48 chicares Exp $

#ifndef mc_enum_fwd_hpp
#define mc_enum_fwd_hpp

#include "config.hpp"

#include <cstddef> // std::size_t

template<typename T, std::size_t n, T const (*e)[n], char const*const (*c)[n]>
class mc_enum;

#endif // mc_enum_fwd_hpp

