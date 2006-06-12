// Life-insurance illustration input--control harmonization.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: input_harmonization.cpp,v 1.23 2006-06-12 19:32:31 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "alert.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "inputillus.hpp"
#include "input_sequence.hpp"
#include "value_cast.hpp"
#include "xenum.hpp"

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

/// Implementation notes: general-account rate.
///
/// If the general-account interest-rate field holds the default value
/// for the former product before the product changed, then change its
/// contents to the new product's default value. What's tested is
/// literal equality, not equivalence: even typing a blank at the end
/// of the field makes in no longer equal to the default string.
/// Similarly, given
///   product X: credited rate 0.052
///   product Y: credited rate 0.037
/// if product X is selected and a rate of 0.037 is given, and the
/// product is then changed to Y, then the default-rate behavior is
/// in effect; and if the product is later changed back to X, then
/// the rate changes to X's default of 0.052.
///
/// Take the same action if the field is empty.
///
/// Otherwise, leave it alone, deeming it to represent intentional
/// user input that should be preserved--even if it exceeds the new
/// product's current credited rate and will therefore be disallowed.
///
/// This behavior seems complicated, but generally does exactly what
/// is desired. An alternative for future consideration is to add a
/// "use current rate" checkbox. Until lmi has a historical database,
/// that would only frustrate users running inforce or backdated
/// illustrations.

void Input::reset_database()
{
    // This early-exit condition has to fail the first time this
    // function is called, because the product name is implicitly
    // initialized to an empty string, which cannot match any actual
    // product.
    if
        (
            CachedProductName           == ProductName
        &&  CachedGender                == Gender
        &&  CachedUnderwritingClass     == UnderwritingClass
        &&  CachedSmoking               == Smoking
        &&  CachedIssueAge              == IssueAge
        &&  CachedGroupUnderwritingType == GroupUnderwritingType
        &&  CachedStateOfJurisdiction   == StateOfJurisdiction
        )
        {
        return;
        }

    std::string cached_credited_rate;

    if(database.get())
        {
        cached_credited_rate = current_credited_rate(*database);
        }

    CachedProductName           = ProductName          ;
    CachedGender                = Gender               ;
    CachedUnderwritingClass     = UnderwritingClass    ;
    CachedSmoking               = Smoking              ;
    CachedIssueAge              = IssueAge             ;
    CachedGroupUnderwritingType = GroupUnderwritingType;
    CachedStateOfJurisdiction   = StateOfJurisdiction  ;

    std::string const IhsProductName(ProductName            .str());
    e_gender const    IhsGender     (Gender                 .str());
    e_class const     IhsClass      (UnderwritingClass      .str());
    e_smoking const   IhsSmoker     (Smoking                .str());
    int               IhsIssueAge   (IssueAge             .value());
    e_uw_basis const  IhsUWBasis    (GroupUnderwritingType  .str());
    e_state const     IhsState      (StateOfJurisdiction    .str());

    // TODO ?? Can the copy be avoided?
    database = std::auto_ptr<TDatabase>
        (new TDatabase
            (IhsProductName
            ,IhsGender
            ,IhsClass
            ,IhsSmoker
            ,IhsIssueAge
            ,IhsUWBasis
            ,IhsState
            )
        );

    if
        (   GeneralAccountRate.value().empty()
        ||  cached_credited_rate == GeneralAccountRate.value()
        )
        {
        GeneralAccountRate = datum_string(current_credited_rate(*database));
        }
}

void Input::Harmonize()
{
    reset_database();

    bool anything_goes    = global_settings::instance().ash_nazg();
    bool home_office_only = global_settings::instance().mellon();

    bool allow_sep_acct = database->Query(DB_AllowSepAcct);
    bool allow_gen_acct = database->Query(DB_AllowGenAcct);

    bool sepacct_only = allow_sep_acct && !allow_gen_acct;
    bool genacct_only = allow_gen_acct && !allow_sep_acct;

    bool wd_allowed = database->Query(DB_AllowWD);
    bool loan_allowed = database->Query(DB_AllowLoan);

    DefinitionOfLifeInsurance.allow(mce_gpt, database->Query(DB_AllowGPT));
    DefinitionOfLifeInsurance.allow(mce_cvat, database->Query(DB_AllowCVAT));
    DefinitionOfLifeInsurance.allow(mce_noncompliant, database->Query(DB_AllowNo7702));

    if(mce_noncompliant == DefinitionOfLifeInsurance)
        {
        DefinitionOfMaterialChange.enable(false);
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

    bool enable_reduce_to_avoid_mec   = true;
/* TODO ?? Want something like this?
    bool enable_reduce_to_avoid_mec =
        (
            SpecifiedAmountStrategyFromIssue == mce_sainputscalar
        ||  SpecifiedAmountStrategyFromIssue == mce_sainputvector
        ||  SpecifiedAmountStrategyFromIssue == mce_sa_salary
        );
*/
    bool enable_increase_to_avoid_mec =
            DefinitionOfMaterialChange == mce_benefit_increase
        ||  DefinitionOfMaterialChange == mce_earlier_of_increase_or_unnecessary_premium
        ||  DefinitionOfMaterialChange == mce_adjustment_event
        ;
/* TODO ?? Want something like this?
    bool enable_increase_to_avoid_mec =
            (
                EePmtStrategy == mce_pmtinputscalar
            ||  EePmtStrategy == mce_pmtinputvector
            )
        &&  (
                ErPmtStrategy == mce_pmtinputscalar
            ||  ErPmtStrategy == mce_pmtinputvector
            )
// TODO ?? We could write it more simply this way if we transfered
// the DefnMaterialChange controls first....
//        &&
//            (
//                DefinitionOfMaterialChange == mce_benefit_increase
//            ||  DefinitionOfMaterialChange == mce_earlier_of_increase_or_unnecessary_premium
//            ||  DefinitionOfMaterialChange == mce_adjustment_event
//            )
        ;
*/
    if(DefinitionOfMaterialChange == mce_unnecessary_premium)
        {
        enable_increase_to_avoid_mec = false || anything_goes;
        }
    else if(DefinitionOfMaterialChange == mce_benefit_increase)
        {
        // Do nothing.
        }
    else if(DefinitionOfMaterialChange == mce_later_of_increase_or_unnecessary_premium)
        {
        enable_increase_to_avoid_mec = false || anything_goes;
        }
    else if(DefinitionOfMaterialChange == mce_earlier_of_increase_or_unnecessary_premium)
        {
        // Do nothing.
        }
    else if(DefinitionOfMaterialChange == mce_adjustment_event)
        {
        // Do nothing.
        }
    else
        {
        fatal_error()
            << "No option selected for definition of material change."
            << LMI_FLUSH
            ;
        }

    AvoidMecMethod.allow(mce_increase_specamt, enable_increase_to_avoid_mec);
    AvoidMecMethod.allow(mce_reduce_prem, enable_reduce_to_avoid_mec);

    MaximumNaar.enable(anything_goes);

    AmortizePremiumLoad.enable(database->Query(DB_AllowAmortPremLoad));
    ExtraCompensationOnAssets .enable(database->Query(DB_AllowExtraAssetComp));
    ExtraCompensationOnPremium.enable(database->Query(DB_AllowExtraPremComp));
    OffshoreCorridorFactor.enable(mce_noncompliant == DefinitionOfLifeInsurance);

    RetireesCanEnroll.enable(database->Query(DB_AllowRetirees));

    // TODO ?? There should be flags in the database to allow or
    // forbid paramedical and nonmedical underwriting; arbitrarily,
    // until they are added, those options are always inhibited.
    GroupUnderwritingType.allow(mce_medical, database->Query(DB_AllowFullUW));
    GroupUnderwritingType.allow(mce_paramedical, false);
    GroupUnderwritingType.allow(mce_nonmedical, false);
    GroupUnderwritingType.allow(mce_simplified_issue, database->Query(DB_AllowSimpUW));
    GroupUnderwritingType.allow(mce_guaranteed_issue, database->Query(DB_AllowGuarUW));

    bool part_mort_used = "Yes" == UsePartialMortality;

    PartialMortalityTable     .enable(part_mort_used);
    PartialMortalityMultiplier.enable(part_mort_used);

    CurrentCoiMultiplier      .enable(home_office_only);
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
            database->Query(DB_AllowExpRating)
        &&  part_mort_used
        &&  mce_month_by_month == RunOrder
        ;
    UseExperienceRating.enable(enable_experience_rating);

    // TODO ?? These shouldn't need to depend on 'enable_experience_rating';
    // instead, 'UseExperienceRating' should be transmogrified if it's not
    // enabled.
    ExperienceRatingInitialKFactor.enable
        (   enable_experience_rating
        &&  "Yes" == UseExperienceRating
        );
    OverrideExperienceReserveRate.enable
        (   enable_experience_rating
        &&  "Yes" == UseExperienceRating
        );
    ExperienceReserveRate.enable
        (   enable_experience_rating
        &&  "Yes" == UseExperienceRating
        &&  "Yes" == OverrideExperienceReserveRate
        );
    InforceExperienceReserve.enable
        (   enable_experience_rating
        &&  "Yes" == UseExperienceRating
        );
    NetMortalityChargeHistory.enable
        (   enable_experience_rating
        &&  "Yes" == UseExperienceRating
        );

    EffectiveDate.enable("No" == EffectiveDateToday);

    IssueAge        .enable("No"  == DeprecatedUseDOB);
    DateOfBirth     .enable("Yes" == DeprecatedUseDOB);

    RetirementAge   .enable("No"  == DeprecatedUseDOR);
    DateOfRetirement.enable("Yes" == DeprecatedUseDOR);

    UnderwritingClass.allow(mce_ultrapreferred, database->Query(DB_AllowUltraPrefClass));
    UnderwritingClass.allow(mce_preferred     , database->Query(DB_AllowPreferredClass));

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
    UnderwritingClass.allow(mce_rated, database->Query(DB_AllowSubstdTable));

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

    // Can't have a non-US country multiplier other than unity in a US state.
    bool allow_custom_coi_multiplier =
            mce_c_US != Country
        ||  mce_s_XX == StateOfJurisdiction
            ;
    OverrideCoiMultiplier.enable(allow_custom_coi_multiplier);
    CountryCoiMultiplier.enable
        (   allow_custom_coi_multiplier
        &&  "Yes" == OverrideCoiMultiplier
        );
    FlatExtra.enable(database->Query(DB_AllowFlatExtras));

    BlendGender.enable(database->Query(DB_AllowMortBlendSex));
    bool blend_mortality_by_gender = "Yes" == BlendGender;

    BlendSmoking.enable(database->Query(DB_AllowMortBlendSmoke));
    bool blend_mortality_by_smoking = "Yes" == BlendSmoking;

    MaleProportion     .enable(blend_mortality_by_gender);
    NonsmokerProportion.enable(blend_mortality_by_smoking);

    bool allow_gender_distinct = database->Query(DB_AllowSexDistinct);
    bool allow_unisex          = database->Query(DB_AllowUnisex);

    Gender.allow(mce_female, !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_male  , !blend_mortality_by_gender && allow_gender_distinct);
    Gender.allow(mce_unisex,  blend_mortality_by_gender || allow_unisex);

    bool allow_smoker_distinct = database->Query(DB_AllowSmokeDistinct);
    bool allow_unismoke        = database->Query(DB_AllowUnismoke);

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
        ||  mce_solve_ee_prem_dur == SolveType
        ||  mce_solve_er_prem     == SolveType
        ||  mce_solve_er_prem_dur == SolveType
        ||  mce_reduce_prem       == AvoidMecMethod
        );

    bool specamt_solve = mce_solve_specamt == SolveType;

    bool specamt_from_term_proportion =
           database->Query(DB_AllowTerm)
        && "Yes" == TermRiderUseProportion
        && "Yes" == TermRider
        ;

    bool inhibit_premium_based_strategies =
           prem_indeterminate
        || specamt_solve
        || specamt_from_term_proportion
        ;

    bool salary_used =
// TODO ?? WX PORT !! Figure out how to handle the next line:
//           mce_sa_salary == VectorSpecifiedAmountStrategy[0]
false // Silly workaround for now.
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
    SpecifiedAmountStrategyFromIssue.allow(mce_sa_input_vector, false); // TODO ?? OBSOLETE.
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

    bool never_retire = database->Query(DB_EndtAge) <= RetirementAge.value();
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
    DeathBenefitOptionFromRetirement.allow(mce_rop    , is_dbopt_simply_representable && database->Query(DB_AllowDBO3));
    DeathBenefitOptionFromIssue     .allow(mce_option1, is_dbopt_simply_representable && !never_retire);
    DeathBenefitOptionFromIssue     .allow(mce_option2, is_dbopt_simply_representable && !never_retire && (database->Query(DB_AllowChangeToDBO2) || mce_option2 == DeathBenefitOptionFromRetirement));
    DeathBenefitOptionFromIssue     .allow(mce_rop    , is_dbopt_simply_representable && !never_retire && database->Query(DB_AllowDBO3));

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

    bool prem_solve =
            mce_solve_ee_prem     == SolveType
        ||  mce_solve_ee_prem_dur == SolveType
        ;

    // Many payment strategies are forbidden if specamt is a function
    // of payment.
    bool specamt_indeterminate =
            mce_solve_specamt == SolveType
        ||  AvoidMecMethod == mce_increase_specamt
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
    IndividualPaymentStrategy.allow(mce_pmt_input_vector, !inhibit_prem_simple && false);
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
    InsuredPremiumTableFactor.enable(mce_pmt_table == IndividualPaymentStrategy);

    Payment           .enable(mce_solve_ee_prem != SolveType);
    CorporationPayment.enable(mce_solve_er_prem != SolveType);

    IndividualPaymentMode.allow(mce_annual    , true);
    IndividualPaymentMode.allow(mce_semiannual, true);
    IndividualPaymentMode.allow(mce_quarterly , true);
    IndividualPaymentMode.allow(mce_monthly   , true);

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

// TODO ?? Change the legacy enumerators in the calculations library.
// They conflate disparate concepts. Here's what is really meant:
//
// genacct: legacy system offered only credited
//   earned and credited are conceivable
//   but earned is suppressed for compliance reasons
//   and earned is defectively called gross
//   net is absurd because it's called credited
//
// sepacct: legacy system offered only gross
//   gross and net are conceivable
//   but net is suppressed for compliance reasons
//   credited is absurd because it's called net
//
// The compliance reasons don't seem sensible, but that's another
// matter; at any rate, they belong in the product database.

    GeneralAccountRateType .allow(mce_gross_rate, anything_goes);
    GeneralAccountRateType .allow(mce_cred_rate , true);
    GeneralAccountRateType .allow(mce_net_rate  , false);

    SeparateAccountRateType.allow(mce_gross_rate, true);
    SeparateAccountRateType.allow(mce_cred_rate , false);
    SeparateAccountRateType.allow(mce_net_rate  , anything_goes);

    bool CurrIntRateSolve = false; // May be useful someday.
    GeneralAccountRate .enable(!CurrIntRateSolve);
    SeparateAccountRate.enable(!CurrIntRateSolve);

    // TODO ?? VLR not yet implemented.
    bool allow_vlr =
        (   loan_allowed
        &&  (   database->Query(DB_AllowVLR)
            ||  anything_goes
            )
        );
    LoanRateType.allow(mce_variable_loan_rate, allow_vlr);
    LoanRate.enable(mce_variable_loan_rate == LoanRateType);

    UseAverageOfAllFunds.enable(!genacct_only);
    bool enable_custom_fund =
            !genacct_only
        &&  (   database->Query(DB_AllowCustomFund)
            ||  home_office_only
            )
        ;
    OverrideFundManagementFee.enable(enable_custom_fund);

    InputFundManagementFee.enable("Yes" == OverrideFundManagementFee || mce_fund_override == FundChoiceType);

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

    bool wd_solve =
        (
            mce_solve_wd           == SolveType
        ||  mce_solve_wd_then_loan == SolveType
        );
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

    TermRider.enable(database->Query(DB_AllowTerm));

    bool enable_term = "Yes" == TermRider;

    TermAdjustmentMethod.allow(mce_adjust_base, enable_term);
    TermAdjustmentMethod.allow(mce_adjust_term, enable_term);
    TermAdjustmentMethod.allow(mce_adjust_both, enable_term);

/* TODO ??
      bool specamt_indeterminate_for_term =
           mce_solve_specamt == SolveType
        || mce_sa_input_scalar != SpecifiedAmountStrategyFromIssue
        ;

    TermRiderUseProportion.enable(enable_term && !specamt_indeterminate_for_term);

// TODO ?? WX PORT !! Huh? A yes-no radiobox, instead of a checkbox?
// This doesn't seem to be in the legacy interface.
//    TermRiderUseProportion.allow();

// wx: TermProportion
    if(specamt_indeterminate_for_term && "Yes" == TermRiderUseProportion)
        {
        TERM_USE_AMOUNT     ->SetCheck(BF_CHECKED);
        }
    TERM_USE_AMOUNT     ->EnableWindow(enable_term);

    bool term_use_amount = BF_CHECKED == TERM_USE_AMOUNT->GetCheck();
    LMI_ASSERT(term_use_amount || BF_CHECKED == TermRiderUseProportion->GetCheck());
    TERM_SPECAMT        ->EnableWindow(enable_term &&  term_use_amount);
    TotalSpecifiedAmount       ->EnableWindow(enable_term && !term_use_amount);
    TERM_PROPORTION     ->EnableWindow(enable_term && !term_use_amount);
*/

    WaiverOfPremiumBenefit.enable(database->Query(DB_AllowWP));
    AccidentalDeathBenefit.enable(database->Query(DB_AllowADD));

    // TODO ?? Logic differs from term rider handling above.
    // Which is better? Check it out. For term, choose a policy
    // form that allows term, elect the term rider, and fill in
    // a nonzero amount; then change to a policy form that does
    // not allow term, and return to the rider tab: the term
    // amount has been changed to zero. For spouse or child,
    // the amount is left as it was, but grayed out. I believe
    // the latter behavior is better, but I hesitate to change
    // the code above for term rider, because I don't know
    // whether a nonzero amount for a nonelected rider has any
    // actual effect (it shouldn't, but I don't know what really
    // happens).

    ChildRider       .enable(database->Query(DB_AllowChild));
    ChildRiderAmount .enable("Yes" == ChildRider);
    SpouseRider      .enable(database->Query(DB_AllowSpouse));
    SpouseRiderAmount.enable("Yes" == SpouseRider);
    SpouseIssueAge   .enable("Yes" == SpouseRider);


    HoneymoonEndorsement .enable(database->Query(DB_AllowHoneymoon));
    PostHoneymoonSpread  .enable("Yes" == HoneymoonEndorsement);
    HoneymoonValueSpread .enable("Yes" == HoneymoonEndorsement);

    // TODO ?? Is this a useful innovation?
    // If so, should it propagate to other inforce fields?
    InforceHoneymoonValue.enable("Yes" == HoneymoonEndorsement);

    bool loan_solve_allowed = loan_allowed && home_office_only;  // TODO ?? Until we fix loan calculations.
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
    SolveType.allow(mce_solve_loan        , solves_allowed && loan_solve_allowed);
    SolveType.allow(mce_solve_wd          , solves_allowed && wd_allowed);
    SolveType.allow(mce_solve_ee_prem_dur , solves_allowed && false);
    SolveType.allow(mce_solve_er_prem_dur , solves_allowed && false);
    SolveType.allow(mce_solve_wd_then_loan, solves_allowed && wd_allowed && loan_solve_allowed);

    bool actually_solving = solves_allowed && mce_solve_none != SolveType;

    DeprecatedSolveFromWhich  .allow(mce_solve_from_issue     , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_solve_from_year      , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_solve_from_age       , actually_solving);
    DeprecatedSolveFromWhich  .allow(mce_solve_from_retirement, actually_solving);
    DeprecatedSolveFromWhich  .enable(actually_solving);

    DeprecatedSolveToWhich    .allow(mce_solve_to_retirement  , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_solve_to_year        , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_solve_to_age         , actually_solving);
    DeprecatedSolveToWhich    .allow(mce_solve_to_maturity    , actually_solving);
    DeprecatedSolveToWhich    .enable(actually_solving);

    DeprecatedSolveTgtAtWhich .allow(mce_target_at_retirement , actually_solving && mce_solve_for_target == SolveTarget);
    DeprecatedSolveTgtAtWhich .allow(mce_target_at_year       , actually_solving && mce_solve_for_target == SolveTarget);
    DeprecatedSolveTgtAtWhich .allow(mce_target_at_age        , actually_solving && mce_solve_for_target == SolveTarget);
    DeprecatedSolveTgtAtWhich .allow(mce_target_at_maturity   , actually_solving && mce_solve_for_target == SolveTarget);
    DeprecatedSolveTgtAtWhich .enable(actually_solving && mce_solve_for_target == SolveTarget);

// TODO ?? This is a mess. Here's what's really needed:
//   separate variables for {begin, end, target} X {age, duration}
//   remove ancient code that mangles this stuff, e.g. InputParms::SetSolveDurations()
//
// This is what things should probably look like:
//
//    SolveBeginYear .enable(actually_solving && mce_solve_from_year == DeprecatedSolveFromWhich);
//    SolveEndYear   .enable(actually_solving && mce_solve_to_year   == DeprecatedSolveToWhich);
//    SolveTargetYear.enable(actually_solving && mce_target_at_year  == DeprecatedSolveTgtAtWhich && mce_solve_for_target == SolveTarget);
//
//    SolveBeginAge  .enable(actually_solving && mce_solve_from_age == DeprecatedSolveFromWhich);
//    SolveEndAge    .enable(actually_solving && mce_solve_to_age   == DeprecatedSolveToWhich);
//    SolveTargetAge .enable(actually_solving && mce_target_at_age  == DeprecatedSolveTgtAtWhich && mce_solve_for_target == SolveTarget);
//
// but for now, as a temporary workaround, these '-Year' variables are mapped
// to 'age' controls, merely so that we can inhibit them...
    SolveBeginYear .enable(false);
    SolveEndYear   .enable(false);
    SolveTargetYear.enable(false);
// ...and this actually 'works' for duration, because legacy code translates
// it so that it seems to do the right thing:
    SolveBeginTime .enable(actually_solving && mce_solve_from_year == DeprecatedSolveFromWhich);
    SolveEndTime   .enable(actually_solving && mce_solve_to_year   == DeprecatedSolveToWhich);
    SolveTargetTime.enable(actually_solving && mce_target_at_year  == DeprecatedSolveTgtAtWhich && mce_solve_for_target == SolveTarget);

    SolveTarget.enable(actually_solving);
    SolveTarget.allow(mce_solve_for_endt  , actually_solving);
    SolveTarget.allow(mce_solve_for_target, actually_solving);

    SolveBasis .enable(actually_solving);
    SolveBasis .allow(mce_curr_basis, actually_solving);
    SolveBasis .allow(mce_guar_basis, actually_solving);
    SolveBasis .allow(mce_mdpt_basis, actually_solving && is_subject_to_ill_reg(database->Query(DB_LedgerType)));

    SolveSeparateAccountBasis.enable(actually_solving);
    SolveSeparateAccountBasis.allow(mce_sep_acct_full, actually_solving && allow_sep_acct);
    SolveSeparateAccountBasis.allow(mce_sep_acct_zero, actually_solving && allow_sep_acct);
    SolveSeparateAccountBasis.allow(mce_sep_acct_half, actually_solving && allow_sep_acct && is_three_rate_nasd(database->Query(DB_LedgerType)));

    SolveTargetCashSurrenderValue.enable(actually_solving && mce_solve_for_target == SolveTarget);
    DeprecatedSolveTgtAtWhich    .enable(actually_solving && mce_solve_for_target == SolveTarget);

    Transmogrify(); // TODO ?? This obviously doesn't belong here.
}

void Input::Transmogrify()
{
    bool use_anb = database->Query(DB_AgeLastOrNearest);

    // TODO ?? This syntax seems ugly. Should there be a ctor
    // calendar_date(int)? Alternatively, should tnr_date use a
    // calendar_date instead of an int?
    calendar_date effective_date;
    effective_date.julian_day_number(EffectiveDate.value());

    calendar_date date_of_birth;
    date_of_birth.julian_day_number(DateOfBirth.value());

    if("Yes" == DeprecatedUseDOB)
        {
        IssueAge = calculate_age(date_of_birth, effective_date, use_anb);
        }
    else
        {
        // Note on initial values.
        //
        // A default-constructed instance of this class initially has
        // date of birth set to the current date, which of course
        // requires adjustment. From issue age, the year of birth can
        // be deduced approximately, but the month or day cannot. In
        // this case, a birthday is deemed to occur on the effective
        // date--as good an assumption as any, and the simplest.
        //
        // Of course, when an instance is read from a file (either
        // deliberately, or because 'default.ill' exists), then the
        // date of birth is simply read from the file; the adjustment
        // here has no effect as long as the file is consistent.

        int apparent_age = calculate_age
            (date_of_birth
            ,effective_date
            ,use_anb
            );
        date_of_birth.add_years(apparent_age - IssueAge.value(), use_anb);
        DateOfBirth = date_of_birth.julian_day_number();
        }
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
////        ,YearsToMaturity()
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
////        DIAGNOSTICS->SetText(s.c_str());
////        set_validity(*WITHDRAWAL, false);
////        return;
////        }

    InputSequence s
        (Withdrawal
        ,YearsToMaturity()
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
// TODO ??            RetirementAge < database->Query(DB_EndtAge)
                {
                s += "0, retirement";
                s += "; ";
                }
            }
            break;
        case enumerator_fromage:
            {
            if(IssueAge < local_rep->WDBegTime)
// TODO ??            local_rep->WDBegTime < database->Query(DB_EndtAge)
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
// TODO ??                < database->Query(DB_EndtAge)
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
            if(RetirementAge < database->Query(DB_EndtAge))
                {
                s += ", retirement";
                s += "; 0";
                }
            }
            break;
        case enumerator_toage:
            {
            if(local_rep->WDEndTime < database->Query(DB_EndtAge))
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
                < database->Query(DB_EndtAge)
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

    WITHDRAWAL->SetText(s.c_str());
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

