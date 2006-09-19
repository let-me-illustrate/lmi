// Trammels for specific tn_range types.
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

// $Id: tn_range_type_trammels.hpp,v 1.6 2006-09-19 02:59:19 chicares Exp $

#ifndef tn_range_type_trammels_hpp
#define tn_range_type_trammels_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"

#include <limits>

// TODO ?? Move this one to a 'test' file.

template<typename T>
class percentage_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return   0;}
    T default_value()   const {return   0;}
    T nominal_maximum() const {return 100;}
};

// TODO ?? Consider generating these with macros, e.g.
//   TRAMMEL(1,0,0)

template<typename T>
class proportion_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return 0;}
    T default_value()   const {return 0;}
    T nominal_maximum() const {return 1;}
};

template<typename T>
class unrestricted_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return -std::numeric_limits<T>::max();}
    T default_value()   const {return  0;}
    T nominal_maximum() const {return  std::numeric_limits<T>::max();}
};

template<typename T>
class nonnegative_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return 0;}
    T default_value()   const {return 0;}
    T nominal_maximum() const {return std::numeric_limits<T>::max();}
};

// TODO ?? These three
//    issue_age      [0, omega - 1]
//    attained_age   [x, omega - 1]
//    duration       [0, omega-x-1]
// are identical for now; they're distinguished so that the
// limit semantics above can be added later.

template<typename T>
class issue_age_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return  0;}
    T default_value()   const {return  0;}
    T nominal_maximum() const {return 99;}
};

template<typename T>
class attained_age_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return  0;}
    T default_value()   const {return  0;}
    T nominal_maximum() const {return 99;}
};

template<typename T>
class duration_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return  0;}
    T default_value()   const {return  0;}
    T nominal_maximum() const {return 99;}
};

template<typename T>
class month_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return  0;}
    T default_value()   const {return  0;}
    T nominal_maximum() const {return 11;}
};

template<typename T>
class corridor_factor_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return 1;}
    T default_value()   const {return 1;}
    T nominal_maximum() const {return std::numeric_limits<T>::max();}
};

template<typename T>
class date_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const {return 2361222;} // Gregorian epoch (English).
    T default_value()   const {return 2453371;} // 2004-12-31 .
    T nominal_maximum() const {return 5373484;} // 9999-12-31 .
};

#endif // tn_range_type_trammels_hpp

