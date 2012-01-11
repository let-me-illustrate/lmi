// SGI extensions to STL.
//
// Copyright (C) 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

// Copyright (C) 1994
// Hewlett-Packard Company
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Hewlett-Packard Company makes no
// representations about the suitability of this software for any
// purpose.  It is provided "as is" without express or implied warranty.
//
// Copyright (C) 1996-1998
// Silicon Graphics Computer Systems, Inc.
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Silicon Graphics makes no
// representations about the suitability of this software for any
// purpose.  It is provided "as is" without express or implied warranty.

// This implementation is a derived work based on code SGI released
// with the above disclaimers.
//
// Gregory W. Chicares modified it trivially in 2002 and 2005, and in
// later years as indicated in 'ChangeLog'. Any defect in it should
// not reflect on SGI's or HP's reputation.

#ifndef stl_extensions_hpp
#define stl_extensions_hpp

#include "config.hpp"

namespace nonstd
{
template<typename InputIterator, typename Size, typename OutputIterator>
void copy_n(InputIterator first, Size count, OutputIterator result)
{
    Size j = count;
    for(; 0 < j; --j)
        {
        *result = *first;
        ++first;
        ++result;
        }
}

/// is_sorted, a predicate testing whether a range is sorted in
/// nondescending order.  This is an extension, not part of the C++
/// standard.

template<typename ForwardIterator>
bool is_sorted
    (ForwardIterator first
    ,ForwardIterator last
    )
{
    if(first == last)
        {
        return true;
        }

    ForwardIterator next = first;
    for(++next; next != last; first = next, ++next)
        {
        if(*next < *first)
            {
            return false;
            }
        }

    return true;
}

template<typename ForwardIterator, typename StrictWeakOrdering>
bool is_sorted
    (ForwardIterator first
    ,ForwardIterator last
    ,StrictWeakOrdering comp
    )
{
    if(first == last)
        {
        return true;
        }

    ForwardIterator next = first;
    for(++next; next != last; first = next, ++next)
        {
        if(comp(*next, *first))
            {
            return false;
            }
        }

    return true;
}
} // namespace nonstd

#endif // stl_extensions_hpp

