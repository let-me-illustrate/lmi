// Trammeled Numeric range type: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef tn_range_test_aux_hpp
#define tn_range_test_aux_hpp

#include "config.hpp"

#include "tn_range_fwd.hpp"

#include <istream>
#include <limits>
#include <ostream>

// Each derived trammel class must be defined here so that its size is
// known to all translation units that might use it.

template<typename T>
class percentage_trammel
    :public trammel_base<T>
{
    static_assert(100 <= std::numeric_limits<T>::max());
    T nominal_minimum() const override {return   0;}
    T default_value()   const override {return   0;}
    T nominal_maximum() const override {return 100;}
};

typedef tn_range<int, percentage_trammel<int>> r_int_percentage;

/// Class RangeUDT demonstrates the requirements for the Number
/// parameter of class template tn_range. It must be:
///  - DefaultConstructible
///  - Streamable
///  - Constructible from an arithmetic scalar
///  - EqualityComparable
///  - LessThanComparable
///  - LessThanOrEqualComparable

class RangeUDT
{
  public:
    RangeUDT(): f_(3.14f) {}
    RangeUDT(float f): f_(f) {}

    bool operator==(RangeUDT const&) const {return true;}
    bool operator<( RangeUDT const&) const {return true;}
    bool operator<=(RangeUDT const&) const {return true;}

    float f_;
};

inline std::ostream& operator<<(std::ostream& os, RangeUDT const& z)
{
    return os << z.f_;
}

inline std::istream& operator>>(std::istream& is, RangeUDT& z)
{
    is >> z.f_;
    return is;
}

template<typename T>
class range_udt_trammel
    :public trammel_base<T>
{
    T nominal_minimum() const override {return   0;}
    T default_value()   const override {return   0;}
    T nominal_maximum() const override {return 100;}
};

typedef tn_range<RangeUDT, range_udt_trammel<RangeUDT>> r_range_udt;

#endif // tn_range_test_aux_hpp
