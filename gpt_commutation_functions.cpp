// Internal Revenue Code section 7702: GPT commutation functions.
//
// Copyright (C) 2013 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "gpt_commutation_functions.hpp"

#include "assert_lmi.hpp"
#include "commutation_functions.hpp"
#include "et_vector.hpp"

#include <numeric>                      // std::partial_sum()
#include <stdexcept>

namespace
{
/// Backward sum of a PETE expression.
///
/// PETE provides only what APL calls "primitive scalar functions",
/// which apply to each element or pair of elements. This function
/// performs the equivalent of APL "rotate plus scan rotate". It
/// would be nice to extend PETE along these lines, but that's too
/// ambitious for now; apparently Mullin's 'psi calculus' does
/// exactly that, but the code is unpublished.

template<typename T, typename E>
std::vector<T>& back_sum(std::vector<T>& vt, E e)
{
    assign(vt, e);
    std::partial_sum(vt.rbegin(), vt.rend(), vt.rbegin());
    return vt;
}
} // Unnamed namespace.

/// Constructor.
///
/// All vectors, including those in the parameter object, must have
/// the same length. It may at first appear that assertions to this
/// effect belong upstream; however, writing them in the body of
/// gpt_cf_triad::gpt_cf_triad() would cause them to be executed
/// after the present ctor is called.
///
/// We are aware of one old UL policy form that accumulates values
/// quarterly instead of monthly. This could be accommodated by adding
/// an extra parameter, but doesn't seem worth the trouble.

gpt_commfns::gpt_commfns
    (std::vector<double> const& qc
    ,std::vector<double> const& ic
    ,std::vector<double> const& ig
    ,mcenum_dbopt_7702   const  dbo
    ,gpt_vector_parms    const& charges
    )
    :length_       (qc.size())
    ,M_            (length_)
    ,D_endt_       (0.0)
    ,D_net_tgt_    (length_)
    ,D_net_exc_    (length_)
    ,N_net_tgt_    (length_)
    ,N_net_exc_    (length_)
    ,N_chg_pol_    (length_)
    ,N_chg_sa_     (length_)
    ,N_qab_gio_    (length_)
    ,N_qab_adb_    (length_)
    ,N_qab_term_   (length_)
    ,N_qab_spouse_ (length_)
    ,N_qab_child_  (length_)
    ,N_qab_waiver_ (length_)
{
    LMI_ASSERT(length_ == qc                          .size());
    LMI_ASSERT(length_ == ic                          .size());
    LMI_ASSERT(length_ == ig                          .size());
    LMI_ASSERT(length_ == charges.prem_load_target    .size());
    LMI_ASSERT(length_ == charges.prem_load_excess    .size());
    LMI_ASSERT(length_ == charges.policy_fee_monthly  .size());
    LMI_ASSERT(length_ == charges.policy_fee_annual   .size());
    LMI_ASSERT(length_ == charges.specamt_load_monthly.size());
    LMI_ASSERT(length_ == charges.qab_gio_rate        .size());
    LMI_ASSERT(length_ == charges.qab_adb_rate        .size());
    LMI_ASSERT(length_ == charges.qab_term_rate       .size());
    LMI_ASSERT(length_ == charges.qab_spouse_rate     .size());
    LMI_ASSERT(length_ == charges.qab_child_rate      .size());
    LMI_ASSERT(length_ == charges.qab_waiver_rate     .size());

    ULCommFns const cf(qc, ic, ig, dbo, mce_monthly);

    M_                    = cf.kM();
    D_endt_               = cf.aDomega();
    assign  (D_net_tgt_   , cf.aD() * (1.0 - charges.prem_load_target));
    assign  (D_net_exc_   , cf.aD() * (1.0 - charges.prem_load_excess));
    back_sum(N_net_tgt_   , cf.aD() * (1.0 - charges.prem_load_target));
    back_sum(N_net_exc_   , cf.aD() * (1.0 - charges.prem_load_excess));
    back_sum(N_chg_pol_   , cf.aD() * charges.policy_fee_annual
                          + cf.kD() * charges.policy_fee_monthly);
    back_sum(N_chg_sa_    , cf.kD() * charges.specamt_load_monthly);
    back_sum(N_qab_gio_   , cf.kD() * charges.qab_gio_rate);
    back_sum(N_qab_adb_   , cf.kD() * charges.qab_adb_rate);
    back_sum(N_qab_term_  , cf.kD() * charges.qab_term_rate);
    back_sum(N_qab_spouse_, cf.kD() * charges.qab_spouse_rate);
    back_sum(N_qab_child_ , cf.kD() * charges.qab_child_rate);
    back_sum(N_qab_waiver_, cf.kD() * charges.qab_waiver_rate);
}

gpt_commfns::~gpt_commfns()
{}

/// Calculate GLP or GSP.

double gpt_commfns::calculate_premium
    (oenum_glp_or_gsp        glp_or_gsp
    ,gpt_scalar_parms const& args
    ) const
{
    double endowment = D_endt_ * args.endt_bft;
    double charges =
          M_           [args.duration] * args.f3bft
        + N_chg_pol_   [args.duration]
        + N_chg_sa_    [args.duration] * args.chg_sa_amt
        + N_qab_gio_   [args.duration] * args.qab_gio_amt
        + N_qab_adb_   [args.duration] * args.qab_adb_amt
        + N_qab_term_  [args.duration] * args.qab_term_amt
        + N_qab_spouse_[args.duration] * args.qab_spouse_amt
        + N_qab_child_ [args.duration] * args.qab_child_amt
        + N_qab_waiver_[args.duration] * args.qab_waiver_amt
        ;
    double den_tgt = (glp_or_gsp ? D_net_tgt_ : N_net_tgt_)[args.duration];
    double den_exc = (glp_or_gsp ? D_net_exc_ : N_net_exc_)[args.duration];
    double z = (endowment + charges) / den_tgt;
    if(z <= args.target)
        {
        return z;
        }

    charges += args.target * (den_exc - den_tgt);
    return (endowment + charges) / den_exc;
}

gpt_cf_triad::gpt_cf_triad
    (std::vector<double> const& qc
    ,std::vector<double> const& glp_ic
    ,std::vector<double> const& glp_ig
    ,std::vector<double> const& gsp_ic
    ,std::vector<double> const& gsp_ig
    ,gpt_vector_parms    const& charges
    )
    :cf_glp_dbo_1(qc, glp_ic, glp_ig, mce_option1_for_7702, charges)
    ,cf_glp_dbo_2(qc, glp_ic, glp_ig, mce_option2_for_7702, charges)
    ,cf_gsp      (qc, gsp_ic, gsp_ig, mce_option1_for_7702, charges)
{
}

gpt_cf_triad::~gpt_cf_triad()
{}

/// Calculate GLP or GSP.
///
/// For GSP, 'dbo' is disregarded because it is irrelevant. That
/// argument might instead have been written last and defaulted for
/// calls that calculate GSP, but that's needlessly complicated.
/// Alternatively, distinct functions might have been provided for
/// GLP and GSP calculations, but that's not worth the bother; or
/// a single function might calculate and return a {GLP,GSP} pair,
/// but then sometimes one would need to be thrown away (as when
/// specified amount is determined by a GLP or GSP strategy).

double gpt_cf_triad::calculate_premium
    (oenum_glp_or_gsp        glp_or_gsp
    ,mcenum_dbopt_7702       dbo
    ,gpt_scalar_parms const& args
    ) const
{
    gpt_commfns const& cf =
          (oe_glp == glp_or_gsp && mce_option1_for_7702 == dbo) ? cf_glp_dbo_1
        : (oe_glp == glp_or_gsp && mce_option2_for_7702 == dbo) ? cf_glp_dbo_2
        : (oe_gsp == glp_or_gsp                               ) ? cf_gsp
        : throw std::runtime_error("Cannot determine GPT assumptions.")
        ;
    return cf.calculate_premium(glp_or_gsp, args);
}

