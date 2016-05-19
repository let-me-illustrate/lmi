// PETE with std::vector.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

#ifndef et_vector_hpp
#define et_vector_hpp

#if 0
#include "config.hpp"
#endif // 0

// These headers must be included before "et_vector_operators.hpp"
// because the latter doesn't include them.
#include "PETE/PETE.h"
#include <vector>

#include "et_vector_operators.hpp"

/// Create vector-iterator leaves.

template<class T>
struct CreateLeaf<std::vector<T> >
{
    typedef typename std::vector<T>::const_iterator Leaf_t;
    static Leaf_t make(std::vector<T> const& v) {return v.begin();}
};

/// All PETE assignment operators call evaluate().

template<class T, class Op, class U>
inline void evaluate(std::vector<T>& t, Op const& op, U const& u)
{
    typedef typename std::vector<T>::iterator svi;
    for(svi i = t.begin(); i != t.end(); ++i)
        {
        op(*i, forEach(u, DereferenceLeaf(), OpCombine()));
        forEach(u, IncrementLeaf(), NullCombine());
        }
}

#endif // et_vector_hpp

