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

#ifndef gpt_commutation_functions_hpp
#define gpt_commutation_functions_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"       // mcenum_dbopt_7702

#include <vector>

/// Loads, fees, and monthly QAB rates used in GPT calculations.
///
/// It is simpler to pass these data around as one "parameter object"
/// than as separate parameters.
///
/// Copying vectors into this struct doesn't cost much. The cost could
/// be avoided by making them reference members.
///
/// Mortality and interest rates could have been included. However,
/// unlike other parameters, they may need various adjustments--e.g.,
/// conversion from annual to monthly, subject to some maximum or
/// minimum.

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

/// Commutation functions specialized for GPT calculations.
///
/// See section 14.3 here:
///   http://www.nongnu.org/lmi/7702.html
///
/// Only these specialized functions are needed. Once they're set, the
/// canonical commutation functions {C,D,N} from which they're derived
/// can be discarded. The function M is retained because it is used in
/// an unspecialized form. The value of D at the endowment duration is
/// kept for valuing the endowment benefit; it may at first seem that
/// it could be subsumed into M, but it is always multiplied by the
/// specified amount, whereas M may be multiplied by death benefit.
///
/// Ctor arguments are assumed to include any applicable rating, such
/// as an occupational extra on an accident benefit.
///
/// QABs are enumerated in 7702(f)(5)(i-iv). The benefit amounts by
/// which their rates are multiplied are all specified explicitly.
/// This uniform approach is the most general, and entails little
/// duplication in the case that the QAB rate is multiplied by the
/// specified amount, as frequently occurs with accident and waiver
/// benefits. A waiver benefit whose charge depends on actual monthly
/// deductions cannot be treated as a QAB--see section 11/7 here:
///   http://www.nongnu.org/lmi/7702.html
/// A vector of zeros may of course be passed as the rate vector for
/// any QAB that is not offered or not elected, or that the insurer
/// chooses not to treat as a QAB.
///
/// Implicitly-declared special member functions do the right thing.

class gpt_commfns
{
    gpt_commfns
        (unsigned int        const  length
        ,std::vector<double> const& qc
        ,std::vector<double> const& ic
        ,std::vector<double> const& ig
        ,mcenum_dbopt_7702   const  dbo
        ,gpt_vector_parms    const& charges
        );
    ~gpt_commfns();

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

#endif // gpt_commutation_functions_hpp

