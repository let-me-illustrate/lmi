// Metadata for string-Mapped, value-Constrained Enumerations.
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

// Acknowledgment
//
// The valuable idea of associating immutable arrays with a class
// as non-type template parameters was taken from an article in
// comp.lang.c++.moderated by Hyman Rosen <hymie@prolifics.com>,
// archived at
//   http://groups.google.com/groups?selm=t7aeqycnze.fsf%40calumny.jyacc.com
// which bears no copyright notice, as is usual in usenet. This
// implementation descends from work GWC did in 1998, which predates
// that article; any defect here should not reflect on Hyman Rosen's
// reputation.

#ifndef mc_enum_metadata_hpp
#define mc_enum_metadata_hpp

#include "config.hpp"

/// Associate an mc_enum type with its metadata through its enum type.
///
/// This class template is intended to be specialized for each type,
/// each specialization deriving from class mc_enum_data.

template<typename T>
struct mc_enum_key
{};

/// Metadata for class template mc_enum.
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
/// The header should define a mc_enum_key specialization that derives
/// from this struct; that requires declarations of the arrays used as
/// non-type arguments. Those array declarations must specify bounds
/// explicitly, because an array of unknown bound is an incomplete
/// type that prevents instantiation of class template mc_enum_data.
/// However, the header must not specify array initializers, because
/// including it in two TUs would violate the ODR.
///
/// The instantiation TU, however, should omit the bound specifiers,
/// causing the bounds to be 'calculated' [8.3.4/3]. Passing them as
/// non-type arguments, as '(&array)[n]' rather than as 'array[n],
/// then causes explicit instantiation to fail if the calculated
/// bounds do not match the size explicitly specified as a template
/// parameter. Limitation: this safeguard is ineffective for a TU
/// other than the instantiation TU that includes both the header and
/// the code in the accompanying '.tpp' file that implements the
/// class template, because implicit instantiation would occur; but
/// that is easily avoided in the physical design.
///
/// Because both the header and the instantiation TU require the
/// definition of the underlying enum type, that type must be defined
/// in a separate header that both these files include.
///
/// The same benefit could be realized through consistent use of a
/// macro. This built-in approach is preferred because it avoids using
/// the preprocessor and its compile-time checking is automatic.
///
/// It is contemplated that this class template will be instantiated
/// to create numerous types in one translation unit for use in other
/// translation units. Given that usage, it makes sense to instantiate
/// those types explicitly in that one translation unit, in order to
/// avoid bloat.
///
/// Implementation notes.
///
/// Array arguments are adjusted to pointers [14.1/8].
///
/// e() and c() could have been provided as static data members.
/// However, they're both pointers, so they can't be used in ICEs, in
/// particular as constant-initializers within the struct definition;
/// and writing them outside the struct definition would be balky.
///
/// 'n' could have been provided as a function rather than a constant,
/// but a constant is preferable because it can be used in an ICE.

template<typename T, auto n, T const (&E)[n], char const*const (&C)[n]>
struct mc_enum_data
{
#ifndef LMI_MSC
    static_assert(0 != n);
#endif

    static int const          n_  {n};
    static T    const*        e() {return E;}
    static char const* const* c() {return C;}
};

#endif // mc_enum_metadata_hpp
