// Extended enumeration type.
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

// $Id: xenum.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef xenum_hpp
#define xenum_hpp

#include "config.hpp"

#include <algorithm> // find()
#include <cstddef>   // ptrdiff_t
#include <iosfwd>
#include <stdexcept> // range_error
#include <string>
#include <typeinfo>
#include <vector>

// C enums represent an enumerable range of values known at compile time.
// This wrapper transforms them into class types, pairing each enumeration
// value with a string, in order to implement the following useful behaviors:
//
// Assignment and initialization are permitted only for valid enumerators
// or their corresponding strings; an invalid value triggers an exception.
//
// Enumerators can be converted into strings and vice versa.
//
// A vector of all valid strings can be obtained.
//
// The motivation is to create types that are useful for validated input in
// accordance with the principle that invalid input should never be accepted
// and an input class should never be permitted to enter an invalid state.
// We embody the knowledge needed for validating each item in a UDT that
// lets the item validate itself.

// Perhaps this is relevant to the discussion here:
// http://groups.yahoo.com/group/boost/message/6406
// http://groups.yahoo.com/group/boost/message/1208
// discussion around 2001-08 in clc++m

template<typename EnumType, int N>
class xenum
{
public:
    typedef EnumType enumeration_type;

    xenum();
    explicit xenum(EnumType e);
    explicit xenum(std::string const& s);

    xenum(xenum<EnumType, N> const& x);

    xenum& operator=(xenum<EnumType, N> const& x);
    xenum& operator=(EnumType e);
    xenum& operator=(std::string const& s);

    bool operator==(xenum<EnumType, N> const& x) const;
    bool operator==(EnumType e) const;
    bool operator==(std::string const& s) const;

// TODO ?? Boost coding guidelines 14.3:
// 14.3. Do not define conversion operators, particularly to numeric types.
// Write explicit conversion functions instead.
//
// At least for the short-term purpose of porting, we violate this rule.
// Reconsider this later. Even if we don't follow the rule, this should
// be defined outside the class.

    operator EnumType const&() const {return representation;}

    EnumType value() const;
    std::string const& str() const;

    static std::vector<std::string> const& all_strings();

private:
    void set_value(std::string const& s);
#if defined BC_BEFORE_5_5
    static std::vector<std::string> const& borland_502_workaround();
#endif // old borland compiler

    EnumType representation;
    static EnumType const enumerators[N];
    static char const*const names[N];
};

// Prohibit declaration of a xenum based on an empty enumeration.
// COMPILER !! bc++5.02 cannot handle this.
#if !defined BC_BEFORE_5_5
template <typename EnumType>
class xenum<EnumType, 0>
{
public:
    virtual ~xenum() = 0;
};
#endif // old borland compiler

#define LMI_EXPLICIT_INSTANTIATION
// Explicit instantiation of the extended-enum template yields about a
// three percent build-time improvement for all three commmand-line
// compilers tested, and a twenty-three percent improvement for the cli
// target using the bc502 IDE compiler.

#ifdef LMI_EXPLICIT_INSTANTIATION
    template<typename EnumType, int N>
    std::istream& operator>> (std::istream& is, xenum<EnumType, N>& x);

    template<typename EnumType, int N>
    std::ostream& operator<< (std::ostream& os, xenum<EnumType, N> const& x);
#else // not LMI_EXPLICIT_INSTANTIATION
#   define OK_TO_COMPILE_XENUM_CPP
#   include "xenum.cpp"
#   undef OK_TO_COMPILE_XENUM_CPP
#endif // not LMI_EXPLICIT_INSTANTIATION

#endif // xenum_hpp

