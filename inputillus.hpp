// Input parameters.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: inputillus.hpp,v 1.7 2005-10-16 16:36:15 chicares Exp $

#ifndef inputillus_hpp
#define inputillus_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "expimp.hpp"
#include "inputs.hpp"
#include "streamable.hpp"
#include "xenumtypes.hpp"
#include "xrangetypes.hpp"

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

class LMI_EXPIMP IllusInputParms
    :virtual public streamable
    ,public InputParms
    ,public MemberSymbolTable<IllusInputParms>
{
    typedef std::map<std::string,std::string> string_map;
    typedef std::map<std::string,std::string>::const_iterator string_map_iterator;

  public:
    IllusInputParms();
    IllusInputParms(IllusInputParms const&);
    virtual ~IllusInputParms();

    IllusInputParms& operator=(IllusInputParms const&);
    bool operator==(IllusInputParms const&) const;

    void ascribe_members();

    void read(xml::node&);
    void write(xml::node&) const;
    int class_version() const;
    std::string xml_root_name() const;

    void propagate_changes_from_base_and_finalize(bool report_errors = true);
    void propagate_changes_to_base_and_finalize(bool report_errors = true);

    // Term rider can be specified either as a scalar or as a
    // proportion of a specified total (term + base) specified amount.
    // In the latter case, base specified amount generally needs to be
    // modified. However, due to rounding, transforming values from
    // one method to the other and back does not necessarily preserve
    // the original value. It is therefore useful sometimes to avoid
    // modifying the base specified amount by passing 'false' as the
    // 'aggressively' argument, for instance in a GUI when it may be
    // desirable to display the proportional term specified amount
    // without necessarily changing the base specified amount.
    void make_term_rider_consistent(bool aggressively = true);

    std::vector<std::string> realize_all_sequence_strings
        (bool report_errors = true
        );

    string_map const permissible_specified_amount_strategy_keywords();
    string_map const permissible_death_benefit_option_keywords();
    string_map const permissible_payment_strategy_keywords();
    string_map const permissible_payment_mode_keywords();

    std::string realize_sequence_string_for_add_on_monthly_custodial_fee();
    std::string realize_sequence_string_for_add_on_comp_on_assets();
    std::string realize_sequence_string_for_add_on_comp_on_premium();
    std::string realize_sequence_string_for_non_us_corridor_factor();
    std::string realize_sequence_string_for_partial_mortality_multiplier();
    std::string realize_sequence_string_for_current_coi_multiplier();
    std::string realize_sequence_string_for_current_coi_grading();
    std::string realize_sequence_string_for_cash_value_enhancement_rate();
    std::string realize_sequence_string_for_corp_tax_bracket();
    std::string realize_sequence_string_for_indv_tax_bracket();
    std::string realize_sequence_string_for_projected_salary();
    std::string realize_sequence_string_for_specified_amount();
    std::string realize_sequence_string_for_death_benefit_option();
    std::string realize_sequence_string_for_indv_payment();
    std::string realize_sequence_string_for_indv_payment_mode();
    std::string realize_sequence_string_for_corp_payment();
    std::string realize_sequence_string_for_corp_payment_mode();
    std::string realize_sequence_string_for_gen_acct_int_rate();
    std::string realize_sequence_string_for_sep_acct_int_rate();
    std::string realize_sequence_string_for_new_loan();
    std::string realize_sequence_string_for_withdrawal();
    std::string realize_sequence_string_for_flat_extra();
    std::string realize_sequence_string_for_policy_level_flat_extra();
    std::string realize_sequence_string_for_honeymoon_value_spread();
    std::string realize_sequence_string_for_premium_history();
    std::string realize_sequence_string_for_specamt_history();

// TODO ?? Data should be private.

    std::string AddonMonthlyCustodialFee;
    std::string AddonCompOnAssets;
    std::string AddonCompOnPremium;
    std::string NonUsCorridorFactor;
    std::string PartialMortalityMultiplier;
    std::string CurrentCoiMultiplier;
    std::string CurrentCoiGrading; // TODO ?? Obsolete: expunge.
    std::string CashValueEnhancementRate;
    std::string NetMortalityChargeHistory;

    std::string CorpTaxBracket;
    std::string IndvTaxBracket;
    std::string ProjectedSalary;
    std::string SpecifiedAmount;
    std::string DeathBenefitOption;
    std::string IndvPayment;
    std::string IndvPaymentMode;
    std::string CorpPayment;
    std::string CorpPaymentMode;
    std::string GenAcctIntRate;
    std::string SepAcctIntRate;
    std::string NewLoan;
    std::string Withdrawal;
    std::string FlatExtra;
    std::string PolicyLevelFlatExtra; // Useful for survivorship?
    std::string HoneymoonValueSpread;
    std::string PremiumHistory;
    std::string SpecamtHistory;
    std::string FundAllocations;

    e_yes_or_no              EffectiveDateToday              ;
    e_dbopt                  DeathBenefitOptionFromRetirement;
    e_dbopt                  DeathBenefitOptionFromIssue     ;
    r_nonnegative_double     SpecifiedAmountFromRetirement   ;
    r_nonnegative_double     SpecifiedAmountFromIssue        ;
//    mce_                     SpecifiedAmountStrategyFromRetirement; // TODO ?? Rethink this.
    e_sa_strategy            SpecifiedAmountStrategyFromIssue;
    e_mode                   IndividualPaymentMode           ;
    e_kludge_to              IndividualPaymentToAlternative  ;
    r_attained_age           IndividualPaymentToAge          ;
    r_duration               IndividualPaymentToDuration     ;
    r_nonnegative_double     IndividualPaymentAmount         ;
    e_pmt_strategy           IndividualPaymentStrategy       ;
    r_nonnegative_double     LoanAmount                      ;
    r_attained_age           LoanFromAge                     ;
    e_kludge_from            LoanFromAlternative             ;
    r_duration               LoanFromDuration                ;
    r_attained_age           LoanToAge                       ;
    e_kludge_to              LoanToAlternative               ;
    r_duration               LoanToDuration                  ;
    r_nonnegative_double     WithdrawalAmount                ;
    r_attained_age           WithdrawalFromAge               ;
    e_kludge_from            WithdrawalFromAlternative       ;
    r_duration               WithdrawalFromDuration          ;
    r_attained_age           WithdrawalToAge                 ;
    e_kludge_to              WithdrawalToAlternative         ;
    r_duration               WithdrawalToDuration            ;

// TODO ?? Obsolete:
    r_spec_amt      sSpecAmount ; // obsolescent
    e_dbopt         sDBOpt      ; // obsolescent
    e_dbopt         sRetDBOpt   ; // obsolescent
    r_pmt           sEePremium  ; // obsolescent
    e_mode          sEeMode     ; // obsolescent
    r_curr_int_rate sGenAcctRate; // obsolescent

// TODO ?? This stuff really ought to be moved to the base class.
// We wish we could make these references, but any_member can't
// handle that, at least not now.
    r_iss_age            Status_IssueAge         ;
    r_ret_age            Status_RetAge           ;
    e_gender             Status_Gender           ;
    e_smoking            Status_Smoking          ;
    e_class              Status_Class            ;
    e_yes_or_no          Status_HasWP            ;
    e_yes_or_no          Status_HasADD           ;
    e_yes_or_no          Status_HasTerm          ;
    r_nonnegative_double Status_TermAmt          ;
    r_nonnegative_double Status_TotalSpecAmt     ;
    r_proportion         Status_TermProportion   ;
    e_yes_or_no          Status_TermUseProportion;
    r_date               Status_DOB              ;
    e_yes_or_no          Status_UseDOB           ;
    r_date               Status_DOR              ;
    e_yes_or_no          Status_UseDOR           ;
    e_table_rating       Status_SubstdTable      ;

  private:
    void propagate_status_from_alii();
    void propagate_status_to_alii();
    void propagate_fund_allocations_from_string();
    void propagate_fund_allocations_to_string();
};

#endif // inputillus_hpp

