// Interpret sequence strings.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef input_seq_helpers_hpp
#define input_seq_helpers_hpp

#include "config.hpp"

#include "alert.hpp"
#include "input_sequence.hpp"

#include <map>
#include <string>
#include <vector>

namespace detail
{
    typedef
        std::map<std::string, std::string, std::less<std::string> >
        stringmap
        ;

    typedef
        std::map<std::string, std::string, std::less<std::string> >::const_iterator
        stringmap_iterator
        ;

    stringmap invert_map(stringmap const& sm);

    std::vector<std::string> extract_keys_from_string_map
        (stringmap const& keyword_dictionary
        );

    template<typename T>
    void convert_vector
        (std::vector<T>           & dst
        ,std::vector<double> const& src
        )
    {
        int len = src.size();
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
        int len = src.size();
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
                fatal_error()
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

// Input-sequence fields use their own keywords, which might differ from
// xenum strings.
//
// TODO ?? Perhaps we should just use the same strings.
//
// TODO ?? Perhaps we should move this elsewhere.
//
// We return a map by value so that we can modify it, e.g. by removing
// a death benefit option not available with a certain policy form.

std::map<std::string, std::string, std::less<std::string> > dbo_map();
std::map<std::string, std::string, std::less<std::string> > mode_map();
std::map<std::string, std::string, std::less<std::string> > sastrategy_map();
std::map<std::string, std::string, std::less<std::string> > pmtstrategy_map();

#endif // input_seq_helpers_hpp

