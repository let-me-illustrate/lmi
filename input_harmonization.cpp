// Life-insurance illustration input--control harmonization.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: input_harmonization.cpp,v 1.106 2009-09-29 01:18:24 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "mc_enum_types_aux.hpp"
#include "value_cast.hpp"

#include <algorithm> // std::min(), std::max()

// Harmonization is physically separated for no better reason than to
// facilitate its development at a time when it frequently changes.

namespace
{
    std::string current_credited_rate(TDatabase const& database)
        {
        std::vector<double> z;
        database.Query(z, DB_MaxGenAcctRate);
        return InputSequence(z).mathematical_representation();
        }
} // Unnamed namespace.

/// Implementation notes: DoAdaptExternalities().
///
/// Reset database_ if necessary, i.e., if the product or any database
/// axis changed.

void Input::DoAdaptExternalities()
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

    GleanedLedgerType_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (database_->Query(DB_LedgerType)
                )
            );
}

void Input::DoCustomizeInitialValues()
{
    if(mce_yes == UseCurrentDeclaredRate)
        {
        GeneralAccountRate = current_credited_rate(*database_);
        }
}

void Input::DoHarmonize()
{
    bool anything_goes    = global_settings::instance().ash_nazg();
    bool home_office_only = global_settings::instance().mellon();
    bool egregious_kludge = global_settings::instance().regression_testing();

    bool allow_sep_acct = database_->Query(DB_AllowSepAcct);
    bool allow_gen_acct = database_->Query(DB_AllowGenAcct);

    bool sepacct_only = allow_sep_acct && !allow_gen_acct;
    bool genacct_only = allow_gen_acct && !allow_sep_acct;

    bool wd_allowed = database_->Query(DB_AllowWD);
    bool loan_allowed = database_->Query(DB_AllowLoan);
    bool pref_loan_allowed = loan_allowed && database_->Query(DB_AllowPrefLoan);

    DefinitionOfLifeInsurance.allow(mce_gpt, database_->Query(DB_AllowGPT));
    DefinitionOfLifeInsurance.allow(mce_cvat, database_->Query(DB_AllowCVAT));
    DefinitionOfLifeInsurance.allow(mce_noncompliant, database_->Query(DB_AllowNo7702));

    DefinitionOfMaterialChange.enable(mce_noncompliant != DefinitionOfLifeInsurance);
    if(mce_noncompliant == DefinitionOfLifeInsurance)
        {
        // Nothing to do: all choices ignored because control is disabled.
        }
    else if(mce_cvat == DefinitionOfLifeInsurance)
        {
        DefinitionOfMaterialChange.allow(mce_unnecessary_premium                        ,anything_goes);
        DefinitionOfMaterialChange.allow(mce_benefit_increase                           ,anything_goes);
        DefinitionOfMaterialChange.allow(mce_later_of_increase_or_unnecessary_premium   ,anything_goes);
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

    MaximumNaar.enable(anything_goes);

    AmortizePremiumLoad.enable(database_->Query(DB_AllowAmortPremLoad));
    ExtraCompensationOnAssets .enable(database_->Query(DB_AllowExtraAssetComp));
    ExtraCompensationOnPremium.enable(database_->Query(DB_AllowExtraPremComp));
    OffshoreCorridorFactor.enable(mce_noncompliant == DefinitionOfLifeInsurance);

    RetireesCanEnroll.enable(database_->Query(DB_AllowRetirees));

    // TODO ?? There should be flags in the database to allow or
    // forbid paramedical and nonmedical underwriting; arbitrarily,
    // until they are added, those options are always inhibited.
    GroupUnderwritingType.allow(mce_medical, database_->Query(DB_AllowFullUW));
    GroupUnderwritingType.allow(mce_paramedical, false);
    GroupUnderwritingType.allow(mce_nonmedical, false);
    GroupUnderwritingType.allow(mce_simplified_issue, database_->Query(DB_AllowSimpUW));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database_->Query(DB_AllowGuarUW));

    bool part_mort_used = mce_yes == UsePartialMortality;

    PartialMortalityTable     .enable(part_mort_used);
    PartialMortalityMultiplier.enable(part_mort_used);

    CurrentCoiGrading         .enable(part_mort_used && home_office_only);
    CashValueEnhancementRate  .enable(home_office_only);

    SurviveToType             .allow(mce_no_survival_limit    , part_mort_used);
    SurviveToType             .allow(mce_survive_to_age       , part_mort_used);
    SurviveToType             .allow(mce_survive_to_year      , part_mort_used);
    SurviveToType             .allow(mce_survive_to_expectancy, part_mort_used);
    SurviveToType             .enable(part_mort_used);

    SurviveToYear             .enable(part_mort_used && mce_survive_to_year == SurviveToType);
    SurviveToAge              .enable(part_mort_used && mce_survive_to_age  == SurviveToType);

    bool enable_experience_rating =
            database_->Query(DB_AllowExpRating)
        &&  part_mort_used
        &&  mce_month_by_month == RunOrder
        ;
    UseExperienceRating.enable(enable_experience_rating);

    // TODO ?? These shouldn't need to depend on 'enable_experience_rating';
    // instead, 'UseExperienceRating' should be transmogrified if it's not
    // enabled.
    OverrideExperienceReserveRate.enable
        (   enable_experience_rating
        &&  mce_yes == UseExperienceRating
        );
    ExperienceReserveRate.enable
        (   enable_experience_rating
        &&  mce_yes == UseExperienceRating
        &&  mce_yes == OverrideExperienceReserveRate
        );
    ExperienceRatingInitialKFactor.enable
        (   enable_experience_rating
        &&  mce_yes == UseExperienceRating
        );
    InforceNetExperienceReserve.enable
        (   enable_experience_rating
        &&  mce_yes == UseExperienceRating
        );
    InforceYtdNetCoiCharge.enable
        (   enable_experience_rating
        &&  mce_yes == UseExperienceRating
        );

    EffectiveDate.enable(mce_no == EffectiveDateToday);

    IssueAge        .enable(mce_no  == DeprecatedUseDOB);
    DateOfBirth     .enable(mce_yes == DeprecatedUseDOB);

    // The ranges of both EffectiveDate and IssueAge are treated as
    // independent, to prevent one's value from affecting the other's
    // range and therefore possibly forcing its value to change. Thus,
    // if the maximum conceivable IssueAge is 100, then the earliest
    // permitted EffectiveDate is approximately the centennial of the
    // gregorian epoch.

#if 0
// TODO ?? Temporarily suppress this while exploring automatic-
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

    RetirementAge   .enable(mce_no  == DeprecatedUseDOR);
    DateOfRetirement.enable(mce_yes == DeprecatedUseDOR);

    // DATABASE !! Maximum illustrated age should be distinguished
    // from maturity age (which shouldn't be called 'EndtAge' because
    // the contract needn't endow).
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

    InforceCumulativeGlp.enable(mce_gpt == DefinitionOfLifeInsurance);
    InforceGlp          .enable(mce_gpt == DefinitionOfLifeInsurance);
    InforceGsp          .enable(mce_gpt == DefinitionOfLifeInsurance);

    bool non_mec = mce_no == InforceIsMec;

    InforceSevenPayPremium  .enable(non_mec);
    LastMaterialChangeDate  .enable(non_mec);
    InforceDcv              .enable(non_mec && mce_cvat == DefinitionOfLifeInsurance);
    InforceAvBeforeLastMc   .enable(non_mec);
    InforceContractYear     .enable(non_mec);
    InforceContractMonth    .enable(non_mec);
    InforceLeastDeathBenefit.enable(non_mec);

    if(std::string::npos != global_settings::instance().pyx().find("old_inforce"))
        {
        // These fields have no effect for now. They're suppressed to
        // avoid confusion.
        InforceAsOfDate.enable(false);
        LastMaterialChangeDate.enable(false);
        }
    else
        {
        // These will soon be removed from the GUI:
        InforceYear         .enable(false);
        InforceMonth        .enable(false);
        InforceContractYear .enable(false);
        InforceContractMonth.enable(false);
        }

// TODO ?? Nomen est omen.
if(!egregious_kludge)
  {
    UnderwritingClass.allow(mce_ultrapreferred, database_->Query(DB_AllowUltraPrefClass));
    UnderwritingClass.allow(mce_preferred     , database_->Query(DB_AllowPreferredClass));

    // It would seem generally reasonable to forbid table ratings on
    // guaranteed-issue contracts. No such principle is hardcoded here
    // because the database is the proper place to express such a
    // judgment.
    //
    // TODO ?? OTOH, the basic-values class allows table ratings only
    // if the group underwriting type is full medical underwriting,
    // i.e. if
    //   GroupUnderwritingType is mce_medical
    // and enablement here is inconsistent with that, which is bad.
    // The real question is: to which rate table should table ratings
    // be applied? Probably the rule in the basic-values class is
    // overbroad and should be removed, with very careful attention
    // paid to databases. One could conceive of a product that has
    // only SI rates and allows table ratings to be applied to them.
    // Another product might offer GI, SI, and full underwriting, but
    // allow table ratings only with a full-underwriting rate table.
    // It is important to put aside prior notions of what GI or SI
    // might connote, and realize that to the table-access code they
    // are simply lookup axes.
    //
    UnderwritingClass.allow(mce_rated, database_->Query(DB_AllowSubstdTable));

    // TODO ?? WX PORT !! Nasty interaction here.
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
  } // end if(!egregious_kludge)

    // Can't have a non-US country multiplier other than unity in a US state.
    bool allow_custom_coi_multiplier =
            mce_c_US != Country
        ||  mce_s_XX == StateOfJurisdiction
            ;
    OverrideCoiMultiplier.enable(allow_custom_coi_multiplier);
    CountryCoiMultiplier.enable
        (   allow_custom_coi_multiplier
        &&  mce_yes == OverrideCoiMultiplier
        );
    FlatExtra.enable(database_->Query(DB_AllowFlatExtras));

    calendar_date const most_recent_anniversary = add_years
        (EffectiveDate.value()
        ,InforceYear  .value()
        ,true
        );
    LastCoiReentryDate.maximum(most_recent_anniversary);
    // DATABASE !! Here, 'e_reenter_upon_rate_reset' would be better
    // than the hardcoded '2'. However, '2' is already hardcoded in
    // the 'dbnames.xpp' explanation of 'LastCoiReentryDate', so
    // hardcoding it here doesn't introduce a new kind of defect.
    // Ultimately, the product database should probably use mc_enum
    // types instead; until then, this will do.
    LastCoiReentryDate.enable(2 == database_->Query(DB_CoiInforceReentry));

    BlendGender.enable(database_->Query(DB_AllowMortBlendSex));
    bool blend_mortality_by_gender = mce_yes == BlendGender;

    BlendSmoking.enable(database_->Query(DB_AllowMortBlendSmoke));
    bool blend_mortality_by_smoking = mce_yes == BlendSmoking;

    MaleProportion     .enable(blend_mortality_by_gender);
    NonsmokerProportion.enable(blend_mortality_by_smoking);

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

    // TODO ?? WX PORT !! Perhaps those rules leave no choice allowed
    // for gender or smoker.

    // Many SA strategies forbidden if premium is a function of SA.
    bool prem_indeterminate =
        (
            IndividualPaymentStrategy != mce_pmt_input_scalar
// TODO ??        ||  mce_pmt_input_scalar != ErPmtStrategy
        ||  mce_solve_ee_prem     == SolveType
        ||  mce_solve_er_prem     == SolveType
        ||  mce_reduce_prem       == AvoidMecMethod
        );

    bool specamt_solve = mce_solve_specamt == SolveType;

    bool specamt_from_term_proportion =
           database_->Query(DB_AllowTerm)
        && mce_yes == TermRiderUseProportion
        && mce_yes == TermRider
        ;

    bool inhibit_premium_based_strategies =
           prem_indeterminate
        || specamt_solve
        || specamt_from_term_proportion
        ;

    bool salary_used =
// TODO ?? WX PORT !! Figure out how to handle the next line:
//           mce_sa_salary == VectorSpecifiedAmountStrategy[0]
true // Silly workaround for now.
        || mce_sa_salary == SpecifiedAmountStrategyFromIssue
        ;

    SalarySpecifiedAmountFactor .enable(!specamt_solve && salary_used);
    SalarySpecifiedAmountCap    .enable(!specamt_solve && salary_used);
    SalarySpecifiedAmountOffset .enable(!specamt_solve && salary_used);

    // Strategies based on glp and gsp are permitted even for
    // contracts that don't use gpt. One might want to select such a
    // strategy, then toggle back and forth between gpt and cvat to
    // see what difference that makes.

    SpecifiedAmountStrategyFromIssue.allow(mce_sa_input_scalar, !specamt_solve && !specamt_from_term_proportion);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_salary      , !specamt_solve && !specamt_from_term_proportion);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_maximum     , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_target      , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_mep         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_glp         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_gsp         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_corridor    , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.enable(!specamt_solve && !specamt_from_term_proportion && mce_sa_input_scalar == SpecifiedAmountStrategyFromIssue);

    SpecifiedAmountFromIssue.enable(!specamt_solve && !specamt_from_term_proportion && mce_sa_input_scalar == SpecifiedAmountStrategyFromIssue);

    bool inhibit_sequence = specamt_solve || specamt_from_term_proportion;
    SpecifiedAmount.enable(!inhibit_sequence);

    bool never_retire = database_->Query(DB_EndtAge) <= RetirementAge.value();
/*
// TODO ?? WX PORT !! Figure out how to handle the next line:
    if(!is_specamt_simply_representable)
        {
        SCALAR              ->EnableWindow(false);
        AMOUNT              ->EnableWindow(false);
        IS_SALARY_PCT       ->EnableWindow(false);

        MAX                 ->EnableWindow(false);
        TGT                 ->EnableWindow(false);
        MEC                 ->EnableWindow(false);
        GLP                 ->EnableWindow(false);
        GSP                 ->EnableWindow(false);
        CORRIDOR            ->EnableWindow(false);
        }
*/

/* TODO ?? WX PORT !! Post-retirment specamt strategy not ported.
    POSTRET_SAME_AS     ->EnableWindow(!specamt_solve && !specamt_from_term_proportion);
    POSTRET_SCALAR      ->EnableWindow(!specamt_solve && !specamt_from_term_proportion);
    POSTRET_IS_PCT      ->EnableWindow(!specamt_solve && !specamt_from_term_proportion);

    POSTRET_AMOUNT      ->EnableWindow
        (   !specamt_solve
        &&  !specamt_from_term_proportion
        &&  BF_CHECKED == POSTRET_SCALAR->GetCheck()
        );
    POSTRET_PCT         ->EnableWindow
        (   !specamt_solve
        &&  !specamt_from_term_proportion
        &&  BF_CHECKED == POSTRET_IS_PCT->GetCheck()
        );

    if(!is_specamt_simply_representable)
        {
        POSTRET_SAME_AS     ->EnableWindow(false);
        POSTRET_SCALAR      ->EnableWindow(false);
        POSTRET_IS_PCT      ->EnableWindow(false);
        POSTRET_AMOUNT      ->EnableWindow(false);
        POSTRET_PCT         ->EnableWindow(false);
        }

    if(never_retire)
        {
        POSTRET_SAME_AS     ->EnableWindow(false);
        POSTRET_SCALAR      ->EnableWindow(false);
        POSTRET_AMOUNT      ->EnableWindow(false);
        }
*/

    // TODO ?? WX PORT !! Figure out how to do this properly.
    bool is_dbopt_simply_representable = true;

    DeathBenefitOptionFromRetirement.allow(mce_option1, is_dbopt_simply_representable);
    DeathBenefitOptionFromRetirement.allow(mce_option2, is_dbopt_simply_representable);
    DeathBenefitOptionFromRetirement.allow(mce_rop    , is_dbopt_simply_representable && database_->Query(DB_AllowDBO3));
    DeathBenefitOptionFromIssue     .allow(mce_option1, is_dbopt_simply_representable && !never_retire);
    DeathBenefitOptionFromIssue     .allow(mce_option2, is_dbopt_simply_representable && !never_retire && (database_->Query(DB_AllowChangeToDBO2) || mce_option2 == DeathBenefitOptionFromRetirement));
    DeathBenefitOptionFromIssue     .allow(mce_rop    , is_dbopt_simply_representable && !never_retire && database_->Query(DB_AllowDBO3));

/*
    // TODO ?? WX PORT !! Figure out how to do this properly.
    if(is_dbopt_sequence_empty)
        {
        // Input sequence governs, and if it's empty, defaults
        // are used, so make radiobuttons reflect that.
        DBOPT_INIT_1        ->SetCheck(BF_CHECKED);
        DBOPT_INIT_2        ->SetCheck(BF_UNCHECKED);
        DBOPT_INIT_ROP      ->SetCheck(BF_UNCHECKED);
        DBOPT_POSTRET_1     ->SetCheck(BF_CHECKED);
        DBOPT_POSTRET_2     ->SetCheck(BF_UNCHECKED);
        DBOPT_POSTRET_ROP   ->SetCheck(BF_UNCHECKED);
        }
*/

    bool prem_solve = mce_solve_ee_prem == SolveType;

    // Many payment strategies are forbidden if specamt is a function
    // of payment.
    bool specamt_indeterminate =
            mce_solve_specamt == SolveType
        ||  (
               mce_sa_input_scalar != SpecifiedAmountStrategyFromIssue
            && mce_sa_salary       != SpecifiedAmountStrategyFromIssue
            )
        ;

    bool inhibit_prem_simple =
// TODO ?? WX PORT !! Figure out how to do this properly:
//           !is_indv_pmt_simply_representable
false // Silly workaround for now.
        || mce_solve_ee_prem == SolveType
        ;

    IndividualPaymentToAlternative.allow(mce_to_retirement, !inhibit_prem_simple && !prem_solve);
    IndividualPaymentToAlternative.allow(mce_to_year      , !inhibit_prem_simple && !prem_solve);
    IndividualPaymentToAlternative.allow(mce_to_age       , !inhibit_prem_simple && !prem_solve);
    IndividualPaymentToAlternative.allow(mce_to_maturity  , !inhibit_prem_simple && !prem_solve);
    IndividualPaymentToAlternative.enable(!prem_solve);

    IndividualPaymentStrategy.allow(mce_pmt_input_scalar, !inhibit_prem_simple && !prem_solve);
    IndividualPaymentStrategy.allow(mce_pmt_minimum     , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_target      , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_mep         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_glp         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_gsp         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_corridor    , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_table       , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.enable(!inhibit_prem_simple && !prem_solve);

    IndividualPaymentAmount    .enable(mce_pmt_input_scalar == IndividualPaymentStrategy);
    IndividualPaymentToAge     .enable(mce_to_age  == IndividualPaymentToAlternative);
    IndividualPaymentToDuration.enable(mce_to_year == IndividualPaymentToAlternative);

//    InsuredPremiumTableNumber.enable(mce_pmt_table == IndividualPaymentStrategy); // TODO ?? Not yet implemented.
// In the legacy system, that control, 'InsuredPremiumTableFactor',
// and their 'Corporation'- congeners were enabled iff a
// 'mce_pmt_table' strategy was selected in a scalar control--but
// no such scalar control was ported. For payment strategy, lmi offers
// only input sequences that are enabled by default.

    Payment           .enable(mce_solve_ee_prem != SolveType);
    CorporationPayment.enable(mce_solve_er_prem != SolveType);

    IndividualPaymentMode.allow_all(true);
    // TODO ?? Should the following be permitted? If so, then either
    // enumerators must always be simple zero-based ordinals, or
    // arguments to allow() must be values instead of ordinals.
//    IndividualPaymentMode.allow(mce_annual    , true);
//    IndividualPaymentMode.allow(mce_semiannual, true);
//    IndividualPaymentMode.allow(mce_quarterly , true);
//    IndividualPaymentMode.allow(mce_monthly   , true);

/*
    // TODO ?? WX PORT !! Figure out how to do this properly:

    if(is_indv_mode_sequence_empty)
        {
        MODE_ANNUAL     ->SetCheck(BF_CHECKED);
        MODE_SEMI       ->SetCheck(BF_UNCHECKED);
        MODE_QUARTERLY  ->SetCheck(BF_UNCHECKED);
        MODE_MONTHLY    ->SetCheck(BF_UNCHECKED);
        }
    if(!is_indv_mode_simply_representable)
        {
        MODE_ANNUAL     ->EnableWindow(false);
        MODE_SEMI       ->EnableWindow(false);
        MODE_QUARTERLY  ->EnableWindow(false);
        MODE_MONTHLY    ->EnableWindow(false);
        }
*/

// genacct: earned is suppressed for "compliance" reasons
// sepacct: net is suppressed for "compliance" reasons
//
// The "compliance" reasons don't seem sensible, but that's another
// matter. DATABASE !! Control that in the product database.

    GeneralAccountRateType .allow(mce_credited_rate , true);
    GeneralAccountRateType .allow(mce_earned_rate, anything_goes && mce_no == UseCurrentDeclaredRate);

    SeparateAccountRateType.allow(mce_gross_rate, true);
    SeparateAccountRateType.allow(mce_net_rate  , anything_goes);

    bool curr_int_rate_solve = false; // May be useful someday.
    UseCurrentDeclaredRate .enable(!curr_int_rate_solve && allow_gen_acct);
    GeneralAccountRate     .enable(!curr_int_rate_solve && allow_gen_acct && mce_no == UseCurrentDeclaredRate);
    GeneralAccountRateType .enable(!curr_int_rate_solve && allow_gen_acct && mce_no == UseCurrentDeclaredRate);
    SeparateAccountRate    .enable(!curr_int_rate_solve && allow_sep_acct);
    SeparateAccountRateType.enable(!curr_int_rate_solve && allow_sep_acct);

    InforceGeneralAccountValue .enable(allow_gen_acct);
    InforceSeparateAccountValue.enable(allow_sep_acct);

    // TODO ?? VLR not yet implemented.
    bool allow_vlr =
        (   loan_allowed
        &&  (   database_->Query(DB_AllowVLR)
            ||  anything_goes
            )
        );
    LoanRateType.allow(mce_variable_loan_rate, allow_vlr);
    LoanRate.enable(mce_variable_loan_rate == LoanRateType);

    UseAverageOfAllFunds.enable(!genacct_only);
    bool enable_custom_fund =
            !genacct_only
        &&  (   database_->Query(DB_AllowCustomFund)
            ||  home_office_only
            )
        ;
    OverrideFundManagementFee.enable(enable_custom_fund);

    InputFundManagementFee.enable(mce_yes == OverrideFundManagementFee || mce_fund_override == FundChoiceType);

// TODO ?? WX PORT !! There seems to be some confusion here. We seem to have
// checkboxes 'OverrideFundManagementFee' and 'UseAverageOfAllFunds'
// that duplicate enumerative control 'FundChoiceType'.
//
//    mce_yes_or_no            UseAverageOfAllFunds            ;
//    mce_yes_or_no            OverrideFundManagementFee       ;
//    mce_fund_input_method    FundChoiceType                  ;
// The last duplicates the information borne by the first two.
//     mce_fund_average
//    ,mce_fund_override
//    ,mce_fund_selection
//
    FundChoiceType.allow(mce_fund_average  , !genacct_only);
    FundChoiceType.allow(mce_fund_override , enable_custom_fund);

// Always true, even for genacct-only products, which do offer one 'choice';
// though perhaps not for products that offer no general account and offer
// only 'custom' separate accounts.
//
// TODO ?? WX PORT !! But for now, use this workaround: products that have no
// general account can't select non-custom funds--there's no GUI for
// that anyway. DATABASE !! Consider adding an 'allow fund choice' entity.
//
    FundChoiceType.allow(mce_fund_selection, !sepacct_only);

/* TODO ?? WX PORT !! Not ported:
    SELECTED_FUND_ALLOC->EnableWindow
        (  !genacct_only
        && BF_CHECKED == CHOOSE_ALLOCATIONS->GetCheck()
        );
    FUND_LIST->EnableWindow
        (  !genacct_only
        && BF_CHECKED == CHOOSE_ALLOCATIONS->GetCheck()
        );
*/

    bool wd_solve = (mce_solve_wd == SolveType);
    bool wd_forbidden = !wd_allowed;
    // TODO ?? Also need to inhibit affected solves.

    Withdrawal.enable(!wd_forbidden && !wd_solve);

    bool wd_inhibit = wd_solve || wd_forbidden;
// TODO ?? WX PORT !! Figure out how to do this properly:
    bool wd_inhibit_simple = wd_inhibit; // TODO ?? || !is_wd_simply_representable;

    WithdrawalFromAlternative.allow(mce_from_issue     , !wd_inhibit_simple);
    WithdrawalFromAlternative.allow(mce_from_year      , !wd_inhibit_simple);
    WithdrawalFromAlternative.allow(mce_from_age       , !wd_inhibit_simple);
    WithdrawalFromAlternative.allow(mce_from_retirement, !wd_inhibit_simple);
    WithdrawalFromAlternative.enable(!wd_inhibit_simple);

    WithdrawalToAlternative  .allow(mce_to_retirement  , !wd_inhibit_simple);
    WithdrawalToAlternative  .allow(mce_to_year        , !wd_inhibit_simple);
    WithdrawalToAlternative  .allow(mce_to_age         , !wd_inhibit_simple);
    WithdrawalToAlternative  .allow(mce_to_maturity    , !wd_inhibit_simple);
    WithdrawalToAlternative  .enable(!wd_inhibit_simple);

    WithdrawalAmount         .enable(!wd_inhibit_simple);
    WithdrawalFromAge        .enable(!wd_inhibit_simple && mce_from_age  == WithdrawalFromAlternative);
    WithdrawalFromDuration   .enable(!wd_inhibit_simple && mce_from_year == WithdrawalFromAlternative);
    WithdrawalToAge          .enable(!wd_inhibit_simple && mce_to_age    == WithdrawalToAlternative);
    WithdrawalToDuration     .enable(!wd_inhibit_simple && mce_to_year   == WithdrawalToAlternative);

    bool loan_solve = mce_solve_loan == SolveType;
    bool loan_forbidden = !loan_allowed;

    WithdrawToBasisThenLoan.enable(!wd_forbidden && !loan_forbidden);

    bool loan_inhibit = loan_solve || loan_forbidden;
// TODO ?? WX PORT !! Figure out how to do this properly:
    bool loan_inhibit_simple = loan_inhibit; // TODO ?? || !is_loan_simply_representable;

    NewLoan.enable(!loan_inhibit);

    InforceRegularLoanValue    .enable(loan_allowed);
    InforcePreferredLoanValue  .enable(pref_loan_allowed);
    InforceRegularLoanBalance  .enable(loan_allowed);
    InforcePreferredLoanBalance.enable(pref_loan_allowed);

    LoanFromAlternative.allow(mce_from_issue     , !loan_inhibit_simple);
    LoanFromAlternative.allow(mce_from_year      , !loan_inhibit_simple);
    LoanFromAlternative.allow(mce_from_age       , !loan_inhibit_simple);
    LoanFromAlternative.allow(mce_from_retirement, !loan_inhibit_simple);
    LoanFromAlternative.enable(!loan_inhibit_simple);

    LoanToAlternative  .allow(mce_to_retirement  , !loan_inhibit_simple);
    LoanToAlternative  .allow(mce_to_year        , !loan_inhibit_simple);
    LoanToAlternative  .allow(mce_to_age         , !loan_inhibit_simple);
    LoanToAlternative  .allow(mce_to_maturity    , !loan_inhibit_simple);
    LoanToAlternative  .enable(!loan_inhibit_simple);

    LoanAmount         .enable(!loan_inhibit_simple);
    LoanFromAge        .enable(!loan_inhibit_simple && mce_from_age  == LoanFromAlternative);
    LoanFromDuration   .enable(!loan_inhibit_simple && mce_from_year == LoanFromAlternative);
    LoanToAge          .enable(!loan_inhibit_simple && mce_to_age    == LoanToAlternative);
    LoanToDuration     .enable(!loan_inhibit_simple && mce_to_year   == LoanToAlternative);

    TermRider.enable(database_->Query(DB_AllowTerm));
    TermRider.allow(mce_yes, database_->Query(DB_AllowTerm));

    bool enable_term = mce_yes == TermRider;
    bool specamt_indeterminate_for_term =
           mce_solve_specamt == SolveType
        || mce_sa_input_scalar != SpecifiedAmountStrategyFromIssue
        ;

    TermRiderUseProportion.enable(enable_term && !specamt_indeterminate_for_term);
    TermRiderUseProportion.allow(mce_yes, !specamt_indeterminate_for_term);
    bool term_is_proportional = mce_yes == TermRiderUseProportion;
    TermRiderAmount     .enable(enable_term && !term_is_proportional);
    TotalSpecifiedAmount.enable(enable_term &&  term_is_proportional);
    TermRiderProportion .enable(enable_term &&  term_is_proportional);

    TermAdjustmentMethod.allow(mce_adjust_base, enable_term);
    TermAdjustmentMethod.allow(mce_adjust_term, enable_term);
    TermAdjustmentMethod.allow(mce_adjust_both, enable_term);

    WaiverOfPremiumBenefit.enable(        database_->Query(DB_AllowWP));
    WaiverOfPremiumBenefit.allow(mce_yes, database_->Query(DB_AllowWP));
    AccidentalDeathBenefit.enable(        database_->Query(DB_AllowADD));
    AccidentalDeathBenefit.allow(mce_yes, database_->Query(DB_AllowADD));

    ChildRider       .enable(        database_->Query(DB_AllowChild));
    ChildRider       .allow(mce_yes, database_->Query(DB_AllowChild));
    ChildRiderAmount .enable(mce_yes == ChildRider);
    SpouseRider      .enable(        database_->Query(DB_AllowSpouse));
    SpouseRider      .allow(mce_yes, database_->Query(DB_AllowSpouse));
    SpouseRiderAmount.enable(mce_yes == SpouseRider);
    SpouseIssueAge   .enable(mce_yes == SpouseRider);
#if 0
// DATABASE !! Add spouse minimum and maximum issue ages, as well as
// minimum and maximum amounts for both spouse and child.
    SpouseIssueAge.minimum_and_maximum
        (static_cast<int>(database_->Query(DB_MinIssAge))
        ,static_cast<int>(database_->Query(DB_MaxIssAge))
        );
#endif // 0

    HoneymoonEndorsement .enable(        database_->Query(DB_AllowHoneymoon));
    HoneymoonEndorsement .allow(mce_yes, database_->Query(DB_AllowHoneymoon));
    PostHoneymoonSpread  .enable(mce_yes == HoneymoonEndorsement);
    HoneymoonValueSpread .enable(mce_yes == HoneymoonEndorsement);
    InforceHoneymoonValue.enable(mce_yes == HoneymoonEndorsement);

    bool solves_allowed = mce_life_by_life == RunOrder;

    bool enable_prem_and_specamt_solves =
            true
        &&  (
                mce_pmt_input_scalar == IndividualPaymentStrategy
            )
// TODO ?? WX PORT !! There is no employer payment strategy.
//        &&  (
//                mce_pmt_input_scalar == ErPmtStrategy
//            )
        &&  (
                mce_sa_input_scalar == SpecifiedAmountStrategyFromIssue
            ||  mce_sa_salary       == SpecifiedAmountStrategyFromIssue
            )
        ;

    SolveType.allow(mce_solve_none        , true);
    SolveType.allow(mce_solve_specamt     , solves_allowed && enable_prem_and_specamt_solves);
    SolveType.allow(mce_solve_ee_prem     , solves_allowed && enable_prem_and_specamt_solves);
    SolveType.allow(mce_solve_er_prem     , solves_allowed && enable_prem_and_specamt_solves);
    SolveType.allow(mce_solve_loan        , solves_allowed && loan_allowed);
    SolveType.allow(mce_solve_wd          , solves_allowed && wd_allowed);

    bool actually_solving = solves_allowed && mce_solve_none != SolveType;

    DeprecatedSolveFromWhich  .allow(mce_from_issue     , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_from_year      , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_from_age       , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_from_retirement, actually_solving);
    DeprecatedSolveFromWhich  .enable(actually_solving);

    DeprecatedSolveToWhich    .allow(mce_to_retirement  , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_to_year        , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_to_age         , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_to_maturity    , actually_solving);
    DeprecatedSolveToWhich    .enable(actually_solving);

    DeprecatedSolveTgtAtWhich .allow(mce_to_retirement  , actually_solving);
    DeprecatedSolveTgtAtWhich .allow(mce_to_year        , actually_solving);
    DeprecatedSolveTgtAtWhich .allow(mce_to_age         , actually_solving);
    DeprecatedSolveTgtAtWhich .allow(mce_to_maturity    , actually_solving);
    DeprecatedSolveTgtAtWhich .enable(actually_solving && mce_solve_for_non_mec != SolveTarget);

    SolveBeginYear .enable(actually_solving && mce_from_year == DeprecatedSolveFromWhich);
    SolveEndYear   .enable(actually_solving && mce_to_year   == DeprecatedSolveToWhich);
    SolveTargetYear.enable(actually_solving && mce_to_year   == DeprecatedSolveTgtAtWhich && mce_solve_for_non_mec != SolveTarget);

    SolveTargetYear.minimum(1);
    // INPUT !! The minimum 'SolveEndYear' and 'SolveTargetYear' set
    // here mean that a solve to or at retirement is a request, not a
    // command.
#if 0 // http://lists.nongnu.org/archive/html/lmi/2008-08/msg00036.html
    SolveBeginYear .minimum_and_maximum(0                         , years_to_maturity());
    SolveEndYear   .minimum_and_maximum(    SolveBeginYear.value(), years_to_maturity());
    SolveTargetYear.minimum_and_maximum(1 + SolveBeginYear.value(), years_to_maturity());
#endif // 0

    // INPUT !! Temporarily, existing -'Time' names are used where
    // -'Age' names would be clearer.
    SolveBeginTime .enable(actually_solving && mce_from_age == DeprecatedSolveFromWhich);
    SolveEndTime   .enable(actually_solving && mce_to_age   == DeprecatedSolveToWhich);
    SolveTargetTime.enable(actually_solving && mce_to_age   == DeprecatedSolveTgtAtWhich && mce_solve_for_non_mec != SolveTarget);

#if 0 // http://lists.nongnu.org/archive/html/lmi/2008-08/msg00036.html
    SolveBeginTime .minimum_and_maximum(issue_age()           , maturity_age());
    SolveEndTime   .minimum_and_maximum(SolveBeginTime.value(), maturity_age());
    SolveTargetTime.minimum_and_maximum(SolveBeginTime.value(), maturity_age());
#endif // 0

    SolveTarget.enable(actually_solving);
    SolveTarget.allow(mce_solve_for_endt     , actually_solving);
    SolveTarget.allow(mce_solve_for_target   , actually_solving);
    SolveTarget.allow(mce_solve_for_tax_basis, actually_solving);
    SolveTarget.allow(mce_solve_for_non_mec  , actually_solving && mce_solve_loan != SolveType);

    SolveBasis .enable(actually_solving && mce_solve_for_non_mec != SolveTarget);
    SolveBasis .allow(mce_gen_curr, actually_solving);
    SolveBasis .allow(mce_gen_guar, actually_solving);
    SolveBasis .allow(mce_gen_mdpt, actually_solving && is_subject_to_ill_reg(GleanedLedgerType_));

    SolveSeparateAccountBasis.enable(actually_solving && mce_solve_for_non_mec != SolveTarget);
    SolveSeparateAccountBasis.allow(mce_sep_full, actually_solving);
    SolveSeparateAccountBasis.allow(mce_sep_zero, actually_solving && allow_sep_acct);
    SolveSeparateAccountBasis.allow(mce_sep_half, actually_solving && allow_sep_acct && is_three_rate_nasd(GleanedLedgerType_));

    SolveTargetCashSurrenderValue.enable(actually_solving && mce_solve_for_target == SolveTarget);

    bool enable_reduce_to_avoid_mec =
            !(actually_solving && mce_solve_for_non_mec == SolveTarget)
        &&  !specamt_indeterminate
        ;
    AvoidMecMethod.allow(mce_reduce_prem, enable_reduce_to_avoid_mec);

    bool create_supplemental_report = mce_yes == CreateSupplementalReport;
    SupplementalReportColumn00.enable(create_supplemental_report);
    SupplementalReportColumn01.enable(create_supplemental_report);
    SupplementalReportColumn02.enable(create_supplemental_report);
    SupplementalReportColumn03.enable(create_supplemental_report);
    SupplementalReportColumn04.enable(create_supplemental_report);
    SupplementalReportColumn05.enable(create_supplemental_report);
    SupplementalReportColumn06.enable(create_supplemental_report);
    SupplementalReportColumn07.enable(create_supplemental_report);
    SupplementalReportColumn08.enable(create_supplemental_report);
    SupplementalReportColumn09.enable(create_supplemental_report);
    SupplementalReportColumn10.enable(create_supplemental_report);
    SupplementalReportColumn11.enable(create_supplemental_report);

    // TODO ?? EGREGIOUS_DEFECT: instead, don't offer these columns at all.
    std::vector<mcenum_report_column> weird_report_columns;
    weird_report_columns.push_back(mce_current_0_account_value_general_account    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_general_account );
    weird_report_columns.push_back(mce_current_0_account_value_separate_account   );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_separate_account);
    weird_report_columns.push_back(mce_current_0_account_value                    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value                 );
    weird_report_columns.push_back(mce_current_0_cash_surrender_value             );
    weird_report_columns.push_back(mce_guaranteed_0_cash_surrender_value          );

    typedef std::vector<mcenum_report_column>::const_iterator vrci;
    for(vrci i = weird_report_columns.begin(); i != weird_report_columns.end(); ++i)
        {
        SupplementalReportColumn00.allow(*i, home_office_only);
        SupplementalReportColumn01.allow(*i, home_office_only);
        SupplementalReportColumn02.allow(*i, home_office_only);
        SupplementalReportColumn03.allow(*i, home_office_only);
        SupplementalReportColumn04.allow(*i, home_office_only);
        SupplementalReportColumn05.allow(*i, home_office_only);
        SupplementalReportColumn06.allow(*i, home_office_only);
        SupplementalReportColumn07.allow(*i, home_office_only);
        SupplementalReportColumn08.allow(*i, home_office_only);
        SupplementalReportColumn09.allow(*i, home_office_only);
        SupplementalReportColumn10.allow(*i, home_office_only);
        SupplementalReportColumn11.allow(*i, home_office_only);
        }
}

/// Change values as required for consistency.
///
/// 'EffectiveDateToday' might have been a pushbutton, except that the
/// MVC implementation doesn't support pushbuttons: it's driven by
/// control state, and pushbuttons have no state. That may seem to be
/// a harsh limitation, but, as in this case, it can also suggest the
/// richer behavior that results from using a checkbox instead. While
/// the checkbox is checked, 'EffectiveDate' is bound to the current
/// date, even if the current date changes (e.g., because a saved file
/// is reloaded)--a behavior not available with a pushbutton. To force
/// today's date without leaving the date control bound to the current
/// date, the checkbox can be checked and then unchecked, producing
/// the same behavior as a pushbutton. This creates a relationship
/// between the checkbox and the date control that requires resetting
/// the latter's value as well as the ranges of other controls that
/// depend on it; that's not just Transmogrification or Harmonization,
/// but a different relationship that partakes of both, and should
/// perhaps be handled separately from both.
///
/// A default-constructed instance of this class initially has date of
/// birth set to the current date, which of course needs adjustment.
/// From issue age, the year of birth can be deduced approximately,
/// but the month or day cannot. In this case, a birthday is deemed to
/// occur on the effective date--as good an assumption as any, and the
/// simplest. Of course, when an instance is read from a file (either
/// deliberately, or because 'default.ill' exists), then the date of
/// birth is simply read from the file; the adjustment here has no
/// effect as long as the file is consistent.

void Input::DoTransmogrify()
{
    if(mce_yes == EffectiveDateToday)
        {
        EffectiveDate = calendar_date();
        // TODO ?? Consider factoring out date calculations and making
        // them conditional, if justified by measurement of their cost.
        DoHarmonize();
        }

    if(std::string::npos != global_settings::instance().pyx().find("old_inforce"))
        {
        InforceAsOfDate = add_years_and_months
            (EffectiveDate.value()
            ,InforceYear  .value()
            ,InforceMonth .value()
            ,true
            );
        LastMaterialChangeDate = add_years_and_months
            (EffectiveDate.value()
            ,InforceYear  .value() - InforceContractYear .value()
            ,InforceMonth .value() - InforceContractMonth.value()
            ,true
            );
        }
    else
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
        }

    // USER !! This is the credited rate as of the database date,
    // regardless of the date of illustration, because the database
    // does not yet store historical rates.
    if(mce_yes == UseCurrentDeclaredRate)
        {
        GeneralAccountRate = current_credited_rate(*database_);
        }

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

bool egregious_kludge = global_settings::instance().regression_testing();
// TODO ?? Nomen est omen.
if(!egregious_kludge)
  {
    // TODO ?? WX PORT !! Icky kludge.
    UseAverageOfAllFunds =
        (mce_fund_average  == FundChoiceType)
        ? mce_yes
        : mce_no
        ;
    OverrideFundManagementFee =
        (mce_fund_override == FundChoiceType)
        ? mce_yes
        : mce_no
        ;
  } // end if(!egregious_kludge)

    SetSolveDurations();
}

void Input::SetSolveDurations()
{
    switch(DeprecatedSolveTgtAtWhich.value())
        {
        case mce_to_year:
            {
            ; // Do nothing.
            }
            break;
        case mce_to_age:
            {
            SolveTargetYear = SolveTargetTime.value() - issue_age();
            }
            break;
        case mce_to_retirement:
            {
            SolveTargetYear = years_to_retirement();
            }
            break;
        case mce_to_maturity:
            {
            SolveTargetYear = years_to_maturity();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << DeprecatedSolveTgtAtWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(DeprecatedSolveFromWhich.value())
        {
        case mce_from_year:
            {
            ; // Do nothing.
            }
            break;
        case mce_from_age:
            {
            SolveBeginYear = SolveBeginTime.value() - issue_age();
            }
            break;
        case mce_from_issue:
            {
            SolveBeginYear = 0;
            }
            break;
        case mce_from_retirement:
            {
            SolveBeginYear = years_to_retirement();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << DeprecatedSolveFromWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(DeprecatedSolveToWhich.value())
        {
        case mce_to_year:
            {
            ; // Do nothing.
            }
            break;
        case mce_to_age:
            {
            SolveEndYear = SolveEndTime.value() - issue_age();
            }
            break;
        case mce_to_retirement:
            {
            SolveEndYear = years_to_retirement();
            }
            break;
        case mce_to_maturity:
            {
            SolveEndYear = years_to_maturity();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << DeprecatedSolveToWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Remove the following three lines (and <algorithm>) after fixing this:
    //   http://lists.nongnu.org/archive/html/lmi/2008-08/msg00036.html
    SolveTargetYear = std::max(0, std::min(years_to_maturity(), SolveTargetYear.value()));
    SolveBeginYear  = std::max(0, std::min(years_to_maturity(), SolveBeginYear .value()));
    SolveEndYear    = std::max(0, std::min(years_to_maturity(), SolveEndYear   .value()));

    SolveTargetTime = issue_age() + SolveTargetYear.value();
    SolveBeginTime  = issue_age() + SolveBeginYear .value();
    SolveEndTime    = issue_age() + SolveEndYear   .value();
}

#if 0

// What follows is a reimplementation of parts of the legacy system
// whose sole author is GWC.

// TODO ?? Much more work is needed here. Lines that seem unnecessary
// or haven't been tested are marked with four slashes.

void Input::WithdrawalChanged()
{
////    if(!IsFlagSet(wfFullyCreated))
////        {
////        return;
////        }
////    Changed();

    // Reinitialize vectors to bland defaults. Otherwise, since these
    // are always of length 100, values past the maturity year could
    // cause problems, because elsewhere we replicate the maturity-year
    // value through element 100 after setting the vectors from sequence
    // strings at certain times. For instance, setting death benefit
    // option "b" gives us 100 occurrences of "b"; if we change that to
    // "a" here, then a change from B to A is detected before the elements
    // after maturity are reset, and some policy forms don't allow such
    // a change. Obviously this is just a workaround for a bad design
    // that should be fixed eventually.
////    WD.assign(KludgeLength, r_wd(0.0));

////    transfer_mft_string
////        (WD
////        ,*DIAGNOSTICS
////        ,*WITHDRAWAL
////        ,Withdrawal
////        ,years_to_maturity()
////        ,static_cast<int>(IssueAge)
////        ,static_cast<int>(RetirementAge)
////        ,static_cast<int>(InforceYear)
////        ,calendar_date(EffDate).year()
////        ,false
////        );
////
////    if(0 == DIAGNOSTICS->GetTextLen())
////        {
////        std::string s;
////        s = realize_sequence_string_for_withdrawal();
////        if(s.empty())
////            {
////            return;
////            }
////        DIAGNOSTICS->SetText(s);
////        set_validity(*WITHDRAWAL, false);
////        return;
////        }

    InputSequence s
        (Withdrawal
        ,years_to_maturity()
        ,static_cast<int>(IssueAge)
        ,static_cast<int>(RetirementAge)
        ,static_cast<int>(InforceYear)
        ,calendar_date(EffDate).year()
        ,0
        );
    TransferWithdrawalInputSequenceToSimpleControls(s);
}

void Input::TransferWithdrawalSimpleControlsToInputSequence()
{
////    if(!IsFlagSet(wfFullyCreated))
////        {
////        return;
////        }
////
////    ClearFlag(wfFullyCreated);
////    TXferPropertyPage::TransferData(tdGetData);

    std::string s;

    switch(local_rep->WDFromWhich)
        {
        case enumerator_fromret:
            {
            if(IssueAge < RetirementAge)
// TODO ??            RetirementAge < database_->Query(DB_EndtAge)
                {
                s += "0, retirement";
                s += "; ";
                }
            }
            break;
        case enumerator_fromage:
            {
            if(IssueAge < local_rep->WDBegTime)
// TODO ??            local_rep->WDBegTime < database_->Query(DB_EndtAge)
                {
                s += "0, @" + value_cast<std::string>(local_rep->WDBegTime);
                s += "; ";
                }
            }
            break;
        case enumerator_fromyear:
            {
            if(0 < local_rep->WDBegTime)
// TODO ??                ( IssueAge + local_rep->WDBegTime
// TODO ??                < database_->Query(DB_EndtAge)
// TODO ??                )
                {
                s += "0, " + value_cast<std::string>(local_rep->WDBegTime);
                s += "; ";
                }
            }
            break;
        case enumerator_fromissue:
            {
            // Do nothing.
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << local_rep->WDFromWhich
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    static const int n = 1000;
    char z[n];
    WD_AMT->GetText(z, n);
    s += z;

    switch(local_rep->WDToWhich)
        {
        case enumerator_toret:
            {
            if(RetirementAge < database_->Query(DB_EndtAge))
                {
                s += ", retirement";
                s += "; 0";
                }
            }
            break;
        case enumerator_toage:
            {
            if(local_rep->WDEndTime < database_->Query(DB_EndtAge))
                {
                s += ", @" + value_cast<std::string>(local_rep->WDEndTime);
                s += "; 0";
                }
            }
            break;
        case enumerator_toyear:
            {
            if
                ( IssueAge + local_rep->WDEndTime
                < database_->Query(DB_EndtAge)
                )
                {
                s += ", " + value_cast<std::string>(local_rep->WDEndTime);
                s += "; 0";
                }
            }
            break;
        case enumerator_toend:
            {
            // Do nothing.
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << local_rep->WDToWhich
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    // Simplify the input sequence if WD is zero.
    if("0" == std::string(z))
        {
        s = "0";
        }

    WITHDRAWAL->SetText(s);
////    wxSafeYield();
////    SetFlag(wfFullyCreated);
}

void Input::TransferWithdrawalInputSequenceToSimpleControls
    (InputSequence const& s
    )
{
//////    if(!IsFlagSet(wfFullyCreated))
//////        {
//////        return;
//////        }

    bool is_valid = s.formatted_diagnostics().empty(); // && WITHDRAWAL->IsWindowEnabled(); // TODO ?? Kludge.
    std::vector<ValueInterval> const& intervals = s.interval_representation();
    bool is_simple =
            0 == intervals.size()
        ||  (
                (   1 == intervals.size()
                &&  0 == intervals[0].begin_duration
                )
            ||  (   2 == intervals.size()
                &&  (   0.0 == intervals[0].value_number
                    ||  0.0 == intervals[1].value_number
                    )
                )
            ||  (   3   == intervals.size()
                &&  0.0 == intervals[0].value_number
                &&  0.0 == intervals[2].value_number
                )
            )
        ;
    is_wd_simply_representable = is_valid && is_simple;
    if(!is_wd_simply_representable || 0 == intervals.size())
        {
        Enabler(); // Enablement of simple controls.

        ClearFlag(wfFullyCreated);
        WD_AMT->SetText("0");
        wxSafeYield();
        SetFlag(wfFullyCreated);

        return;
        }

    local_rep->sWD = WD[0];

    bool wd_in_second_interval =
            0.0 == WD[0]
        &&  2 <= intervals.size()
        ;
    int wd_interval;
    if(wd_in_second_interval)
        {
        local_rep->sWD = WD[intervals[1].begin_duration];
        wd_interval = 1;
        }
    else
        {
        local_rep->sWD = WD[0];
        wd_interval = 0;
        }

    switch(intervals[wd_interval].begin_mode)
        {
        case e_number_of_years:
            {
            local_rep->WDFromWhich = enumerator_fromyear;
            local_rep->WDBegTime = intervals[wd_interval].begin_duration;
            }
            break;
        case e_duration:
            {
            local_rep->WDFromWhich = enumerator_fromyear;
            local_rep->WDBegTime = intervals[wd_interval].begin_duration;
            }
            break;
        case e_attained_age:
            {
            local_rep->WDFromWhich = enumerator_fromage;
            local_rep->WDBegTime =
                  intervals[wd_interval].begin_duration
                + IssueAge
                ;
            }
            break;
        case e_inception:
            {
            local_rep->WDFromWhich = enumerator_fromyear;
            local_rep->WDBegTime = 0;
            }
            break;
        case e_retirement:
            {
            local_rep->WDFromWhich = enumerator_fromret;
            }
            break;
        case e_inforce:  // Fall through: not implemented.
        case e_maturity: // Fall through: illogical.
        default:
            {
            fatal_error()
                << "Case '"
                << intervals[wd_interval].begin_mode
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(intervals[wd_interval].end_mode)
        {
        case e_number_of_years:
            // Fall through. In this special case, number of years
            // and duration are the same, because the interval
            // must begin at duration zero.
        case e_duration:
            {
            local_rep->WDToWhich = enumerator_toyear;
            local_rep->WDEndTime = intervals[wd_interval].end_duration;
            }
            break;
        case e_attained_age:
            {
            local_rep->WDToWhich = enumerator_toage;
            local_rep->WDEndTime =
                intervals[wd_interval].end_duration + IssueAge
                ;
            }
            break;
        case e_retirement:
            {
            local_rep->WDToWhich = enumerator_toret;
            }
            break;
        case e_maturity:
            {
            local_rep->WDToWhich = enumerator_toend;
            }
            break;
        case e_inception: // Fall through: illogical.
        case e_inforce:   // Fall through: not implemented.
        default:
            {
            fatal_error()
                << "Case '"
                << intervals[wd_interval].end_mode
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

////    ClearFlag(wfFullyCreated);
////    Enabler();
////    EnableTransferToSequenceControls(false);
////    TXferPropertyPage::TransferData(tdSetData);
////    wxSafeYield();
////    EnableTransferToSequenceControls(true);
////    Enabler();
////    SetFlag(wfFullyCreated);
}

// TRICKY !! Use this with caution. It works only if all affected
// controls come last in the transfer struct. The legacy GUI library
// was not smart enough to skip through the transfer struct based on
// dynamic transfer enablement.
void Input::EnableTransferToSequenceControls(bool enable)
{
////    if(enable)
////        {
////        NEWLOAN        ->EnableTransfer();
////        WITHDRAWAL     ->EnableTransfer();
////        }
////    else
////        {
////        NEWLOAN        ->DisableTransfer();
////        WITHDRAWAL     ->DisableTransfer();
////        }
}

#endif // 0

