// Trammeled Numeric range type: unit test.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: tn_range_test_aux.hpp,v 1.3 2006-01-29 13:52:00 chicares Exp $

#ifndef tn_range_test_aux_hpp
#define tn_range_test_aux_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"

// The derived trammel class must be defined here so that its size is
// known to all translation units that might use it.

// An assertion such as
//    BOOST_STATIC_ASSERT(100 <= std::numeric_limits<T>::max());
// cannot work: the macro's argument must be an integral constant
// expression, so it can't involve a function call and can't perform
// any non-integer operation.

template<typename T>
struct percentage_trammel
    :public trammel_base<T>
{
    T nominal_maximum() const {return 100;}
    T nominal_minimum() const {return   0;}
    T default_value()   const {return   0;}
};

typedef tn_range<int, percentage_trammel<int> > r_int_percentage;

#endif // tn_range_test_aux_hpp

