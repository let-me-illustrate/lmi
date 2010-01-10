// MVC Model for MEC testing.
//
// Copyright (C) 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_input.hpp"

#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_seq_helpers.hpp"
#include "miscellany.hpp" // lmi_array_size()
#include "xml_lmi.hpp"

#include <xmlwrapp/nodes_view.h>

#include <algorithm>
#include <limits>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{
/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool is_detritus(std::string const& s)
{
    static std::string const a[] =
        {"EffectiveDateToday"
        ,"InforceSevenPayPremium"
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return v.end() != std::find(v.begin(), v.end(), s);
}

template<typename T>
std::string realize_sequence_string
    (mec_input           & input
    ,std::vector<T>      & v
    ,datum_sequence const& sequence_string
    ,int                   index_origin = 0
    )
{
    InputSequence s
        (sequence_string.value()
        ,input.years_to_maturity()
        ,input.issue_age        ()
        ,input.maturity_age     () // This class has no "retirement age".
        ,input.inforce_year     ()
        ,input.effective_year   ()
        ,index_origin
        );
    detail::convert_vector(v, s.linear_number_representation());
    return s.formatted_diagnostics(true);
}
} // Unnamed namespace.

/// Values are initialized by UDT defaults where appropriate, and here
/// in the initializer-list otherwise. All "ascribed" data members are
/// listed here for clarity and maintainability, and commented out if
/// UDT defaults are presently appropriate.

mec_input::mec_input()
    :Use7702ATables                   ("No")
    ,IssueAge                         ("45")
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
    ,DefinitionOfMaterialChange       ("Unnecessary premium")
//    ,GroupUnderwritingType            ("")
//    ,Comments                         ("")
//    ,InforceYear                      ("")
//    ,InforceMonth                     ("")
    ,InforceTargetSpecifiedAmount     ("1000000")
//    ,InforceAccountValue              ("")
//    ,InforceAsOfDate                  ("")
//    ,InforceIsMec                     ("")
//    ,LastMaterialChangeDate           ("")
//    ,InforceDcv                       ("")
//    ,InforceAvBeforeLastMc            ("")
//    ,InforceContractYear              ("")
//    ,InforceContractMonth             ("")
    ,InforceLeastDeathBenefit         ("1000000")
    ,StateOfJurisdiction              ("CT")
    ,FlatExtra                        ("0")
    ,PaymentHistory                   ("0")
    ,BenefitHistory                   ("1000000")
//    ,DeprecatedUseDOB                 ("")
    ,Payment                          ("0")
    ,BenefitAmount                    ("1000000")
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
    ascribe("Use7702ATables"                        , &mec_input::Use7702ATables                        );
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
    ascribe("InforceTargetSpecifiedAmount"          , &mec_input::InforceTargetSpecifiedAmount          );
    ascribe("InforceAccountValue"                   , &mec_input::InforceAccountValue                   );
    ascribe("InforceAsOfDate"                       , &mec_input::InforceAsOfDate                       );
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
    ascribe("Payment"                               , &mec_input::Payment                               );
    ascribe("BenefitAmount"                         , &mec_input::BenefitAmount                         );
}

/// Reset database_ if necessary, i.e., if the product or any database
/// axis changed.

void mec_input::DoAdaptExternalities()
{
    // This early-exit condition has to fail the first time this
    // function is called, because database_ is initialized only here.
    if
        (
            database_.get()
        &&  CachedProductName_           == ProductName
        &&  CachedGender_                == Gender
        &&  CachedUnderwritingClass_     == UnderwritingClass
        &&  CachedSmoking_               == Smoking
        &&  CachedIssueAge_              == IssueAge
        &&  CachedGroupUnderwritingType_ == GroupUnderwritingType
        &&  CachedStateOfJurisdiction_   == StateOfJurisdiction
        )
        {
        return;
        }

    CachedProductName_           = ProductName          .value();
    CachedGender_                = Gender               .value();
    CachedUnderwritingClass_     = UnderwritingClass    .value();
    CachedSmoking_               = Smoking              .value();
    CachedIssueAge_              = IssueAge             .value();
    CachedGroupUnderwritingType_ = GroupUnderwritingType.value();
    CachedStateOfJurisdiction_   = StateOfJurisdiction  .value();

    database_.reset
        (new TDatabase
            (CachedProductName_
            ,CachedGender_
            ,CachedUnderwritingClass_
            ,CachedSmoking_
            ,CachedIssueAge_
            ,CachedGroupUnderwritingType_
            ,CachedStateOfJurisdiction_
            )
        );

    GleanedMaturityAge_ = static_cast<int>(database_->Query(DB_EndtAge));
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

/// Cf. Input::DoHarmonize().

void mec_input::DoHarmonize()
{
    bool anything_goes    = global_settings::instance().ash_nazg();

    DefinitionOfLifeInsurance.allow(mce_gpt , database_->Query(DB_AllowGPT ));
    DefinitionOfLifeInsurance.allow(mce_cvat, database_->Query(DB_AllowCVAT));
    DefinitionOfLifeInsurance.allow(mce_noncompliant, false);

    DefinitionOfMaterialChange.enable(mce_noncompliant != DefinitionOfLifeInsurance);
    if(mce_noncompliant == DefinitionOfLifeInsurance)
        {
        // Nothing to do: all choices ignored because control is disabled.
        }
    else if(mce_cvat == DefinitionOfLifeInsurance)
        {
        DefinitionOfMaterialChange.allow(mce_unnecessary_premium                        ,true         );
        DefinitionOfMaterialChange.allow(mce_benefit_increase                           ,true         );
        DefinitionOfMaterialChange.allow(mce_later_of_increase_or_unnecessary_premium   ,anything_goes); // Not yet implemented.
        DefinitionOfMaterialChange.allow(mce_earlier_of_increase_or_unnecessary_premium ,true         );
        DefinitionOfMaterialChange.allow(mce_adjustment_event                           ,false        );
        }
    else if(mce_gpt == DefinitionOfLifeInsurance)
        {
        DefinitionOfMaterialChange.allow(mce_unnecessary_premium                        ,false        );
        DefinitionOfMaterialChange.allow(mce_benefit_increase                           ,false        );
        DefinitionOfMaterialChange.allow(mce_later_of_increase_or_unnecessary_premium   ,false        );
        DefinitionOfMaterialChange.allow(mce_earlier_of_increase_or_unnecessary_premium ,false        );
        DefinitionOfMaterialChange.allow(mce_adjustment_event                           ,true         );
        }
    else
        {
        fatal_error()
            << "No option selected for definition of life insurance."
            << LMI_FLUSH
            ;
        }

    // DATABASE !! There should be flags in the database to allow or
    // forbid paramedical and nonmedical underwriting; arbitrarily,
    // until they are added, those options are always inhibited.
    GroupUnderwritingType.allow(mce_medical, database_->Query(DB_AllowFullUW));
    GroupUnderwritingType.allow(mce_paramedical, false);
    GroupUnderwritingType.allow(mce_nonmedical, false);
    GroupUnderwritingType.allow(mce_simplified_issue, database_->Query(DB_AllowSimpUW));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database_->Query(DB_AllowGuarUW));

    IssueAge        .enable(mce_no  == DeprecatedUseDOB);
    DateOfBirth     .enable(mce_yes == DeprecatedUseDOB);

    // The ranges of both EffectiveDate and IssueAge are treated as
    // independent, to prevent one's value from affecting the other's
    // range and therefore possibly forcing its value to change. Thus,
    // if the maximum conceivable IssueAge is 100, then the earliest
    // permitted EffectiveDate is approximately the centennial of the
    // gregorian epoch.

#if 0
// Temporarily suppress this while exploring automatic-
// enforcement options in the skeleton trunk.
    IssueAge.minimum_and_maximum
        (static_cast<int>(database_->Query(DB_MinIssAge))
        ,static_cast<int>(database_->Query(DB_MaxIssAge))
        );
#endif // 0

    EffectiveDate.minimum
        (minimum_as_of_date
            (     IssueAge.trammel().maximum_maximorum()
            ,EffectiveDate.trammel().minimum_minimorum()
            )
        );

    bool const use_anb = database_->Query(DB_AgeLastOrNearest);
    DateOfBirth.minimum_and_maximum
        (minimum_birthdate(IssueAge.maximum(), EffectiveDate.value(), use_anb)
        ,maximum_birthdate(IssueAge.minimum(), EffectiveDate.value(), use_anb)
        );

    int max_age = static_cast<int>(database_->Query(DB_EndtAge));
    InforceAsOfDate.minimum_and_maximum
        (EffectiveDate.value()
        ,add_years_and_months
            (EffectiveDate.value()
            ,-1 + max_age - IssueAge.value()
            ,11
            ,true
            )
        );
    // SOMEDAY !! Here, it's important to use std::max(): otherwise,
    // when values change, the maximum could be less than the minimum,
    // because 'InforceAsOfDate' has not yet been constrained to the
    // limit just set. Should the MVC framework handle this somehow?
    LastMaterialChangeDate.minimum_and_maximum
        (EffectiveDate.value()
        ,std::max(InforceAsOfDate.value(), InforceAsOfDate.minimum())
        );

    double maximum_1035 =
          InforceAsOfDate == EffectiveDate
        ? std::numeric_limits<double>::max()
        : 0.0
        ;
    External1035ExchangeAmount.maximum(maximum_1035);
    Internal1035ExchangeAmount.maximum(maximum_1035);

    External1035ExchangeFromMec.allow (mce_yes, 0.0 != External1035ExchangeAmount);
    External1035ExchangeFromMec.enable(         0.0 != External1035ExchangeAmount);
    Internal1035ExchangeFromMec.allow (mce_yes, 0.0 != Internal1035ExchangeAmount);
    Internal1035ExchangeFromMec.enable(         0.0 != Internal1035ExchangeAmount);

    // SOMEDAY !! Do this in class Input as well.
    bool mec_due_to_1035 =
            mce_yes == External1035ExchangeFromMec
        ||  mce_yes == Internal1035ExchangeFromMec
        ;
    InforceIsMec.allow (mce_no, !mec_due_to_1035);
    InforceIsMec.enable(        !mec_due_to_1035);
    bool non_mec = mce_no == InforceIsMec;

    double maximum_7702A_csv_at_issue =
          InforceAsOfDate == EffectiveDate
        ? 0.0
        : std::numeric_limits<double>::max()
        ;
    InforceAccountValue.maximum(maximum_7702A_csv_at_issue);
    InforceDcv         .maximum(maximum_7702A_csv_at_issue);

    InforceTargetSpecifiedAmount.enable(non_mec);
    InforceAccountValue         .enable(non_mec);
    LastMaterialChangeDate      .enable(non_mec);
    InforceDcv                  .enable(non_mec && mce_cvat == DefinitionOfLifeInsurance);
    InforceAvBeforeLastMc       .enable(non_mec);
    InforceLeastDeathBenefit    .enable(non_mec);
    PaymentHistory              .enable(non_mec);
    BenefitHistory              .enable(non_mec);

    UnderwritingClass.allow(mce_ultrapreferred, database_->Query(DB_AllowUltraPrefClass));
    UnderwritingClass.allow(mce_preferred     , database_->Query(DB_AllowPreferredClass));
    UnderwritingClass.allow(mce_rated, database_->Query(DB_AllowSubstdTable));

    SubstandardTable.enable(mce_rated == UnderwritingClass);

    SubstandardTable.allow(mce_table_a, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_b, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_c, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_d, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_e, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_f, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_h, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_j, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_l, mce_rated == UnderwritingClass);
    SubstandardTable.allow(mce_table_p, mce_rated == UnderwritingClass);

    FlatExtra.enable(database_->Query(DB_AllowFlatExtras));

    bool blend_mortality_by_gender  = false;
    bool blend_mortality_by_smoking = false;

    bool allow_gender_distinct = database_->Query(DB_AllowSexDistinct);
    bool allow_unisex          = database_->Query(DB_AllowUnisex);

    Gender.allow(mce_female, !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_male  , !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_unisex,  blend_mortality_by_gender || allow_unisex);

    bool allow_smoker_distinct = database_->Query(DB_AllowSmokeDistinct);
    bool allow_unismoke        = database_->Query(DB_AllowUnismoke);

    Smoking.allow(mce_smoker,    !blend_mortality_by_smoking && allow_smoker_distinct);
    Smoking.allow(mce_nonsmoker, !blend_mortality_by_smoking && allow_smoker_distinct);
    Smoking.allow(mce_unismoke,   blend_mortality_by_smoking || allow_unismoke);
}

/// Change values as required for consistency.

void mec_input::DoTransmogrify()
{
    std::pair<int,int> ym0 = years_and_months_since
        (EffectiveDate  .value()
        ,InforceAsOfDate.value()
        );
    InforceYear  = ym0.first;
    InforceMonth = ym0.second;

    std::pair<int,int> ym1 = years_and_months_since
        (LastMaterialChangeDate.value()
        ,InforceAsOfDate       .value()
        );
    InforceContractYear  = ym1.first;
    InforceContractMonth = ym1.second;

    bool const use_anb = database_->Query(DB_AgeLastOrNearest);

    int apparent_age = attained_age
        (DateOfBirth.value()
        ,EffectiveDate.value()
        ,use_anb
        );
    if(mce_no == DeprecatedUseDOB)
        {
        // If no DOB is supplied, assume a birthday occurs on the
        // issue date--as good an assumption as any, and the simplest.
        // It may need to be a day earlier for a contract issued on a
        // leap-year day.
        DateOfBirth = add_years
            (DateOfBirth.value()
            ,apparent_age - IssueAge.value()
            ,true
            );
        }
    else
        {
        IssueAge = apparent_age;
        }
}

std::vector<std::string> mec_input::RealizeAllSequenceInput(bool report_errors)
{
    LMI_ASSERT(years_to_maturity() == database_->length());

    std::vector<std::string> s;
    s.push_back(RealizeFlatExtra                  ());
    s.push_back(RealizePaymentHistory             ());
    s.push_back(RealizeBenefitHistory             ());

    if(report_errors)
        {
        for
            (std::vector<std::string>::iterator i = s.begin()
            ;i != s.end()
            ;++i
            )
            {
            std::ostringstream oss;
            bool diagnostics_present = false;
            if(!i->empty())
                {
                diagnostics_present = true;
                oss << (*i) << "\n";
                }
            if(diagnostics_present)
                {
                fatal_error()
                    << "Input validation problems:\n"
                    << oss.str()
                    << LMI_FLUSH
                    ;
                }
            }
        }

    return s;
}

std::string mec_input::RealizeFlatExtra()
{
// We could enforce a maximum of the monthly equivalent of unity,
// and a minimum of zero; is that worth the bother though?
    std::string s = realize_sequence_string
        (*this
        ,FlatExtraRealized_
        ,FlatExtra
        );
    if(s.size())
        {
        return s;
        }

    if(database_->Query(DB_AllowFlatExtras))
        {
        return "";
        }

    if(!each_equal(FlatExtraRealized_.begin(), FlatExtraRealized_.end(), 0.0))
        {
        return "Flat extras may not be illustrated on this policy form.";
        }

    return "";
}

std::string mec_input::RealizePaymentHistory()
{
    return realize_sequence_string
        (*this
        ,PaymentHistoryRealized_
        ,PaymentHistory
        );
}

std::string mec_input::RealizeBenefitHistory()
{
    return realize_sequence_string
        (*this
        ,BenefitHistoryRealized_
        ,BenefitHistory
        );
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

    Reconcile();
    RealizeAllSequenceInput(false);
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
/// version 0: 20090627T2249Z

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

namespace
{
template<typename Number, typename Trammel>
std::vector<Number> convert_vector_type
    (std::vector<tn_range<Number,Trammel> > const& vr
    )
{
    std::vector<Number> z;
    typename std::vector<tn_range<Number,Trammel> >::const_iterator vr_i;
    for(vr_i = vr.begin(); vr_i != vr.end(); ++vr_i)
        {
        z.push_back(vr_i->value());
        }
    return z;
}
} // Unnamed namespace.

std::vector<double> mec_input::FlatExtraRealized() const
{
    return convert_vector_type<double>(FlatExtraRealized_);
}

std::vector<double> mec_input::PaymentHistoryRealized() const
{
    return convert_vector_type<double>(PaymentHistoryRealized_);
}

std::vector<double> mec_input::BenefitHistoryRealized() const
{
    return convert_vector_type<double>(BenefitHistoryRealized_);
}

