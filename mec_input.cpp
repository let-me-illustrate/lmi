// MVC Model for MEC testing.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "mec_input.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"            // attained_age()
#include "contains.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_sequence_aux.hpp"       // convert_vector(), convert_vector_type()
#include "map_lookup.hpp"
#include "miscellany.hpp"               // each_equal()

#include <algorithm>                    // max()
#include <exception>
#include <limits>
#include <sstream>
#include <utility>                      // pair

template class xml_serializable<mec_input>;

namespace
{
template<typename T>
std::string realize_sequence_string
    (mec_input           & input
    ,std::vector<T>      & v
    ,sequence_base  const& sequence_string
    )
{
    try
        {
        InputSequence s
            (sequence_string.value()
            ,input.years_to_maturity()
            ,input.issue_age        ()
            ,input.maturity_age     () // This class has no "retirement age".
            ,input.inforce_year     ()
            ,input.effective_year   ()
            );
        detail::convert_vector(v, s.seriatim_numbers());
        }
    catch(std::exception const& e)
        {
        return abridge_diagnostics(e.what());
        }
    return std::string();
}
} // Unnamed namespace.

/// Values are initialized by UDT defaults where appropriate, and here
/// in the initializer-list otherwise. All "ascribed" data members are
/// listed here for clarity and maintainability, and commented out if
/// UDT defaults are presently appropriate.

mec_input::mec_input()
    :Use7702ATables                   {"No"}
    ,IssueAge                         {"45"}
    ,Gender                           {"Male"}
    ,Smoking                          {"Nonsmoker"}
    ,UnderwritingClass                {"Standard"}
//    ,DateOfBirth                      {""}
//    ,SubstandardTable                 {""}
//    ,ProductName                      {""}
//    ,External1035ExchangeAmount       {""}
//    ,External1035ExchangeFromMec      {""}
//    ,Internal1035ExchangeAmount       {""}
//    ,Internal1035ExchangeFromMec      {""}
//    ,EffectiveDate                    {""}
//    ,DefinitionOfLifeInsurance        {""}
    ,DefinitionOfMaterialChange       {"Unnecessary premium"}
//    ,GroupUnderwritingType            {""}
//    ,Comments                         {""}
//    ,InforceAsOfDate                  {""}
//    ,InforceYear                      {""}
//    ,InforceMonth                     {""}
    ,InforceTargetSpecifiedAmount     {"1000000"}
//    ,InforceAccountValue              {""}
//    ,InforceIsMec                     {""}
//    ,LastMaterialChangeDate           {""}
//    ,InforceContractYear              {""}
//    ,InforceContractMonth             {""}
//    ,InforceAvBeforeLastMc            {""}
//    ,InforceDcv                       {""}
    ,InforceLeastDeathBenefit         {"1000000"}
    ,PaymentHistory                   {"0"}
    ,BenefitHistory                   {"1000000"}
    ,StateOfJurisdiction              {"CT"}
    ,PremiumTaxState                  {"CT"}
    ,FlatExtra                        {"0"}
//    ,UseDOB                           {""}
    ,Payment                          {"0"}
    ,BenefitAmount                    {"1000000"}
{
    AscribeMembers();
    DoAdaptExternalities(); // Initialize database, e.g.
    DoTransmogrify();       // Make DOB and age consistent, e.g.
}

mec_input::mec_input(mec_input const& z)
    :xml_serializable  <mec_input> {}
    ,MvcModel                      {}
    ,MemberSymbolTable <mec_input> {}
{
    AscribeMembers();
    MemberSymbolTable<mec_input>::assign(z);
    DoAdaptExternalities();
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.

mec_input::~mec_input() = default;

mec_input& mec_input::operator=(mec_input const& z)
{
    MemberSymbolTable<mec_input>::assign(z);
    DoAdaptExternalities();
    return *this;
}

bool mec_input::operator==(mec_input const& z) const
{
    return MemberSymbolTable<mec_input>::equals(z);
}

int mec_input::maturity_age() const {return GleanedMaturityAge_;}

int mec_input::years_to_maturity  () const {return maturity_age() - issue_age();}
int mec_input::issue_age          () const {return IssueAge     .value();}
int mec_input::inforce_year       () const {return InforceYear  .value();}
int mec_input::effective_year     () const {return EffectiveDate.value().year();}

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
    ascribe("InforceAsOfDate"                       , &mec_input::InforceAsOfDate                       );
    ascribe("InforceYear"                           , &mec_input::InforceYear                           );
    ascribe("InforceMonth"                          , &mec_input::InforceMonth                          );
    ascribe("InforceTargetSpecifiedAmount"          , &mec_input::InforceTargetSpecifiedAmount          );
    ascribe("InforceAccountValue"                   , &mec_input::InforceAccountValue                   );
    ascribe("InforceIsMec"                          , &mec_input::InforceIsMec                          );
    ascribe("LastMaterialChangeDate"                , &mec_input::LastMaterialChangeDate                );
    ascribe("InforceContractYear"                   , &mec_input::InforceContractYear                   );
    ascribe("InforceContractMonth"                  , &mec_input::InforceContractMonth                  );
    ascribe("InforceAvBeforeLastMc"                 , &mec_input::InforceAvBeforeLastMc                 );
    ascribe("InforceDcv"                            , &mec_input::InforceDcv                            );
    ascribe("InforceLeastDeathBenefit"              , &mec_input::InforceLeastDeathBenefit              );
    ascribe("PaymentHistory"                        , &mec_input::PaymentHistory                        );
    ascribe("BenefitHistory"                        , &mec_input::BenefitHistory                        );
    ascribe("StateOfJurisdiction"                   , &mec_input::StateOfJurisdiction                   );
    ascribe("PremiumTaxState"                       , &mec_input::PremiumTaxState                       );
    ascribe("FlatExtra"                             , &mec_input::FlatExtra                             );
    ascribe("UseDOB"                                , &mec_input::UseDOB                                );
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
        (::new product_database
            (CachedProductName_
            ,CachedGender_
            ,CachedUnderwritingClass_
            ,CachedSmoking_
            ,CachedIssueAge_
            ,CachedGroupUnderwritingType_
            ,CachedStateOfJurisdiction_
            )
        );

    GleanedMaturityAge_ = database_->query<int>(DB_MaturityAge);
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

    DefinitionOfLifeInsurance.allow(mce_gpt , database_->query<bool>(DB_AllowGpt ));
    DefinitionOfLifeInsurance.allow(mce_cvat, database_->query<bool>(DB_AllowCvat));
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
        alarum()
            << "No option selected for definition of life insurance."
            << LMI_FLUSH
            ;
        }

    GroupUnderwritingType.allow(mce_medical         , database_->query<bool>(DB_AllowFullUw   ));
    GroupUnderwritingType.allow(mce_paramedical     , database_->query<bool>(DB_AllowParamedUw));
    GroupUnderwritingType.allow(mce_nonmedical      , database_->query<bool>(DB_AllowNonmedUw ));
    GroupUnderwritingType.allow(mce_simplified_issue, database_->query<bool>(DB_AllowSimpUw   ));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database_->query<bool>(DB_AllowGuarUw   ));

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
        (database_->query<int>(DB_MinIssAge)
        ,database_->query<int>(DB_MaxIssAge)
        );
#endif // 0

    EffectiveDate.minimum
        (minimum_as_of_date
            (     IssueAge.trammel().maximum_maximorum()
            ,EffectiveDate.trammel().minimum_minimorum()
            )
        );

    auto const alb_anb = database_->query<oenum_alb_or_anb>(DB_AgeLastOrNearest);

    DateOfBirth.minimum_and_maximum
        (minimum_birthdate(IssueAge.maximum(), EffectiveDate.value(), alb_anb)
        ,maximum_birthdate(IssueAge.minimum(), EffectiveDate.value(), alb_anb)
        );

    int max_age = database_->query<int>(DB_MaturityAge);
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

    UnderwritingClass.allow(mce_ultrapreferred, database_->query<bool>(DB_AllowUltraPrefClass));
    UnderwritingClass.allow(mce_preferred     , database_->query<bool>(DB_AllowPreferredClass));
    UnderwritingClass.allow(mce_rated         , database_->query<bool>(DB_AllowSubstdTable));

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

    FlatExtra.enable(database_->query<bool>(DB_AllowFlatExtras));

    bool blend_mortality_by_gender  = false;
    bool blend_mortality_by_smoking = false;

    bool allow_gender_distinct = database_->query<bool>(DB_AllowSexDistinct);
    bool allow_unisex          = database_->query<bool>(DB_AllowUnisex);

    Gender.allow(mce_female, !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_male  , !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_unisex,  blend_mortality_by_gender || allow_unisex);

    bool allow_smoker_distinct = database_->query<bool>(DB_AllowSmokeDistinct);
    bool allow_unismoke        = database_->query<bool>(DB_AllowUnismoke);

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
        ,true
        );
    InforceYear  = ym0.first;
    InforceMonth = ym0.second;

    std::pair<int,int> ym1 = years_and_months_since
        (LastMaterialChangeDate.value()
        ,InforceAsOfDate       .value()
        ,true
        );
    InforceContractYear  = ym1.first;
    InforceContractMonth = ym1.second;

    auto const alb_anb = database_->query<oenum_alb_or_anb>(DB_AgeLastOrNearest);

    int apparent_age = attained_age
        (DateOfBirth.value()
        ,EffectiveDate.value()
        ,alb_anb
        );
    if(mce_no == UseDOB)
        {
        // If DOB does not govern, adjust the birthdate appropriately,
        // with particular caution on February twenty-ninth. See:
        //   https://lists.nongnu.org/archive/html/lmi/2008-07/msg00006.html
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
        for(auto const& i : s)
            {
            std::ostringstream oss;
            bool diagnostics_present = false;
            if(!i.empty())
                {
                diagnostics_present = true;
                oss << i << "\n";
                }
            if(diagnostics_present)
                {
                alarum()
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

    if(database_->query<bool>(DB_AllowFlatExtras))
        {
        return "";
        }

    if(!each_equal(FlatExtraRealized_, 0.0))
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

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20090627T2249Z
/// version 1: 20100719T1349Z

int mec_input::class_version() const
{
    return 1;
}

std::string const& mec_input::xml_root_name() const
{
    static std::string const s("mec");
    return s;
}

bool mec_input::is_detritus(std::string const& s) const
{
    static std::vector<std::string> const v
        {"DeprecatedUseDOB"              // Renamed (without 'Deprecated'-).
        ,"EffectiveDateToday"            // Withdrawn.
        ,"InforceSevenPayPremium"        // Withdrawn.
        };
    return contains(v, s);
}

void mec_input::redintegrate_ex_ante
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

void mec_input::redintegrate_ex_post
    (int                                       file_version
    ,std::map<std::string, std::string> const& detritus_map
    ,std::list<std::string>             const& residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }

    if(file_version < 1)
        {
        LMI_ASSERT(contains(residuary_names, "PremiumTaxState"));
        PremiumTaxState = StateOfJurisdiction;
        LMI_ASSERT(contains(residuary_names, "UseDOB"));
        UseDOB = map_lookup(detritus_map, "DeprecatedUseDOB");
        }
}

void mec_input::redintegrate_ad_terminum()
{
    Reconcile();
    RealizeAllSequenceInput(false);
}

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
