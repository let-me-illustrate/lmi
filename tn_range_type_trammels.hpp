// Trammels for specific tn_range types.
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

#ifndef tn_range_type_trammels_hpp
#define tn_range_type_trammels_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"

#include "calendar_date.hpp"

#include <limits>
#include <type_traits>

// SOMEDAY !! Move this one to a 'test' file.

template<typename T>
class percentage_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return   0;}
    T default_value()   const override {return   0;}
    T nominal_maximum() const override {return 100;}
};

template<typename T>
class proportion_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 0;}
    T default_value()   const override {return 0;}
    T nominal_maximum() const override {return 1;}
};

template<typename T>
class unrestricted_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return -std::numeric_limits<T>::max();}
    T default_value()   const override {return  0;}
    T nominal_maximum() const override {return  std::numeric_limits<T>::max();}
};

template<typename T>
class nonnegative_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 0;}
    T default_value()   const override {return 0;}
    T nominal_maximum() const override {return std::numeric_limits<T>::max();}
};

template<typename T>
class age_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return  0;}
    T default_value()   const override {return  0;}
    T nominal_maximum() const override {return 99;}
};

template<typename T>
class duration_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return   0;}
    T default_value()   const override {return   0;}
    T nominal_maximum() const override {return 100;}
};

template<typename T>
class month_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return  0;}
    T default_value()   const override {return  0;}
    T nominal_maximum() const override {return 11;}
};

template<typename T>
class corridor_factor_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return 1;}
    T default_value()   const override {return 1;}
    T nominal_maximum() const override {return std::numeric_limits<T>::max();}
};

template<typename T>
class date_trammel
    :public trammel_base<T>
{
    static_assert(std::is_same_v<calendar_date,T>);

    T nominal_minimum() const override {return gregorian_epoch();}
    T default_value()   const override {return today          ();}
    T nominal_maximum() const override {return last_yyyy_date ();}
};

#endif // tn_range_type_trammels_hpp
