// SGI's is_sorted() enhancement to STL.
//
// Copyright (C) 2002, 2005 Gregory W. Chicares.
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

// $Id: is_sorted_.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

// Copyright (c) 1994
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
// Copyright (c) 1996
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
// Gregory W. Chicares modified it trivially in 2002 and 2005.
// Any defect in it should not reflect on SGI's or HP's reputation.

#ifndef is_sorted_hpp
#define is_sorted_hpp

#include "config.hpp"

// is_sorted, a predicate testing whether a range is sorted in
// nondescending order.  This is an extension, not part of the C++
// standard.

namespace nonstd
{
    template<typename ForwardIterator>
    bool is_sorted_
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
    bool is_sorted_
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
}

#endif // is_sorted_hpp

