// M C Enums: string-Mapped, value-Constrained Enumerations.
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

#include "mc_enum.hpp"

#include "alert.hpp"
#include "facets.hpp"
#include "rtti_lmi.hpp"

#include <algorithm> // std::find()
#include <typeinfo>

// TODO ?? Should there be a runtime check that all elements in 'e'
// and in 'c' are unique? Can that be asserted at compile time?

template<typename T>
mc_enum<T>::mc_enum()
    :mc_enum_base(enum_info::n)
    ,value_(enum_info::enums()[0])
{
    // These checks have to be performed in mc_enum.tpp rather than in
    // mc_enum.hpp, because mc_enum.hpp can otherwise be compiled even without
    // mc_enum_type_info<> specializations.
    //
    // This place is arbitrarily chosen, it could be in any other mc_enum
    // method in this file.

    BOOST_STATIC_ASSERT(0 < enum_info::n);

    BOOST_STATIC_ASSERT(sizeof(typename enum_info::enums_t) == sizeof(enum_info::enums()));
    BOOST_STATIC_ASSERT(sizeof(typename enum_info::strings_t) == sizeof(enum_info::strings()));
    BOOST_STATIC_ASSERT(enum_info::n == sizeof(enum_info::enums())/sizeof(T));
    BOOST_STATIC_ASSERT(enum_info::n == sizeof(enum_info::strings())/sizeof(const char*));
}

template<typename T>
mc_enum<T>::mc_enum(T t)
    :mc_enum_base(enum_info::n)
    ,value_(t)
{}

template<typename T>
mc_enum<T>::mc_enum(std::string const& s)
    :mc_enum_base(enum_info::n)
    ,value_(enum_info::enums()[ordinal(s)])
{}

template<typename T>
mc_enum<T>& mc_enum<T>::operator=(T t)
{
    value_ = t;
    return *this;
}

template<typename T>
mc_enum<T>& mc_enum<T>::operator=(std::string const& s)
{
    value_ = enum_info::enums()[ordinal(s)];
    return *this;
}

template<typename T>
bool mc_enum<T>::operator==(mc_enum<T> const& z) const
{
    return z.value_ == value_;
}

template<typename T>
bool mc_enum<T>::operator==(T t) const
{
    return t == value_;
}

template<typename T>
bool mc_enum<T>::operator==(std::string const& s) const
{
    return s == str();
}

namespace
{
/// A whilom version of a vetust class substituted underbars for
/// spaces, for reasons that, well, seemed good at the time.

std::string provide_for_backward_compatibility(std::string const& s)
{
    static std::string const underbar("_");
    std::string r(s);
    for(;;)
        {
        std::string::size_type xpos = r.find_first_of(underbar);
        if(std::string::npos == xpos)
            {
            break;
            }
        else
            {
            r[xpos] = ' ';
            }
        }
    return r;
}
} // Unnamed namespace.

template<typename T>
std::istream& mc_enum<T>::read(std::istream& is)
{
    std::locale old_locale = is.imbue(blank_is_not_whitespace_locale());
    std::string s;
    is >> s;
    is.imbue(old_locale);

    typename enum_info::strings_t c = enum_info::strings();
    std::size_t v = std::find(c, c + enum_info::n, s) - c;
    if(enum_info::n == v)
        {
        v = std::find(c, c + enum_info::n, provide_for_backward_compatibility(s)) - c;
        }
    if(enum_info::n == v)
        {
        ordinal(s); // Throws.
        throw "Unreachable.";
        }
    value_ = enum_info::enums()[v];

    return is;
}

template<typename T>
std::ostream& mc_enum<T>::write(std::ostream& os) const
{
    return os << str();
}

template<typename T>
std::size_t mc_enum<T>::cardinality() const
{
    return enum_info::n;
}

template<typename T>
void mc_enum<T>::enforce_proscription()
{
    if(is_allowed(ordinal()))
        {
        return;
        }

    std::size_t z = first_allowed_ordinal();
    if(z < cardinality())
        {
        value_ = enum_info::enums()[z];
        }
}

template<typename T>
std::size_t mc_enum<T>::ordinal() const
{
    typename enum_info::enums_t e = enum_info::enums();
    std::size_t i = std::find(e, e + enum_info::n, value_) - e;
    if(i == enum_info::n)
        {
        fatal_error()
            << "Value "
            << value_
            << " invalid for type '"
            << lmi::TypeInfo(typeid(T))
            << "'."
            << LMI_FLUSH
            ;
        }
    return i;
}

template<typename T>
std::string mc_enum<T>::str(int j) const
{
    return enum_info::strings()[j];
}

template<typename T>
T mc_enum<T>::value() const
{
    return value_;
}

template<typename T>
std::size_t mc_enum<T>::ordinal(std::string const& s)
{
    typename enum_info::strings_t c = enum_info::strings();
    std::size_t v = std::find(c, c + enum_info::n, s) - c;
    if(v == enum_info::n)
        {
        fatal_error()
            << "Value '"
            << s
            << "' invalid for type '"
            << lmi::TypeInfo(typeid(T))
            << "'."
            << LMI_FLUSH
            ;
        }
    return v;
}

template<typename T>
std::string mc_enum<T>::str() const
{
    return enum_info::strings()[ordinal()];
}

template<typename T>
std::vector<std::string> const& mc_enum<T>::all_strings()
{
    typename enum_info::strings_t c = enum_info::strings();
    static std::vector<std::string> const v(c, c + enum_info::n);
    return v;
}

