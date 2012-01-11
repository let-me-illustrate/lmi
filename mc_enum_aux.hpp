// Auxiliary functions for Mapped Constrained Enumerations.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

// $Id$

#ifndef mc_enum_aux_hpp
#define mc_enum_aux_hpp

#include "config.hpp"

#include "mc_enum_fwd.hpp"

#include <string>
#include <vector>

template<typename T>
std::vector<std::string> mc_e_vector_to_string_vector
    (std::vector<mc_enum<T> > const& ve
    )
{
    std::vector<std::string> vs;
    typename std::vector<mc_enum<T> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        vs.push_back(ve_i->str());
        }
    return vs;
}

#endif // mc_enum_aux_hpp

