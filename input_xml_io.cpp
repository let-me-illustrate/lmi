// MVC Model for life-insurance illustrations: xml I/O.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: input_xml_io.cpp,v 1.8 2008-08-14 12:11:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <algorithm> // std::find()

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
        ,"CorporationPremiumTableNumber" // Never implemented.
        ,"CorporationTaxpayerId"         // Would violate privacy.
        ,"CurrentCoiGrading"             // Withdrawn.
        ,"FirstName"                     // Single name instead.
        ,"InforceDcvDeathBenefit"        // Misbegotten.
        ,"InforceExperienceReserve"      // Renamed before implementation.
        ,"InsuredPremiumTableNumber"     // Never implemented.
        ,"LastName"                      // Single name instead.
        ,"MiddleName"                    // Single name instead.
        ,"NetMortalityChargeHistory"     // Renamed before implementation.
        ,"PartialMortalityTable"         // Never implemented.
        ,"PayLoanInterestInCash"         // Never implemented.
        ,"PolicyLevelFlatExtra"          // Never implemented; poor name.
        ,"SocialSecurityNumber"          // Withdrawn: would violate privacy.
        ,"TermProportion"                // 'TermRiderProportion' instead.
        ,"YearsOfZeroDeaths"             // Withdrawn.
        };
    static std::vector<std::string> const v(s, s + lmi_array_size(s));
    return v;
}

std::string full_name
    (std::string first_name
    ,std::string middle_name
    ,std::string last_name
    )
{
    std::string s(first_name);
    if(!s.empty() && !middle_name.empty())
        {
        s += " ";
        }
    s += middle_name;
    if(!s.empty() && !last_name.empty())
        {
        s += " ";
        }
    s += last_name;
    return s;
}
} // Unnnamed namespace.

//============================================================================
void Input::read(xml::element const& x)
{
    if(xml_root_name() != x.get_name())
        {
        fatal_error()
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    std::string cell_version_string;
    if(!xml_lmi::get_attr(x, "version", cell_version_string))
        {
        fatal_error()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    int cell_version = value_cast<int>(cell_version_string);

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::map<std::string, std::string> detritus_map;

    std::vector<std::string> member_names
        (Input::member_names()
        );
    std::vector<std::string>::iterator current_member;
// XMLWRAPP !! The unit test demonstrates that the suppressed code is
// twenty-five percent slower. What would be really desirable is an
// (efficient) element-iterator class.
//
#if 0
    xml_lmi::ElementContainer const elements(xml_lmi::child_elements(x));
    typedef xml_lmi::ElementContainer::const_iterator eci;
    for(eci i = elements.begin(); i != elements.end(); ++i)
        {
        xml::node::const_iterator const& child = *i;
#else  // not 0
    xml::node::const_iterator child;
    for(child = x.begin(); child != x.end(); ++child)
        {
#endif // not 0
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
        if(member_names.end() != current_member)
            {
            operator[](node_tag) = xml_lmi::get_content(*child);
            // TODO ?? Perhaps a std::list would perform better.
            member_names.erase(current_member);
            }
        else if
            (detritus().end() != std::find
                (detritus().begin()
                ,detritus().end()
                ,node_tag
                )
            )
            {
            // Hold certain obsolete entities that must be translated.
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
        }

    if(0 == cell_version)
        {
        // An older version with no distinct 'cell_version' didn't
        // have 'DefinitionOfMaterialChange', whose default value is
        // unacceptable for GPT.
        if
            (member_names.end() != std::find
                (member_names.begin()
                ,member_names.end()
                ,std::string("DefinitionOfMaterialChange")
                )
            )
            {
            if(mce_gpt == DefinitionOfLifeInsurance)
                {
                DefinitionOfMaterialChange = mce_adjustment_event;
                }
            else
                {
                DefinitionOfMaterialChange = mce_earlier_of_increase_or_unnecessary_premium;
                }
            }

        std::string AgentFirstName  = detritus_map["AgentFirstName" ];
        std::string AgentMiddleName = detritus_map["AgentMiddleName"];
        std::string AgentLastName   = detritus_map["AgentLastName"  ];
        std::string InsdFirstName   = detritus_map["FirstName"      ];
        std::string InsdMiddleName  = detritus_map["MiddleName"     ];
        std::string InsdLastName    = detritus_map["LastName"       ];

        operator[]("AgentName"  ) = full_name
            (AgentFirstName
            ,AgentMiddleName
            ,AgentLastName
            );
        operator[]("InsuredName") = full_name
            (InsdFirstName
            ,InsdMiddleName
            ,InsdLastName
            );
        }

    // Older versions lacked 'UseCurrentDeclaredRate', whose
    // default value of "Yes" would break backward compatibility.
    if
        (member_names.end() != std::find
            (member_names.begin()
            ,member_names.end()
            ,std::string("UseCurrentDeclaredRate")
            )
        )
        {
        UseCurrentDeclaredRate = "No";
        }

    if(EffectiveDateToday.value() && !global_settings::instance().regression_testing())
        {
        EffectiveDate = calendar_date();
        }

    // 'LastCoiReentryDate' was introduced 20071017T1454Z. For files
    // saved before then, its default value may be inappropriate.
    LastCoiReentryDate = std::min
        (LastCoiReentryDate.value()
        ,add_years(EffectiveDate.value(), InforceYear.value(), true)
        );

    if(1 == cell_version)
        {
        // Solve 'Year' values were saved in solve 'Time' entities,
        // in this version only.
        SolveTargetYear = SolveTargetTime.value();
        SolveBeginYear  = SolveBeginTime .value();
        SolveEndYear    = SolveEndTime   .value();

        SolveTargetTime = issue_age() + SolveTargetYear.value();
        SolveBeginTime  = issue_age() + SolveBeginYear .value();
        SolveEndTime    = issue_age() + SolveEndYear   .value();
        }

// If you want to see the ones that didn't get assigned:
//std::ostringstream oss;
//std::ostream_iterator<std::string> osi(oss, "\r\n");
//std::copy(member_names.begin(), member_names.end(), osi);
//warning() << oss.str() << "Parameters absent in xml file" << LMI_FLUSH;
}

//============================================================================
void Input::write(xml::element& x) const
{
    xml::element root(xml_root_name().c_str());

// XMLWRAPP !! There's no way to set an integer attribute.
    std::string const version(value_cast<std::string>(class_version()));
    xml_lmi::set_attr(root, "version", version.c_str());

    std::vector<std::string> const member_names
        (Input::member_names()
        );
    std::vector<std::string>::const_iterator i;
    for(i = member_names.begin(); i != member_names.end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = operator[](*i).str();
        root.push_back(xml::element(node_tag.c_str(), value.c_str()));
        }

    x.push_back(root);
}

//============================================================================
int Input::class_version() const
{
    return 2;
}

//============================================================================
std::string Input::xml_root_name() const
{
    return "cell";
}

