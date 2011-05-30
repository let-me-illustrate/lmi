// Premium tax.
//
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include "premium_tax.hpp"

#include "alert.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "mc_enum_types_aux.hpp" // mc_str()
#include "stratified_charges.hpp"

premium_tax::premium_tax
    (mcenum_state              premium_tax_state
    ,mcenum_state              state_of_domicile
    ,bool                      amortize_premium_load
    ,product_database   const& database
    ,stratified_charges const& strata
    )
    :premium_tax_state_     (premium_tax_state)
    ,state_of_domicile_     (state_of_domicile)
    ,amortize_premium_load_ (amortize_premium_load)
{
    set_parameters(database, strata);
}

premium_tax::~premium_tax()
{}

/// Set all parameters that depend on premium-tax state.
///
/// These database entities should be looked up by tax state:
///  - DB_PremTaxLoad
///  - DB_PremTaxRate
/// These probably (for inchoate amortization) shouldn't:
///  - DB_PremTaxAmortPeriod
///  - DB_PremTaxAmortIntRate
/// This definitely shouldn't be:
///  - DB_PremTaxState
/// These aren't used anywhere yet:
///  - DB_PremTaxFundCharge
///  - DB_PremTaxTable
///  - DB_PremTaxTierGroup
///  - DB_PremTaxTierPeriod
///  - DB_PremTaxTierNonDecr

void premium_tax::set_parameters
    (product_database   const& database
    ,stratified_charges const& strata
    )
{
    load_is_tiered_in_premium_tax_state_ = strata.premium_tax_is_tiered(premium_tax_state_);
    load_is_tiered_in_state_of_domicile_ = strata.premium_tax_is_tiered(state_of_domicile_);

    premium_tax_is_retaliatory_ = ::premium_tax_is_retaliatory
        (premium_tax_state_
        ,state_of_domicile_
        );

    least_load_rate_ = lowest_premium_tax_load
        (premium_tax_state_
        ,state_of_domicile_
        ,amortize_premium_load_
        ,database
        ,strata
        );

    // TODO ?? It would be better not to constrain so many things
    // not to vary by duration by using Query(enumerator).

    database_index index = database.index().state(premium_tax_state_);
    levy_rate_ = database.Query(DB_PremTaxRate, index);
    load_rate_ = database.Query(DB_PremTaxLoad, index);

    {
    database_index index = database.index().state(state_of_domicile_);
    domiciliary_load_rate_ = 0.0;
    if(!amortize_premium_load_)
        {
        double domiciliary_levy_rate = database.Query(DB_PremTaxRate, index);
        domiciliary_load_rate_       = database.Query(DB_PremTaxLoad, index);
        if(premium_tax_is_retaliatory_)
            {
            levy_rate_ = std::max(levy_rate_, domiciliary_levy_rate );
            load_rate_ = std::max(load_rate_, domiciliary_load_rate_);
            }
        }
    }

    test_consistency();
}

/// Test consistency of premium-tax loads.
///
/// In particular, if the tiered premium-tax load isn't zero, then the
/// corresponding non-tiered load must be zero.
///
/// Premium-tax pass-through for AK, DE, and SD insurers is not
/// supported. If the state of domicile has a tiered rate, then most
/// likely the premium-tax state does not, and retaliation would often
/// override the tiering. When those two states are the same, then no
/// retaliation occurs, and calculations would presumably be correct.
/// When both states have tiered rates, but they are different states,
/// then the calculation could be complicated; but DE tiering is not
/// supported at all yet, and AK (SD) companies probably write few
/// contracts in SD (AK), so these exotic cases haven't commanded any
/// attention. If premium tax is not passed through as a load, then
/// there's no problem at all.

void premium_tax::test_consistency() const
{
    if(load_is_tiered_in_premium_tax_state_)
        {
        if(0.0 != load_rate())
            {
            fatal_error()
                << "Premium-tax load is tiered in premium-tax state "
                << mc_str(premium_tax_state_)
                << ", but the product database specifies a scalar load of "
                << load_rate()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        }

    if(load_is_tiered_in_state_of_domicile_)
        {
        if(0.0 != domiciliary_load_rate())
            {
            fatal_error()
                << "Premium-tax load is tiered in state of domicile "
                << mc_str(state_of_domicile_)
                << ", but the product database specifies a scalar load of "
                << domiciliary_load_rate()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        fatal_error()
            << "Premium-tax load is tiered in state of domicile "
            << mc_str(state_of_domicile_)
            << ", but that case is not supported."
            << LMI_FLUSH
            ;
        }
}

/// Determine whether premium tax is retaliatory.
///
/// Here's a general discussion:
///   http://leg2.state.va.us/dls/h&sdocs.nsf/fc86c2b17a1cf388852570f9006f1299/461afa310d4d3d528525646500562282/$FILE/HD78_1997.pdf
///
/// Premium tax is retaliatory in most states. Exceptions:
///   - MA, MN, NY, and RI are mutually nonretaliatory.
///   - HI never retaliates; neither does fictitious state XX.
///   - AK and SD retaliate only on the bottom tier; this is best
///     implemented by adjusting that tier's rate and treating them
///     as otherwise nonretaliatory.

bool premium_tax_is_retaliatory
    (mcenum_state premium_tax_state
    ,mcenum_state state_of_domicile
    )
{
    static int const n = 4;
    static mcenum_state const d[n] = {mce_s_MA, mce_s_MN, mce_s_NY, mce_s_RI};
    static std::vector<mcenum_state> const reciprocal_nonretaliation_states(d, d + n);
    bool const reciprocally_nonretaliatory =
            contains(reciprocal_nonretaliation_states, premium_tax_state)
        &&  contains(reciprocal_nonretaliation_states, state_of_domicile)
        ;
    bool const nonretaliatory =
            reciprocally_nonretaliatory
        ||  mce_s_HI == premium_tax_state
        ||  mce_s_XX == premium_tax_state
        ||  mce_s_AK == premium_tax_state
        ||  mce_s_SD == premium_tax_state
        ;
    return !nonretaliatory;
}

/// Lowest premium-tax load, for 7702 and 7702A purposes.

double lowest_premium_tax_load
    (mcenum_state              premium_tax_state
    ,mcenum_state              state_of_domicile
    ,bool                      amortize_premium_load
    ,product_database   const& db
    ,stratified_charges const& stratified
    )
{
    // TRICKY !! Here, we use 'DB_PremTaxLoad', not 'DB_PremTaxRate',
    // to determine the lowest premium-tax load. Premium-tax loads
    // (charged by the insurer to the contract) and rates (charged by
    // the state to the insurer) really shouldn't be mixed. The
    // intention is to support products that pass actual premium tax
    // through as a load, taking into account retaliation and tiered
    // premium-tax rates.
    //
    // While a more complicated model would be more aesthetically
    // satisfying, this gives the right answer in practice for the
    // two cases we believe will arise in practice. In the first case,
    // premium-tax load doesn't vary by state--perhaps a flat load
    // such as two percent might be used, or maybe zero percent with
    // premium-tax expense covered elsewhere in pricing--and tiering
    // is ignored, so this implementation just returns the flat load.
    // In the second case, the exact premium tax is passed through,
    // so the tax rate equals the tax load.

    double z = 0.0;
    if(amortize_premium_load)
        {
        return z;
        }

    database_index index = db.index().state(premium_tax_state);
    z = db.Query(DB_PremTaxLoad, index);

    if(premium_tax_is_retaliatory(premium_tax_state, state_of_domicile))
        {
        index = db.index().state(state_of_domicile);
        z = std::max(z, db.Query(DB_PremTaxLoad, index));
        }

    if(!db.varies_by_state(DB_PremTaxLoad))
        {
        return z;
        }

    // If premium-tax load varies by state, we're assuming that
    // it equals premium-tax rate--i.e. that premium tax is passed
    // through exactly--and that therefore tiered tax rates determine
    // loads where applicable and implemented.
    if(!db.are_equivalent(DB_PremTaxLoad, DB_PremTaxRate))
        {
        fatal_error()
            << "Premium-tax load varies by state, but differs"
            << " from premium-tax rates. Probably the database"
            << " is incorrect.\n"
            << LMI_FLUSH
            ;
        }

    if(stratified.premium_tax_is_tiered(premium_tax_state))
        {
        if(0.0 != z)
            {
            fatal_error()
                << "Premium-tax load is tiered in state "
                << mc_str(premium_tax_state)
                << ", but the product database specifies a scalar load of "
                << z
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        z = stratified.minimum_tiered_premium_tax_rate(premium_tax_state);
        }

    return z;
}

