// Life insurance illustration inputs: xml I/O.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: inputillus_xml_io.cpp,v 1.12.2.8 2006-10-18 02:03:05 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

// Entities that were present in older versions and then removed
// are recognized and ignored. If they're resurrected in a later
// version, then they aren't ignored.

namespace
{
std::vector<std::string> const& detritus()
{
    static std::string const s[] =
        {"AgentFirstName"                // Single name instead.
        ,"AgentLastName"                 // Single name instead.
        ,"AgentMiddleName"               // Single name instead.
        ,"AssumedCaseNumberOfLives"      // Withdrawn.
        ,"CaseAssumedAssets"             // Withdrawn.
        ,"CorporationPremiumTableFactor" // Never implemented.
        ,"CorporationPremiumTableNumber" // Never implemented.
        ,"CorporationTaxBracket"         // Never implemented.
        ,"CorporationTaxpayerId"         // Would violate privacy.
        ,"CurrentCoiGrading"             // Withdrawn.
        ,"InforceDcvDeathBenefit"        // Misbegotten.
        ,"InsuredPremiumTableNumber"     // Never implemented.
        ,"FirstName"                     // Single name instead.
        ,"LastName"                      // Single name instead.
        ,"MiddleName"                    // Single name instead.
        ,"PartialMortalityTable"         // Never implemented.
        ,"PayLoanInterestInCash"         // Never implemented.
        ,"PolicyLevelFlatExtra"          // Never implemented; poor name.
        ,"SocialSecurityNumber"          // Withdrawn: would violate privacy.
        ,"TaxBracket"                    // Never implemented.
        ,"YearsOfZeroDeaths"             // Withdrawn.
        };
    static std::vector<std::string> const v(s, s + lmi_array_size(s));
    return v;
}
} // Unnnamed namespace.

//============================================================================
void IllusInputParms::read(xml_lmi::Element const& x)
{
    if(xml_root_name() != x.get_name())
        {
        std::ostringstream msg;
        msg
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            ;
        throw std::runtime_error(msg.str());
        }

    xml_lmi::Attribute const* cell_version_node = x.get_attribute("version");
    if(!cell_version_node)
        {
        std::ostringstream msg;
        msg
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            ;
        throw std::runtime_error(msg.str());
        }
    int cell_version = value_cast<int>(cell_version_node->get_value());

    std::map<std::string, std::string> detritus_map;

    std::vector<std::string> member_names
        (IllusInputParms::member_names()
        );
    std::vector<std::string>::iterator current_member;
    xml_lmi::NodeContainer const c = x.get_children();
    for
        (xml_lmi::NodeContainer::const_iterator iter = c.begin()
        ;iter != c.end()
        ;++iter
        )
        {
        xml_lmi::Element const* child = dynamic_cast<xml_lmi::Element const*>(*iter);
        if(!child) // child is a text node
            {
            continue;
            }
        std::string node_tag(child->get_name());
        current_member = std::find
            (member_names.begin()
            ,member_names.end()
            ,node_tag
            );
        if(member_names.end() == current_member)
            {
            // Hold certain obsolete entities that must be translated.
            if
                (detritus().end() != std::find
                    (detritus().begin()
                    ,detritus().end()
                    ,node_tag
                    )
                )
                {
                detritus_map[node_tag] = xml_lmi::get_content(*child);
                }
            else
                {
                warning()
                    << "XML tag '"
                    << node_tag
                    << "' not recognized by this version of the program."
                    << LMI_FLUSH
                    ;
                }
            continue;
            }
        (*this)[node_tag] = xml_lmi::get_content(*child);
        // TODO ?? Perhaps a std::list would perform better.
        member_names.erase(current_member);
        }

    if(0 == cell_version)
        {
        // An older version with no distinct 'cell_version' didn't
        // have 'DefnMaterialChange', whose default value is
        // unacceptable for GPT.
        if
            (member_names.end() != std::find
                (member_names.begin()
                ,member_names.end()
                ,std::string("DefinitionOfMaterialChange")
                )
            )
            {
            if(DefnLifeIns == e_defn_life_ins(e_gpt))
                {
                DefnMaterialChange = e_adjustment_event;
                }
            else
                {
                DefnMaterialChange = e_earlier_of_increase_or_unnecessary_premium;
                }
            }

        InputParms::AgentFirstName  = detritus_map["AgentFirstName" ];
        InputParms::AgentLastName   = detritus_map["AgentLastName"  ];
        InputParms::AgentMiddleName = detritus_map["AgentMiddleName"];
        InputParms::InsdFirstName   = detritus_map["FirstName"      ];
        InputParms::InsdLastName    = detritus_map["LastName"       ];
        InputParms::InsdMiddleName  = detritus_map["MiddleName"     ];

        operator[]("AgentName"  ) = AgentFullName();
        operator[]("InsuredName") = InsdFullName();

        InputParms::AgentLastName   = "";
        InputParms::AgentMiddleName = "";
        InputParms::InsdLastName    = "";
        InputParms::InsdMiddleName  = "";
        }

    propagate_changes_to_base_and_finalize();

    if(EffectiveDateToday && !global_settings::instance().regression_testing())
        {
        EffDate = calendar_date();
        }

// If you want to see the ones that didn't get assigned:
//std::ostringstream oss;
//std::ostream_iterator<std::string> osi(oss, "\r\n");
//std::copy(member_names.begin(), member_names.end(), osi);
//warning() << oss.str() << "Parameters absent in xml file" << LMI_FLUSH;
}

//============================================================================
void IllusInputParms::write(xml_lmi::Element& x) const
{
    xml_lmi::Element& root = *x.add_child(xml_root_name());

    root.set_attribute("version", value_cast<std::string>(class_version()).c_str());

    std::vector<std::string> const member_names
        (IllusInputParms::member_names()
        );
    std::vector<std::string>::const_iterator i;
    for(i = member_names.begin(); i != member_names.end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = operator[](*i).str();
        root.add_child(node_tag)->add_child_text(value);
        }
}

//============================================================================
int IllusInputParms::class_version() const
{
    return 1;
}

//============================================================================
std::string IllusInputParms::xml_root_name() const
{
    return "cell";
}

