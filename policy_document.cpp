// Document class for product data.
//
// Copyright (C) 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: policy_document.cpp,v 1.10 2008-12-27 02:56:52 chicares Exp $

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
    values_["DatabaseFilename"       ] = &product_data_.DatabaseFilename;
    values_["FundFilename"           ] = &product_data_.FundFilename;
    values_["CorridorFilename"       ] = &product_data_.CorridorFilename;
    values_["CurrCOIFilename"        ] = &product_data_.CurrCOIFilename;
    values_["GuarCOIFilename"        ] = &product_data_.GuarCOIFilename;
    values_["WPFilename"             ] = &product_data_.WPFilename;
    values_["ADDFilename"            ] = &product_data_.ADDFilename;
    values_["ChildRiderFilename"     ] = &product_data_.ChildRiderFilename;
    values_["CurrSpouseRiderFilename"] = &product_data_.CurrSpouseRiderFilename;
    values_["GuarSpouseRiderFilename"] = &product_data_.GuarSpouseRiderFilename;
    values_["CurrTermFilename"       ] = &product_data_.CurrTermFilename;
    values_["GuarTermFilename"       ] = &product_data_.GuarTermFilename;
    values_["TableYFilename"         ] = &product_data_.TableYFilename;
    values_["PremTaxFilename"        ] = &product_data_.PremTaxFilename;
    values_["TAMRA7PayFilename"      ] = &product_data_.TAMRA7PayFilename;
    values_["TgtPremFilename"        ] = &product_data_.TgtPremFilename;
    values_["IRC7702Filename"        ] = &product_data_.IRC7702Filename;
    values_["Gam83Filename"          ] = &product_data_.Gam83Filename;
    values_["SubstdTblMultFilename"  ] = &product_data_.SubstdTblMultFilename;
    values_["CurrSpecAmtLoadFilename"] = &product_data_.CurrSpecAmtLoadFilename;
    values_["GuarSpecAmtLoadFilename"] = &product_data_.GuarSpecAmtLoadFilename;
    values_["RoundingFilename"       ] = &product_data_.RoundingFilename;
    values_["TierFilename"           ] = &product_data_.TierFilename;
    values_["PolicyForm"             ] = &product_data_.PolicyForm;
    values_["PolicyMktgName"         ] = &product_data_.PolicyMktgName;
    values_["PolicyLegalName"        ] = &product_data_.PolicyLegalName;
    values_["InsCoShortName"         ] = &product_data_.InsCoShortName;
    values_["InsCoName"              ] = &product_data_.InsCoName;
    values_["InsCoAddr"              ] = &product_data_.InsCoAddr;
    values_["InsCoStreet"            ] = &product_data_.InsCoStreet;
    values_["InsCoPhone"             ] = &product_data_.InsCoPhone;
    values_["InsCoDomicile"          ] = &product_data_.InsCoDomicile;
    values_["MainUnderwriter"        ] = &product_data_.MainUnderwriter;
    values_["MainUnderwriterAddress" ] = &product_data_.MainUnderwriterAddress;
    values_["CoUnderwriter"          ] = &product_data_.CoUnderwriter;
    values_["CoUnderwriterAddress"   ] = &product_data_.CoUnderwriterAddress;
    values_["AvName"                 ] = &product_data_.AvName;
    values_["CsvName"                ] = &product_data_.CsvName;
    values_["CsvHeaderName"          ] = &product_data_.CsvHeaderName;
    values_["NoLapseProvisionName"   ] = &product_data_.NoLapseProvisionName;
    values_["InterestDisclaimer"     ] = &product_data_.InterestDisclaimer;
    values_["GuarMortalityFootnote"  ] = &product_data_.GuarMortalityFootnote;
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
    product_data_.Read(filename);
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
    product_data_.Write(filename);
}

