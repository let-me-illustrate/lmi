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

class product_database;
class stratified_charges;

class premium_tax
  :        private lmi::uncopyable <premium_tax>
  ,virtual private obstruct_slicing<premium_tax>
{
  public:
    premium_tax
        (mcenum_state              premium_tax_state
        ,mcenum_state              state_of_domicile
        ,bool                      amortize_premium_load
        ,product_database   const& database
        ,stratified_charges const& strata
        );
    ~premium_tax();

    double                PremiumTaxRate()             const; ////
    double                PremiumTaxLoad()             const; ////
    double                LowestPremiumTaxLoad()       const; ////
    double                DomiciliaryPremiumTaxLoad()  const; ////
    bool                  PremiumTaxLoadIsTiered()     const; ////

  private:
    void SetPremiumTaxParameters(); ////
    void TestPremiumTaxLoadConsistency() const; ////

    double              PremiumTaxRate_; ////
    double              PremiumTaxLoad_; ////
    double              LowestPremiumTaxLoad_; ////
    double              DomiciliaryPremiumTaxLoad_; ////
    bool                    PremiumTaxLoadIsTieredInStateOfDomicile_; ////
    bool                    PremiumTaxLoadIsTieredInPremiumTaxState_; ////
    bool                    premium_tax_is_retaliatory_; ////
};

bool premium_tax_is_retaliatory
    (mcenum_state premium_tax_state
    ,mcenum_state state_of_domicile
    );

double lowest_premium_tax_load
    (mcenum_state              premium_tax_state
    ,mcenum_state              state_of_domicile
    ,bool                      amortize_premium_load
    ,product_database   const& db
    ,stratified_charges const& stratified
    );

inline double premium_tax::PremiumTaxRate() const ////
{
    return PremiumTaxRate_;
}

inline double premium_tax::PremiumTaxLoad() const ////
{
    return PremiumTaxLoad_;
}

inline double premium_tax::LowestPremiumTaxLoad() const ////
{
    return LowestPremiumTaxLoad_;
}

inline double premium_tax::DomiciliaryPremiumTaxLoad() const ////
{
    return DomiciliaryPremiumTaxLoad_;
}

inline bool premium_tax::PremiumTaxLoadIsTiered() const ////
{
    return PremiumTaxLoadIsTieredInPremiumTaxState_;
}

#endif // premium_tax_hpp

