// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: mc_enum.tpp,v 1.2 2005-02-19 03:27:45 chicares Exp $

#include "config.hpp"

#include "mc_enum.hpp"

#include "facets.hpp"

#include <algorithm> // std::find()
#include <istream>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <typeinfo>

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
mc_enum<T,n,e,c>::mc_enum()
    :mc_enum_base(n)
    ,value_(e[0])
{}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
mc_enum<T,n,e,c>::mc_enum(T t)
    :mc_enum_base(n)
    ,value_(t)
{}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
mc_enum<T,n,e,c>::mc_enum(std::string const& s)
    :mc_enum_base(n)
    ,value_(e[ordinal(s)])
{}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
mc_enum<T,n,e,c>& mc_enum<T,n,e,c>::operator=(T t)
{
    value_ = t;
    return *this;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
mc_enum<T,n,e,c>& mc_enum<T,n,e,c>::operator=(std::string const& s)
{
    value_ = e[ordinal(s)];
    return *this;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
bool mc_enum<T,n,e,c>::operator==(mc_enum<T,n,e,c> const& z) const
{
    return z.value_ == value_;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
bool mc_enum<T,n,e,c>::operator==(T t) const
{
    return t == value_;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
bool mc_enum<T,n,e,c>::operator==(std::string const& s) const
{
    return s == str();
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::istream& mc_enum<T,n,e,c>::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    std::string s;
    is >> s;
    operator=(s);
    is.imbue(old_locale);
    return is;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::ostream& mc_enum<T,n,e,c>::write(std::ostream& os) const
{
    return os << str();
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::size_t mc_enum<T,n,e,c>::cardinality() const
{
    return n;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::size_t mc_enum<T,n,e,c>::ordinal() const
{
    std::size_t i = std::find(e, e + n, value_) - e;
    if(i == n)
        {
        std::ostringstream oss;
        oss
            << "Value "
            << value_
            << " invalid for type '"
            << typeid(T).name()
            << "'."
            ;
        throw std::logic_error(oss.str());
        }
    return i;
}


template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::string mc_enum<T,n,e,c>::str(int j) const
{
    return c[j];
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
T mc_enum<T,n,e,c>::value() const
{
    return value_;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::size_t mc_enum<T,n,e,c>::ordinal(std::string const& s)
{
    std::size_t v = std::find(c, c + n, s) - c;
    if(v == n)
        {
        std::ostringstream oss;
        oss
            << "Value '"
            << s
            << "' invalid for type '"
            << typeid(T).name()
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    return v;
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::string mc_enum<T,n,e,c>::str() const
{
    return c[ordinal()];
}

