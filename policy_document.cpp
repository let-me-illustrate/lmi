// Document class for product data.
//
// Copyright (C) 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "policy_document.hpp"

#include "policy_view.hpp"
#include "view_ex.tpp"

// EVGENIY !! Doesn't it seem strange that class wxTextCtrl,
// a GUI class, is required in a document implementation?

#include <wx/textctrl.h>

IMPLEMENT_DYNAMIC_CLASS(PolicyDocument, ProductEditorDocument)

PolicyDocument::PolicyDocument()
    :ProductEditorDocument()
    ,product_data_()
    ,values_()
{
    values_["DatabaseFilename"       ] = &product_data_.DatabaseFilename       .datum_;
    values_["FundFilename"           ] = &product_data_.FundFilename           .datum_;
    values_["CorridorFilename"       ] = &product_data_.CorridorFilename       .datum_;
    values_["CurrCOIFilename"        ] = &product_data_.CurrCOIFilename        .datum_;
    values_["GuarCOIFilename"        ] = &product_data_.GuarCOIFilename        .datum_;
    values_["WPFilename"             ] = &product_data_.WPFilename             .datum_;
    values_["ADDFilename"            ] = &product_data_.ADDFilename            .datum_;
    values_["ChildRiderFilename"     ] = &product_data_.ChildRiderFilename     .datum_;
    values_["CurrSpouseRiderFilename"] = &product_data_.CurrSpouseRiderFilename.datum_;
    values_["GuarSpouseRiderFilename"] = &product_data_.GuarSpouseRiderFilename.datum_;
    values_["CurrTermFilename"       ] = &product_data_.CurrTermFilename       .datum_;
    values_["GuarTermFilename"       ] = &product_data_.GuarTermFilename       .datum_;
    values_["TableYFilename"         ] = &product_data_.TableYFilename         .datum_;
    values_["PremTaxFilename"        ] = &product_data_.PremTaxFilename        .datum_;
    values_["TAMRA7PayFilename"      ] = &product_data_.TAMRA7PayFilename      .datum_;
    values_["TgtPremFilename"        ] = &product_data_.TgtPremFilename        .datum_;
    values_["IRC7702Filename"        ] = &product_data_.IRC7702Filename        .datum_;
    values_["Gam83Filename"          ] = &product_data_.Gam83Filename          .datum_;
    values_["SubstdTblMultFilename"  ] = &product_data_.SubstdTblMultFilename  .datum_;
    values_["CurrSpecAmtLoadFilename"] = &product_data_.CurrSpecAmtLoadFilename.datum_;
    values_["GuarSpecAmtLoadFilename"] = &product_data_.GuarSpecAmtLoadFilename.datum_;
    values_["RoundingFilename"       ] = &product_data_.RoundingFilename       .datum_;
    values_["TierFilename"           ] = &product_data_.TierFilename           .datum_;
    values_["PolicyForm"             ] = &product_data_.PolicyForm             .datum_;
    values_["PolicyMktgName"         ] = &product_data_.PolicyMktgName         .datum_;
    values_["PolicyLegalName"        ] = &product_data_.PolicyLegalName        .datum_;
    values_["InsCoShortName"         ] = &product_data_.InsCoShortName         .datum_;
    values_["InsCoName"              ] = &product_data_.InsCoName              .datum_;
    values_["InsCoAddr"              ] = &product_data_.InsCoAddr              .datum_;
    values_["InsCoStreet"            ] = &product_data_.InsCoStreet            .datum_;
    values_["InsCoPhone"             ] = &product_data_.InsCoPhone             .datum_;
    values_["InsCoDomicile"          ] = &product_data_.InsCoDomicile          .datum_;
    values_["MainUnderwriter"        ] = &product_data_.MainUnderwriter        .datum_;
    values_["MainUnderwriterAddress" ] = &product_data_.MainUnderwriterAddress .datum_;
    values_["CoUnderwriter"          ] = &product_data_.CoUnderwriter          .datum_;
    values_["CoUnderwriterAddress"   ] = &product_data_.CoUnderwriterAddress   .datum_;
    values_["AvName"                 ] = &product_data_.AvName                 .datum_;
    values_["CsvName"                ] = &product_data_.CsvName                .datum_;
    values_["CsvHeaderName"          ] = &product_data_.CsvHeaderName          .datum_;
    values_["NoLapseProvisionName"   ] = &product_data_.NoLapseProvisionName   .datum_;
    values_["InterestDisclaimer"     ] = &product_data_.InterestDisclaimer     .datum_;
    values_["GuarMortalityFootnote"  ] = &product_data_.GuarMortalityFootnote  .datum_;
}

PolicyDocument::~PolicyDocument()
{
}

PolicyView& PolicyDocument::PredominantView() const
{
    return ::PredominantView<PolicyView>(*this);
}

void PolicyDocument::ReadDocument(std::string const& filename)
{
    product_data_.load(filename);
    if(!GetViews().empty())
        {
        PolicyView& view = PredominantView();
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

void PolicyDocument::WriteDocument(std::string const& filename)
{
    if(!GetViews().empty())
        {
        PolicyView& view = PredominantView();
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
    product_data_.save(filename);
}

