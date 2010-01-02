// Auxiliary functions for Mapped Constrained Enumerations.
//
// Copyright (C) 2008, 2009 Gregory W. Chicares.
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

// $Id: mc_enum_aux.hpp,v 1.2 2008-12-27 02:56:48 chicares Exp $

#ifndef mc_enum_aux_hpp
#define mc_enum_aux_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"

#include <string>
#include <vector>

template<typename T, std::size_t n, T const (*e)[n], char const*const (*c)[n]>
std::vector<std::string> mc_e_vector_to_string_vector
    (std::vector<mc_enum<T,n,e,c> > const& ve
    )
{
    std::vector<std::string> vs;
    typename std::vector<mc_enum<T,n,e,c> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        vs.push_back(ve_i->str());
        }
    return vs;
}

#endif // mc_enum_aux_hpp

