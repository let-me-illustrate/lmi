// Convert between types as extractors and inserters do.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
//  - restructure, trading private-class encapsulation for simplicity
//  - remove deprecated std::strstream support
//  - optimize conversions between std::string and char*
//  - 'To' type can be deduced from optional argument
//  - diagnostic reports argument types and says which operation failed
//  - permit conversion from a std::string with embedded blanks
// and in any later year shown above; any defect here should not
// reflect on Kevlin Henney's reputation.

#ifndef stream_cast_hpp
#define stream_cast_hpp

#include "config.hpp"

#include "facets.hpp"
#include "rtti_lmi.hpp"

#include <istream>                      // ws
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

/// Design notes for function template stream_cast.
///
/// stream_cast converts between types as extractors and inserters do.
///
/// Suppose a std::string is to be cast to a std::string, and suppose
/// it contains embedded whitespace. What is the least astonishing
/// behavior? The boost library is founded on passing data in and out
/// of a std::stringstream; it uses a standard extractor that breaks
/// the data at any whitespace. This implementation chooses instead to
/// preserve the string's value by replacing the default std::ctype
/// facet with one that does not treat blanks as whitespace.
///
/// The boost library, at least in version 1.31.0, uses
///   interpreter.unsetf(std::ios::skipws);
/// (as explained here:
///    http://lists.boost.org/MailArchives/boost-users/msg03656.php
/// that change was intended to fix one problem, yet caused another)
/// but that fails for strings with embedded spaces, whereas the
/// approach used here does not. With the former option, that defect
/// is masked by the specializations here for std::string, but that
/// option would require those specializations for correctness; with
/// the option used here, they're merely optimizations.
///
/// INELEGANT !! Specializations for conversion from string to string
/// here should be removed (value_cast is better), and the preceding
/// paragraph should be revised accordingly. It should be pointed out
/// that conversion from an empty string to another string fails even
/// with the current implementation unless both strings are of exact
/// type std::string (but it works with value_cast).
///
/// Blank is the only whitespace character not treated as whitespace,
/// because blanks are deliberately used in strings like "Allow MEC"
/// that are mapped to enumerators in 'mc_enum_types.xpp', where other
/// whitespace characters would not be used.
///
/// This technique is generally inappropriate (and the implementation
/// prevents its use) for arithmetic types, and especially for
/// floating types: instead, use numeric_io_cast, or, better yet, use
/// value_cast to select the most appropriate cast automatically.

template<typename To, typename From>
To stream_cast(From from, To = To())
{
    static_assert
        (   !std::is_arithmetic_v<From>
        ||  !std::is_arithmetic_v<To  >
        );
    static_assert(!std::is_pointer_v<To>);

    auto complain = [&](auto const& reason)
        {
        std::ostringstream err;
        err
            << reason
            << " converting '"
            << from
            << "' from type '"
            << lmi::TypeInfo(typeid(From))
            << "' to type '"
            << lmi::TypeInfo(typeid(To))
            << "'."
            ;
        throw std::runtime_error(err.str());
        };

    static std::stringstream interpreter = []
        {
        std::stringstream ss {};
        ss.imbue(blank_is_not_whitespace_locale());
        return ss;
        } ();
    interpreter.str(std::string{});
    interpreter.clear();

    To result = To();

    if(!(interpreter << from))
        {
        complain("Failure in ostream inserter");
        }
    else if(!(interpreter >> result))
        {
        complain("Failure in istream extractor");
        }
    else if(!(interpreter >> std::ws).eof())
        {
        complain("Unconverted data remains");
        }

    return result;
}

template<>
inline std::string stream_cast<std::string>(char* from, std::string)
{
    if(nullptr == from)
        {
        throw std::runtime_error
            ("Cannot convert (char*)(0) to std::string."
            );
        }
    return from;
}

template<>
inline std::string stream_cast<std::string>(char const* from, std::string)
{
    if(nullptr == from)
        {
        throw std::runtime_error
            ("Cannot convert (char const*)(0) to std::string."
            );
        }
    return from;
}

// It would be nice to return a std::string const& in this case, in
// order to avoid copying a string (which some string classes would
// avoid anyway); but boost::lexical_cast returns a value instead of
// a reference, and it seems good not to change Henney's deliberate
// design decision.
//
// No distinct specialization for std::string const is necessary or
// possible, because it's a value argument, so const is ignored for
// overload resolution.
//
template<>
inline std::string stream_cast<std::string>
    (std::string from
    ,std::string
    )
{
    return from;
}

#endif // stream_cast_hpp
