// Cast template to convert between types, e.g. number <-> string.
//
// Copyright (C) 2004 Gregory W. Chicares.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: value_cast.hpp,v 1.1.1.1 2004-05-15 19:59:43 chicares Exp $

// This is a derived work based on boost::lexical_cast, which bears
// the following copyright and permissions notice:
// [boost::lexical_cast notice begins]
// Copyright Kevlin Henney, 2000-2003. All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.
// [boost::lexical_cast notice ends]

// Modified by Gregory W. Chicares in 2004:
//  - conversions between std::string and char* optimized
//  - 'To' type can be deduced from optional argument
//  - diagnostic message reports argument types
// and in any later year shown above; any defect here should not
// reflect on Kevlin Henney's reputation.

// TODO ?? Floating-point numbers aren't handled well. Examine the
// latest boost implementation, which addresses this in some way:
// is the precision it uses sufficient to preserve values though?

// TODO ?? Shouldn't there be a conversion from string const& as well?
// TODO ?? This needs a unit test.

#ifndef value_cast_hpp
#define value_cast_hpp

#include "config.hpp"

#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

template<typename To, typename From>
To value_cast(From from, To = To())
{
    std::stringstream interpreter;
    To result;

    if
        (   !(interpreter << from)
        ||  !(interpreter >> result)
        ||  !(interpreter >> std::ws).eof()
        )
        {
        std::ostringstream oss;
        oss
            << "Failure converting '"
            << from
            << "' from type "
            << typeid(From).name()
            << " to type "
            << typeid(From).name()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }

    return result;
}

template<>
inline std::string value_cast<std::string>(char* from, std::string)
{
    return from;
}

template<>
inline std::string value_cast<std::string>(char const* from, std::string)
{
    return from;
}

// It would be nice to return a T const& in this case,
// but boost::lexical_cast returns a T so we can't
// avoid a copy (although a COW string class can).
template<>
inline std::string value_cast<std::string>
    (std::string const& from
    ,std::string
    )
{
    return from;
}

#endif // value_cast_hpp

