// Extended enumerations types.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: xenumtypes.hpp,v 1.8 2006-01-29 13:52:01 chicares Exp $

#ifndef xenumtypes_hpp
#define xenumtypes_hpp

#include "config.hpp"

#include "enums7702.hpp"
#include "xenum.hpp"

enum enum_yes_or_no
    {e_yes = 1
    ,e_no  = 0
    };
typedef xenum<enum_yes_or_no, 2> e_yes_or_no;

enum enum_gender
    {e_female
    ,e_male
    ,e_unisex
    };
typedef xenum<enum_gender, 3> e_gender;

enum enum_smoking
    {e_smoker
    ,e_nonsmoker
    ,e_unismoke
    };
typedef xenum<enum_smoking, 3> e_smoking;

enum enum_class
    {e_preferred
    ,e_standard
    ,e_rated
    ,e_ultrapref
    };
typedef xenum<enum_class, 4> e_class;

enum enum_dbopt
    {e_option1
    ,e_option2
    ,e_rop
    };
typedef xenum<enum_dbopt, 3> e_dbopt;

// Section 7702 recognizes only options 1 and 2.
enum enum_dbopt_7702
    {e_option1_for_7702
    ,e_option2_for_7702
    };
typedef xenum<enum_dbopt_7702, 2> e_dbopt_7702;

enum enum_mode
    {e_annual     =  1
    ,e_semiannual =  2
    ,e_quarterly  =  4
    ,e_monthly    = 12
    };
typedef xenum<enum_mode, 4> e_mode;

enum enum_basis
    {e_currbasis
    ,e_guarbasis
    ,e_mdptbasis
    // Keep this the last enumerator.
    ,n_illreg_bases
    };
typedef xenum<enum_basis, 3> e_basis;

enum enum_sep_acct_basis
    {e_sep_acct_full
    ,e_sep_acct_zero
    ,e_sep_acct_half
    // Keep this the last enumerator.
    ,n_sepacct_bases
    };
typedef xenum<enum_sep_acct_basis, 3> e_sep_acct_basis;

enum enum_rate_period
    {e_annual_rate
    ,e_monthly_rate
    // Keep this the last enumerator.
    ,n_rate_periods
    };
typedef xenum<enum_rate_period, 2> e_rate_period;

enum enum_run_basis
    {e_run_curr_basis
    ,e_run_guar_basis
    ,e_run_mdpt_basis
    ,e_run_curr_basis_sa_zero
    ,e_run_guar_basis_sa_zero
    ,e_run_curr_basis_sa_half
    ,e_run_guar_basis_sa_half
    };
typedef xenum<enum_run_basis, 7> e_run_basis;

enum enum_ledger_type
    {e_ill_reg
    ,e_nasd
    ,e_prospectus                   // Obsolete.
    ,e_group_private_placement
    ,e_offshore_private_placement
    ,e_ill_reg_private_placement    // Obsolete.
    ,e_individual_private_placement
    };
typedef xenum<enum_ledger_type, 7> e_ledger_type;

enum enum_amount_type
    {e_face
    ,e_prem
    };
typedef xenum<enum_amount_type, 2> e_amount_type;

enum enum_uw_basis
    {e_medical
    ,e_paramedical
    ,e_nonmedical
    ,e_simplifiedissue
    ,e_guaranteedissue
    };
typedef xenum<enum_uw_basis, 5> e_uw_basis;

enum enum_table_rating
    {e_table_none // =   0%
    ,e_table_a    // =  25%
    ,e_table_b    // =  50%
    ,e_table_c    // =  75%
    ,e_table_d    // = 100%
    ,e_table_e    // = 125%
    ,e_table_f    // = 150%
    ,e_table_h    // = 200%
    ,e_table_j    // = 250%
    ,e_table_l    // = 300%
    ,e_table_p    // = 400%
    };
typedef xenum<enum_table_rating, 11> e_table_rating;

enum enum_solve_type
    {e_solve_none
    ,e_solve_specamt
    ,e_solve_ee_prem
    ,e_solve_er_prem
    ,e_solve_loan
    ,e_solve_wd
    ,e_solve_ee_prem_dur
    ,e_solve_er_prem_dur
    ,e_solve_wd_then_loan
    };
typedef xenum<enum_solve_type, 9> e_solve_type;

enum enum_solve_target
    {e_solve_for_endt
    ,e_solve_for_target
    };
typedef xenum<enum_solve_target, 2> e_solve_target;

// TRICKY !! Be careful about changing these unused enumerators: they
// need to retain binary compatibility with obsolete enumerators elsewhere.

enum enum_solve_tgt_at
    {e_tgtatret
    ,e_tgtatyear
    ,e_tgtatage
    ,e_tgtatend
    };
typedef xenum<enum_solve_tgt_at, 4> e_solve_tgt_at;

enum enum_solve_from
    {e_fromissue
    ,e_fromyear
    ,e_fromage
    ,e_fromret
    };
typedef xenum<enum_solve_from, 4> e_solve_from;

enum enum_solve_to
    {e_toret
    ,e_toyear
    ,e_toage
    ,e_toend
    };
typedef xenum<enum_solve_to, 4> e_solve_to;

enum enum_kludge_from
    {e_kludge_fromissue
    ,e_kludge_fromyear
    ,e_kludge_fromage
    ,e_kludge_fromret
    };
typedef xenum<enum_kludge_from, 4> e_kludge_from;

enum enum_kludge_to
    {e_kludge_toret
    ,e_kludge_toyear
    ,e_kludge_toage
    ,e_kludge_toend
    };
typedef xenum<enum_kludge_to, 4> e_kludge_to;

enum enum_pmt_strategy
    {e_pmtinputscalar
    ,e_pmtinputvector
    ,e_pmtminimum
    ,e_pmttarget
    ,e_pmtmep
    ,e_pmtglp
    ,e_pmtgsp
    ,e_pmtcorridor
    ,e_pmttable
    };
typedef xenum<enum_pmt_strategy, 9> e_pmt_strategy;

enum enum_sa_strategy
    {e_sainputscalar
    ,e_sainputvector
    ,e_samaximum
    ,e_satarget
    ,e_samep
    ,e_saglp
    ,e_sagsp
    ,e_sacorridor
    ,e_sasalary
    };
typedef xenum<enum_sa_strategy, 9> e_sa_strategy;

// TODO ?? Don't understand the inputscalar/inputvector difference;
// it's prolly not needed.
enum enum_loan_strategy
    {e_loanasinput
    ,e_loanmaximum
    ,e_loaninputscalar
    ,e_loaninputvector
    };
typedef xenum<enum_loan_strategy, 4> e_loan_strategy;

// TODO ?? Don't understand the inputscalar/inputvector difference;
// it's prolly not needed.
enum enum_wd_strategy
    {e_wdasinput
    ,e_wdmaximum
    ,e_wdinputscalar
    ,e_wdinputvector
    };
typedef xenum<enum_wd_strategy, 4> e_wd_strategy;

enum enum_int_rate_type
    {e_netrate
    ,e_grossrate
    };
// TODO ?? See implementation file, which handles a deprecated enum-name.
typedef xenum<enum_int_rate_type, 3> e_int_rate_type;

enum enum_loan_rate_type
    {e_fixed_loan_rate
    ,e_variable_loan_rate
    };
typedef xenum<enum_loan_rate_type, 2> e_loan_rate_type;

enum enum_fund_input_method
    {e_fund_average
    ,e_fund_override
    ,e_fund_selection
    };
typedef xenum<enum_fund_input_method, 3> e_fund_input_method;

enum enum_run_order
    {e_life_by_life
    ,e_month_by_month
    };
typedef xenum<enum_run_order, 2> e_run_order;

enum enum_survival_limit
    {e_no_survival_limit
    ,e_survive_to_age
    ,e_survive_to_year
    ,e_survive_to_ex
    };
typedef xenum<enum_survival_limit, 4> e_survival_limit;

enum enum_term_adj_method
    {e_adjust_base
    ,e_adjust_term
    ,e_adjust_both
    };
typedef xenum<enum_term_adj_method, 3> e_term_adj_method;

enum enum_plan
    {e_sample1
    ,e_sample2
    };
typedef xenum<enum_plan, 2> e_plan;

enum enum_state
    {e_s_AL,e_s_AK,e_s_AZ,e_s_AR,e_s_CA,e_s_CO,e_s_CT,e_s_DE,e_s_DC,e_s_FL
    ,e_s_GA,e_s_HI,e_s_ID,e_s_IL,e_s_IN,e_s_IA,e_s_KS,e_s_KY,e_s_LA,e_s_ME
    ,e_s_MD,e_s_MA,e_s_MI,e_s_MN,e_s_MS,e_s_MO,e_s_MT,e_s_NE,e_s_NV,e_s_NH
    ,e_s_NJ,e_s_NM,e_s_NY,e_s_NC,e_s_ND,e_s_OH,e_s_OK,e_s_OR,e_s_PA,e_s_PR
    ,e_s_RI,e_s_SC,e_s_SD,e_s_TN,e_s_TX,e_s_UT,e_s_VT,e_s_VA,e_s_WA,e_s_WV
    ,e_s_WI,e_s_WY
    ,e_s_XX
    };
typedef xenum<enum_state, 53> e_state;

// ISO 3166-1-Alpha-2 as of 2000-08-21
enum enum_country
    {e_c_AD,e_c_AE,e_c_AF,e_c_AG,e_c_AI,e_c_AL,e_c_AM,e_c_AN,e_c_AO,e_c_AQ
    ,e_c_AR,e_c_AS,e_c_AT,e_c_AU,e_c_AW,e_c_AZ,e_c_BA,e_c_BB,e_c_BD,e_c_BE
    ,e_c_BF,e_c_BG,e_c_BH,e_c_BI,e_c_BJ,e_c_BM,e_c_BN,e_c_BO,e_c_BR,e_c_BS
    ,e_c_BT,e_c_BV,e_c_BW,e_c_BY,e_c_BZ,e_c_CA,e_c_CC,e_c_CD,e_c_CF,e_c_CG
    ,e_c_CH,e_c_CI,e_c_CK,e_c_CL,e_c_CM,e_c_CN,e_c_CO,e_c_CR,e_c_CU,e_c_CV
    ,e_c_CX,e_c_CY,e_c_CZ,e_c_DE,e_c_DJ,e_c_DK,e_c_DM,e_c_DO,e_c_DZ,e_c_EC
    ,e_c_EE,e_c_EG,e_c_EH,e_c_ER,e_c_ES,e_c_ET,e_c_FI,e_c_FJ,e_c_FK,e_c_FM
    ,e_c_FO,e_c_FR,e_c_GA,e_c_GB,e_c_GD,e_c_GE,e_c_GF,e_c_GH,e_c_GI,e_c_GL
    ,e_c_GM,e_c_GN,e_c_GP,e_c_GQ,e_c_GR,e_c_GS,e_c_GT,e_c_GU,e_c_GW,e_c_GY
    ,e_c_HK,e_c_HM,e_c_HN,e_c_HR,e_c_HT,e_c_HU,e_c_ID,e_c_IE,e_c_IL,e_c_IN
    ,e_c_IO,e_c_IQ,e_c_IR,e_c_IS,e_c_IT,e_c_JM,e_c_JO,e_c_JP,e_c_KE,e_c_KG
    ,e_c_KH,e_c_KI,e_c_KM,e_c_KN,e_c_KP,e_c_KR,e_c_KW,e_c_KY,e_c_KZ,e_c_LA
    ,e_c_LB,e_c_LC,e_c_LI,e_c_LK,e_c_LR,e_c_LS,e_c_LT,e_c_LU,e_c_LV,e_c_LY
    ,e_c_MA,e_c_MC,e_c_MD,e_c_MG,e_c_MH,e_c_MK,e_c_ML,e_c_MM,e_c_MN,e_c_MO
    ,e_c_MP,e_c_MQ,e_c_MR,e_c_MS,e_c_MT,e_c_MU,e_c_MV,e_c_MW,e_c_MX,e_c_MY
    ,e_c_MZ,e_c_NA,e_c_NC,e_c_NE,e_c_NF,e_c_NG,e_c_NI,e_c_NL,e_c_NO,e_c_NP
    ,e_c_NR,e_c_NU,e_c_NZ,e_c_OM,e_c_PA,e_c_PE,e_c_PF,e_c_PG,e_c_PH,e_c_PK
    ,e_c_PL,e_c_PM,e_c_PN,e_c_PR,e_c_PS,e_c_PT,e_c_PW,e_c_PY,e_c_QA,e_c_RE
    ,e_c_RO,e_c_RU,e_c_RW,e_c_SA,e_c_SB,e_c_SC,e_c_SD,e_c_SE,e_c_SG,e_c_SH
    ,e_c_SI,e_c_SJ,e_c_SK,e_c_SL,e_c_SM,e_c_SN,e_c_SO,e_c_SR,e_c_ST,e_c_SV
    ,e_c_SY,e_c_SZ,e_c_TC,e_c_TD,e_c_TF,e_c_TG,e_c_TH,e_c_TJ,e_c_TK,e_c_TM
    ,e_c_TN,e_c_TO,e_c_TP,e_c_TR,e_c_TT,e_c_TV,e_c_TW,e_c_TZ,e_c_UA,e_c_UG
    ,e_c_UM,e_c_US,e_c_UY,e_c_UZ,e_c_VA,e_c_VC,e_c_VE,e_c_VG,e_c_VI,e_c_VN
    ,e_c_VU,e_c_WF,e_c_WS,e_c_YE,e_c_YT,e_c_YU,e_c_ZA,e_c_ZM,e_c_ZW
    };
typedef xenum<enum_country, 239> e_country;

// enum enum_defn_life_ins defined in "enums7702.hpp".
typedef xenum<enum_defn_life_ins, 3> e_defn_life_ins;

// enum enum_mec_avoid_method defined in "enums7702.hpp".
typedef xenum<enum_mec_avoid_method, 3> e_mec_avoid_method;

// enum enum_defn_material_change defined in "enums7702.hpp".
typedef xenum<enum_defn_material_change, 5> e_defn_material_change;

enum enum_spread_method
    {e_spread_is_effective_annual
    ,e_spread_is_nominal_daily
    };
typedef xenum<enum_spread_method, 2> e_spread_method;

enum enum_coi_rate_method
    {e_coi_rate_subtractive
    ,e_coi_rate_exponential
    };
typedef xenum<enum_coi_rate_method, 2> e_coi_rate_method;

enum enum_anticipated_deduction
    {e_twelve_times_last
    ,e_to_next_anniversary
    ,e_to_next_modal_pmt_date
    ,e_eighteen_times_last
    };
typedef xenum<enum_anticipated_deduction, 4> e_anticipated_deduction;

enum enum_asset_charge_type
    {e_asset_charge_spread
    ,e_asset_charge_load
    };
typedef xenum<enum_asset_charge_type, 2> e_asset_charge_type;

enum enum_part_mort_table
    {e_default_part_mort_table
    };
typedef xenum<enum_part_mort_table, 1> e_part_mort_table;

enum enum_premium_table
    {e_default_premium_table
    };
typedef xenum<enum_premium_table, 1> e_premium_table;

// TODO ?? Not used yet.
enum enum_spread_freq
    {e_spread_annual     = 1
    ,e_spread_semiannual = 2
    ,e_spread_quarterly  = 4
    ,e_spread_monthly    = 12
    ,e_spread_daily      = 365
    };
typedef xenum<enum_spread_freq, 5> e_spread_freq;

// TODO ?? Obsolescent.
enum enum_post_ret_sa_strategy
    {e_obsolete_same_as_initial
    ,e_obsolete_varying
    ,e_obsolete_scalar
    ,e_obsolete_percent_of_initial
// TODO ?? These values would correspond to the old ihs enum.
//    {e_obsolete_same_as_initial    = 10
//    ,e_obsolete_varying            =  1
//    ,e_obsolete_scalar             =  2
//    ,e_obsolete_percent_of_initial = 11
    };
typedef xenum<enum_post_ret_sa_strategy, 4> e_post_ret_sa_strategy;

// Extended enumeration types aren't yet needed for these:

    enum e_increment_method
        {e_proportional
        ,e_progressive
        };
    enum e_allocation_method
        {e_input_allocation
        ,e_override_allocation
        };
    enum e_increment_account_preference
        {e_prefer_general_account
        ,e_prefer_separate_account
        };

    enum e_premium_tax_tier_period
        {e_premium_tax_tiering_by_policy_year
        ,e_premium_tax_tiering_by_calendar_year
        };

    enum e_waiver_charge_method
        {e_waiver_times_deductions
        ,e_waiver_times_naar
        };

// TODO ?? This stuff is a little funky....

enum e_premium_tax_state
    {e_ee_state
    ,e_er_state
    };

// TODO ?? This one seems especially funky.
enum e_modal_prem_type
    {e_monthly_deduction
    ,e_modal_nonmec
    ,e_modal_table
    };

enum e_smoking_or_tobacco
    {e_smoker_nonsmoker
    ,e_tobacco_nontobacco
    };

enum e_preferred_class_name
    {e_called_preferred
    ,e_called_select
    };

// Illustration reg requires midpoint basis and guaranteed premium.
bool is_subject_to_ill_reg(e_ledger_type const&);
bool is_subject_to_ill_reg(enum_ledger_type);
bool is_subject_to_ill_reg(double);

// NASD illustrations may have three interest bases (e.g. 0, 6, 12%)
// instead of only two (e.g. 0, 12%).
bool is_three_rate_nasd(e_ledger_type const&);
bool is_three_rate_nasd(enum_ledger_type);
bool is_three_rate_nasd(double);

void set_separate_bases_from_run_basis
    (e_run_basis const&      a_RunBasis
    ,e_basis&                a_ExpAndGABasis
    ,e_sep_acct_basis&       a_SABasis
    );

void set_run_basis_from_separate_bases
    (e_run_basis&            a_RunBasis
    ,e_basis const&          a_ExpAndGABasis
    ,e_sep_acct_basis const& a_SABasis
    );

#endif // xenumtypes_hpp

