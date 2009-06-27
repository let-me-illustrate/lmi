// MVC Model for MEC testing.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

// $Id: mec_input.cpp,v 1.1 2009-06-27 17:47:39 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_input.hpp"

#include "alert.hpp"
#include "database.hpp"   // Needed only for database_'s dtor.
#include "global_settings.hpp"
#include "miscellany.hpp" // lmi_array_size()
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

namespace
{
/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool is_detritus(std::string const& s)
{
    static std::string const a[] =
        {"Remove this string when adding the first removed entity."
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return v.end() != std::find(v.begin(), v.end(), s);
}
} // Unnnamed namespace.

/// Values are initialized by UDT defaults where appropriate, and here
/// in the initializer-list otherwise. All "ascribed" data members are
/// listed here for clarity and maintainability, and commented out if
/// UDT defaults are presently appropriate.

mec_input::mec_input()
    :IssueAge                         ("45")
    ,Gender                           ("Male")
    ,Smoking                          ("Nonsmoker")
    ,UnderwritingClass                ("Standard")
//    ,DateOfBirth                      ("")
//    ,SubstandardTable                 ("")
//    ,ProductName                      ("")
//    ,External1035ExchangeAmount       ("")
//    ,External1035ExchangeFromMec      ("")
//    ,Internal1035ExchangeAmount       ("")
//    ,Internal1035ExchangeFromMec      ("")
//    ,EffectiveDate                    ("")
//    ,DefinitionOfLifeInsurance        ("")
    ,DefinitionOfMaterialChange       ("Earlier of increase or unnecessary premium")
//    ,GroupUnderwritingType            ("")
//    ,Comments                         ("")
//    ,InforceYear                      ("")
//    ,InforceMonth                     ("")
//    ,InforceAccountValue              ("")
//    ,InforceAsOfDate                  ("")
//    ,InforceSevenPayPremium           ("")
//    ,InforceIsMec                     ("")
//    ,LastMaterialChangeDate           ("")
//    ,InforceDcv                       ("")
//    ,InforceAvBeforeLastMc            ("")
//    ,InforceContractYear              ("")
//    ,InforceContractMonth             ("")
//    ,InforceLeastDeathBenefit         ("")
    ,StateOfJurisdiction              ("CT")
    ,FlatExtra                        ("0")
    ,PaymentHistory                   ("0")
    ,BenefitHistory                   ("0")
//    ,DeprecatedUseDOB                 ("")
//    ,EffectiveDateToday               ("")
    ,Payment                          ("0")
    ,BenefitAmount                    ("0")
{
    AscribeMembers();
    DoAdaptExternalities(); // Initialize database, e.g.
    DoTransmogrify();       // Make DOB and age consistent, e.g.
}

mec_input::mec_input(mec_input const& z)
    :obstruct_slicing<mec_input>()
    ,streamable()
    ,MvcModel()
    ,MemberSymbolTable<mec_input>()
{
    AscribeMembers();
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        // This would be wrong:
        //   operator[](*i) = z[*i];
        // because it would swap in a copy of z's *members*.
        operator[](*i) = z[*i].str();
        }
    DoAdaptExternalities();
}

mec_input::~mec_input()
{
}

mec_input& mec_input::operator=(mec_input const& z)
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = z[*i].str();
        }
    DoAdaptExternalities();
    return *this;
}

bool mec_input::operator==(mec_input const& z) const
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const s0 = operator[](*i).str();
        std::string const s1 = z[*i].str();
        if(s0 != s1)
            {
            return false;
            }
        }
    return true;
}

void mec_input::AscribeMembers()
{
    ascribe("IssueAge"                              , &mec_input::IssueAge                              );
    ascribe("Gender"                                , &mec_input::Gender                                );
    ascribe("Smoking"                               , &mec_input::Smoking                               );
    ascribe("UnderwritingClass"                     , &mec_input::UnderwritingClass                     );
    ascribe("DateOfBirth"                           , &mec_input::DateOfBirth                           );
    ascribe("SubstandardTable"                      , &mec_input::SubstandardTable                      );
    ascribe("ProductName"                           , &mec_input::ProductName                           );
    ascribe("External1035ExchangeAmount"            , &mec_input::External1035ExchangeAmount            );
    ascribe("External1035ExchangeFromMec"           , &mec_input::External1035ExchangeFromMec           );
    ascribe("Internal1035ExchangeAmount"            , &mec_input::Internal1035ExchangeAmount            );
    ascribe("Internal1035ExchangeFromMec"           , &mec_input::Internal1035ExchangeFromMec           );
    ascribe("EffectiveDate"                         , &mec_input::EffectiveDate                         );
    ascribe("DefinitionOfLifeInsurance"             , &mec_input::DefinitionOfLifeInsurance             );
    ascribe("DefinitionOfMaterialChange"            , &mec_input::DefinitionOfMaterialChange            );
    ascribe("GroupUnderwritingType"                 , &mec_input::GroupUnderwritingType                 );
    ascribe("Comments"                              , &mec_input::Comments                              );
    ascribe("InforceYear"                           , &mec_input::InforceYear                           );
    ascribe("InforceMonth"                          , &mec_input::InforceMonth                          );
    ascribe("InforceAccountValue"                   , &mec_input::InforceAccountValue                   );
    ascribe("InforceAsOfDate"                       , &mec_input::InforceAsOfDate                       );
    ascribe("InforceSevenPayPremium"                , &mec_input::InforceSevenPayPremium                );
    ascribe("InforceIsMec"                          , &mec_input::InforceIsMec                          );
    ascribe("LastMaterialChangeDate"                , &mec_input::LastMaterialChangeDate                );
    ascribe("InforceDcv"                            , &mec_input::InforceDcv                            );
    ascribe("InforceAvBeforeLastMc"                 , &mec_input::InforceAvBeforeLastMc                 );
    ascribe("InforceContractYear"                   , &mec_input::InforceContractYear                   );
    ascribe("InforceContractMonth"                  , &mec_input::InforceContractMonth                  );
    ascribe("InforceLeastDeathBenefit"              , &mec_input::InforceLeastDeathBenefit              );
    ascribe("StateOfJurisdiction"                   , &mec_input::StateOfJurisdiction                   );
    ascribe("FlatExtra"                             , &mec_input::FlatExtra                             );
    ascribe("PaymentHistory"                        , &mec_input::PaymentHistory                        );
    ascribe("BenefitHistory"                        , &mec_input::BenefitHistory                        );
    ascribe("DeprecatedUseDOB"                      , &mec_input::DeprecatedUseDOB                      );
    ascribe("EffectiveDateToday"                    , &mec_input::EffectiveDateToday                    );
    ascribe("Payment"                               , &mec_input::Payment                               );
    ascribe("BenefitAmount"                         , &mec_input::BenefitAmount                         );
}

void mec_input::DoAdaptExternalities()
{
}

datum_base const* mec_input::DoBaseDatumPointer
    (std::string const& name
    ) const
{
    return member_cast<datum_base>(operator[](name));
}

any_entity& mec_input::DoEntity(std::string const& name)
{
    return MemberSymbolTable<mec_input>::operator[](name);
}

any_entity const& mec_input::DoEntity(std::string const& name) const
{
    return MemberSymbolTable<mec_input>::operator[](name);
}

MvcModel::NamesType const& mec_input::DoNames() const
{
    return member_names();
}

MvcModel::StateType mec_input::DoState() const
{
    return member_state(*this);
}

void mec_input::DoCustomizeInitialValues()
{
}

void mec_input::DoEnforceCircumscription(std::string const& name)
{
    datum_base* base_datum = member_cast<datum_base>(operator[](name));
    tn_range_base* datum = dynamic_cast<tn_range_base*>(base_datum);
    if(datum)
        {
        datum->enforce_circumscription();
        }
}

void mec_input::DoEnforceProscription(std::string const& name)
{
    // Here one could handle special cases for which the generic
    // behavior is not wanted.

    datum_base* base_datum = member_cast<datum_base>(operator[](name));
    mc_enum_base* datum = dynamic_cast<mc_enum_base*>(base_datum);
    if(datum)
        {
        datum->enforce_proscription();
        }
}

void mec_input::DoHarmonize()
{
}

void mec_input::DoTransmogrify()
{
}

std::vector<std::string> mec_input::RealizeAllSequenceInput(bool /* report_errors */)
{
    return std::vector<std::string>();
}

void mec_input::read(xml::element const& x)
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

    std::string file_version_string;
    if(!xml_lmi::get_attr(x, "version", file_version_string))
        {
        fatal_error()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    int file_version = value_cast<int>(file_version_string);

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::map<std::string, std::string> detritus_map;

    std::list<std::string> residuary_names;
    std::copy
        (member_names().begin()
        ,member_names().end()
        ,std::back_inserter(residuary_names)
        );
    std::list<std::string>::iterator current_member;

    xml::const_nodes_view const elements(x.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi child = elements.begin(); child != elements.end(); ++child)
        {
        std::string node_tag(child->get_name());
        current_member = std::find
            (residuary_names.begin()
            ,residuary_names.end()
            ,node_tag
            );
        if(residuary_names.end() != current_member)
            {
            operator[](node_tag) = RedintegrateExAnte
                (file_version
                ,node_tag
                ,xml_lmi::get_content(*child)
                );
            residuary_names.erase(current_member);
            }
        else if(is_detritus(node_tag))
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

    RedintegrateExPost(file_version, detritus_map, residuary_names);

    if(EffectiveDateToday.value() && !global_settings::instance().regression_testing())
        {
        EffectiveDate = calendar_date();
        }
}

void mec_input::write(xml::element& x) const
{
    xml::element root(xml_root_name().c_str());

// XMLWRAPP !! There's no way to set an integer attribute.
    std::string const version(value_cast<std::string>(class_version()));
    xml_lmi::set_attr(root, "version", version.c_str());

    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = operator[](*i).str();
        root.push_back(xml::element(node_tag.c_str(), value.c_str()));
        }

    x.push_back(root);
}

/// Serial number of this class's xml version.
///
/// version 0: 20099999T9999Z

int mec_input::class_version() const
{
    return 0;
}

std::string mec_input::xml_root_name() const
{
    return "mec";
}

/// Provide for backward compatibility before assigning values.

std::string mec_input::RedintegrateExAnte
    (int                file_version
    ,std::string const& // name
    ,std::string const& value
    ) const
{
    if(class_version() == file_version)
        {
        return value;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        return value; // Stifle compiler warning.
        }
}

/// Provide for backward compatibility after assigning values.

void mec_input::RedintegrateExPost
    (int                                file_version
    ,std::map<std::string, std::string> // detritus_map
    ,std::list<std::string>             // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        }
}

