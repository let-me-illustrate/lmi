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

#include "pchfile.hpp"

#include "database.hpp"

#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "dbdict.hpp"
#include "dbvalue.hpp"
#include "lmi.hpp"                      // is_antediluvian_fork()
#include "oecumenic_enumerations.hpp"   // methuselah
#include "product_data.hpp"
#include "yare_input.hpp"

#include <algorithm>                    // min()

/// Construct from essential input (product and axes).

product_database::product_database
    (std::string const& ProductName
    ,mcenum_gender      Gender
    ,mcenum_class       UnderwritingClass
    ,mcenum_smoking     Smoking
    ,int                IssueAge
    ,mcenum_uw_basis    GroupUnderwritingType
    ,mcenum_state       StateOfJurisdiction
    )
    :index_
        (Gender
        ,UnderwritingClass
        ,Smoking
        ,IssueAge
        ,GroupUnderwritingType
        ,StateOfJurisdiction
        )
{
    initialize(ProductName);
}

/// Construct from normal illustration input.

product_database::product_database(yare_input const& input)
    :index_
        (input.Gender
        ,input.UnderwritingClass
        ,input.Smoking
        ,input.IssueAge
        ,input.GroupUnderwritingType
        ,input.StateOfJurisdiction
        )
{
    initialize(input.ProductName);
}

/// Number of years to maturity for default lookup index.
///
/// Almost all database queries use the default index, so caching this
/// value improves performance. For a query with an overridden index
/// that modifies issue age, this cached value is incorrect, so member
/// functions that use a different index don't rely on it.

int product_database::length() const
{
    return length_;
}

/// Default lookup index for database queries.

database_index product_database::index() const
{
    return index_;
}

/// Query database; write result into vector argument.

void product_database::query_into
    (e_database_key        k
    ,std::vector<double>&  dst
    ,database_index const& i
    ) const
{
    int const local_length = maturity_age_ - i.issue_age();
    LMI_ASSERT(0 < local_length && local_length <= methuselah);
    database_entity const& v = entity_from_key(k);
    double const*const z = v[i];
    if(1 == v.extent())
        {
        dst.assign(local_length, *z);
        }
    else
        {
        dst.reserve(local_length);
        dst.assign(z, z + std::min(local_length, v.extent()));
        dst.resize(local_length, dst.back());
        }
}

/// Query database, using default index; write result into vector argument.

void product_database::query_into(e_database_key k, std::vector<double>& dst) const
{
    return query_into(k, dst, index_);
}

/// Query database; return a scalar.
///
/// Throw if the database entity is not scalar.
///
/// Return a double because it is convertible to the most common
/// arithmetic types.

double product_database::query(e_database_key k, database_index const& i) const
{
    database_entity const& v = entity_from_key(k);
    LMI_ASSERT(1 == v.extent());
    return *v[i];
}

/// Ascertain whether two database entities are equivalent.
///
/// Equivalence here means that the dimensions and data are identical.
/// For example, these distinct entities:
///  - DB_PremTaxRate (what the state charges the insurer)
///  - DB_PremTaxLoad (what the insurer charges the customer)
/// may be equivalent when premium tax is passed through as a load.

bool product_database::are_equivalent
    (e_database_key k0
    ,e_database_key k1
    ) const
{
    database_entity const& e0 = entity_from_key(k0);
    database_entity const& e1 = entity_from_key(k1);
    return
           e0.axis_lengths() == e1.axis_lengths()
        && e0.data_values () == e1.data_values ()
        ;
}

/// Ascertain whether a database entity varies by state.

bool product_database::varies_by_state(e_database_key k) const
{
    return 1 != entity_from_key(k).axis_lengths().at(e_axis_state);
}

namespace
{
/// Antediluvian database for static initialization.
///
/// Postcondition: returned pointer is not null; throws otherwise.

std::shared_ptr<DBDictionary> antediluvian_db()
{
    std::shared_ptr<DBDictionary> z(new DBDictionary);
    z->InitAntediluvian();
    LMI_ASSERT(z);
    return z;
}
} // Unnamed namespace.

/// Initialize upon construction.
///
/// Set maturity age and default length (number of years to maturity).

void product_database::initialize(std::string const& product_name)
{
    if(is_antediluvian_fork())
        {
        static std::shared_ptr<DBDictionary> z(antediluvian_db());
        db_ = z;
        }
    else
        {
        std::string const f = filename_from_product_name(product_name);
        product_data const& p(*product_data::read_via_cache(f));
        std::string const filename(p.datum("DatabaseFilename"));
        LMI_ASSERT(!filename.empty());
        db_ = DBDictionary::read_via_cache(AddDataDir(filename));
        }
    query_into(DB_MaturityAge, maturity_age_);
    length_ = maturity_age_ - index_.issue_age();
    LMI_ASSERT(0 < length_ && length_ <= methuselah);
}

DBDictionary const& product_database::db() const
{
    return *db_;
}

/// Database entity corresponding to the given key.

database_entity const& product_database::entity_from_key(e_database_key k) const
{
    return db().datum(db_name_from_key(k));
}
