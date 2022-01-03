// Document class for Rounding rules.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "rounding_document.hpp"

// EVGENIY !! Doesn't it seem strange that class RoundingButtons,
// a GUI class, is required in a document implementation?

#include "rounding_view.hpp"
#include "rounding_view_editor.hpp"     // class RoundingButtons
#include "view_ex.tpp"

IMPLEMENT_DYNAMIC_CLASS(RoundingDocument, ProductEditorDocument)

RoundingDocument::RoundingDocument()
    :ProductEditorDocument {}
    ,rounding_rules_       {}
    ,values_               {}
{
    values_["round_specamt"           ] = &rounding_rules_.round_specamt_           ;
    values_["round_death_benefit"     ] = &rounding_rules_.round_death_benefit_     ;
    values_["round_naar"              ] = &rounding_rules_.round_naar_              ;
    values_["round_coi_rate"          ] = &rounding_rules_.round_coi_rate_          ;
    values_["round_coi_charge"        ] = &rounding_rules_.round_coi_charge_        ;
    values_["round_rider_charges"     ] = &rounding_rules_.round_rider_charges_     ;
    values_["round_gross_premium"     ] = &rounding_rules_.round_gross_premium_     ;
    values_["round_net_premium"       ] = &rounding_rules_.round_net_premium_       ;
    values_["round_interest_rate"     ] = &rounding_rules_.round_interest_rate_     ;
    values_["round_interest_credit"   ] = &rounding_rules_.round_interest_credit_   ;
    values_["round_withdrawal"        ] = &rounding_rules_.round_withdrawal_        ;
    values_["round_loan"              ] = &rounding_rules_.round_loan_              ;
    values_["round_interest_rate_7702"] = &rounding_rules_.round_interest_rate_7702_;
    values_["round_corridor_factor"   ] = &rounding_rules_.round_corridor_factor_   ;
    values_["round_nsp_rate_7702"     ] = &rounding_rules_.round_nsp_rate_7702_     ;
    values_["round_seven_pay_rate"    ] = &rounding_rules_.round_seven_pay_rate_    ;
    values_["round_surrender_charge"  ] = &rounding_rules_.round_surrender_charge_  ;
    values_["round_irr"               ] = &rounding_rules_.round_irr_               ;
    values_["round_min_specamt"       ] = &rounding_rules_.round_min_specamt_       ;
    values_["round_max_specamt"       ] = &rounding_rules_.round_max_specamt_       ;
    values_["round_min_premium"       ] = &rounding_rules_.round_min_premium_       ;
    values_["round_max_premium"       ] = &rounding_rules_.round_max_premium_       ;
    values_["round_minutiae"          ] = &rounding_rules_.round_minutiae_          ;
}

void RoundingDocument::ReadDocument(std::string const& filename)
{
    load(rounding_rules_, filename);
    if(!GetViews().empty())
        {
        RoundingView& view = PredominantView();
        for(auto const& i : values_)
            {
            view.controls()[i.first]->SetValue(*i.second);
            }
        }
}

void RoundingDocument::WriteDocument(std::string const& filename)
{
    if(!GetViews().empty())
        {
        RoundingView& view = PredominantView();
        for(auto const& i : values_)
            {
            *i.second = view.controls()[i.first]->GetValue();
            }
        }
    save(rounding_rules_, filename);
}

RoundingView& RoundingDocument::PredominantView() const
{
    return ::PredominantView<RoundingView>(*this);
}
