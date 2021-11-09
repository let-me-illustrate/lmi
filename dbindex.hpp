// Product-database lookup index.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef dbindex_hpp
#define dbindex_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "mc_enum_type_enums.hpp"

#include <array>

/// Number of database axes, excluding the special duration axis.

constexpr int number_of_indices {6};

/// Ordered database axes.

constexpr int e_axis_gender    {0};
constexpr int e_axis_uw_class  {1};
constexpr int e_axis_smoking   {2};
constexpr int e_axis_issue_age {3};
constexpr int e_axis_uw_basis  {4};
constexpr int e_axis_state     {5};
constexpr int e_axis_duration  {6};

/// These constants facilitate compile-time assertions in the
/// product-database GUI, q.v.: an array cannot be indexed to
/// produce an arithmetic constant expression [5.19/3].

constexpr int e_number_of_axes    {1 + number_of_indices};
constexpr int e_max_dim_gender    {  3};
constexpr int e_max_dim_uw_class  {  4};
constexpr int e_max_dim_smoking   {  3};
constexpr int e_max_dim_issue_age {100};
constexpr int e_max_dim_uw_basis  {  5};
constexpr int e_max_dim_state     { 53};
constexpr int e_max_dim_duration  {100};

/// Product-database lookup index.
///
/// Implicitly-declared special member functions do the right thing.
///
/// Members such as state(mcenum_state) depart from this idiom:
///   https://isocpp.org/wiki/faq/ctors#named-parameter-idiom
/// so that idx_ can be const. In practice, they're used only to
/// create throwaway database_index objects, so modifying the current
/// object was actually undesirable. Arguably they should have more
/// verbose names.
///
/// Arguably enumerators should be used rather than literal integers
/// in the implementation, e.g.,
///   s/5/e_axis_state/
/// but OTOH compactness increases readability.

class database_index
{
  public:
    database_index
        (mcenum_gender   gender
        ,mcenum_class    uw_class
        ,mcenum_smoking  smoking
        ,int             issue_age
        ,mcenum_uw_basis uw_basis
        ,mcenum_state    state
        )
        :idx_ {gender, uw_class, smoking, issue_age, uw_basis, state}
    {
        check_issue_age();
    }

    std::array<int,number_of_indices> const& index_array() const {return idx_;}

    mcenum_gender   gender   () const {return static_cast<mcenum_gender  >(idx_[0]);}
    mcenum_class    uw_class () const {return static_cast<mcenum_class   >(idx_[1]);}
    mcenum_smoking  smoking  () const {return static_cast<mcenum_smoking >(idx_[2]);}
    int             issue_age() const {return static_cast<int            >(idx_[3]);}
    mcenum_uw_basis uw_basis () const {return static_cast<mcenum_uw_basis>(idx_[4]);}
    mcenum_state    state    () const {return static_cast<mcenum_state   >(idx_[5]);}

    database_index gender   (mcenum_gender   z) const {auto i = idx_; i[0] = z; return {i};}
    database_index uw_class (mcenum_class    z) const {auto i = idx_; i[1] = z; return {i};}
    database_index smoking  (mcenum_smoking  z) const {auto i = idx_; i[2] = z; return {i};}
    database_index issue_age(int             z) const {auto i = idx_; i[3] = z; return {i};}
    database_index uw_basis (mcenum_uw_basis z) const {auto i = idx_; i[4] = z; return {i};}
    database_index state    (mcenum_state    z) const {auto i = idx_; i[5] = z; return {i};}

  private:
    database_index(std::array<int,number_of_indices> idx)
        :idx_ {idx}
    {
        check_issue_age();
    }

    void check_issue_age()
    {
        LMI_ASSERT(0 <= issue_age() && issue_age() < e_max_dim_issue_age);
    }

    std::array<int,number_of_indices> const idx_;
};

#endif // dbindex_hpp
