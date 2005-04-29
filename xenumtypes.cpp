// Extended enumeration types.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: xenumtypes.cpp,v 1.2 2005-04-29 16:14:08 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#ifdef BORLAND_502_IDE_BLACK_MAGIC
// This, uh, works. I don't know why. I suspect that these might be
// 'guiding declarations', but I'm not a C++ prehistorian. This
// workaround seems necessary only with the bc++5.02 IDE, but not
// with the command-line compiler. It does not seem necessary with
// gui builds in any case. Such is magic.
class magic_stream;
class magic_class;
magic_stream& operator>> (magic_stream&, magic_class&);
magic_stream& operator<< (magic_stream&, magic_class const&);
#endif // BORLAND_502_IDE_BLACK_MAGIC

#include "xenumtypes.hpp"

#include <sstream>

#ifdef LMI_EXPLICIT_INSTANTIATION

#   define OK_TO_COMPILE_XENUM_CPP
#   include "xenum.cpp"
#   undef OK_TO_COMPILE_XENUM_CPP

// Special case: this type is of boolean essence, and we want it to accept
// "1" or "0" as well as "Yes" or "No" for assignment.
LMI_SPECIALIZATION void xenum<enum_yes_or_no, 2>::set_value(std::string const& s)
{
    int const expanded_N = 4;

    static enum_yes_or_no const expanded_enumerators[] =
        {e_yes
        ,e_no
        ,e_yes
        ,e_no
        };

    static char const*const expanded_names[] =
        {"Yes"
        ,"No"
        ,"1"
        ,"0"
        };

    std::ptrdiff_t index =
        std::find
            (expanded_names
            ,expanded_names + expanded_N
            ,s
            )
        - expanded_names
        ;

    if(expanded_N == index)
        {
        std::ostringstream error;
        error
            << s
            << " out of range for type "
            << typeid(enum_yes_or_no).name()
            ;
        throw std::range_error(error.str());
        }
    else
        {
        representation = expanded_enumerators[index];
        }
}

template class xenum<enum_yes_or_no, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_yes_or_no, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_yes_or_no, 2> const&);

template class xenum<enum_gender, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_gender, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_gender, 3> const&);

template class xenum<enum_smoking, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_smoking, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_smoking, 3> const&);

template class xenum<enum_class, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_class, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_class, 4> const&);

template class xenum<enum_dbopt, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_dbopt, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_dbopt, 3> const&);

template class xenum<enum_dbopt_7702, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_dbopt_7702, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_dbopt_7702, 2> const&);

template class xenum<enum_mode, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_mode, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_mode, 4> const&);

template class xenum<enum_basis, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_basis, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_basis, 3> const&);

template class xenum<enum_sep_acct_basis, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_sep_acct_basis, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_sep_acct_basis, 3> const&);

template class xenum<enum_rate_period, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_rate_period, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_rate_period, 2> const&);

template class xenum<enum_run_basis, 7>;
template std::istream& operator>> (std::istream& is, xenum<enum_run_basis, 7>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_run_basis, 7> const&);

template class xenum<enum_ledger_type, 6>;
template std::istream& operator>> (std::istream& is, xenum<enum_ledger_type, 7>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_ledger_type, 7> const&);

template class xenum<enum_amount_type, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_amount_type, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_amount_type, 2> const&);

template class xenum<enum_uw_basis, 5>;
template std::istream& operator>> (std::istream& is, xenum<enum_uw_basis, 5>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_uw_basis, 5> const&);

template class xenum<enum_table_rating, 11>;
template std::istream& operator>> (std::istream& is, xenum<enum_table_rating, 11>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_table_rating, 11> const&);

template class xenum<enum_solve_type, 9>;
template std::istream& operator>> (std::istream& is, xenum<enum_solve_type, 9>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_solve_type, 9> const&);

template class xenum<enum_solve_target, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_solve_target, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_solve_target, 2> const&);

template class xenum<enum_solve_tgt_at, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_solve_tgt_at, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_solve_tgt_at, 4> const&);

template class xenum<enum_solve_from, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_solve_from, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_solve_from, 4> const&);

template class xenum<enum_solve_to, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_solve_to, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_solve_to, 4> const&);

template class xenum<enum_kludge_from, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_kludge_from, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_kludge_from, 4> const&);

template class xenum<enum_kludge_to, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_kludge_to, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_kludge_to, 4> const&);

template class xenum<enum_pmt_strategy, 9>;
template std::istream& operator>> (std::istream& is, xenum<enum_pmt_strategy, 9>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_pmt_strategy, 9> const&);

template class xenum<enum_sa_strategy, 9>;
template std::istream& operator>> (std::istream& is, xenum<enum_sa_strategy, 9>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_sa_strategy, 9> const&);

template class xenum<enum_loan_strategy, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_loan_strategy, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_loan_strategy, 4> const&);

template class xenum<enum_wd_strategy, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_wd_strategy, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_wd_strategy, 4> const&);

template class xenum<enum_int_rate_type, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_int_rate_type, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_int_rate_type, 3> const&);

template class xenum<enum_loan_rate_type, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_loan_rate_type, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_loan_rate_type, 2> const&);

template class xenum<enum_fund_input_method, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_fund_input_method, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_fund_input_method, 3> const&);

template class xenum<enum_run_order, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_run_order, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_run_order, 2> const&);

template class xenum<enum_survival_limit, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_survival_limit, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_survival_limit, 4> const&);

template class xenum<enum_term_adj_method, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_term_adj_method, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_term_adj_method, 3> const&);

template class xenum<enum_plan, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_plan, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_plan, 2> const&);

template class xenum<enum_state, 53>;
template std::istream& operator>> (std::istream& is, xenum<enum_state, 53>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_state, 53> const&);

template class xenum<enum_country, 239>;
template std::istream& operator>> (std::istream& is, xenum<enum_country, 239>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_country, 239> const&);

template class xenum<enum_defn_life_ins, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_defn_life_ins, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_defn_life_ins, 3> const&);

template class xenum<enum_mec_avoid_method, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_mec_avoid_method, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_mec_avoid_method, 3> const&);

template class xenum<enum_defn_material_change, 5>;
template std::istream& operator>> (std::istream& is, xenum<enum_defn_material_change, 5>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_defn_material_change, 5> const&);

template class xenum<enum_spread_method, 2>;
template std::istream& operator>> (std::istream& is, xenum<enum_spread_method, 2>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_spread_method, 2> const&);

template class xenum<enum_anticipated_deduction, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_anticipated_deduction, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_anticipated_deduction, 4> const&);

template class xenum<enum_asset_charge_type, 3>;
template std::istream& operator>> (std::istream& is, xenum<enum_asset_charge_type, 3>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_asset_charge_type, 3> const&);

template class xenum<enum_part_mort_table, 1>;
template std::istream& operator>> (std::istream& is, xenum<enum_part_mort_table, 1>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_part_mort_table, 1> const&);

template class xenum<enum_premium_table, 1>;
template std::istream& operator>> (std::istream& is, xenum<enum_premium_table, 1>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_premium_table, 1> const&);

template class xenum<enum_post_ret_sa_strategy, 4>;
template std::istream& operator>> (std::istream& is, xenum<enum_post_ret_sa_strategy, 4>&);
template std::ostream& operator<< (std::ostream& os, xenum<enum_post_ret_sa_strategy, 4> const&);

#endif // LMI_EXPLICIT_INSTANTIATION

// TODO ?? Do we really want 'enum_yes_or_no' here? Why not a typedef?
LMI_SPECIALIZATION enum_yes_or_no const e_yes_or_no::enumerators[] =
    {e_yes
    ,e_no
    };
LMI_SPECIALIZATION char const*const e_yes_or_no::names[] =
    {"Yes"
    ,"No"
    };

LMI_SPECIALIZATION enum_gender const e_gender::enumerators[] =
    {e_female
    ,e_male
    ,e_unisex
    };
LMI_SPECIALIZATION char const*const e_gender::names[] =
    {"Female"
    ,"Male"
    ,"Unisex"
    };

LMI_SPECIALIZATION enum_smoking const e_smoking::enumerators[] =
    {e_smoker
    ,e_nonsmoker
    ,e_unismoke
    };
LMI_SPECIALIZATION char const*const e_smoking::names[] =
    {"Smoker"
    ,"Nonsmoker"
    ,"Unismoke"
    };

LMI_SPECIALIZATION enum_class const e_class::enumerators[] =
    {e_preferred
    ,e_standard
    ,e_rated
    ,e_ultrapref
    };
LMI_SPECIALIZATION char const*const e_class::names[] =
    {"Preferred"
    ,"Standard"
    ,"Rated"
    ,"Ultra"
    };

LMI_SPECIALIZATION enum_dbopt const e_dbopt::enumerators[] =
    {e_option1
    ,e_option2
    ,e_rop
    };
LMI_SPECIALIZATION char const*const e_dbopt::names[] =
    {"A"
    ,"B"
    ,"ROP"
    };

LMI_SPECIALIZATION enum_dbopt_7702 const e_dbopt_7702::enumerators[] =
    {e_option1_for_7702
    ,e_option2_for_7702
    };
LMI_SPECIALIZATION char const*const e_dbopt_7702::names[] =
    {"A"
    ,"B"
    };

LMI_SPECIALIZATION enum_mode const e_mode::enumerators[] =
    {e_annual
    ,e_semiannual
    ,e_quarterly
    ,e_monthly
    };
LMI_SPECIALIZATION char const*const e_mode::names[] =
    {"Annual"
    ,"Semiannual"
    ,"Quarterly"
    ,"Monthly"
    };

LMI_SPECIALIZATION enum_basis const e_basis::enumerators[] =
    {e_currbasis
    ,e_guarbasis
    ,e_mdptbasis    // midpoint
    };
LMI_SPECIALIZATION char const*const e_basis::names[] =
    {"Current basis"
    ,"Guaranteed basis"
    ,"Midpoint basis"
    };

LMI_SPECIALIZATION enum_sep_acct_basis const e_sep_acct_basis::enumerators[] =
    {e_sep_acct_full
    ,e_sep_acct_zero
    ,e_sep_acct_half
    };
LMI_SPECIALIZATION char const*const e_sep_acct_basis::names[] =
    {"Input %"         // "Full"
    ,"Zero %"          // "Zero"
    ,"Half of input %" // "Half"
    };

LMI_SPECIALIZATION enum_rate_period const e_rate_period::enumerators[] =
    {e_annual_rate
    ,e_monthly_rate
    };
LMI_SPECIALIZATION char const*const e_rate_period::names[] =
    {"Annual rate"
    ,"Monthly rate"
    };

LMI_SPECIALIZATION enum_run_basis const e_run_basis::enumerators[] =
    {e_run_curr_basis
    ,e_run_guar_basis
    ,e_run_mdpt_basis
    ,e_run_curr_basis_sa_zero
    ,e_run_guar_basis_sa_zero
    ,e_run_curr_basis_sa_half
    ,e_run_guar_basis_sa_half
    };
LMI_SPECIALIZATION char const*const e_run_basis::names[] =
    {"curr charges, curr GA int, curr SA int"
    ,"guar charges, guar GA int, curr SA int"
    ,"mdpt charges, mdpt GA int, no SA"
    ,"curr charges, curr GA int, 0 SA int"
    ,"guar charges, guar GA int, 0 SA int"
    ,"curr charges, curr GA int, 1/2 SA int"
    ,"guar charges, guar GA int, 1/2 SA int"
    };

LMI_SPECIALIZATION enum_ledger_type const e_ledger_type::enumerators[] =
    {e_ill_reg
    ,e_nasd
    ,e_group_private_placement
    ,e_offshore_private_placement
    ,e_ill_reg_private_placement
    ,e_individual_private_placement
    };
LMI_SPECIALIZATION char const*const e_ledger_type::names[] =
    {"Illustration reg"
    ,"NASD"
    ,"Group private placement"
    ,"Offshore private placement"
    ,"Private placement subject to illustation reg"
    ,"Individual private placement"
    };

LMI_SPECIALIZATION enum_amount_type const e_amount_type::enumerators[] =
    {e_face
    ,e_prem
    };
LMI_SPECIALIZATION char const*const e_amount_type::names[] =
    {"Face"
    ,"Prem"
    };

LMI_SPECIALIZATION enum_uw_basis const e_uw_basis::enumerators[] =
    {e_medical
    ,e_paramedical
    ,e_nonmedical
    ,e_simplifiedissue
    ,e_guaranteedissue
    };
LMI_SPECIALIZATION char const*const e_uw_basis::names[] =
    {"Medical"
    ,"Paramedical"
    ,"Nonmedical"
    ,"Simplified issue"
    ,"Guaranteed issue"
    };

LMI_SPECIALIZATION enum_table_rating const e_table_rating::enumerators[] =
    {e_table_none
    ,e_table_a
    ,e_table_b
    ,e_table_c
    ,e_table_d
    ,e_table_e
    ,e_table_f
    ,e_table_h
    ,e_table_j
    ,e_table_l
    ,e_table_p
    };
LMI_SPECIALIZATION char const*const e_table_rating::names[] =
    {"None",
    "A=+25%",
    "B=+50%",
    "C=+75%",
    "D=+100%",
    "E=+125%",
    "F=+150%",
    "H=+200%",
    "J=+250%",
    "L=+300%",
    "P=+400%",
    };

LMI_SPECIALIZATION enum_solve_type const e_solve_type::enumerators[] =
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
LMI_SPECIALIZATION char const*const e_solve_type::names[] =
    {"SolveNone"
    ,"SolveSpecAmt"
    ,"SolveEePrem"
    ,"SolveErPrem"
    ,"SolveLoan"
    ,"SolveWD"
    ,"SolveEePremDur"
    ,"SolveErPremDur"
    ,"SolveWDThenLoan"
    };

LMI_SPECIALIZATION enum_solve_target const e_solve_target::enumerators[] =
    {e_solve_for_endt
    ,e_solve_for_target
    };
LMI_SPECIALIZATION char const*const e_solve_target::names[] =
    {"SolveForEndt"
    ,"SolveForTarget"
    };

LMI_SPECIALIZATION enum_solve_tgt_at const e_solve_tgt_at::enumerators[] =
    {e_tgtatret
    ,e_tgtatyear
    ,e_tgtatage
    ,e_tgtatend
    };
LMI_SPECIALIZATION char const*const e_solve_tgt_at::names[] =
    {"TgtAtRetirement"
    ,"TgtAtYear"
    ,"TgtAtAge"
    ,"TgtAtMaturity"
    };

LMI_SPECIALIZATION enum_solve_from const e_solve_from::enumerators[] =
    {e_fromissue
    ,e_fromyear
    ,e_fromage
    ,e_fromret
    };
LMI_SPECIALIZATION char const*const e_solve_from::names[] =
    {"FromIssue"
    ,"FromYear"
    ,"FromAge"
    ,"FromRetirement"
    };

LMI_SPECIALIZATION enum_solve_to const e_solve_to::enumerators[] =
    {e_toret
    ,e_toyear
    ,e_toage
    ,e_toend
    };
LMI_SPECIALIZATION char const*const e_solve_to::names[] =
    {"ToRetirement"
    ,"ToYear"
    ,"ToAge"
    ,"ToMaturity"
    };

LMI_SPECIALIZATION enum_kludge_from const e_kludge_from::enumerators[] =
    {e_kludge_fromissue
    ,e_kludge_fromyear
    ,e_kludge_fromage
    ,e_kludge_fromret
    };
LMI_SPECIALIZATION char const*const e_kludge_from::names[] =
    {"Issue"
    ,"Year"
    ,"Age"
    ,"Retirement"
    };

LMI_SPECIALIZATION enum_kludge_to const e_kludge_to::enumerators[] =
    {e_kludge_toret
    ,e_kludge_toyear
    ,e_kludge_toage
    ,e_kludge_toend
    };
LMI_SPECIALIZATION char const*const e_kludge_to::names[] =
    {"Retirement"
    ,"Year"
    ,"Age"
    ,"Maturity"
    };

LMI_SPECIALIZATION enum_pmt_strategy const e_pmt_strategy::enumerators[] =
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
LMI_SPECIALIZATION char const*const e_pmt_strategy::names[] =
    {"PmtInputScalar"
    ,"PmtInputVector"
    ,"PmtMinimum"
    ,"PmtTarget"
    ,"PmtMEP"
    ,"PmtGLP"
    ,"PmtGSP"
    ,"PmtCorridor"
    ,"PmtTable"
    };

LMI_SPECIALIZATION enum_sa_strategy const e_sa_strategy::enumerators[] =
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
LMI_SPECIALIZATION char const*const e_sa_strategy::names[] =
    {"SAInputScalar"
    ,"SAInputVector"
    ,"SAMaximum"
    ,"SATarget"
    ,"SAMEP"
    ,"SAGLP"
    ,"SAGSP"
    ,"SACorridor"
    ,"SASalary"
    };

LMI_SPECIALIZATION enum_loan_strategy const e_loan_strategy::enumerators[] =
    {e_loanasinput
    ,e_loanmaximum
    ,e_loaninputscalar
    ,e_loaninputvector
    };
LMI_SPECIALIZATION char const*const e_loan_strategy::names[] =
    {"LoanAsInput"
    ,"LoanMaximum"
    ,"LoanInputScalar"
    ,"LoanInputVector"
    };

LMI_SPECIALIZATION enum_wd_strategy const e_wd_strategy::enumerators[] =
    {e_wdasinput
    ,e_wdmaximum
    ,e_wdinputscalar
    ,e_wdinputvector
    };
LMI_SPECIALIZATION char const*const e_wd_strategy::names[] =
    {"WDAsInput"
    ,"WDMaximum"
    ,"WDInputScalar"
    ,"WDInputVector"
    };

// TODO ?? "NetRate" would be better than "CredRate", but changing it
// breaks backward compatibility.    
LMI_SPECIALIZATION enum_int_rate_type const e_int_rate_type::enumerators[] =
    {e_netrate
    ,e_grossrate
    ,e_netrate
    };
LMI_SPECIALIZATION char const*const e_int_rate_type::names[] =
    {"CredRate"
    ,"GrossRate"
    ,"NetRate"
    };

LMI_SPECIALIZATION enum_loan_rate_type const e_loan_rate_type::enumerators[] =
    {e_fixed_loan_rate
    ,e_variable_loan_rate
    };
LMI_SPECIALIZATION char const*const e_loan_rate_type::names[] =
    {"Fixed"
    ,"VLR"
    };

LMI_SPECIALIZATION enum_fund_input_method const e_fund_input_method::enumerators[] =
    {e_fund_average
    ,e_fund_override
    ,e_fund_selection
    };
LMI_SPECIALIZATION char const*const e_fund_input_method::names[] =
    {"Average fund"
    ,"Override fund"
    ,"Choose funds"
    };

LMI_SPECIALIZATION enum_run_order const e_run_order::enumerators[] =
    {e_life_by_life
    ,e_month_by_month
    };
LMI_SPECIALIZATION char const*const e_run_order::names[] =
    {"Life by life"
    ,"Month by month"
    };

LMI_SPECIALIZATION enum_survival_limit const e_survival_limit::enumerators[] =
    {e_no_survival_limit
    ,e_survive_to_age
    ,e_survive_to_year
    ,e_survive_to_ex
    };
LMI_SPECIALIZATION char const*const e_survival_limit::names[] =
    {"No survival limit"
    ,"Survive to age limit"
    ,"Survive to duration limit"
    ,"Survive to life expectancy"
    };

LMI_SPECIALIZATION enum_term_adj_method const e_term_adj_method::enumerators[] =
    {e_adjust_base
    ,e_adjust_term
    ,e_adjust_both
    };
LMI_SPECIALIZATION char const*const e_term_adj_method::names[] =
    {"Adjust base first"
    ,"Adjust term first"
    ,"Adjust base and term proportionately"
    };

LMI_SPECIALIZATION enum_plan const e_plan::enumerators[] =
    {e_sample1
    ,e_sample2
    };
LMI_SPECIALIZATION char const*const e_plan::names[] =
    {"Sample1"
    ,"Sample2"
    };

LMI_SPECIALIZATION enum_state const e_state::enumerators[] =
    {e_s_AL,e_s_AK,e_s_AZ,e_s_AR,e_s_CA,e_s_CO,e_s_CT,e_s_DE,e_s_DC,e_s_FL
    ,e_s_GA,e_s_HI,e_s_ID,e_s_IL,e_s_IN,e_s_IA,e_s_KS,e_s_KY,e_s_LA,e_s_ME
    ,e_s_MD,e_s_MA,e_s_MI,e_s_MN,e_s_MS,e_s_MO,e_s_MT,e_s_NE,e_s_NV,e_s_NH
    ,e_s_NJ,e_s_NM,e_s_NY,e_s_NC,e_s_ND,e_s_OH,e_s_OK,e_s_OR,e_s_PA,e_s_PR
    ,e_s_RI,e_s_SC,e_s_SD,e_s_TN,e_s_TX,e_s_UT,e_s_VT,e_s_VA,e_s_WA,e_s_WV
    ,e_s_WI,e_s_WY
    ,e_s_XX
    };
LMI_SPECIALIZATION char const*const e_state::names[] =
    {"AL",  "AK",  "AZ",  "AR",  "CA",  "CO",  "CT",  "DE",  "DC",  "FL"
    ,"GA",  "HI",  "ID",  "IL",  "IN",  "IA",  "KS",  "KY",  "LA",  "ME"
    ,"MD",  "MA",  "MI",  "MN",  "MS",  "MO",  "MT",  "NE",  "NV",  "NH"
    ,"NJ"  ,"NM",  "NY",  "NC",  "ND",  "OH",  "OK",  "OR",  "PA",  "PR"
    ,"RI"  ,"SC"  ,"SD",  "TN",  "TX",  "UT",  "VT",  "VA",  "WA",  "WV"
    ,"WI"  ,"WY"
    ,"XX"
    };

LMI_SPECIALIZATION enum_country const e_country::enumerators[] =
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
    ,e_c_VU,e_c_WF,e_c_WS,e_c_YE,e_c_YT,e_c_YU,e_c_ZA,e_c_ZM,e_c_ZW,
    };
LMI_SPECIALIZATION char const*const e_country::names[] =
    {"AD",  "AE",  "AF",  "AG",  "AI",  "AL",  "AM",  "AN",  "AO",  "AQ"
    ,"AR",  "AS",  "AT",  "AU",  "AW",  "AZ",  "BA",  "BB",  "BD",  "BE"
    ,"BF",  "BG",  "BH",  "BI",  "BJ",  "BM",  "BN",  "BO",  "BR",  "BS"
    ,"BT",  "BV",  "BW",  "BY",  "BZ",  "CA",  "CC",  "CD",  "CF",  "CG"
    ,"CH",  "CI",  "CK",  "CL",  "CM",  "CN",  "CO",  "CR",  "CU",  "CV"
    ,"CX",  "CY",  "CZ",  "DE",  "DJ",  "DK",  "DM",  "DO",  "DZ",  "EC"
    ,"EE",  "EG",  "EH",  "ER",  "ES",  "ET",  "FI",  "FJ",  "FK",  "FM"
    ,"FO",  "FR",  "GA",  "GB",  "GD",  "GE",  "GF",  "GH",  "GI",  "GL"
    ,"GM",  "GN",  "GP",  "GQ",  "GR",  "GS",  "GT",  "GU",  "GW",  "GY"
    ,"HK",  "HM",  "HN",  "HR",  "HT",  "HU",  "ID",  "IE",  "IL",  "IN"
    ,"IO",  "IQ",  "IR",  "IS",  "IT",  "JM",  "JO",  "JP",  "KE",  "KG"
    ,"KH",  "KI",  "KM",  "KN",  "KP",  "KR",  "KW",  "KY",  "KZ",  "LA"
    ,"LB",  "LC",  "LI",  "LK",  "LR",  "LS",  "LT",  "LU",  "LV",  "LY"
    ,"MA",  "MC",  "MD",  "MG",  "MH",  "MK",  "ML",  "MM",  "MN",  "MO"
    ,"MP",  "MQ",  "MR",  "MS",  "MT",  "MU",  "MV",  "MW",  "MX",  "MY"
    ,"MZ",  "NA",  "NC",  "NE",  "NF",  "NG",  "NI",  "NL",  "NO",  "NP"
    ,"NR",  "NU",  "NZ",  "OM",  "PA",  "PE",  "PF",  "PG",  "PH",  "PK"
    ,"PL",  "PM",  "PN",  "PR",  "PS",  "PT",  "PW",  "PY",  "QA",  "RE"
    ,"RO",  "RU",  "RW",  "SA",  "SB",  "SC",  "SD",  "SE",  "SG",  "SH"
    ,"SI",  "SJ",  "SK",  "SL",  "SM",  "SN",  "SO",  "SR",  "ST",  "SV"
    ,"SY",  "SZ",  "TC",  "TD",  "TF",  "TG",  "TH",  "TJ",  "TK",  "TM"
    ,"TN",  "TO",  "TP",  "TR",  "TT",  "TV",  "TW",  "TZ",  "UA",  "UG"
    ,"UM",  "US",  "UY",  "UZ",  "VA",  "VC",  "VE",  "VG",  "VI",  "VN"
    ,"VU",  "WF",  "WS",  "YE",  "YT",  "YU",  "ZA",  "ZM",  "ZW",
    };

LMI_SPECIALIZATION enum_defn_life_ins const e_defn_life_ins::enumerators[] =
    {e_cvat
    ,e_gpt
    ,e_noncompliant
    };
LMI_SPECIALIZATION char const*const e_defn_life_ins::names[] =
    {"CVAT"
    ,"GPT"
    ,"Not 7702 compliant"
    };

LMI_SPECIALIZATION enum_mec_avoid_method const e_mec_avoid_method::enumerators[] =
    {e_allow_mec
    ,e_reduce_prem
    ,e_increase_specamt
    };
LMI_SPECIALIZATION char const*const e_mec_avoid_method::names[] =
    {"Allow MEC"
    ,"Reduce premium"
    ,"Increase specified amount"
    };

LMI_SPECIALIZATION enum_defn_material_change const e_defn_material_change::enumerators[] =
    {e_unnecessary_premium
    ,e_benefit_increase
    ,e_later_of_increase_or_unnecessary_premium
    ,e_earlier_of_increase_or_unnecessary_premium
    ,e_adjustment_event
    };
LMI_SPECIALIZATION char const*const e_defn_material_change::names[] =
    {"Unnecessary premium"
    ,"Benefit increase"
    ,"Later of increase or unnecessary premium"
    ,"Earlier of increase or unnecessary premium"
    ,"GPT adjustment event"
    };

LMI_SPECIALIZATION enum_spread_method const e_spread_method::enumerators[] =
    {e_spread_is_effective_annual
    ,e_spread_is_nominal_daily
    };
LMI_SPECIALIZATION char const*const e_spread_method::names[] =
    {"Effective annual"
    ,"Nominal daily"
    };

LMI_SPECIALIZATION enum_coi_rate_method const e_coi_rate_method::enumerators[] =
    {e_coi_rate_subtractive
    ,e_coi_rate_exponential
    };
LMI_SPECIALIZATION char const*const e_coi_rate_method::names[] =
    {"Subtractive"
    ,"Exponential"
    };

LMI_SPECIALIZATION enum_anticipated_deduction const e_anticipated_deduction::enumerators[] =
    {e_twelve_times_last
    ,e_to_next_anniversary
    ,e_to_next_modal_pmt_date
    ,e_eighteen_times_last
    };
LMI_SPECIALIZATION char const*const e_anticipated_deduction::names[] =
    {"Twelve times most recent"
    ,"To next anniversary"
    ,"To next modal payment date"
    ,"Eighteen times most recent"
    };

// TODO ?? This is used in a convoluted way that's probably incorrect.
LMI_SPECIALIZATION enum_asset_charge_type const e_asset_charge_type::enumerators[] =
    {e_asset_charge_spread
    ,e_asset_charge_load_after_ded
    ,e_asset_charge_load_before_ded
    };
LMI_SPECIALIZATION char const*const e_asset_charge_type::names[] =
    {"Asset charge assessed as spread"
    ,"Asset charge assessed as load after monthly deduction"
    ,"Asset charge assessed as load before monthly deduction"
    };

LMI_SPECIALIZATION enum_part_mort_table const e_part_mort_table::enumerators[] =
    {e_default_part_mort_table
    };
LMI_SPECIALIZATION char const*const e_part_mort_table::names[] =
// TODO ?? Change in next update:
//    {"Default partial mortality table (83 GAM)"
    {"0"
    };

LMI_SPECIALIZATION enum_premium_table const e_premium_table::enumerators[] =
    {e_default_premium_table
    };
LMI_SPECIALIZATION char const*const e_premium_table::names[] =
// TODO ?? Change in next update:
//    {"Default partial mortality table (Table Y)"
    {"0"
    };

// TODO ?? Obsolescent.

LMI_SPECIALIZATION enum_post_ret_sa_strategy const e_post_ret_sa_strategy::enumerators[] =
    {e_obsolete_same_as_initial
    ,e_obsolete_varying
    ,e_obsolete_scalar
    ,e_obsolete_percent_of_initial
    };
LMI_SPECIALIZATION char const*const e_post_ret_sa_strategy::names[] =
    {"Same as pre-retirement"
    ,"Vector"
    ,"Scalar"
    ,"% of pre-retirement"
    };

// TODO ?? This stuff is a little funky....

bool is_subject_to_ill_reg(e_ledger_type const& z)
{
    return
           e_ill_reg                   == z
        || e_ill_reg_private_placement == z
        ;
}

bool is_subject_to_ill_reg(enum_ledger_type z)
{
    return
           e_ill_reg                   == z
        || e_ill_reg_private_placement == z
        ;
}

bool is_subject_to_ill_reg(double z)
{
    return
           e_ill_reg                   == enum_ledger_type(z)
        || e_ill_reg_private_placement == enum_ledger_type(z)
        ;
}

// Formerly, three-rate illustrations were required for prospectuses.
// Since these functions were orignally written, that requirement has
// inoperative, but the functions are preserved as stubs in case such
// a format becomes desirable for some other reason.

bool is_three_rate_nasd(e_ledger_type const&)
{
    return false;
}

bool is_three_rate_nasd(enum_ledger_type)
{
    return false;
}

bool is_three_rate_nasd(double)
{
    return false;
}

void set_separate_bases_from_run_basis
    (e_run_basis const&      a_RunBasis
    ,e_basis&                a_ExpAndGABasis
    ,e_sep_acct_basis&       a_SABasis
    )
{
    switch(a_RunBasis)
        {
        case e_run_curr_basis:
            {
            a_ExpAndGABasis = e_currbasis;
            a_SABasis       = e_sep_acct_full;
            }
            break;
        case e_run_guar_basis:
            {
            a_ExpAndGABasis = e_guarbasis;
            a_SABasis       = e_sep_acct_full;
            }
            break;
        case e_run_mdpt_basis:
            {
            a_ExpAndGABasis = e_mdptbasis;
            a_SABasis       = e_sep_acct_full;
            }
            break;
        case e_run_curr_basis_sa_zero:
            {
            a_ExpAndGABasis = e_currbasis;
            a_SABasis       = e_sep_acct_zero;
            }
            break;
        case e_run_guar_basis_sa_zero:
            {
            a_ExpAndGABasis = e_guarbasis;
            a_SABasis       = e_sep_acct_zero;
            }
            break;
        case e_run_curr_basis_sa_half:
            {
            a_ExpAndGABasis = e_currbasis;
            a_SABasis       = e_sep_acct_half;
            }
            break;
        case e_run_guar_basis_sa_half:
            {
            a_ExpAndGABasis = e_guarbasis;
            a_SABasis       = e_sep_acct_half;
            }
            break;
        default:
            {
            throw std::logic_error
                (
                "Internal error: unknown rate basis in "
                __FILE__
                );
            }
        }
}

void set_run_basis_from_separate_bases
    (e_run_basis&            a_RunBasis
    ,e_basis const&          a_ExpAndGABasis
    ,e_sep_acct_basis const& a_SABasis
    )
{
        if
            (   a_ExpAndGABasis == e_currbasis
            &&  a_SABasis       == e_sep_acct_full
            )
            {
            a_RunBasis = e_run_curr_basis;
            }
        else if
            (   a_ExpAndGABasis == e_guarbasis
            &&  a_SABasis       == e_sep_acct_full
            )
            {
            a_RunBasis = e_run_guar_basis;
            }
        else if
            (   a_ExpAndGABasis == e_mdptbasis
            &&  a_SABasis       == e_sep_acct_full
            )
            {
            a_RunBasis = e_run_mdpt_basis;
            }
        else if
            (   a_ExpAndGABasis == e_currbasis
            &&  a_SABasis       == e_sep_acct_zero
            )
            {
            a_RunBasis = e_run_curr_basis_sa_zero;
            }
        else if
            (   a_ExpAndGABasis == e_guarbasis
            &&  a_SABasis       == e_sep_acct_zero
            )
            {
            a_RunBasis = e_run_guar_basis_sa_zero;
            }
        else if
            (   a_ExpAndGABasis == e_currbasis
            &&  a_SABasis       == e_sep_acct_half
            )
            {
            a_RunBasis = e_run_curr_basis_sa_half;
            }
        else if
            (   a_ExpAndGABasis == e_guarbasis
            &&  a_SABasis       == e_sep_acct_half
            )
            {
            a_RunBasis = e_run_guar_basis_sa_half;
            }
        else
            {
            throw std::logic_error
                (
                "Internal error: unknown rate basis in "
                __FILE__
                );
            }
}

