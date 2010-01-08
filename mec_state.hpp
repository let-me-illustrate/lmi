// Transient state of MEC testing.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

#ifndef mec_state_hpp
#define mec_state_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "streamable.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/operators.hpp>

#include <list>
#include <map>
#include <string>

/// Transient state of MEC testing.
///
/// For design discussion, see:
///   http://lists.nongnu.org/archive/html/lmi/2009-07/msg00002.html
///
/// Variables are prefixed '[A-Z][0-9]_' so that they sort by groups:
/// in particular, when their names are used to nominate xml elements.
/// 'A*_' is reserved in case it's wanted later--e.g., for arguments.

class LMI_SO mec_state
    :virtual private obstruct_slicing<mec_state>
    ,virtual public streamable
    ,public MemberSymbolTable<mec_state>
    ,private boost::equality_comparable<mec_state>
{
    friend class Irc7702A;

  public:
    mec_state();
    mec_state(mec_state const&);
    virtual ~mec_state();

    mec_state& operator=(mec_state const&);
    bool operator==(mec_state const&) const;

    std::string format_as_html(std::string const& heading) const;
    void save(fs::path const&) const;

  private:
    void AscribeMembers();

    // Class 'streamable' required implementation.
    virtual void read (xml::element const&);
    virtual void write(xml::element&) const;
    virtual int class_version() const;
    virtual std::string xml_root_name() const;

    // Backward compatibility.
    std::string RedintegrateExAnte
        (int                file_version
        ,std::string const& name
        ,std::string const& value
        ) const;
    void        RedintegrateExPost
        (int                                file_version
        ,std::map<std::string, std::string> detritus_map
        ,std::list<std::string>             residuary_names
        );

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
};

#endif // mec_state_hpp

