// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef database_hpp
#define database_hpp

#include "config.hpp"

#include "bourn_cast.hpp"
#include "dbindex.hpp"
#include "dbnames.hpp"                  // e_database_key
#include "mc_enum_type_enums.hpp"
#include "so_attributes.hpp"

#include <memory>                       // shared_ptr
#include <string>
#include <type_traits>                  // is_integral_v, underlying_type_t
#include <vector>

class database_entity;
class DBDictionary;
class yare_input;

/// Database of product parameters.

class LMI_SO product_database final
{
    friend struct LoadsTest;       // For product_database(int).
    friend class input_test;       // For test_product_database().
    friend class premium_tax_test; // For test_rates().

  public:
    product_database
        (std::string const& ProductName
        ,mcenum_gender      Gender
        ,mcenum_class       UnderwritingClass
        ,mcenum_smoking     Smoking
        ,int                IssueAge
        ,mcenum_uw_basis    GroupUnderwritingType
        ,mcenum_state       StateOfJurisdiction
        );
    explicit product_database(yare_input const&);
    product_database(product_database &&) = default;
    product_database(product_database const&) = default;
    ~product_database() = default;

    int length() const;
    database_index index() const;

    void query_into
        (e_database_key
        ,std::vector<double>&
        ,database_index const&
        ) const;
    void query_into(e_database_key, std::vector<double>&) const;

    double query(e_database_key, database_index const&) const;

    template<typename T>
    T query(e_database_key) const;

    template<typename T>
    void query_into(e_database_key, T&) const;

    bool are_equivalent(e_database_key, e_database_key) const;
    bool varies_by_state(e_database_key) const;

  private:
    // Special ctor used only in the unit-test TU for class Loads.
    explicit product_database(int length)
        :index_  {mce_male, mce_rated, mce_smoker, 0, mce_medical, mce_s_XX}
        ,length_ {length}
        {}

    product_database& operator=(product_database const&) = delete;

    void initialize(std::string const& product_name);

    DBDictionary const& db() const;
    database_entity const& entity_from_key(e_database_key) const;

    database_index const index_;
    int                  length_;
    int                  maturity_age_;

    std::shared_ptr<DBDictionary> db_;
};

/// Query database, using default index; return a scalar.
///
/// Cast result to type T, preserving value by using bourn_cast.
///
/// Throw if the database entity is not scalar, or if casting fails
/// (because T is neither enumerative nor arithmetic, or because the
/// result cannot be represented exactly in type T).

template<typename T>
T product_database::query(e_database_key k) const
{
    double d = query(k, index_);
    if constexpr(std::is_enum_v<T>)
        {
        return static_cast<T>(bourn_cast<std::underlying_type_t<T>>(d));
        }
    else
        {
        return bourn_cast<T>(d);
        }
}

/// Query database, using default index; write result into scalar argument.
///
/// Cast result to type T, preserving value by using bourn_cast.
///
/// Throw if the database entity is not scalar, or if casting fails
/// (because T is neither enumerative nor arithmetic, or because the
/// result cannot be represented exactly in type T).

template<typename T>
void product_database::query_into(e_database_key k, T& dst) const
{
    dst = query<T>(k);
}

#endif // database_hpp
