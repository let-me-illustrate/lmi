// Interpret sequence strings containing mc_enum and tn_range types.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

// Input sequences are often used with mc_enum and tn_range types.
// This header contains some auxiliary functions that are useful for
// those UDTs in combination with sequences, while keeping all those
// entities physically separate otherwise.

#ifndef input_sequence_aux_hpp
#define input_sequence_aux_hpp

#include "config.hpp"

#include "alert.hpp"
#include "input_sequence.hpp"
#include "mc_enum.hpp"
#include "so_attributes.hpp"
#include "ssize_lmi.hpp"
#include "tn_range.hpp"

#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace detail
{
    typedef std::map<std::string,std::string> stringmap;

    LMI_SO std::vector<std::string> extract_keys_from_string_map
        (stringmap const& keyword_dictionary
        );

    template<typename T>
    void convert_vector
        (std::vector<T>           & dst
        ,std::vector<double> const& src
        )
    {
        int len = lmi::ssize(src);
        dst.resize(len);
        for(int j = 0; j < len; ++j)
            {
            dst[j] = src[j];
            }
    }

    template<typename T>
    void convert_vector
        (std::vector<T>                & dst
        ,std::vector<std::string> const& src
        ,stringmap                const& dictionary
        ,std::string              const& default_keyword
        )
    {
        int len = lmi::ssize(src);
        dst.resize(len);
        for(int j = 0; j < len; ++j)
            {
            if(default_keyword == src[j])
                {
                dst[j] = T();
                }
            else if(dictionary.find(src[j]) != dictionary.end())
                {
                dst[j] = (*dictionary.find(src[j])).second;
                }
            else
                {
                dst[j] = T(); // COMPILER !! Cuz we can't throw...see below.
                alarum()
                    << "Key '"
                    << src[j]
                    << "' not found in map."
                    << LMI_FLUSH
                    ;
// COMPILER !! mingw gcc-2.95.2-1 doesn't throw an exception here; instead,
// the app just sits there eating resources, apparently forever.
//                throw std::logic_error(error.str());
// So instead we do this, although we'd rather have made this file
// independent of the MB- stuff.
                }
            }
    }
} // namespace detail

template<typename T>
std::vector<T> convert_vector_type
    (std::vector<mc_enum<T>> const& ve
    ,typename std::enable_if<std::is_enum_v<T>>::type* = nullptr
    )
{
    std::vector<T> z;
    for(auto const& i : ve)
        {
        z.push_back(i.value());
        }
    return z;
}

template<typename Number, typename Trammel>
std::vector<Number> convert_vector_type
    (std::vector<tn_range<Number,Trammel>> const& vr
    )
{
    std::vector<Number> z;
    for(auto const& i : vr)
        {
        z.push_back(i.value());
        }
    return z;
}

#endif // input_sequence_aux_hpp
