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

#ifndef mc_enum_hpp
#define mc_enum_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include <deque>
#include <string>
#include <type_traits>
#include <vector>

/// This abstract non-template base class serves two design purposes.
/// It permits calling virtual member functions for arbitrary mc_enum
/// instances through a base-class pointer. And it adds capabilities
/// useful in GUI applications without polluting the enumeration class
/// (which can be recovered for general use by removing the derivation
/// from the base, and optionally making function members such as
/// cardinality() static instead of virtual).
///
///   ordinal()
/// The 'ordinal' is the index of the current value in the array of
/// conceivable values--some of which might not be allowed in context.
/// Radiobuttons display all conceivable values, but listboxes display
/// only permissible values.
///
///   enforce_proscription()
/// If the current value is impermissible in context, then change it,
/// iff possible, to one that's permissible.

class LMI_SO mc_enum_base
    :public datum_base
{
  public:
    explicit mc_enum_base(int);

    void allow(int, bool);
    void allow_all(bool);
    int  first_allowed_ordinal() const;
    bool is_allowed(int) const;

    virtual std::vector<std::string> const& all_strings() const = 0;
    virtual int cardinality() const = 0;
    virtual void enforce_proscription() = 0;
    virtual int ordinal() const = 0;
    virtual std::string str(int) const = 0;

  private:
    std::deque<bool> allowed_;
};

/// M C Enums: string-Mapped, value-Constrained Enumerations.
///
/// Encapsulate C++ enumerations in a class template that pairs them
/// with symbolic names and constrains them to given values. The
/// underlying enumeration must have a non-empty enumerator-list.
///
/// The implicitly-defined copy ctor and copy assignment operator do
/// the right thing.
///
/// It is contemplated that this class template will be instantiated
/// to create numerous types in one translation unit for use in other
/// translation units. Given that usage, it makes sense to instantiate
/// those types explicitly in that one translation unit, in order to
/// avoid bloat.
///
/// Metadata is deliberately excluded from this header, for reasons
/// explained in the documentation for class mc_enum_data.

template<typename T>
class mc_enum final
    :public mc_enum_base
{
    static_assert(std::is_enum_v<T>);

    friend class mc_enum_test;
    template<typename U> friend std::vector<std::string> const& all_strings();

  public:
    typedef T enum_type;

    mc_enum();
    explicit mc_enum(T);
    explicit mc_enum(std::string const&);

    mc_enum& operator=(T);
    mc_enum& operator=(std::string const&);

    bool operator==(mc_enum<T>  const&) const;
    bool operator==(T                 ) const;
    bool operator==(std::string const&) const;
    bool operator!=(mc_enum<T>  const&) const;
    bool operator!=(T                 ) const;
    bool operator!=(std::string const&) const;

    static int ordinal(std::string const&);

    // mc_enum_base required implementation.
    std::vector<std::string> const& all_strings() const override;
    int cardinality() const override;
    void enforce_proscription() override;
    int ordinal() const override;
    std::string str(int) const override;

    std::string str() const;
    T value() const;

  private:
    static int                n();
    static T    const*        e();
    static char const* const* c();
    static std::vector<std::string> const& s();

    // datum_base required implementation.
    std::istream& read (std::istream&) override;
    std::ostream& write(std::ostream&) const override;

    T value_;
};

template<typename U>
bool operator==(U u, mc_enum<U> const& z)
{
    return z.operator==(u);
}

template<typename U>
bool operator==(std::string const& s, mc_enum<U> const& z)
{
    return z.operator==(s);
}

template<typename U>
bool operator!=(U u, mc_enum<U> const& z)
{
    return !z.operator==(u);
}

template<typename U>
bool operator!=(std::string const& s, mc_enum<U> const& z)
{
    return !z.operator==(s);
}

template<typename U>
std::vector<std::string> const& all_strings()
{
    return mc_enum<U>::s();
}

#endif // mc_enum_hpp
