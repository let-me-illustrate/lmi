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

// $Id: mc_enum.tpp,v 1.4 2005-03-17 02:34:19 chicares Exp $

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

// TODO ?? Document this in the header?
//
// The 'ordinal' is the index in the array 'e' of conceivable values
// at which the current value is found.
// TODO ?? Should there be a runtime check that each element is unique?
//
// Not all conceivable values are actually permitted in every context;
// is_allowed(ordinal_number) returns true iff 'ordinal_number' is
// permitted.
//
// If the current value is permissible, then return its ordinal.
// Else, return the first ordinal that is permissible. But if no
// value is permissible, then return the ordinal of the current value.
//
// TODO ?? Explain how this is intended to be used, resolve the issues
// noted below, and make the above comments agree with the code by
// changing one or both--and add unit tests.
//
template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::size_t mc_enum<T,n,e,c>::allowed_ordinal() const
{
    int i = ordinal();
    if(!is_allowed(ordinal()))
        {
        for(i = 0; i < static_cast<int>(cardinality()); ++i)
            {
            if(is_allowed(i))
                {
                break;
                }
            }
        // TODO ?? Else is there really an error?
        }

    if(!is_allowed(i))
        {
        i = ordinal();
        }

    // TODO ?? Isn't this impossible? Doesn't ordinal() detect this problem?
    if(i < static_cast<int>(n))
        {
        return i;
        }
    else // TODO ?? Throw here?
        {
        return 0;
        }
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

