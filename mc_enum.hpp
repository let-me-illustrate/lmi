// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: mc_enum.hpp,v 1.21 2008-12-27 02:56:48 chicares Exp $

// Acknowledgment
//
// The valuable idea of associating immutable arrays with this class
// as non-type template parameters was taken from an article in
// comp.lang.c++.moderated by Hyman Rosen <hymie@prolifics.com>,
// archived at
//   http://groups.google.com/groups?selm=t7aeqycnze.fsf%40calumny.jyacc.com
// which bears no copyright notice, as is usual in usenet. This
// implementation descends from work GWC did in 1998, which predates
// that article; any defect here should not reflect on Hyman Rosen's
// reputation.

#ifndef mc_enum_hpp
#define mc_enum_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <cstddef>
#include <deque>
#include <string>
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
    std::size_t first_allowed_ordinal() const;
    bool is_allowed(int) const;

    virtual std::size_t cardinality() const = 0;
    virtual void enforce_proscription() = 0;
    virtual std::size_t ordinal() const = 0;
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
/// Careful attention to detail enables compile-time checking of the
/// sizes of the arrays used as non-type parameters. If too many
/// initializers are given, the compiler must emit a diagnostic
/// [8.5.1/6]. Supplying too few is acceptable [8.5.1/7] to the
/// language, but likely to cause run-time errors--which can be
/// turned into compile-time errors by the technique presented here.
///
/// Specific types require one translation unit (TU) for the
/// instantiation and a header to make them available to other TUs.
///
/// The header should have a typedef declaration, which requires
/// declarations of the arrays used as non-type arguments. Those array
/// declarations must specify bounds explicitly, because an array of
/// unknown bound is an incomplete type that prevents instantiation of
/// the class template--and initializers must not be specified in the
/// header, because including it in two TUs would violate the ODR.
///
/// The instantiation TU, however, should omit the bound specifiers,
/// causing the bounds to be 'calculated' [8.3.4/3]. Passing them as
/// non-type arguments, as '(&array)[n]' rather than as 'array[n],
/// then causes explicit instantiation to fail if the calculated
/// bounds do not match the size explicitly specified as a template
/// parameter. Limitation: this safeguard is ineffective for a TU
/// other than the instantiation TU that includes both the header and
/// the code in the accompanying '.tpp' file that implements the
/// template class, because implicit instantiation would occur; but
/// that is easily avoided in the physical design.
///
/// Because both the header and the instantiation TU require the
/// definition of the underlying enum type, that type must be defined
/// in a separate header that both these files include.
///
/// The same benefit could be realized through consistent use of a
/// macro. This built-in approach is preferred because it avoids using
/// the preprocessor and its compile-time checking is automatic.

template<typename T, std::size_t n, T const (*e)[n], char const*const (*c)[n]>
class mc_enum
    :public mc_enum_base
    ,private boost::equality_comparable<mc_enum<T,n,e,c>, mc_enum<T,n,e,c> >
    ,private boost::equality_comparable<mc_enum<T,n,e,c>, T>
    ,private boost::equality_comparable<mc_enum<T,n,e,c>, std::string>
{
    BOOST_STATIC_ASSERT(boost::is_enum<T>::value);
    BOOST_STATIC_ASSERT(0 < n);

    friend class mc_enum_test;

  public:
    typedef T enum_type;
    enum{Cardinality = n};

    mc_enum();
    explicit mc_enum(T);
    explicit mc_enum(std::string const&);

    mc_enum& operator=(T);
    mc_enum& operator=(std::string const&);

    bool operator==(mc_enum<T,n,e,c> const&) const;
    bool operator==(T) const;
    bool operator==(std::string const&) const;

    static std::size_t ordinal(std::string const&);

    // mc_enum_base required implementation.
    virtual std::size_t cardinality() const;
    virtual std::size_t ordinal() const;
    virtual std::string str(int) const;

    std::string str() const;
    T value() const;

    static std::vector<std::string> const& all_strings();

  private:
    // datum_base required implementation.
    // TODO ?? Consider moving the implementation into the base class.
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

    // mc_enum_base required implementation.
    virtual void enforce_proscription();

    T value_;
};

#endif // mc_enum_hpp

