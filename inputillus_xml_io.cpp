// Life insurance illustration inputs: xml I/O.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: inputillus_xml_io.cpp,v 1.3 2005-05-24 04:00:02 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"

#include "alert.hpp"
#include "miscellany.hpp"
#include "value_cast.hpp"

#include <xmlwrapp/init.h>
#include <xmlwrapp/node.h>
#include <xmlwrapp/tree_parser.h>

#include <istream>
#include <ostream>
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
        ,"AssumedCaseNumberOfLives"      // Obsolete and incorrectly implemented.
        ,"CaseAssumedAssets"             // Obsolete and incorrectly implemented.
        ,"CorporationPremiumTableFactor" // Never implemented.
        ,"CorporationPremiumTableNumber" // Never implemented.
        ,"CorporationTaxBracket"         // Never implemented.
        ,"CorporationTaxpayerId"         // Violates privacy.
        ,"InforceDcvDeathBenefit"        // Misbegotten.
        ,"InsuredPremiumTableNumber"     // Never implemented.
        ,"FirstName"                     // Single name instead.
        ,"LastName"                      // Single name instead.
        ,"MiddleName"                    // Single name instead.
        ,"PartialMortalityTable"         // Never implemented.
        ,"PayLoanInterestInCash"         // Never implemented.
        ,"PolicyLevelFlatExtra"          // Never implemented; poor name.
        ,"SocialSecurityNumber"          // Violates privacy.
        ,"TaxBracket"                    // Never implemented.
        ,"YearsOfZeroDeaths"             // Obsolete; implementation untested.
        };
    static std::vector<std::string> const v(s, s + lmi_array_size(s));
    return v;
}
} // Unnnamed namespace.

//============================================================================
void IllusInputParms::read(xml::node& x)
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

    std::string cell_version_string;
    if(!x.get_attr("version", cell_version_string))
        {
        std::ostringstream msg;
        msg
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            ;
        throw std::runtime_error(msg.str());
        }
    int cell_version = value_cast<int>(cell_version_string);

    // "Use" this variable. Eventually we will. Until then we don't
    // care to see warnings that it's unused.
    (void)cell_version;

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::map<std::string, std::string> detritus_map;

    std::vector<std::string> member_names
        (IllusInputParms::member_names()
        );
    std::vector<std::string>::iterator current_member;
    xml::node::const_iterator child;
    for(child = x.begin(); child != x.end(); ++child)
        {
        if(child->is_text())
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
                char const* content = child->get_content();
                if(!content)
                    {
                    content = "";
                    }
                detritus_map[node_tag] = content;
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
        char const* content = child->get_content();
        if(!content)
            {
            content = "";
            }
        operator[](node_tag) = content;
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

// If you want to see the ones that didn't get assigned:
//std::ostringstream oss;
//std::ostream_iterator<std::string> osi(oss, "\r\n");
//std::copy(member_names.begin(), member_names.end(), osi);
//warning() << oss.str() << "Parameters absent in xml file" << LMI_FLUSH;
}

//============================================================================
void IllusInputParms::write(xml::node& x) const
{
    // TODO ?? Experimental.
    xml::init init;
    xml::node root(xml_root_name().c_str());
// XMLWRAPP !! There's no way to set an integer attribute; and function
// set_attr() seems to be missing in the doxygen stuff at pmade.org .
    root.set_attr("version", value_cast<std::string>(class_version()).c_str());

    std::vector<std::string> const member_names
        (IllusInputParms::member_names()
        );
    std::vector<std::string>::const_iterator i;
    for(i = member_names.begin(); i != member_names.end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = operator[](*i).str();
        root.push_back(xml::node(node_tag.c_str(), value.c_str()));
        }

    x.push_back(root);
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

