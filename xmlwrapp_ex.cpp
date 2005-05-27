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

// $Id: xmlwrapp_ex.cpp,v 1.3 2005-05-27 10:37:06 chicares Exp $

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
#if !defined __BORLANDC__
// COMPILER !! bc++5.5.1 has istreambuf_iterator, but it doesn't work here.
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
#else // defined __BORLANDC__
    is.unsetf(std::ios_base::skipws);
    std::vector<char>v;
    std::copy
        (std::istream_iterator<char>(is)
        ,std::istream_iterator<char>()
        ,std::back_inserter(v)
        );
    if(!is.eof())
        {
        throw std::runtime_error("Unable to read input file into string.");
        }
// COMPILER !! This takes way too long with bc++5.02:
//    std::string s(v.begin(), v.end());
    std::string s;
    s.reserve(v.size());
    return s.assign(&v[0], v.size());
#endif // defined __BORLANDC__
}

