// Transient state of guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef gpt_state_hpp
#define gpt_state_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <string>

/// Transient state of guideline premium test.
///
/// For design discussion, see:
///   https://lists.nongnu.org/archive/html/lmi/2009-07/msg00002.html
///
/// When any GPT work is to be done, some 7702A work is probably
/// needed as well. Therefore, this class is more a superset than a
/// sibling of the corresponding 'mec_' class.
///
/// Variables are prefixed '[A-Z][0-9]_' so that they sort by groups:
/// in particular, when their names are used to nominate xml elements.
/// 'A*_' is reserved in case it's wanted later--e.g., for arguments.

class LMI_SO gpt_state final
    :public  xml_serializable           <gpt_state>
    ,public  MemberSymbolTable          <gpt_state>
{
    friend class Irc7702A;

  public:
    gpt_state();
    gpt_state(gpt_state const&);
    ~gpt_state() override = default;

    gpt_state& operator=(gpt_state const&);
    bool operator==(gpt_state const&) const;

    std::string format_as_html(std::string const& heading) const;

  private:
    void AscribeMembers();

    // xml_serializable required implementation.
    int                class_version() const override;
    std::string const& xml_root_name() const override;

    // xml_serializable overrides.
    bool is_detritus(std::string const&) const override;

    int    B0_deduced_policy_year;
    int    B1_deduced_contract_year;
    double B2_deduced_px7_rate;
    double B3_deduced_nsp_rate;
    double B4_deduced_target_premium;
    double B5_deduced_target_load;
    double B6_deduced_excess_load;

    double C0_init_bft;
    double C1_init_ldb;
    double C2_init_amt_pd;
    bool   C3_init_is_mc;
    double C4_init_dcv;
    double C5_init_px7;
    bool   C6_init_mec;

    double D0_incr_bft;
    double D1_incr_ldb;
    double D2_incr_amt_pd;
    bool   D3_incr_is_mc;
    double D4_incr_dcv;
    double D5_incr_px7;
    bool   D6_incr_mec;

    double E0_decr_bft;
    double E1_decr_ldb;
    double E2_decr_amt_pd;
    bool   E3_decr_is_mc;
    double E4_decr_dcv;
    double E5_decr_px7;
    bool   E6_decr_mec;

    double F0_nec_pm_bft;
    double F1_nec_pm_ldb;
    double F2_nec_pm_amt_pd;
    bool   F3_nec_pm_is_mc;
    double F4_nec_pm_dcv;
    double F5_nec_pm_px7;
    bool   F6_nec_pm_mec;

    double G0_do_mc_bft;
    double G1_do_mc_ldb;
    double G2_do_mc_amt_pd;
    bool   G3_do_mc_is_mc;
    double G4_do_mc_dcv;
    double G5_do_mc_px7;
    bool   G6_do_mc_mec;

    double H0_unnec_pm_bft;
    double H1_unnec_pm_ldb;
    double H2_unnec_pm_amt_pd;
    bool   H3_unnec_pm_is_mc;
    double H4_unnec_pm_dcv;
    double H5_unnec_pm_px7;
    bool   H6_unnec_pm_mec;

    double Q0_net_1035;
    double Q1_max_nec_prem_net;
    double Q2_max_nec_prem_gross;
    double Q3_cv_before_last_mc;
    double Q4_cum_px7;
    double Q5_cum_amt_pd;
    double Q6_max_non_mec_prem;

    double X0_glp;
    double X1_gsp;
    double X2_glp_a;
    double X3_gsp_a;
    double X4_glp_b;
    double X5_gsp_b;
    double X6_glp_c;
    double X7_gsp_c;
};

#endif // gpt_state_hpp
