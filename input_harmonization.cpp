// Life-insurance illustration input--control harmonization.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "actuarial_table.hpp"          // e_reenter_upon_rate_reset
#include "alert.hpp"
#include "calendar_date.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "mc_enum_types_aux.hpp"        // is_subject_to_ill_reg(), is_three_rate_nasd()

#include <algorithm>                    // std::min(), std::max()
#include <utility>                      // std::pair

// Harmonization is physically separated for no better reason than to
// facilitate its development at a time when it frequently changes.

namespace
{
    std::string current_credited_rate(product_database const& database)
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

    GleanedLedgerType_ =
        static_cast<mcenum_ledger_type>
            (static_cast<int>
                (database_->Query(DB_LedgerType)
                )
            );
}

void Input::DoCustomizeInitialValues()
{
    // First of all, initialize obsolete variables exactly as the
    // schema prescribes, to accommodate admin extracts that don't
    // necessarily do so.

    // INPUT !! These are ready to expunge.
    DeathBenefitOptionFromIssue      = "A";
    DeathBenefitOptionFromRetirement = "A";
    IndividualPaymentAmount          = 0.0;
    IndividualPaymentMode            = "Annual";
    IndividualPaymentToAge           = 0;
    IndividualPaymentToAlternative   = "Retirement";
    IndividualPaymentToDuration      = 0;
    LoanAmount                       = 0.0;
    LoanFromAge                      = 0;
    LoanFromAlternative              = "Issue";
    LoanFromDuration                 = 0;
    LoanToAge                        = 0;
    LoanToAlternative                = "Retirement";
    LoanToDuration                   = 0;
    SpecifiedAmountFromIssue         = 0.0;
    SpecifiedAmountFromRetirement    = 0.0;
    WithdrawalAmount                 = 0.0;
    WithdrawalFromAge                = 0;
    WithdrawalFromAlternative        = "Issue";
    WithdrawalFromDuration           = 0;
    WithdrawalToAge                  = 0;
    WithdrawalToAlternative          = "Retirement";
    WithdrawalToDuration             = 0;

    // These are kept because their conditional-enablement code may be
    // useful someday.
    IndividualPaymentStrategy        = "PmtInputScalar";
    SpecifiedAmountStrategyFromIssue = "SAInputScalar";

    // These require special treatment. Initialize them to zero, as
    // the schema prescribes, just to be sure that prescription works;
    // then set them from the applicable date variables.
    InforceContractMonth             = 0;
    InforceContractYear              = 0;
    InforceMonth                     = 0;
    InforceYear                      = 0;
    set_inforce_durations_from_dates();

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

    bool wd_allowed = database_->Query(DB_AllowWd);
    bool loan_allowed = database_->Query(DB_AllowLoan);
    bool pref_loan_allowed = loan_allowed && database_->Query(DB_AllowPrefLoan);

    DefinitionOfLifeInsurance.allow(mce_gpt, database_->Query(DB_AllowGpt));
    DefinitionOfLifeInsurance.allow(mce_cvat, database_->Query(DB_AllowCvat));
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

    RetireesCanEnroll.enable(database_->Query(DB_AllowRetirees));

    GroupUnderwritingType.allow(mce_medical         , database_->Query(DB_AllowFullUw   ));
    GroupUnderwritingType.allow(mce_paramedical     , database_->Query(DB_AllowParamedUw));
    GroupUnderwritingType.allow(mce_nonmedical      , database_->Query(DB_AllowNonmedUw ));
    GroupUnderwritingType.allow(mce_simplified_issue, database_->Query(DB_AllowSimpUw   ));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database_->Query(DB_AllowGuarUw   ));

    bool part_mort_used = mce_yes == UsePartialMortality;

    PartialMortalityTable     .enable(part_mort_used);
    PartialMortalityMultiplier.enable(part_mort_used);

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

    IssueAge        .enable(mce_no  == UseDOB);
    DateOfBirth     .enable(mce_yes == UseDOB);

    // The ranges of both EffectiveDate and IssueAge are treated as
    // independent, to prevent one's value from affecting the other's
    // range and therefore possibly forcing its value to change. Thus,
    // if the maximum conceivable IssueAge is 100, then the earliest
    // permitted EffectiveDate is approximately the centennial of the
    // gregorian epoch.

#if 0
// TODO ?? Temporarily suppress this while exploring automatic-
// enforcement options in the skeleton trunk. Certain limits are
// interdependent:
//    issue_age      [0, omega - 1] (taken as an independent variable)
//    attained_age   [x, omega - 1]
//    duration       [0, omega-x-1]
    IssueAge.minimum_and_maximum
        (static_cast<int>(database_->Query(DB_MinIssAge))
        ,static_cast<int>(database_->Query(DB_MaxIssAge))
        );
//    RetirementAge.minimum_and_maximum(...
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

    InforceGlp                      .enable(mce_gpt == DefinitionOfLifeInsurance);
    InforceCumulativeGlp            .enable(mce_gpt == DefinitionOfLifeInsurance);
    InforceGsp                      .enable(mce_gpt == DefinitionOfLifeInsurance);
    InforceCumulativeGptPremiumsPaid.enable(mce_gpt == DefinitionOfLifeInsurance);

    bool non_mec = mce_no == InforceIsMec;

    InforceSevenPayPremium  .enable(non_mec);
    LastMaterialChangeDate  .enable(non_mec);
    InforceDcv              .enable(non_mec && mce_cvat == DefinitionOfLifeInsurance);
    InforceAvBeforeLastMc   .enable(non_mec);
    InforceLeastDeathBenefit.enable(non_mec);

    // These four variables, formerly independent, are now dependent:
    // set_inforce_durations_from_dates() calculates them from dates.
    // They're retained in the GUI (always disabled) only because some
    // end users have grown accustomed to them and want them kept.
    InforceYear         .enable(false);
    InforceMonth        .enable(false);
    InforceContractYear .enable(false);
    InforceContractMonth.enable(false);

    bool no_lapse_offered =
           0 != database_->Query(DB_NoLapseMinDur)
        || 0 != database_->Query(DB_NoLapseMinAge)
        ;
    InforceNoLapseActive            .enable(no_lapse_offered);
    InforceMonthlyNoLapsePremium    .enable(no_lapse_offered && mce_yes == InforceNoLapseActive);
    InforceCumulativeNoLapsePremium .enable(no_lapse_offered && mce_yes == InforceNoLapseActive);
    InforceCumulativeNoLapsePayments.enable(no_lapse_offered && mce_yes == InforceNoLapseActive);

    InforceCumulativeRopPayments.enable(database_->Query(DB_AllowDbo3));

    // It would be possible to enable 'InforceCumulativeSalesLoad' iff
    // 'DB_LoadRfdProportion' is nonzero in the inforce year. However,
    // analysis of database vector quantities is generally avoided in
    // this function, in the interest of simplicity and speed.
    //
    // Selectively enabling 'InforceSpecAmtLoadBase' would be even
    // more complicated: it would require inspecting not only the
    // database, but also a rate table.

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
    calendar_date reset_min(jdn_t(static_cast<int>(database_->Query(DB_CoiResetMinDate))));
    calendar_date reset_max(jdn_t(static_cast<int>(database_->Query(DB_CoiResetMaxDate))));
    reset_max = std::min(reset_max, most_recent_anniversary);
    if(!global_settings::instance().regression_testing())
        {
        LastCoiReentryDate.minimum_and_maximum(reset_min, reset_max);
        }
    LastCoiReentryDate.enable(e_reenter_upon_rate_reset == database_->Query(DB_CoiInforceReentry));

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
    // see what difference that makes. TAXATION !! Rethink that.

    SpecifiedAmountStrategyFromIssue.allow(mce_sa_input_scalar, !specamt_solve && !specamt_from_term_proportion);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_salary      , !specamt_solve && !specamt_from_term_proportion);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_maximum     , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_target      , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_mep         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_glp         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_gsp         , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_corridor    , !inhibit_premium_based_strategies);
    SpecifiedAmountStrategyFromIssue.enable(!specamt_solve && !specamt_from_term_proportion && mce_sa_input_scalar == SpecifiedAmountStrategyFromIssue);

    bool inhibit_sequence = specamt_solve || specamt_from_term_proportion;
    SpecifiedAmount.enable(!inhibit_sequence);

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

    IndividualPaymentStrategy.allow(mce_pmt_input_scalar, !inhibit_prem_simple && !prem_solve);
    IndividualPaymentStrategy.allow(mce_pmt_minimum     , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_target      , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_mep         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_glp         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_gsp         , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_corridor    , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.allow(mce_pmt_table       , !inhibit_prem_simple && !prem_solve || specamt_indeterminate);
    IndividualPaymentStrategy.enable(!inhibit_prem_simple && !prem_solve);

//    InsuredPremiumTableNumber.enable(mce_pmt_table == IndividualPaymentStrategy); // // INPUT !! Obsolete scalar alternative control.
// In the legacy system, that control, 'InsuredPremiumTableFactor',
// and their 'Corporation'- congeners were enabled iff a
// 'mce_pmt_table' strategy was selected in a scalar control--but
// no such scalar control was ported. For payment strategy, lmi offers
// only input sequences that are enabled by default.
//
// TODO ?? 'Payment' and 'CorporationPayment' should have certain payment
// strategies conditionally blocked. See 'inhibit_premium_based_strategies'
// and 'prem_indeterminate' above for possible conditions; an old note
// suggested
//   || specamt strategy is neither 'none' nor 'salary-based'
// Ideally, some or all strategy keywords would be blocked, or
// corresponding parts of the input-sequence editor would be disabled,
// only at durations that exhibit an actual conflict: e.g., a premium
// solve for the first ten years only shouldn't inhibit anything after
// the tenth year.
//
// At any rate, keywords should not be blocked when the control is
// disabled: see
//   http://lists.nongnu.org/archive/html/lmi/2010-07/msg00006.html

    Payment           .enable(mce_solve_ee_prem != SolveType);
    CorporationPayment.enable(mce_solve_er_prem != SolveType);

    GeneralAccountRateType .allow(mce_credited_rate , true);
    GeneralAccountRateType .allow(mce_earned_rate, mce_no == UseCurrentDeclaredRate && (anything_goes || database_->Query(DB_AllowGenAcctEarnRate)));

    SeparateAccountRateType.allow(mce_gross_rate, true);
    SeparateAccountRateType.allow(mce_net_rate  , anything_goes || database_->Query(DB_AllowSepAcctNetRate));

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
        &&  (   database_->Query(DB_AllowVlr)
            ||  anything_goes
            )
        );
    LoanRateType.allow(mce_variable_loan_rate, allow_vlr);
    LoanRate.enable(mce_variable_loan_rate == LoanRateType);

    UseAverageOfAllFunds.enable(!genacct_only);
    bool enable_custom_fund =
            !genacct_only
        &&  (   database_->Query(DB_AllowImfOverride)
            ||  home_office_only
            )
        ;
    OverrideFundManagementFee.enable(enable_custom_fund);

    InputFundManagementFee.enable(mce_yes == OverrideFundManagementFee || mce_fund_override == FundChoiceType);

// TODO ?? WX PORT !! There seems to be some confusion here. We have
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
// that anyway. INPUT !! See: http://savannah.nongnu.org/support/?104481
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

    bool wd_inhibit = wd_solve || wd_forbidden;

    Withdrawal.enable(!wd_inhibit);

    bool loan_solve = mce_solve_loan == SolveType;
    bool loan_forbidden = !loan_allowed;

    WithdrawToBasisThenLoan.enable(!wd_forbidden && !loan_forbidden);

    bool loan_inhibit = loan_solve || loan_forbidden;

    NewLoan.enable(!loan_inhibit);

    InforceRegularLoanValue    .enable(loan_allowed);
    InforcePreferredLoanValue  .enable(pref_loan_allowed);
    InforceRegularLoanBalance  .enable(loan_allowed);
    InforcePreferredLoanBalance.enable(pref_loan_allowed);

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

    WaiverOfPremiumBenefit.enable(        database_->Query(DB_AllowWp));
    WaiverOfPremiumBenefit.allow(mce_yes, database_->Query(DB_AllowWp));
    AccidentalDeathBenefit.enable(        database_->Query(DB_AllowAdb));
    AccidentalDeathBenefit.allow(mce_yes, database_->Query(DB_AllowAdb));

    ChildRider       .enable(        database_->Query(DB_AllowChildRider));
    ChildRider       .allow(mce_yes, database_->Query(DB_AllowChildRider));
    ChildRiderAmount .enable(mce_yes == ChildRider);
    SpouseRider      .enable(        database_->Query(DB_AllowSpouseRider));
    SpouseRider      .allow(mce_yes, database_->Query(DB_AllowSpouseRider));
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
// INPUT !! Obsolete scalar alternative control.
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

    SolveFromWhich  .allow(mce_from_issue     , actually_solving);
    SolveFromWhich  .allow(mce_from_year      , actually_solving);
    SolveFromWhich  .allow(mce_from_age       , actually_solving);
    SolveFromWhich  .allow(mce_from_retirement, actually_solving);
    SolveFromWhich  .enable(actually_solving);

    SolveToWhich    .allow(mce_to_retirement  , actually_solving);
    SolveToWhich    .allow(mce_to_year        , actually_solving);
    SolveToWhich    .allow(mce_to_age         , actually_solving);
    SolveToWhich    .allow(mce_to_maturity    , actually_solving);
    SolveToWhich    .enable(actually_solving);

    SolveTgtAtWhich .allow(mce_to_retirement  , actually_solving);
    SolveTgtAtWhich .allow(mce_to_year        , actually_solving);
    SolveTgtAtWhich .allow(mce_to_age         , actually_solving);
    SolveTgtAtWhich .allow(mce_to_maturity    , actually_solving);
    SolveTgtAtWhich .enable(actually_solving && mce_solve_for_non_mec != SolveTarget);

    SolveBeginYear .enable(actually_solving && mce_from_year == SolveFromWhich);
    SolveEndYear   .enable(actually_solving && mce_to_year   == SolveToWhich);
    SolveTargetYear.enable(actually_solving && mce_to_year   == SolveTgtAtWhich && mce_solve_for_non_mec != SolveTarget);

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
    SolveBeginTime .enable(actually_solving && mce_from_age == SolveFromWhich);
    SolveEndTime   .enable(actually_solving && mce_to_age   == SolveToWhich);
    SolveTargetTime.enable(actually_solving && mce_to_age   == SolveTgtAtWhich && mce_solve_for_non_mec != SolveTarget);

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

    SolveExpenseGeneralAccountBasis.enable(actually_solving && mce_solve_for_non_mec != SolveTarget);
    SolveExpenseGeneralAccountBasis.allow(mce_gen_curr, actually_solving);
    SolveExpenseGeneralAccountBasis.allow(mce_gen_guar, actually_solving);
    SolveExpenseGeneralAccountBasis.allow(mce_gen_mdpt, actually_solving && is_subject_to_ill_reg(GleanedLedgerType_));

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

    // TODO ?? EGREGIOUS_DEFECT: instead, don't offer these columns at all. See:
    //   http://lists.nongnu.org/archive/html/lmi/2009-09/msg00012.html
    std::vector<mcenum_report_column> weird_report_columns;
    weird_report_columns.push_back(mce_current_0_account_value_general_account    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_general_account );
    weird_report_columns.push_back(mce_current_0_account_value_separate_account   );
    weird_report_columns.push_back(mce_guaranteed_0_account_value_separate_account);
    weird_report_columns.push_back(mce_current_0_account_value                    );
    weird_report_columns.push_back(mce_guaranteed_0_account_value                 );
    weird_report_columns.push_back(mce_current_0_cash_surrender_value             );
    weird_report_columns.push_back(mce_guaranteed_0_cash_surrender_value          );

    bool enable_weirdness =
           0.0 < InforceGeneralAccountValue .value()
        && 0.0 < InforceSeparateAccountValue.value()
        ;

    typedef std::vector<mcenum_report_column>::const_iterator vrci;
    for(vrci i = weird_report_columns.begin(); i != weird_report_columns.end(); ++i)
        {
        SupplementalReportColumn00.allow(*i, enable_weirdness);
        SupplementalReportColumn01.allow(*i, enable_weirdness);
        SupplementalReportColumn02.allow(*i, enable_weirdness);
        SupplementalReportColumn03.allow(*i, enable_weirdness);
        SupplementalReportColumn04.allow(*i, enable_weirdness);
        SupplementalReportColumn05.allow(*i, enable_weirdness);
        SupplementalReportColumn06.allow(*i, enable_weirdness);
        SupplementalReportColumn07.allow(*i, enable_weirdness);
        SupplementalReportColumn08.allow(*i, enable_weirdness);
        SupplementalReportColumn09.allow(*i, enable_weirdness);
        SupplementalReportColumn10.allow(*i, enable_weirdness);
        SupplementalReportColumn11.allow(*i, enable_weirdness);
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
/// the latter's value as well as the ranges (and therefore, perhaps,
/// the values) of other controls that depend on it. That's not just
/// Transmogrification or Harmonization, but a different relationship
/// that partakes of both; to handle it properly, this function exits
/// early whenever 'EffectiveDate' is forced to change, causing
/// MvcModel::Reconcile() to propagate the change--in effect, as
/// though the user changed it directly in the GUI.
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
    if(mce_yes == EffectiveDateToday && calendar_date() != EffectiveDate)
        {
        EffectiveDate = calendar_date();
        return;
        }

    set_inforce_durations_from_dates();

    // USER !! This is the credited rate as of the database date,
    // regardless of the date of illustration, because the database
    // does not yet store historical rates.
    if(mce_yes == UseCurrentDeclaredRate)
        {
        GeneralAccountRate = current_credited_rate(*database_);
        }

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

    set_solve_durations();
}

void Input::set_solve_durations()
{
    switch(SolveTgtAtWhich.value())
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
                << SolveTgtAtWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(SolveFromWhich.value())
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
                << SolveFromWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(SolveToWhich.value())
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
                << SolveToWhich.value()
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

void Input::set_inforce_durations_from_dates()
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

