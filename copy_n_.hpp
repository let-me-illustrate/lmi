// SGI's copy_n() enhancement to STL.
//
// Copyright (C) 2001, 2005 Gregory W. Chicares.
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

// $Id: copy_n_.hpp,v 1.2 2005-03-10 04:37:17 chicares Exp $

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
// Gregory W. Chicares modified it trivially in 2001 and 2005.
// Any defect in it should not reflect on SGI's or HP's reputation.

#ifndef copy_n_hpp
#define copy_n_hpp

#include "config.hpp"

// TODO ?? Get rid of 'nonstd', which is a little too cute.
namespace nonstd
{
    template<typename InputIterator, typename Size, typename OutputIterator>
    void copy_n_(InputIterator first, Size count, OutputIterator result)
    {
        Size j = count;
        for(; j > 0; --j)
            {
            *result = *first;
            ++first;
            ++result;
            }
    }
}

#endif // copy_n_hpp

