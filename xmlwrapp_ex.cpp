// Stream extraction operator for xmlwrapp.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xmlwrapp_ex.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xmlwrapp_ex.hpp"

#include <cstddef>
#include <ios>
#include <iterator>
#include <stdexcept>
#include <vector>

std::string istream_to_string(std::istream& is)
{
#if !defined __BORLANDC__ && !defined GCC_BEFORE_2_96
// COMPILER !! bc++5.02 and gcc-2.95.2-1 lack istreambuf_iterator;
// bc++5.5.1 has it, but it doesn't work here.
    is >> std::noskipws;
    std::string s
        ((std::istreambuf_iterator<char>(is))
        ,std::istreambuf_iterator<char>()
        );

    if(!is.eof())
        {
// http://groups.google.com/groups?selm=d6651fb6.0210180402.53236e96%40posting.google.com        
//        throw std::runtime_error("Unable to read input file into string.");
        }
    return s;
#else // defined __BORLANDC__ || defined GCC_BEFORE_2_96
// COMPILER !! Incorrect declaration used by bc++5.02:
//    template<typename T, typename Distance> class istream_iterator;
// COMPILER !! incorrect declaration used by gcc-2.95.2-1:
//    template<typename T, typename Distance=ptrdiff_t> class istream_iterator;
    is.unsetf(std::ios_base::skipws);
    std::vector<char>v;
#   ifndef BC_BEFORE_5_5
    std::copy
        (std::istream_iterator<char>(is)
        ,std::istream_iterator<char>()
        ,std::back_inserter(v)
        );
#   else // defined BC_BEFORE_5_5
    std::copy
        (std::istream_iterator<char, std::ptrdiff_t>(is)
        ,std::istream_iterator<char, std::ptrdiff_t>()
        ,std::back_inserter(v)
        );
#   endif // defined BC_BEFORE_5_5
    if(!is.eof())
        {
        throw std::runtime_error("Unable to read input file into string.");
        }
// COMPILER !! This takes way too long with bc++5.02:
//    std::string s(v.begin(), v.end());
    std::string s;
    s.reserve(v.size());
    return s.assign(&v[0], v.size());
#endif // defined BC_BEFORE_5_5 || defined GCC_BEFORE_2_96
}

