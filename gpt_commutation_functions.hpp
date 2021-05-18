// Internal Revenue Code section 7702: GPT commutation functions.
//
// Copyright (C) 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef gpt_commutation_functions_hpp
#define gpt_commutation_functions_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"       // mcenum_dbopt_7702
#include "oecumenic_enumerations.hpp"   // oenum_glp_or_gsp

#include <vector>

/// Loads, fees, and monthly QAB rates used in GPT calculations.
///
/// It is simpler to pass these data around as one "parameter object"
/// than as separate parameters.
///
/// Copying vectors into this struct costs about four percent as much
/// as constructing a gpt_cf_triad object (as the unit test shows).
/// That cost could be avoided by making vector members references,
/// but some flexibility and robustness would be lost.
///
/// Mortality and interest rates could have been included. However,
/// unlike other parameters, they may need various adjustments--e.g.,
/// conversion from annual to monthly, subject to some maximum or
/// minimum--so leaving them out makes const objects of this class
/// potentially more useful.
///
/// Vector parameters run from issue age through the 7702(e)(1)(B)
/// maturity age (thus, neither issue nor maturity age is needed).
/// They are assumed to include any applicable rating, such as an
/// occupational extra on an accident benefit.
///
/// Term rates on the main insured ('qab_term_rate') apply only to
/// coverage not treated as "death benefit"; decreasing term is
/// treated here as level.

struct gpt_vector_parms
{
    std::vector<double> prem_load_target     ;
    std::vector<double> prem_load_excess     ;
    std::vector<double> policy_fee_monthly   ;
    std::vector<double> policy_fee_annual    ;
    std::vector<double> specamt_load_monthly ;
    std::vector<double> qab_gio_rate         ;
    std::vector<double> qab_adb_rate         ;
    std::vector<double> qab_term_rate        ;
    std::vector<double> qab_spouse_rate      ;
    std::vector<double> qab_child_rate       ;
    std::vector<double> qab_waiver_rate      ;
};

/// Scalar parameters for various GPT purposes.
///
/// Includes all scalar parameters that are
///   - used identically for calculating both GLP and GSP; or
///   - required to initialize an object of class gpt7702; or
///   - otherwise useful for GPT processing.
///
/// The 'oenum_glp_or_gsp' calculate_premium() argument is excluded
/// here so that the same set of scalar parameters can be used for
/// both GLP and GSP.
///
/// 'mcenum_dbopt_7702' is included here even though it's irrelevant
/// for GSP.
///
/// 'chg_sa_base' is the base for any specified-amount load. It may
/// differ from 'specamt', e.g., by being limited to a scalar maximum,
/// by including a term amount, or by being set immutably at issue.

struct gpt_scalar_parms
{
    int                  duration        {                 0  };
    double               f3_bft          {                 0.0};
    double               endt_bft        {                 0.0};
    double               target_prem     {                 0.0};
    double               chg_sa_base     {                 0.0};
    mcenum_dbopt_7702    dbopt_7702      {mce_option1_for_7702};
    double               qab_gio_amt     {                 0.0};
    double               qab_adb_amt     {                 0.0};
    double               qab_term_amt    {                 0.0};
    double               qab_spouse_amt  {                 0.0};
    double               qab_child_amt   {                 0.0};
    double               qab_waiver_amt  {                 0.0};

    bool operator==(gpt_scalar_parms const&) const = default;
};

/// Commutation functions specialized for GPT calculations.
///
/// All members are private: only its one friend can use this class.
/// It could have been written as a nested class enclosed in that
/// friend, but that would have given it access to the friend's
/// private members, without protecting it better against accidental
/// (mis)use.
///
/// See section 14.3 here:
///   https://www.nongnu.org/lmi/7702.html
///
/// Only these specialized functions are needed. Once they're set, the
/// canonical commutation functions {C,D,N} from which they're derived
/// can be discarded. The function M is retained because it is used in
/// an unspecialized form. The value of D at the endowment duration is
/// kept for valuing the endowment benefit; it may at first seem that
/// it could be subsumed into M, but it is always multiplied by the
/// specified amount, whereas M may be multiplied by death benefit.
///
/// Vector parameters run from issue age through the 7702(e)(1)(B)
/// maturity age (thus, neither issue nor maturity age is needed).
/// They are assumed to include any applicable rating, such as an
/// occupational extra on an accident benefit.
///
/// QABs are enumerated in 7702(f)(5)(i-iv). The benefit amounts by
/// which their rates are multiplied are all specified explicitly.
/// This uniform approach is the most general, and entails little
/// duplication in the case that the QAB rate is multiplied by the
/// specified amount, as frequently occurs with accident and waiver
/// benefits. A waiver benefit whose charge depends on actual monthly
/// deductions cannot be treated as a QAB--see section 11/7 here:
///   https://www.nongnu.org/lmi/7702.html
/// A vector of zeros may of course be passed as the rate vector for
/// any QAB that is not offered or not elected, or that the insurer
/// chooses not to treat as a QAB.
///
/// Data member 'length_' is a dispensable convenience that makes
/// initialization and precondition checks a little clearer.
///
/// Implicitly-declared special member functions do the right thing.

class gpt_commfns
{
    friend class gpt_cf_triad;

  private:
    gpt_commfns
        (std::vector<double> const& qc
        ,std::vector<double> const& ic
        ,std::vector<double> const& ig
        ,mcenum_dbopt_7702   const  dbo
        ,gpt_vector_parms    const& charges
        );

    double calculate_premium(oenum_glp_or_gsp, gpt_scalar_parms const&) const;

    int const           length_;
    std::vector<double> M_;
    double              D_endt_;
    std::vector<double> D_net_tgt_;
    std::vector<double> D_net_exc_;
    std::vector<double> N_net_tgt_;
    std::vector<double> N_net_exc_;
    std::vector<double> N_chg_pol_;
    std::vector<double> N_chg_sa_;
    std::vector<double> N_qab_gio_;
    std::vector<double> N_qab_adb_;
    std::vector<double> N_qab_term_;
    std::vector<double> N_qab_spouse_;
    std::vector<double> N_qab_child_;
    std::vector<double> N_qab_waiver_;
};

/// Specialized GPT commutation functions on the three required bases.
///
/// '*_ig' (monthly effective death-benefit discount rate for NAAR) is
/// distinct for GLP and GSP. Often it is the monthly equivalent of
/// four and six percent, respectively.
///
/// Data member 'length_' is a dispensable convenience that makes
/// precondition checks a little clearer.
///
/// Implicitly-declared special member functions do the right thing.

class gpt_cf_triad
{
    friend class gpt_cf_triad_test;

  public:
    gpt_cf_triad
        (std::vector<double> const& qc
        ,std::vector<double> const& glp_ic
        ,std::vector<double> const& glp_ig
        ,std::vector<double> const& gsp_ic
        ,std::vector<double> const& gsp_ig
        ,gpt_vector_parms    const& charges
        );

    double calculate_premium(oenum_glp_or_gsp, gpt_scalar_parms const&) const;

  private:
    double calculate_premium
        (oenum_glp_or_gsp
        ,gpt_scalar_parms const&
        ,mcenum_dbopt_7702
        ) const;

    int const   length_;
    gpt_commfns cf_glp_dbo_1;
    gpt_commfns cf_glp_dbo_2;
    gpt_commfns cf_gsp;
};

#endif // gpt_commutation_functions_hpp
