// Premium tax.
//
// Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef premium_tax_hpp
#define premium_tax_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"       // mcenum_state

#include <vector>

class product_database;
class stratified_charges;

/// Calculate premium tax, particularly when passed through as a load.
///
/// Scalar and tiered rates are deliberately maintained in the product
/// files so that they can be readily examined, transferred to other
/// systems, and overridden if desired. The values hardcoded here and
/// used as defaults in product files are generally appropriate, but
/// subject to interpretation because some states impose assessments
/// in addition to their nominal tax rates.
///
/// Retaliation is handled in the code, so rates specified in the
/// product database shouldn't reflect retaliation. (If they did, then
/// different rates would be needed for different domiciles.)
///
/// The product database distinguishes premium-tax loads from rates:
/// 'DB_PremTaxLoad' is the load charged by the insurer against the
/// contract, while 'DB_PremTaxRate' is the tax rate levied by the
/// state upon the insurer.
///
/// At present, however, '.strata' files contain only the (tiered) tax
/// rates: there are no distinct tiered tax loads.
///
/// Despite that limitation, the correct answer is obtained for the
/// two cases that have arisen in practice. In the first case, the
/// premium-tax load doesn't vary by state--perhaps a flat load such
/// as two percent is used, or maybe zero percent with premium-tax
/// expense covered elsewhere in pricing--and tiering is ignored, so
/// the tax load is simply that scalar. In the second case, the exact
/// premium tax is passed through, so the load equals the tax rate.
///
/// Known shortcomings.
///
/// Default values and brackets for tiered rates should be moved here
/// from 'stratified_charges.cpp', in order to keep all premium-tax
/// information in one place.
///
/// The physical complexity should be reduced by adding a new ctor
/// that takes as arguments the actual values and brackets read from
/// the '.strata' and '.database' files, and implementing the existing
/// ctors in a TU of their own. This would probably suggest physical
/// improvements to classes Loads and load_details.
///
/// Once actual values and brackets read from '.strata' files have
/// been copied here, and portions of 'stratified_charges.cpp' moved
/// here, retaliation can be made more correct in "tiered" states.
/// AK and SD retaliate with respect to their initial bands only, but
/// such retaliation is for now presumed not to occur, so AK and SD
/// domiciles are not supported. Furthermore, retaliation against AK
/// and SD contracts is not supported, so premium tax is understated
/// for a few domiciles.
///
/// start_new_year() should be improved as noted in its documentation.

class premium_tax final
{
  public:
    premium_tax
        (mcenum_state              tax_state
        ,mcenum_state              domicile
        ,bool                      amortize_premium_load
        ,product_database   const& db
        ,stratified_charges const& strata
        );
    premium_tax
        (mcenum_state              tax_state
        ,product_database   const& db
        );
    ~premium_tax() = default;

    void   start_new_year();
    double calculate_load(double payment, stratified_charges const& strata);
    double ytd_load               () const;

    double levy_rate              () const;
    double load_rate              () const;
    double maximum_load_rate      () const;
    double minimum_load_rate      () const;
    bool   is_tiered              () const;

  private:
    premium_tax(premium_tax const&) = delete;
    premium_tax& operator=(premium_tax const&) = delete;

    void test_consistency() const;

    double ascertain_maximum_load_rate(stratified_charges const& strata) const;
    double ascertain_minimum_load_rate(stratified_charges const& strata) const;

    // Ctor value-arguments.
    mcenum_state tax_state_;
    mcenum_state domicile_;
    bool         amortize_premium_load_;

    double levy_rate_;
    double load_rate_;
    double tax_state_load_rate_;
    double domiciliary_load_rate_;
    double maximum_load_rate_;
    double minimum_load_rate_;
    bool   is_tiered_in_tax_state_;
    bool   is_tiered_in_domicile_;
    bool   is_retaliatory_;
    bool   varies_by_state_;
    bool   load_rate_is_levy_rate_;

    double ytd_taxable_premium_;
    double ytd_load_;
    double ytd_load_in_tax_state_;
    double ytd_load_in_domicile_;
};

std::vector<double> const& premium_tax_rates_for_life_insurance();

std::vector<double> const& premium_tax_rates_for_annuities();

#endif // premium_tax_hpp
