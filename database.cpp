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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
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
    :Gender   (a_Gender)
    ,Class    (a_Class)
    ,Smoker   (a_Smoker)
    ,IssueAge (a_IssueAge)
    ,UWBasis  (a_UWBasis)
    ,State    (a_State)
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
    initialize();
}

/// Construct from normal illustration input.
///
/// For the nonce, this ctor determines "state of jurisdiction"
/// dynamically, and other code uses that state for multiple purposes.
/// That is a mistake--two states are required:
///   - FilingApprovalState: the state that must approve a policy-form
///     filing (whether affirmatively or by deemer) before a contract
///     can be written; and
///   - PremiumTaxState: the state to which premium tax must be paid,
///     which is crucial for products that pass premium tax through as
///     a load.
/// Those two states can differ, e.g. on cases with more than five
/// hundred lives with a common (employer) issue state: the employer's
/// state approves the policy form, but premium tax follows employee
/// residence. See:
///   http://www.naic.org/documents/frs_summit_presentations_03.pdf
///   http://www.naic.org/documents/committees_e_app_blanks_adopted_2007-42BWG_Modified.pdf
///
/// Soon, both states will be input fields, and these members will be
/// expunged:
///   GetStateOfJurisdiction()
///   Gender
///   Class
///   Smoker
///   IssueAge
///   UWBasis
///   State
/// Database entity DB_PremTaxState will become obsolete, but must be
/// retained (with a different name) for backward compatibility.

product_database::product_database(yare_input const& input)
{
    Gender      = input.Gender;
    Class       = input.UnderwritingClass;
    Smoker      = input.Smoking;
    IssueAge    = input.IssueAge;
    UWBasis     = input.GroupUnderwritingType;
    State       = mce_s_CT; // Dummy initialization.

    if(is_antediluvian_fork())
        {
        DBDictionary::instance().InitAntediluvian();
        }
    else
        {
        std::string filename(product_data(input.ProductName).datum("DatabaseFilename"));
        DBDictionary::instance().Init(AddDataDir(filename));
        }
    initialize();

    // State of jurisdiction must not depend on itself.
    if(varies_by_state(DB_PremTaxState))
        {
        fatal_error()
            << "Database invalid: circular dependency."
            << " State of jurisdiction depends on itself."
            << LMI_FLUSH
            ;
        }

    switch(static_cast<int>(Query(DB_PremTaxState)))
        {
        case oe_ee_state:
            {
            State = input.State;
            }
            break;
        case oe_er_state:
            {
            State = input.CorporationState;
            }
            break;
        default:
            {
            fatal_error()
                << "Cannot determine state of jurisdiction."
                << LMI_FLUSH
                ;
            }
            break;
        }

    // It may seem excessive to do this when only 'State' has changed,
    // but it'll become unnecessary when we handle state of jurisdiction
    // as an input field instead of trying to determine it here.
    index_ = database_index(Gender, Class, Smoker, IssueAge, UWBasis, State);
}

product_database::~product_database()
{
}

mcenum_state product_database::GetStateOfJurisdiction() const
{
    return State;
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

void product_database::initialize()
{
    index_ = database_index(Gender, Class, Smoker, IssueAge, UWBasis, State);
    length_ = static_cast<int>(Query(DB_MaturityAge)) - IssueAge;
    LMI_ASSERT(0 < length_ && length_ <= methuselah);
}

database_entity const& product_database::entity_from_key(e_database_key k) const
{
    DBDictionary const& db = DBDictionary::instance();
    return db.datum(db_name_from_key(k));
}

