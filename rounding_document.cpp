// Document class for Rounding rules.
//
// Copyright (C) 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: rounding_document.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#include "rounding_document.hpp"

#include "alert.hpp"
#include "rounding_rules.hpp"
#include "rounding_view.hpp"
#include "rounding_view_editor.hpp"
#include "view_ex.tpp"


#include <wx/defs.h>

IMPLEMENT_DYNAMIC_CLASS(RoundingDocument, ProductEditorDocument)

RoundingDocument::RoundingDocument()
    :ProductEditorDocument()
    ,rounding_rules_()
    ,values_()
{
    values_["specamt"           ] = &rounding_rules_.round_specamt_;
    values_["death_benefit"     ] = &rounding_rules_.round_death_benefit_;
    values_["naar"              ] = &rounding_rules_.round_naar_;
    values_["coi_rate"          ] = &rounding_rules_.round_coi_rate_;
    values_["coi_charge"        ] = &rounding_rules_.round_coi_charge_;
    values_["gross_premium"     ] = &rounding_rules_.round_gross_premium_;
    values_["net_premium"       ] = &rounding_rules_.round_net_premium_;
    values_["interest_rate"     ] = &rounding_rules_.round_interest_rate_;
    values_["interest_credit"   ] = &rounding_rules_.round_interest_credit_;
    values_["withdrawal"        ] = &rounding_rules_.round_withdrawal_;
    values_["loan"              ] = &rounding_rules_.round_loan_;
    values_["corridor_factor"   ] = &rounding_rules_.round_corridor_factor_;
    values_["surrender_charge"  ] = &rounding_rules_.round_surrender_charge_;
    values_["irr"               ] = &rounding_rules_.round_irr_;
    values_["min_specamt"       ] = &rounding_rules_.round_min_specamt_;
    values_["max_specamt"       ] = &rounding_rules_.round_max_specamt_;
    values_["min_premium"       ] = &rounding_rules_.round_min_premium_;
    values_["max_premium"       ] = &rounding_rules_.round_max_premium_;
    values_["interest_rate_7702"] = &rounding_rules_.round_interest_rate_7702_;
}

RoundingDocument::~RoundingDocument()
{
}

void RoundingDocument::ReadDocument(wxString const& filename)
{
    rounding_rules_.Read(filename);
    if(!GetViews().empty())
        {
        RoundingView& view = PredominantView();
        for
            (values_type::iterator it = values_.begin()
            ,end = values_.end()
            ;it != end
            ;++it
            )
            {
            view.controls()[it->first]->SetValue(*it->second);
            }
        }
}

void RoundingDocument::WriteDocument(wxString const& filename)
{
    if(!GetViews().empty())
        {
        RoundingView& view = PredominantView();
        for
            (values_type::iterator it = values_.begin()
            ,end = values_.end()
            ;it != end
            ;++it
            )
            {
            *it->second = view.controls()[it->first]->GetValue();
            }
        }
    rounding_rules_.Write(filename);
}

RoundingView& RoundingDocument::PredominantView() const
{
    return ::PredominantView<RoundingView>(*this);
}

