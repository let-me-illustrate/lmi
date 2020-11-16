// Document class for product data.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "policy_document.hpp"

#include "policy_view.hpp"
#include "view_ex.tpp"

// EVGENIY !! Doesn't it seem strange that class wxTextCtrl,
// a GUI class, is required in a document implementation?

#include <wx/textctrl.h>

IMPLEMENT_DYNAMIC_CLASS(PolicyDocument, ProductEditorDocument)

PolicyDocument::PolicyDocument()
    :ProductEditorDocument {}
    ,product_data_         {}
    ,values_               {}
{
    values_["DatabaseFilename"        ] = &product_data_.DatabaseFilename        .datum_;
    values_["FundFilename"            ] = &product_data_.FundFilename            .datum_;
    values_["LingoFilename"           ] = &product_data_.LingoFilename           .datum_;
    values_["CvatCorridorFilename"    ] = &product_data_.CvatCorridorFilename    .datum_;
    values_["Irc7702NspFilename"      ] = &product_data_.Irc7702NspFilename      .datum_;
    values_["CurrCOIFilename"         ] = &product_data_.CurrCOIFilename         .datum_;
    values_["GuarCOIFilename"         ] = &product_data_.GuarCOIFilename         .datum_;
    values_["WPFilename"              ] = &product_data_.WPFilename              .datum_;
    values_["ADDFilename"             ] = &product_data_.ADDFilename             .datum_;
    values_["ChildRiderFilename"      ] = &product_data_.ChildRiderFilename      .datum_;
    values_["CurrSpouseRiderFilename" ] = &product_data_.CurrSpouseRiderFilename .datum_;
    values_["GuarSpouseRiderFilename" ] = &product_data_.GuarSpouseRiderFilename .datum_;
    values_["CurrTermFilename"        ] = &product_data_.CurrTermFilename        .datum_;
    values_["GuarTermFilename"        ] = &product_data_.GuarTermFilename        .datum_;
    values_["GroupProxyFilename"      ] = &product_data_.GroupProxyFilename      .datum_;
    values_["SevenPayFilename"        ] = &product_data_.SevenPayFilename        .datum_;
    values_["TgtPremFilename"         ] = &product_data_.TgtPremFilename         .datum_;
    values_["Irc7702QFilename"        ] = &product_data_.Irc7702QFilename        .datum_;
    values_["PartialMortalityFilename"] = &product_data_.PartialMortalityFilename.datum_;
    values_["SubstdTblMultFilename"   ] = &product_data_.SubstdTblMultFilename   .datum_;
    values_["CurrSpecAmtLoadFilename" ] = &product_data_.CurrSpecAmtLoadFilename .datum_;
    values_["GuarSpecAmtLoadFilename" ] = &product_data_.GuarSpecAmtLoadFilename .datum_;
    values_["RoundingFilename"        ] = &product_data_.RoundingFilename        .datum_;
    values_["TierFilename"            ] = &product_data_.TierFilename            .datum_;
    values_["PolicyForm"              ] = &product_data_.PolicyForm              .datum_; // LINGO !! expunge
    values_["PolicyFormAlternative"   ] = &product_data_.PolicyFormAlternative   .datum_; // LINGO !! expunge
    values_["PolicyMktgName"          ] = &product_data_.PolicyMktgName          .datum_;
    values_["PolicyLegalName"         ] = &product_data_.PolicyLegalName         .datum_;
    values_["InsCoShortName"          ] = &product_data_.InsCoShortName          .datum_;
    values_["InsCoName"               ] = &product_data_.InsCoName               .datum_;
    values_["InsCoAddr"               ] = &product_data_.InsCoAddr               .datum_;
    values_["InsCoStreet"             ] = &product_data_.InsCoStreet             .datum_;
    values_["InsCoPhone"              ] = &product_data_.InsCoPhone              .datum_;
    values_["InsCoDomicile"           ] = &product_data_.InsCoDomicile           .datum_;
    values_["MainUnderwriter"         ] = &product_data_.MainUnderwriter         .datum_;
    values_["MainUnderwriterAddress"  ] = &product_data_.MainUnderwriterAddress  .datum_;
    values_["CoUnderwriter"           ] = &product_data_.CoUnderwriter           .datum_;
    values_["CoUnderwriterAddress"    ] = &product_data_.CoUnderwriterAddress    .datum_;
    values_["AvName"                  ] = &product_data_.AvName                  .datum_;
    values_["CsvName"                 ] = &product_data_.CsvName                 .datum_;
    values_["CsvHeaderName"           ] = &product_data_.CsvHeaderName           .datum_;
    values_["NoLapseProvisionName"    ] = &product_data_.NoLapseProvisionName    .datum_;
    values_["InterestDisclaimer"      ] = &product_data_.InterestDisclaimer      .datum_;
    values_["GuarMortalityFootnote"   ] = &product_data_.GuarMortalityFootnote   .datum_;
}

PolicyView& PolicyDocument::PredominantView() const
{
    return ::PredominantView<PolicyView>(*this);
}

void PolicyDocument::ReadDocument(std::string const& filename)
{
    load(product_data_, filename);
    if(!GetViews().empty())
        {
        PolicyView& view = PredominantView();
        for(auto const& i : values_)
            {
            view.controls()[i.first]->SetValue(*i.second);
            }
        }
}

void PolicyDocument::WriteDocument(std::string const& filename)
{
    if(!GetViews().empty())
        {
        PolicyView& view = PredominantView();
        for(auto const& i : values_)
            {
            *i.second = view.controls()[i.first]->GetValue().ToStdString(wxConvUTF8);
            }
        }
    save(product_data_, filename);
}
