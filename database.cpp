// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "dbdict.hpp"
#include "dbvalue.hpp"
#include "lmi.hpp"                    // is_antediluvian_fork()
#include "oecumenic_enumerations.hpp" // methuselah
#include "product_data.hpp"
#include "yare_input.hpp"

#include <algorithm> // std::min()

/// Construct from essential input (product and axes).

product_database::product_database
    (std::string const& a_ProductName
    ,mcenum_gender      a_Gender
    ,mcenum_class       a_Class
    ,mcenum_smoking     a_Smoker
    ,int                a_IssueAge
    ,mcenum_uw_basis    a_UWBasis
    ,mcenum_state       a_State
    )
{
    if(is_antediluvian_fork())
        {
        DBDictionary::instance().InitAntediluvian();
        }
    else
        {
        std::string filename(product_data(a_ProductName).datum("DatabaseFilename"));
        DBDictionary::instance().Init(AddDataDir(filename));
        }
    index_ = database_index
        (a_Gender
        ,a_Class
        ,a_Smoker
        ,a_IssueAge
        ,a_UWBasis
        ,a_State
        );
    length_ = static_cast<int>(Query(DB_MaturityAge)) - a_IssueAge;
    LMI_ASSERT(0 < length_ && length_ <= methuselah);
}

/// Construct from normal illustration input.

product_database::product_database(yare_input const& input)
{
    if(is_antediluvian_fork())
        {
        DBDictionary::instance().InitAntediluvian();
        }
    else
        {
        std::string filename(product_data(input.ProductName).datum("DatabaseFilename"));
        DBDictionary::instance().Init(AddDataDir(filename));
        }
    index_ = database_index
        (input.Gender
        ,input.UnderwritingClass
        ,input.Smoking
        ,input.IssueAge
        ,input.GroupUnderwritingType
        ,input.StateOfJurisdiction
        );
    length_ = static_cast<int>(Query(DB_MaturityAge)) - input.IssueAge;
    LMI_ASSERT(0 < length_ && length_ <= methuselah);
}

product_database::~product_database()
{
}

int product_database::length() const
{
    return length_;
}

double product_database::Query(e_database_key k) const
{
    database_entity const& v = entity_from_key(k);
    LMI_ASSERT(1 == v.extent());
    return *v[index_];
}

void product_database::Query(std::vector<double>& dst, e_database_key k) const
{
    database_entity const& v = entity_from_key(k);
    double const*const z = v[index_];
    if(1 == v.extent())
        {
        dst.assign(length_, *z);
        }
    else
        {
        dst.reserve(length_);
        dst.assign(z, z + std::min(length_, v.extent()));
        dst.resize(length_, dst.back());
        }
}

/// Ascertain whether two database entities are equivalent.
///
/// Equivalence here means that the dimensions and data are identical.
/// For example, these distinct entities:
///  - DB_PremTaxRate (what the state charges the insurer)
///  - DB_PremTaxLoad (what the insurer charges the customer)
/// may be equivalent when premium tax is passed through as a load.

bool product_database::are_equivalent(e_database_key k0, e_database_key k1) const
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

database_entity const& product_database::entity_from_key(e_database_key k) const
{
    DBDictionary const& db = DBDictionary::instance();
    return db.datum(db_name_from_key(k));
}

