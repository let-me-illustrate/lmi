// MVC Model for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "gpt_input.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"            // attained_age()
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_seq_helpers.hpp"        // convert_vector(), convert_vector_type()
#include "map_lookup.hpp"
#include "miscellany.hpp"               // lmi_array_size()

#include <algorithm>                    // std::max()
#include <limits>
#include <sstream>
#include <utility>                      // std::pair

template class xml_serializable<gpt_input>;

namespace
{
template<typename T>
std::string realize_sequence_string
    (gpt_input           & input
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

gpt_input::gpt_input()
    :Use7702ATables                   ("No")
    ,IssueAge                         ("45")
    ,OldGender                        ("Male")
    ,NewGender                        ("Male")
    ,OldSmoking                       ("Nonsmoker")
    ,NewSmoking                       ("Nonsmoker")
    ,UnderwritingClass                ("Standard")
//    ,DateOfBirth                      ("")
//    ,OldSubstandardTable              ("")
//    ,NewSubstandardTable              ("")
//    ,ProductName                      ("")
//    ,External1035ExchangeAmount       ("")
//    ,External1035ExchangeFromMec      ("")
//    ,Internal1035ExchangeAmount       ("")
//    ,Internal1035ExchangeFromMec      ("")
//    ,EffectiveDate                    ("")
    ,DefinitionOfLifeInsurance        ("GPT")
    ,DefinitionOfMaterialChange       ("GPT adjustment event")
//    ,GroupUnderwritingType            ("")
//    ,Comments                         ("")
//    ,InforceAsOfDate                  ("")
//    ,InforceYear                      ("")
//    ,InforceMonth                     ("")
    ,InforceTargetSpecifiedAmount     ("1000000")
//    ,InforceAccountValue              ("")
//    ,InforceIsMec                     ("")
//    ,LastMaterialChangeDate           ("")
//    ,InforceContractYear              ("")
//    ,InforceContractMonth             ("")
//    ,InforceAvBeforeLastMc            ("")
//    ,InforceDcv                       ("")
    ,InforceLeastDeathBenefit         ("1000000")
    ,PaymentHistory                   ("0")
    ,BenefitHistory                   ("1000000")
    ,StateOfJurisdiction              ("CT")
    ,PremiumTaxState                  ("CT")
    ,OldFlatExtra                     ("0")
    ,NewFlatExtra                     ("0")
//    ,UseDOB                           ("")
    ,Payment                          ("0")
    ,OldDbo                           ("A")
    ,NewDbo                           ("A")
    ,OldDeathBft                      ("1000000")
    ,NewDeathBft                      ("1000000")
    ,OldSpecAmt                       ("1000000")
    ,NewSpecAmt                       ("1000000")
    ,BenefitAmount                    ("1000000")
{
    AscribeMembers();
    DoAdaptExternalities(); // Initialize database, e.g.
    DoTransmogrify();       // Make DOB and age consistent, e.g.
}

gpt_input::gpt_input(gpt_input const& z)
    :obstruct_slicing  <gpt_input>()
    ,xml_serializable  <gpt_input>()
    ,MvcModel                     ()
    ,MemberSymbolTable <gpt_input>()
{
    AscribeMembers();
    MemberSymbolTable<gpt_input>::assign(z);
    DoAdaptExternalities();
}

gpt_input::~gpt_input() = default;

gpt_input& gpt_input::operator=(gpt_input const& z)
{
    MemberSymbolTable<gpt_input>::assign(z);
    DoAdaptExternalities();
    return *this;
}

bool gpt_input::operator==(gpt_input const& z) const
{
    return MemberSymbolTable<gpt_input>::equals(z);
}

int gpt_input::maturity_age() const {return GleanedMaturityAge_;}

int gpt_input::years_to_maturity  () const {return maturity_age() - issue_age();}
int gpt_input::issue_age          () const {return IssueAge     .value();}
int gpt_input::inforce_year       () const {return InforceYear  .value();}
int gpt_input::effective_year     () const {return EffectiveDate.value().year();}

void gpt_input::AscribeMembers()
{
    ascribe("ContractNumber"                        , &gpt_input::ContractNumber                        );
    ascribe("Use7702ATables"                        , &gpt_input::Use7702ATables                        );
    ascribe("IssueAge"                              , &gpt_input::IssueAge                              );
    ascribe("OldGender"                             , &gpt_input::OldGender                             );
    ascribe("NewGender"                             , &gpt_input::NewGender                             );
    ascribe("OldSmoking"                            , &gpt_input::OldSmoking                            );
    ascribe("NewSmoking"                            , &gpt_input::NewSmoking                            );
    ascribe("UnderwritingClass"                     , &gpt_input::UnderwritingClass                     );
    ascribe("DateOfBirth"                           , &gpt_input::DateOfBirth                           );
    ascribe("OldSubstandardTable"                   , &gpt_input::OldSubstandardTable                   );
    ascribe("NewSubstandardTable"                   , &gpt_input::NewSubstandardTable                   );
    ascribe("ProductName"                           , &gpt_input::ProductName                           );
    ascribe("External1035ExchangeAmount"            , &gpt_input::External1035ExchangeAmount            );
    ascribe("External1035ExchangeFromMec"           , &gpt_input::External1035ExchangeFromMec           );
    ascribe("Internal1035ExchangeAmount"            , &gpt_input::Internal1035ExchangeAmount            );
    ascribe("Internal1035ExchangeFromMec"           , &gpt_input::Internal1035ExchangeFromMec           );
    ascribe("EffectiveDate"                         , &gpt_input::EffectiveDate                         );
    ascribe("DefinitionOfLifeInsurance"             , &gpt_input::DefinitionOfLifeInsurance             );
    ascribe("DefinitionOfMaterialChange"            , &gpt_input::DefinitionOfMaterialChange            );
    ascribe("GroupUnderwritingType"                 , &gpt_input::GroupUnderwritingType                 );
    ascribe("Comments"                              , &gpt_input::Comments                              );
    ascribe("InforceAsOfDate"                       , &gpt_input::InforceAsOfDate                       );
    ascribe("InforceYear"                           , &gpt_input::InforceYear                           );
    ascribe("InforceMonth"                          , &gpt_input::InforceMonth                          );
    ascribe("InforceTargetSpecifiedAmount"          , &gpt_input::InforceTargetSpecifiedAmount          );
    ascribe("InforceAccountValue"                   , &gpt_input::InforceAccountValue                   );
    ascribe("InforceGlp"                            , &gpt_input::InforceGlp                            );
    ascribe("InforceCumulativeGlp"                  , &gpt_input::InforceCumulativeGlp                  );
    ascribe("InforceGsp"                            , &gpt_input::InforceGsp                            );
    ascribe("InforceCumulativeGptPremiumsPaid"      , &gpt_input::InforceCumulativeGptPremiumsPaid      );
    ascribe("InforceIsMec"                          , &gpt_input::InforceIsMec                          );
    ascribe("LastMaterialChangeDate"                , &gpt_input::LastMaterialChangeDate                );
    ascribe("InforceContractYear"                   , &gpt_input::InforceContractYear                   );
    ascribe("InforceContractMonth"                  , &gpt_input::InforceContractMonth                  );
    ascribe("InforceAvBeforeLastMc"                 , &gpt_input::InforceAvBeforeLastMc                 );
    ascribe("InforceDcv"                            , &gpt_input::InforceDcv                            );
    ascribe("InforceLeastDeathBenefit"              , &gpt_input::InforceLeastDeathBenefit              );
    ascribe("PaymentHistory"                        , &gpt_input::PaymentHistory                        );
    ascribe("BenefitHistory"                        , &gpt_input::BenefitHistory                        );
    ascribe("StateOfJurisdiction"                   , &gpt_input::StateOfJurisdiction                   );
    ascribe("PremiumTaxState"                       , &gpt_input::PremiumTaxState                       );
    ascribe("OldFlatExtra"                          , &gpt_input::OldFlatExtra                          );
    ascribe("NewFlatExtra"                          , &gpt_input::NewFlatExtra                          );
    ascribe("UseDOB"                                , &gpt_input::UseDOB                                );
    ascribe("Payment"                               , &gpt_input::Payment                               );
    ascribe("PremsPaidDecrement"                    , &gpt_input::PremsPaidDecrement                    );
    ascribe("OldTarget"                             , &gpt_input::OldTarget                             );
    ascribe("NewTarget"                             , &gpt_input::NewTarget                             );
    ascribe("OldDbo"                                , &gpt_input::OldDbo                                );
    ascribe("NewDbo"                                , &gpt_input::NewDbo                                );
    ascribe("OldDeathBft"                           , &gpt_input::OldDeathBft                           );
    ascribe("NewDeathBft"                           , &gpt_input::NewDeathBft                           );
    ascribe("OldSpecAmt"                            , &gpt_input::OldSpecAmt                            );
    ascribe("NewSpecAmt"                            , &gpt_input::NewSpecAmt                            );
    ascribe("OldQabTermAmt"                         , &gpt_input::OldQabTermAmt                         );
    ascribe("NewQabTermAmt"                         , &gpt_input::NewQabTermAmt                         );
    ascribe("BenefitAmount"                         , &gpt_input::BenefitAmount                         );
}

/// Reset database_ if necessary, i.e., if the product or any database
/// axis changed.
///
/// To avoid multiple database objects, presume that "Old" axes are
/// valid, and set the database from "New" parameters.

void gpt_input::DoAdaptExternalities()
{
    // This early-exit condition has to fail the first time this
    // function is called, because database_ is initialized only here.
    if
        (
            database_.get()
        &&  CachedProductName_           == ProductName
        &&  CachedGender_                == NewGender
        &&  CachedUnderwritingClass_     == UnderwritingClass
        &&  CachedSmoking_               == NewSmoking
        &&  CachedIssueAge_              == IssueAge
        &&  CachedGroupUnderwritingType_ == GroupUnderwritingType
        &&  CachedStateOfJurisdiction_   == StateOfJurisdiction
        )
        {
        return;
        }

    CachedProductName_           = ProductName          .value();
    CachedGender_                = NewGender            .value();
    CachedUnderwritingClass_     = UnderwritingClass    .value();
    CachedSmoking_               = NewSmoking           .value();
    CachedIssueAge_              = IssueAge             .value();
    CachedGroupUnderwritingType_ = GroupUnderwritingType.value();
    CachedStateOfJurisdiction_   = StateOfJurisdiction  .value();

    database_.reset
        (new product_database
            (CachedProductName_
            ,CachedGender_
            ,CachedUnderwritingClass_
            ,CachedSmoking_
            ,CachedIssueAge_
            ,CachedGroupUnderwritingType_
            ,CachedStateOfJurisdiction_
            )
        );

    GleanedMaturityAge_ = static_cast<int>(database_->Query(DB_MaturityAge));
}

datum_base const* gpt_input::DoBaseDatumPointer
    (std::string const& name
    ) const
{
    return member_cast<datum_base>(operator[](name));
}

any_entity& gpt_input::DoEntity(std::string const& name)
{
    return MemberSymbolTable<gpt_input>::operator[](name);
}

any_entity const& gpt_input::DoEntity(std::string const& name) const
{
    return MemberSymbolTable<gpt_input>::operator[](name);
}

MvcModel::NamesType const& gpt_input::DoNames() const
{
    return member_names();
}

MvcModel::StateType gpt_input::DoState() const
{
    return member_state(*this);
}

void gpt_input::DoCustomizeInitialValues()
{
}

void gpt_input::DoEnforceCircumscription(std::string const& name)
{
    datum_base* base_datum = member_cast<datum_base>(operator[](name));
    tn_range_base* datum = dynamic_cast<tn_range_base*>(base_datum);
    if(datum)
        {
        datum->enforce_circumscription();
        }
}

void gpt_input::DoEnforceProscription(std::string const& name)
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

void gpt_input::DoHarmonize()
{
    bool anything_goes    = global_settings::instance().ash_nazg();

    DefinitionOfLifeInsurance.allow(mce_gpt , database_->Query(DB_AllowGpt ));
    DefinitionOfLifeInsurance.allow(mce_cvat, database_->Query(DB_AllowCvat));
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

    GroupUnderwritingType.allow(mce_medical         , database_->Query(DB_AllowFullUw   ));
    GroupUnderwritingType.allow(mce_paramedical     , database_->Query(DB_AllowParamedUw));
    GroupUnderwritingType.allow(mce_nonmedical      , database_->Query(DB_AllowNonmedUw ));
    GroupUnderwritingType.allow(mce_simplified_issue, database_->Query(DB_AllowSimpUw   ));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database_->Query(DB_AllowGuarUw   ));

    IssueAge        .enable(mce_no  == UseDOB);
    DateOfBirth     .enable(mce_yes == UseDOB);

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

    oenum_alb_or_anb const alb_anb =
        static_cast<oenum_alb_or_anb>
            (static_cast<int>
                (database_->Query(DB_AgeLastOrNearest)
                )
            );
    DateOfBirth.minimum_and_maximum
        (minimum_birthdate(IssueAge.maximum(), EffectiveDate.value(), alb_anb)
        ,maximum_birthdate(IssueAge.minimum(), EffectiveDate.value(), alb_anb)
        );

    int max_age = static_cast<int>(database_->Query(DB_MaturityAge));
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
          EffectiveDate == InforceAsOfDate
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
          EffectiveDate == InforceAsOfDate
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

    OldSubstandardTable.enable(mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_a, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_b, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_c, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_d, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_e, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_f, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_h, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_j, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_l, mce_rated == UnderwritingClass);
    OldSubstandardTable.allow(mce_table_p, mce_rated == UnderwritingClass);

    NewSubstandardTable.enable(mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_a, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_b, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_c, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_d, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_e, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_f, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_h, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_j, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_l, mce_rated == UnderwritingClass);
    NewSubstandardTable.allow(mce_table_p, mce_rated == UnderwritingClass);

    OldFlatExtra.enable(database_->Query(DB_AllowFlatExtras));
    NewFlatExtra.enable(database_->Query(DB_AllowFlatExtras));

    bool blend_mortality_by_gender  = false;
    bool blend_mortality_by_smoking = false;

    bool allow_gender_distinct = database_->Query(DB_AllowSexDistinct);
    bool allow_unisex          = database_->Query(DB_AllowUnisex);

    OldGender.allow(mce_female, !blend_mortality_by_gender && allow_gender_distinct);
    OldGender.allow(mce_male  , !blend_mortality_by_gender && allow_gender_distinct);
    OldGender.allow(mce_unisex,  blend_mortality_by_gender || allow_unisex);

    NewGender.allow(mce_female, !blend_mortality_by_gender && allow_gender_distinct);
    NewGender.allow(mce_male  , !blend_mortality_by_gender && allow_gender_distinct);
    NewGender.allow(mce_unisex,  blend_mortality_by_gender || allow_unisex);

    bool allow_smoker_distinct = database_->Query(DB_AllowSmokeDistinct);
    bool allow_unismoke        = database_->Query(DB_AllowUnismoke);

    OldSmoking.allow(mce_smoker,    !blend_mortality_by_smoking && allow_smoker_distinct);
    OldSmoking.allow(mce_nonsmoker, !blend_mortality_by_smoking && allow_smoker_distinct);
    OldSmoking.allow(mce_unismoke,   blend_mortality_by_smoking || allow_unismoke);

    NewSmoking.allow(mce_smoker,    !blend_mortality_by_smoking && allow_smoker_distinct);
    NewSmoking.allow(mce_nonsmoker, !blend_mortality_by_smoking && allow_smoker_distinct);
    NewSmoking.allow(mce_unismoke,   blend_mortality_by_smoking || allow_unismoke);
}

/// Change values as required for consistency.

void gpt_input::DoTransmogrify()
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

    oenum_alb_or_anb const alb_anb =
        static_cast<oenum_alb_or_anb>
            (static_cast<int>
                (database_->Query(DB_AgeLastOrNearest)
                )
            );

    int apparent_age = attained_age
        (DateOfBirth.value()
        ,EffectiveDate.value()
        ,alb_anb
        );
    if(mce_no == UseDOB)
        {
        // If DOB does not govern, adjust the birthdate appropriately,
        // with particular caution on February twenty-ninth. See:
        //   http://lists.nongnu.org/archive/html/lmi/2008-07/msg00006.html
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

std::vector<std::string> gpt_input::RealizeAllSequenceInput(bool report_errors)
{
    LMI_ASSERT(years_to_maturity() == database_->length());

    std::vector<std::string> s;
    s.push_back(RealizeOldFlatExtra               ());
    s.push_back(RealizeNewFlatExtra               ());
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

std::string gpt_input::RealizeOldFlatExtra()
{
// We could enforce a maximum of the monthly equivalent of unity,
// and a minimum of zero; is that worth the bother though?
    std::string s = realize_sequence_string
        (*this
        ,OldFlatExtraRealized_
        ,OldFlatExtra
        );
    if(s.size())
        {
        return s;
        }

    if(database_->Query(DB_AllowFlatExtras))
        {
        return "";
        }

    if(!each_equal(OldFlatExtraRealized_.begin(), OldFlatExtraRealized_.end(), 0.0))
        {
        return "Flat extras may not be illustrated on this policy form.";
        }

    return "";
}

std::string gpt_input::RealizeNewFlatExtra()
{
// We could enforce a maximum of the monthly equivalent of unity,
// and a minimum of zero; is that worth the bother though?
    std::string s = realize_sequence_string
        (*this
        ,NewFlatExtraRealized_
        ,NewFlatExtra
        );
    if(s.size())
        {
        return s;
        }

    if(database_->Query(DB_AllowFlatExtras))
        {
        return "";
        }

    if(!each_equal(NewFlatExtraRealized_.begin(), NewFlatExtraRealized_.end(), 0.0))
        {
        return "Flat extras may not be illustrated on this policy form.";
        }

    return "";
}

std::string gpt_input::RealizePaymentHistory()
{
    return realize_sequence_string
        (*this
        ,PaymentHistoryRealized_
        ,PaymentHistory
        );
}

std::string gpt_input::RealizeBenefitHistory()
{
    return realize_sequence_string
        (*this
        ,BenefitHistoryRealized_
        ,BenefitHistory
        );
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20140615T1717Z

int gpt_input::class_version() const
{
    return 0;
}

std::string const& gpt_input::xml_root_name() const
{
    static std::string const s("gpt");
    return s;
}

bool gpt_input::is_detritus(std::string const& s) const
{
    static std::string const a[] =
        {"Remove this string when adding the first removed entity."
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return contains(v, s);
}

void gpt_input::redintegrate_ex_ante
    (int                file_version
    ,std::string const& // name
    ,std::string      & // value
    ) const
{
    if(class_version() == file_version)
        {
        return;
        }

    // Nothing to do for now.
}

void gpt_input::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& // detritus_map
    ,std::list<std::string>             const& // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    // Nothing to do for now.
}

void gpt_input::redintegrate_ad_terminum()
{
    Reconcile();
    RealizeAllSequenceInput(false);
}

std::vector<double> gpt_input::OldFlatExtraRealized() const
{
    return convert_vector_type<double>(OldFlatExtraRealized_);
}

std::vector<double> gpt_input::NewFlatExtraRealized() const
{
    return convert_vector_type<double>(NewFlatExtraRealized_);
}

std::vector<double> gpt_input::PaymentHistoryRealized() const
{
    return convert_vector_type<double>(PaymentHistoryRealized_);
}

std::vector<double> gpt_input::BenefitHistoryRealized() const
{
    return convert_vector_type<double>(BenefitHistoryRealized_);
}

