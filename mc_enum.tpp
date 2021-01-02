// M C Enums: string-Mapped, value-Constrained Enumerations.
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

#include "mc_enum.hpp"
#include "mc_enum_metadata.hpp"

#include "alert.hpp"
#include "bourn_cast.hpp"
#include "facets.hpp"
#include "rtti_lmi.hpp"

#include <algorithm>                    // find()
#include <istream>
#include <ostream>
#include <typeinfo>

/// The header that defines class mc_enum is by design unaware of its
/// associated metadata, so static assertions that depend on metadata
/// are written here.

template<typename T>
mc_enum<T>::mc_enum()
    :mc_enum_base (n())
    ,value_       {e()[0]}
{
    typedef mc_enum_key<T> metadata;
    static_assert(0 < metadata::n_);
}

template<typename T>
mc_enum<T>::mc_enum(T t)
    :mc_enum_base (n())
    ,value_       {t}
{}

template<typename T>
mc_enum<T>::mc_enum(std::string const& s)
    :mc_enum_base (n())
    ,value_       {e()[ordinal(s)]}
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
    value_ = e()[ordinal(s)];
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

template<typename T>
bool mc_enum<T>::operator!=(mc_enum<T> const& z) const
{
    return !operator==(z);
}

template<typename T>
bool mc_enum<T>::operator!=(T t) const
{
    return !operator==(t);
}

template<typename T>
bool mc_enum<T>::operator!=(std::string const& s) const
{
    return !operator==(s);
}

template<typename T>
int mc_enum<T>::ordinal(std::string const& s)
{
    auto v = bourn_cast<int>(std::find(c(), c() + n(), s) - c());
    if(v == n())
        {
        alarum()
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
std::vector<std::string> const& mc_enum<T>::all_strings() const
{
    return s();
}

template<typename T>
int mc_enum<T>::cardinality() const
{
    return n();
}

template<typename T>
void mc_enum<T>::enforce_proscription()
{
    if(is_allowed(ordinal()))
        {
        return;
        }

    int z = first_allowed_ordinal();
    if(z < cardinality())
        {
        value_ = e()[z];
        }
}

template<typename T>
int mc_enum<T>::ordinal() const
{
    auto i = bourn_cast<int>(std::find(e(), e() + n(), value_) - e());
    if(i == n())
        {
        alarum()
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
    return c()[j];
}

template<typename T>
std::string mc_enum<T>::str() const
{
    return c()[ordinal()];
}

template<typename T>
T mc_enum<T>::value() const
{
    return value_;
}

// TODO ?? Should there be a runtime check that all elements in
// e() and in c() are unique? Can that be asserted at compile time?

template<typename T>
int                mc_enum<T>::n() {return mc_enum_key<T>::n_;}

template<typename T>
T    const*        mc_enum<T>::e() {return mc_enum_key<T>::e();}

template<typename T>
char const* const* mc_enum<T>::c() {return mc_enum_key<T>::c();}

template<typename T>
std::vector<std::string> const& mc_enum<T>::s()
{
    static std::vector<std::string> const v(c(), c() + n());
    return v;
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

    auto v = bourn_cast<int>(std::find(c(), c() + n(), s) - c());
    if(n() == v)
        {
        v = bourn_cast<int>
            ( std::find(c(), c() + n(), provide_for_backward_compatibility(s))
            - c()
            );
        }
    if(n() == v)
        {
        ordinal(s); // Throws.
        throw "Unreachable.";
        }
    value_ = e()[v];

    return is;
}

template<typename T>
std::ostream& mc_enum<T>::write(std::ostream& os) const
{
    return os << str();
}
