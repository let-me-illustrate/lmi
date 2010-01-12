// Trammels for specific tn_range types.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef tn_range_type_trammels_hpp
#define tn_range_type_trammels_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"

#include "calendar_date.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) class IgNoRe
#endif // Defined __BORLANDC__ .

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
    T nominal_minimum() const {return   0;}
    T default_value()   const {return   0;}
    T nominal_maximum() const {return 100;}
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
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT((boost::is_same<calendar_date,T>::value));

    T nominal_minimum() const {return gregorian_epoch();}
    T default_value()   const {return today          ();}
    T nominal_maximum() const {return last_yyyy_date ();}
};

#endif // tn_range_type_trammels_hpp

