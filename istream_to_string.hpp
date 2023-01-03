// Read stream into a string. Typical use: read an entire file.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef istream_to_string_hpp
#define istream_to_string_hpp

#include "config.hpp"

#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

/// Read a stream into a string. Typical use: read an entire file. See:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/667bab411f51cf67
/// for a discussion of both alternative implementations.
///
/// Don't call basic_ostream::operator<<() if the input stream buffer
/// has no character to provide. Otherwise [27.6.2.5.3/8] it would set
/// 'failbit', and a spurious exception would be thrown when an empty
/// file is read.

template<typename Char_t, typename Traits, typename Allocator>
void istream_to_string
    (std::istream const&                         is
    ,std::basic_string<Char_t,Traits,Allocator>& s
    )
{
#if 0
    // Requires <iterator>.
    typedef std::istreambuf_iterator<Char_t,Traits> bisbi;
    typedef std::basic_string<Char_t,Traits,Allocator> string_type;
    string_type((bisbi(is)), bisbi()).swap(s);
    if(!is)
        {
        throw std::runtime_error("Unable to read stream into string.");
        }
#else // !0
    std::basic_ostringstream<Char_t,Traits,Allocator> oss;
    if(Traits::eof() != is.rdbuf()->sgetc())
        {
        oss << is.rdbuf();
        }
    oss.str().swap(s);

    if(!is || !oss)
        {
        throw std::runtime_error("Unable to read stream into string.");
        }
#endif // !0
}

#endif // istream_to_string_hpp
