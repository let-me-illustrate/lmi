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
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "database.hpp"
#include "materially_equal.hpp"
#include "mc_enum_types_aux.hpp" // mc_str()
#include "stratified_charges.hpp"

#include <algorithm>             // std::max()

namespace {
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

bool premium_tax_is_retaliatory(mcenum_state tax_state, mcenum_state domicile)
{
    static int const n = 4;
    static mcenum_state const d[n] = {mce_s_MA, mce_s_MN, mce_s_NY, mce_s_RI};
    static std::vector<mcenum_state> const reciprocal_nonretaliation_states(d, d + n);
    bool const reciprocally_nonretaliatory =
            contains(reciprocal_nonretaliation_states, tax_state)
        &&  contains(reciprocal_nonretaliation_states, domicile)
        ;
    bool const nonretaliatory =
            reciprocally_nonretaliatory
        ||  mce_s_HI == tax_state
        ||  mce_s_XX == tax_state
        ||  mce_s_AK == tax_state
        ||  mce_s_SD == tax_state
        ;
    return !nonretaliatory;
}
} // Unnamed namespace.

/// Production ctor.
///
/// These database entities should be looked up by premium-tax state:
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

premium_tax::premium_tax
    (mcenum_state              tax_state
    ,mcenum_state              domicile
    ,bool                      amortize_premium_load
    ,product_database   const& db
    ,stratified_charges const& strata
    )
    :tax_state_              (tax_state)
    ,domicile_               (domicile)
    ,amortize_premium_load_  (amortize_premium_load)
    ,levy_rate_              (0.0)   // Reset below.
    ,load_rate_              (0.0)   // Reset below.
    ,least_load_rate_        (0.0)   // Reset below.
    ,domiciliary_load_rate_  (0.0)   // Reset below.
    ,is_tiered_in_tax_state_ (false) // Reset below.
    ,is_tiered_in_domicile_  (false) // Reset below.
    ,is_retaliatory_         (false) // Reset below.
    ,ytd_taxable_premium_    (0.0)
    ,ytd_load_               (0.0)
    ,ytd_load_in_tax_state_  (0.0)
    ,ytd_load_in_domicile_   (0.0)
{
    is_tiered_in_tax_state_ = strata.premium_tax_is_tiered(tax_state_);
    is_tiered_in_domicile_  = strata.premium_tax_is_tiered(domicile_ );

    is_retaliatory_ = premium_tax_is_retaliatory(tax_state_, domicile_);

    least_load_rate_ = lowest_premium_tax_load
        (tax_state_
        ,domicile_
        ,amortize_premium_load_
        ,db
        ,strata
        );

    // TODO ?? It would be better not to constrain so many things
    // not to vary by duration by using Query(enumerator).

    database_index index = db.index().state(tax_state_);
    levy_rate_ = db.Query(DB_PremTaxRate, index);
    load_rate_ = db.Query(DB_PremTaxLoad, index);

    {
    database_index index = db.index().state(domicile_);
    domiciliary_load_rate_ = 0.0;
    if(!amortize_premium_load_)
        {
        double domiciliary_levy_rate = db.Query(DB_PremTaxRate, index);
        domiciliary_load_rate_       = db.Query(DB_PremTaxLoad, index);
        if(is_retaliatory_)
            {
            levy_rate_ = std::max(levy_rate_, domiciliary_levy_rate );
            load_rate_ = std::max(load_rate_, domiciliary_load_rate_);
            }
        }
    }

    test_consistency();
}

/// Antediluvian ctor.

premium_tax::premium_tax
    (mcenum_state            tax_state
    ,product_database const& db
    )
    :tax_state_              (tax_state)
    ,domicile_               (mce_s_XX) // Doesn't matter.
    ,amortize_premium_load_  (false)
    ,levy_rate_              (0.0) // Reset below.
    ,load_rate_              (0.0)
    ,least_load_rate_        (0.0)
    ,domiciliary_load_rate_  (0.0)
    ,is_tiered_in_tax_state_ (false)
    ,is_tiered_in_domicile_  (false)
    ,is_retaliatory_         (false)
    ,ytd_taxable_premium_    (0.0)
    ,ytd_load_               (0.0)
    ,ytd_load_in_tax_state_  (0.0)
    ,ytd_load_in_domicile_   (0.0)
{
    database_index index = db.index().state(tax_state_);
    levy_rate_ = db.Query(DB_PremTaxRate, index);
}

premium_tax::~premium_tax()
{}

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
    if(is_tiered_in_tax_state_)
        {
        if(0.0 != load_rate())
            {
            fatal_error()
                << "Premium-tax load is tiered in premium-tax state "
                << mc_str(tax_state_)
                << ", but the product database specifies a scalar load of "
                << load_rate()
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        }

    if(is_tiered_in_domicile_)
        {
        if(0.0 != domiciliary_load_rate_)
            {
            fatal_error()
                << "Premium-tax load is tiered in state of domicile "
                << mc_str(domicile_)
                << ", but the product database specifies a scalar load of "
                << domiciliary_load_rate_
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        fatal_error()
            << "Premium-tax load is tiered in state of domicile "
            << mc_str(domicile_)
            << ", but that case is not supported."
            << LMI_FLUSH
            ;
        }
}

/// Reinitialize YTD state variables.
///
/// Except for initialization to zero, these variables are used only
/// by calculate_load() and the trivial const accessor ytd_load(), in
/// combination with which (and the present function) they may be
/// thought of as constituting a stateful function subobject.
///
/// TODO ?? This is incorrect for inforce. Suppose the tax state has
/// a tiered rate with a breakpoint that has already been met for an
/// off-anniversary inforce illustration. Any further payment in the
/// same year should be taxed at a lower rate. INPUT !! YTD taxable
/// premium needs to be passed from the admin system; it is not
/// necessary to pass the other variables, because they can be
/// calculated here.

void premium_tax::start_new_year()
{
    ytd_taxable_premium_   = 0.0;
    ytd_load_              = 0.0;
    ytd_load_in_tax_state_ = 0.0;
    ytd_load_in_domicile_  = 0.0;
}

/// Calculate premium-tax load.
///
/// The premium-tax load and the actual premium tax payable by an
/// insurer are distinct concepts. They may have equal values when
/// premium tax is passed through as a load.
///
/// DATABASE !! The '.strata' files ought to differentiate tiered
/// premium-tax load paid by customer from rate paid by insurer.
///
/// An assertion ensures that either tiered or non-tiered premium-tax
/// load is zero.

double premium_tax::calculate_load(double payment, stratified_charges const& strata)
{
    double tax_in_tax_state = load_rate() * payment;
    if(is_tiered_in_tax_state_)
        {
        LMI_ASSERT(0.0 == tax_in_tax_state);
        tax_in_tax_state = strata.tiered_premium_tax
            (tax_state_
            ,payment
            ,ytd_taxable_premium_
            );
        }
    ytd_load_in_tax_state_ += tax_in_tax_state;

    double tax_in_domicile = 0.0;
    if(is_retaliatory_)
        {
        tax_in_domicile = domiciliary_load_rate_ * payment;
        if(is_tiered_in_domicile_)
            {
            LMI_ASSERT(0.0 == tax_in_domicile);
            tax_in_domicile = strata.tiered_premium_tax
                (domicile_
                ,payment
                ,ytd_taxable_premium_
                );
            }
        ytd_load_in_domicile_ += tax_in_domicile;
        }

    ytd_taxable_premium_ += payment;

    // 'x' is more robust, though more prone to roundoff error than
    // 'y', so 'y' is preferred iff they're materially equal.
    double ytd_tax_reflecting_retaliation = std::max
        (ytd_load_in_tax_state_
        ,ytd_load_in_domicile_
        );
    double x = std::max(0.0, ytd_tax_reflecting_retaliation - ytd_load_);
    double y = std::max(tax_in_tax_state, tax_in_domicile);
    double z = materially_equal(x, y) ? y : x;
    ytd_load_ += z;
    return z;
}

/// Premium-tax rates for life insurance without retaliation.
///
/// A single table suffices for every domicile, because retaliation is
/// explicitly performed elsewhere.
///
/// AK and SD have a tiered premium tax that lmi handles; DE has one
/// that it does not. As elsewhere in lmi, tiered and non-tiered
/// charges are calculated separately and added together; therefore,
/// the AK and SD values in this table are zero.
///
/// Fictitious state XX may be used where no premium tax applies, as
/// for offshore business.

std::vector<double> const& premium_tax_rates_for_life_insurance()
{
    static double const tiered = 0.0;
    static int const n = 53;
    static double const d[n] =
        //   AL       AK       AZ       AR       CA       CO       CT
        {0.0230,  tiered,  0.0200,  0.0250,  0.0235,  0.0200,  0.0175
        //   DE       DC       FL       GA       HI       ID
        ,0.0200,  0.0200,  0.0175,  0.0225,  0.0275,  0.0150
        //   IL       IN       IA       KS       KY       LA       ME
        ,0.0050,  0.0130,  0.0100,  0.0200,  0.0150,  0.0225,  0.0200
        //   MD       MA       MI       MN       MS       MO
        ,0.0200,  0.0200,  0.0125,  0.0150,  0.0300,  0.0200
        //   MT       NE       NV       NH       NJ       NM       NY
        ,0.0275,  0.0100,  0.0350,  0.0125,  0.0210,  0.03003, 0.0150
        //   NC       ND       OH       OK       OR       PA
        ,0.0190,  0.0200,  0.0140,  0.0225,  0.0002,  0.0200
        //   PR       RI       SC       SD       TN       TX       UT
        ,0.0400,  0.0200,  0.0075,  tiered,  0.0175,  0.0175,  0.0225
        //   VT       VA       WA       WV       WI       WY       XX
        ,0.0200,  0.0225,  0.0200,  0.0300,  0.0200,  0.0075,  0.0000
        };
    static std::vector<double> const v(d, d + n);
    return v;
}

// Placeholder for annuity rates--for now, just a commented-out copy
// of the life-insurance implementation.
#if 0
std::vector<double> const& premium_tax_rates_for_annuities()
{
    static double const tiered = 0.0;
    static int const n = 53;
    static double const d[n] =
        //   AL       AK       AZ       AR       CA       CO       CT
        {0.0230,  tiered,  0.0200,  0.0250,  0.0235,  0.0200,  0.0175
        //   DE       DC       FL       GA       HI       ID
        ,0.0200,  0.0200,  0.0175,  0.0225,  0.0275,  0.0150
        //   IL       IN       IA       KS       KY       LA       ME
        ,0.0050,  0.0130,  0.0100,  0.0200,  0.0150,  0.0225,  0.0200
        //   MD       MA       MI       MN       MS       MO
        ,0.0200,  0.0200,  0.0125,  0.0150,  0.0300,  0.0200
        //   MT       NE       NV       NH       NJ       NM       NY
        ,0.0275,  0.0100,  0.0350,  0.0125,  0.0210,  0.03003, 0.0150
        //   NC       ND       OH       OK       OR       PA
        ,0.0190,  0.0200,  0.0140,  0.0225,  0.0002,  0.0200
        //   PR       RI       SC       SD       TN       TX       UT
        ,0.0400,  0.0200,  0.0075,  tiered,  0.0175,  0.0175,  0.0225
        //   VT       VA       WA       WV       WI       WY       XX
        ,0.0200,  0.0225,  0.0200,  0.0300,  0.0200,  0.0075,  0.0000
        };
    static std::vector<double> const v(d, d + n);
    return v;
}
#endif // 0

/// Lowest premium-tax load, for 7702 and 7702A purposes.

double lowest_premium_tax_load
    (mcenum_state              tax_state
    ,mcenum_state              domicile
    ,bool                      amortize_premium_load
    ,product_database   const& db
    ,stratified_charges const& strata
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

    database_index index = db.index().state(tax_state);
    z = db.Query(DB_PremTaxLoad, index);

    if(premium_tax_is_retaliatory(tax_state, domicile))
        {
        index = db.index().state(domicile);
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

    if(strata.premium_tax_is_tiered(tax_state))
        {
        if(0.0 != z)
            {
            fatal_error()
                << "Premium-tax load is tiered in state "
                << mc_str(tax_state)
                << ", but the product database specifies a scalar load of "
                << z
                << " instead of zero as expected. Probably the database"
                << " is incorrect."
                << LMI_FLUSH
                ;
            }
        z = strata.minimum_tiered_premium_tax_rate(tax_state);
        }

    return z;
}

double premium_tax::ytd_load() const
{
    return ytd_load_;
}

double premium_tax::levy_rate() const
{
    return levy_rate_;
}

double premium_tax::load_rate() const
{
    return load_rate_;
}

double premium_tax::least_load_rate() const
{
    return least_load_rate_;
}

bool premium_tax::is_tiered() const
{
    return is_tiered_in_tax_state_ || is_tiered_in_domicile_;
}

