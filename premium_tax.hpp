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

#ifndef premium_tax_hpp
#define premium_tax_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp" // mcenum_state
#include "obstruct_slicing.hpp"
#include "uncopyable_lmi.hpp"

#include <vector>

class product_database;
class stratified_charges;

/// Calculate premium tax, particularly when passed through as a load.
///
/// Scalar and tiered rates are deliberately maintained in the product
/// files so that they can be readily examined, transferred to other
/// systems, and overridden if desired. The values hardcoded here are
/// generally appropriate, but subject to interpretation because some
/// states impose assessments in addition to their nominal tax rates.
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
/// such retaliation is for now presumed not to occur.
///
/// A greatest-premium-tax-load function is wanted for the approximate
/// "pay as you go" modal premium (BasicValues::GetModalPremMlyDed()).
/// At present, that premium is insufficient to prevent instant lapse
/// in AK and SD when premium tax is passed through as a load.
///
/// start_new_year() should be improved as noted in its documentation.
///
/// premium_tax_rates_for_annuities() should be implemented.

class premium_tax
  :        private lmi::uncopyable <premium_tax>
  ,virtual private obstruct_slicing<premium_tax>
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
    ~premium_tax();

    void   start_new_year();
    double calculate_load(double payment, stratified_charges const& strata);
    double ytd_load               () const;

    double levy_rate              () const;
    double load_rate              () const;
    double least_load_rate        () const;
    bool   is_tiered              () const;

  private:
    void test_consistency() const;

    // Ctor value-arguments.
    mcenum_state tax_state_;
    mcenum_state domicile_;
    bool         amortize_premium_load_;

    double levy_rate_;
    double load_rate_;
    double least_load_rate_;
    double domiciliary_load_rate_;
    bool   is_tiered_in_tax_state_;
    bool   is_tiered_in_domicile_;
    bool   is_retaliatory_;

    double ytd_taxable_premium_;
    double ytd_load_;
    double ytd_load_in_tax_state_;
    double ytd_load_in_domicile_;
};

std::vector<double> const& premium_tax_rates_for_life_insurance();

std::vector<double> const& premium_tax_rates_for_annuities();

double lowest_premium_tax_load
    (mcenum_state              tax_state
    ,mcenum_state              domicile
    ,bool                      amortize_premium_load
    ,product_database   const& db
    ,stratified_charges const& strata
    );

#endif // premium_tax_hpp

