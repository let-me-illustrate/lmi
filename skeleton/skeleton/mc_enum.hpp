// M C Enums: string-Mapped, value-Constrained Enumerations.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mc_enum.hpp,v 1.1.1.1 2004-05-15 19:58:57 chicares Exp $

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

// Design notes
//
// This class encapsulates C++ enumerations in a class that pairs them
// with symbolic names and constrains them to given values. The
// underlying enumeration must have a non-empty enumerator-list.

// Implementation notes
//
// It is contemplated that this template class will be instantiated
// to create numerous types in one translation unit for use in other
// translation units. Given that usage, it makes sense to instantiate
// those types explicitly in that one translation unit, in order to
// avoid bloat. See special note on explicit instantiation below.
//
// The implicitly-defined copy ctor and copy assignment operator do
// the right thing.

// Explicit instantiation
//
// Careful attention to detail enables compile-time checking of the
// sizes of the arrays used as non-type parameters. If too many
// initializers are given, the compiler must emit a diagnostic
// [8.5.1/6]. Supplying too few is acceptable [8.5.1/7] to the
// language, but likely to cause run-time errors--which can be
// turned into compile-time errors by the technique presented here.
//
// Specific types require one translation unit (TU) for the
// instantiation and a header to make them available to other TUs.
//
// The header should have a typedef declaration, which requires
// declarations of the arrays used as non-type arguments. Those array
// declarations must specify bounds explicitly, because an array of
// unknown bound is an incomplete type that prevents instantiation of
// the class template--and initializers must not be specified in the
// header, otherwise including it in two TUs would violate the ODR.
//
// The instantiation TU, however, should omit the bound specifiers,
// causing the bounds to be 'calculated' [8.3.4/3]. Passing them as
// non-type arguments, as '(&array)[n]' rather than as 'array[n], then
// causes explicit instantiation to fail if the calculated bounds do
// not match the size explicitly specified as a template parameter.
// Limitation: this safeguard is ineffective in the case of a TU other
// than the instantiation TU that includes both the header and the
// code in the accompanying '.tpp' file that implements the template
// class, because implicit instantiation would occur; but that is
// easily avoided in the physical design.
//
// Because both the header and the instantiation TU require the
// definition of the underlying enum type, that type must be defined
// in a separate header that both these files include.
//
// The same benefit could of course be realized through consistent use
// of a macro. This built-in approach is preferred because it avoids
// using the preprocessor and its compile-time checking is automatic.
//
// The streaming operators could be treated similarly, but that would
// triple the number of explicit instantiations with little benefit.
// Templated streaming operators would necessitate member templates
// for read() and write(), which might suggest a different design.

#ifndef mc_enum_hpp
#define mc_enum_hpp

#include "config.hpp"

#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <cstddef>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
class mc_enum
    :private boost::equality_comparable<mc_enum<T,n,e,c>, mc_enum<T,n,e,c> >
    ,private boost::equality_comparable<mc_enum<T,n,e,c>, T>
    ,private boost::equality_comparable<mc_enum<T,n,e,c>, std::string>
{
    BOOST_STATIC_ASSERT(boost::is_enum<T>::value);
    BOOST_STATIC_ASSERT(0 < n);

  public:
    typedef T enum_type;

    mc_enum();
    explicit mc_enum(T);
    explicit mc_enum(std::string const&);

    mc_enum& operator=(T);
    mc_enum& operator=(std::string const&);

    bool operator==(mc_enum<T,n,e,c> const&) const;
    bool operator==(T) const;
    bool operator==(std::string const&) const;

    std::istream& read (std::istream& is);
    std::ostream& write(std::ostream& os) const;

    std::size_t ordinal() const;
    std::string str() const;
    T value() const;

    static std::size_t cardinality();
    static std::vector<std::string> strings();

  private:
    static std::size_t ordinal(std::string const&);

    T value_;

    // TODO ?? Perhaps what follows should be moved to a derived class.
    // It's experimental; members may be added, removed, or changed.
    // Function allowed_ordinals() is of dubious utility. Public data
    // members obviously should be rethought.
  public:

    std::vector<int> allowed_ordinals() const;
    std::vector<std::string> allowed_strings() const;
    bool enabled_;
    std::vector<int> allowed_;
};

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::istream& operator>>(std::istream& is, mc_enum<T,n,e,c>& z)
{
    return z.read(is);
}

template<typename T, std::size_t n, T const (&e)[n], char const*const (&c)[n]>
std::ostream& operator<<(std::ostream& os, mc_enum<T,n,e,c> const& z)
{
    return z.write(os);
}

#endif // mc_enum_hpp

