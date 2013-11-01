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
#include "miscellany.hpp"               // minmax

#include <algorithm>                    // std::min_element()
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
/// Asserted preconditions: All argument vectors, including those in
/// the parameter object, have the same length, and that length is a
/// plausible human age. All these vectors' elements must be within a
/// plausible range, most often [0.0, 1.0). Upper limits (except on q)
/// are arbitrary, though liberal; they're intended only to detect
/// probable errors such as might arise from misplaced decimal point.
/// Particular bounds might be loosened p.r.n.--if negative loads must
/// be supported, for example. These range tests impose an overhead of
/// about twenty percent; it is assumed that a few extra microseconds
/// are affordable.
///
/// It may at first seem that precondition assertions belong upstream;
/// however, writing them in the body of gpt_cf_triad::gpt_cf_triad()
/// would cause them to be executed after the present ctor is called.
///
/// Asserted postconditions: All Dx+t (thus, implicitly, all Nx+t) are
/// greater than zero, including those multiplied by the complement of
/// premium loads, so that they may safely be used as denominators in
/// premium formulas.
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
    LMI_ASSERT(0 < length_ && length_ < methuselah);
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

    typedef minmax<double> mm;
    mm a(        qc                  ); LMI_ASSERT(0.0 <= a && a <= 1.0);
    mm b(        ic                  ); LMI_ASSERT(0.0 <= b && b <  1.0);
    mm c(        ig                  ); LMI_ASSERT(0.0 <= c && c <  1.0);
    // Assertions on the next line are required by section B.8 here:
    //   http://www.nongnu.org/lmi/7702.html
    // and therefore must not blithely be weakened, even if there's
    // a good reason for loosening the assertions above.
    LMI_ASSERT(0.0 <= a && 0.0 <= b && -1.0 < c);
    mm d(charges.prem_load_target    ); LMI_ASSERT(0.0 <= d && d <  1.0);
    mm e(charges.prem_load_excess    ); LMI_ASSERT(0.0 <= e && e <  1.0);
    mm f(charges.policy_fee_monthly  ); LMI_ASSERT(0.0 <= f            );
    mm g(charges.policy_fee_annual   ); LMI_ASSERT(0.0 <= g            );
    mm h(charges.specamt_load_monthly); LMI_ASSERT(0.0 <= h && h <  1.0);
    mm i(charges.qab_gio_rate        ); LMI_ASSERT(0.0 <= i && i <  1.0);
    mm j(charges.qab_adb_rate        ); LMI_ASSERT(0.0 <= j && j <  1.0);
    mm k(charges.qab_term_rate       ); LMI_ASSERT(0.0 <= k && k <  1.0);
    mm l(charges.qab_spouse_rate     ); LMI_ASSERT(0.0 <= l && l <  1.0);
    mm m(charges.qab_child_rate      ); LMI_ASSERT(0.0 <= m && m <  1.0);
    mm n(charges.qab_waiver_rate     ); LMI_ASSERT(0.0 <= n && n <  1.0);

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

    LMI_ASSERT(0.0 < D_endt_);
    LMI_ASSERT(0.0 < *std::min_element(D_net_tgt_.begin(), D_net_tgt_.end()));
    LMI_ASSERT(0.0 < *std::min_element(D_net_exc_.begin(), D_net_exc_.end()));
}

gpt_commfns::~gpt_commfns()
{}

/// Calculate GLP or GSP.
///
/// Divisions are safe because denominators are asserted upstream to
/// be greater than zero.

double gpt_commfns::calculate_premium
    (oenum_glp_or_gsp        glp_or_gsp
    ,gpt_scalar_parms const& args
    ) const
{
    int const j = args.duration;
    double numerator =
          D_endt_          * args.endt_bft
        + M_           [j] * args.f3bft
        + N_chg_pol_   [j]
        + N_chg_sa_    [j] * args.chg_sa_amt
        + N_qab_gio_   [j] * args.qab_gio_amt
        + N_qab_adb_   [j] * args.qab_adb_amt
        + N_qab_term_  [j] * args.qab_term_amt
        + N_qab_spouse_[j] * args.qab_spouse_amt
        + N_qab_child_ [j] * args.qab_child_amt
        + N_qab_waiver_[j] * args.qab_waiver_amt
        ;
    double denom_tgt = glp_or_gsp ? D_net_tgt_[j] : N_net_tgt_[j];
    double z = numerator / denom_tgt;
    if(z <= args.target)
        {
        return z;
        }

    double denom_exc = glp_or_gsp ? D_net_exc_[j] : N_net_exc_[j];
    return (numerator + args.target * (denom_exc - denom_tgt)) / denom_exc;
}

gpt_cf_triad::gpt_cf_triad
    (std::vector<double> const& qc
    ,std::vector<double> const& glp_ic
    ,std::vector<double> const& glp_ig
    ,std::vector<double> const& gsp_ic
    ,std::vector<double> const& gsp_ig
    ,gpt_vector_parms    const& charges
    )
    :length_     (qc.size())
    ,cf_glp_dbo_1(qc, glp_ic, glp_ig, mce_option1_for_7702, charges)
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
///
/// It would seem more natural to use a reference as in revision 5778,
/// instead of a pointer. However, that alternative is more than an
/// order of magnitude slower with both gcc-3.4.5 and como-4.3.10.1,
/// as though they invoke the gpt_commfns copy ctor due to the throw-
/// expression in the conditional-expression.
///
/// Asserted preconditions: Duration is within its natural bounds, and
/// other members of 'args' are nonnegative.
///
/// Asserted postcondition: Returned GLP or GSP is nonnegative; thus,
/// while adjusted premium 'A+B-C' may be negative, {A,B,C} are all
/// individually nonnegative.

double gpt_cf_triad::calculate_premium
    (oenum_glp_or_gsp        glp_or_gsp
    ,mcenum_dbopt_7702       dbo
    ,gpt_scalar_parms const& args
    ) const
{
    LMI_ASSERT(0 <= args.duration);
    LMI_ASSERT(static_cast<unsigned int>(args.duration) < length_);
    LMI_ASSERT(0.0 <= args.target        );
    LMI_ASSERT(0.0 <= args.f3bft         );
    LMI_ASSERT(0.0 <= args.endt_bft      );
    LMI_ASSERT(0.0 <= args.chg_sa_amt    );
    LMI_ASSERT(0.0 <= args.qab_gio_amt   );
    LMI_ASSERT(0.0 <= args.qab_adb_amt   );
    LMI_ASSERT(0.0 <= args.qab_term_amt  );
    LMI_ASSERT(0.0 <= args.qab_spouse_amt);
    LMI_ASSERT(0.0 <= args.qab_child_amt );
    LMI_ASSERT(0.0 <= args.qab_waiver_amt);

    gpt_commfns const*const pcf =
          (oe_glp == glp_or_gsp && mce_option1_for_7702 == dbo) ? &cf_glp_dbo_1
        : (oe_glp == glp_or_gsp && mce_option2_for_7702 == dbo) ? &cf_glp_dbo_2
        : (oe_gsp == glp_or_gsp                               ) ? &cf_gsp
        : throw std::runtime_error("Cannot determine GPT assumptions.")
        ;
    LMI_ASSERT(0 != pcf); // Redundant: demonstrably cannot fail.
    double const z = pcf->calculate_premium(glp_or_gsp, args);
    LMI_ASSERT(0.0 <= z);
    return z;
}

