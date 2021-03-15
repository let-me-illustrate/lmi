// 7702 (and 7702A) interest rates--initialization.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "i7702.hpp"

#include "assert_lmi.hpp"
#include "contains.hpp"                 // 7702 !! pyx
#include "database.hpp"
#include "et_vector.hpp"
#include "global_settings.hpp"          // 7702 !! pyx
#include "math_functions.hpp"
#include "miscellany.hpp"               // minmax
#include "stratified_charges.hpp"

i7702::i7702
    (product_database   const& database
    ,stratified_charges const& stratified
    )
    :length_   {database.length()}
    ,trace_    {contains(global_settings::instance().pyx(), "show_7702i")}
    ,A0_       {}
    ,A1_       {}
    ,Bgen_     (length_)
    ,Bsep_     (length_)
    ,Bflr_     (length_)
    ,Bvlr_     (length_)
    ,Cgen_     (length_)
    ,Csep_     (length_)
    ,Cflr_     (length_)
    ,Cvlr_     (length_)
    ,Dgen_     (length_)
    ,Dsep_     (length_)
    ,Dflr_     (length_)
    ,Dvlr_     (length_)
    ,Em_       (length_)
    ,use_gen_  (length_)
    ,use_sep_  (length_)
    ,use_flr_  (length_)
    ,use_vlr_  (length_)
    ,ic_usual_ (length_)
    ,ic_glp_   (length_)
    ,ic_gsp_   (length_)
    ,ig_usual_ (length_)
    ,ig_glp_   (length_)
    ,ig_gsp_   (length_)
{
    // 7702 !! Should 'C*' members be scalar--first year only?

    std::vector<double> const zero(length_);

    database.query_into(DB_AllowGenAcct  , use_gen_);
    database.query_into(DB_AllowSepAcct  , use_sep_);
    database.query_into(DB_AllowFixedLoan, use_flr_);
    database.query_into(DB_AllowVlr      , use_vlr_);

    if(database.query<bool>(DB_IgnoreLoanRateFor7702))
        {
        use_flr_ = zero;
        use_vlr_ = zero;
        }

    // 7702 !! Assert that all use_* are boolean.
    // 7702 !! Assert (use_gen_ || use_sep_) for each duration.

    // 7702 !! Alternatively, specify A0_ and delta, then calculate A1_?
    A0_ = database.query<double>(DB_AnnInterestRate7702);
    A1_ = 0.02 + A0_;

    database.query_into(DB_GuarInt, Bgen_);

    std::vector<double> fixed_loan_rate;
    database.query_into(DB_FixedLoanRate, fixed_loan_rate);
    // This isn't the actual rate--lmi doesn't yet implement VLR.
    std::vector<double> variable_loan_rate;
    database.query_into(DB_MaxVlrRate, variable_loan_rate);
    std::vector<double> guar_loan_spread;
    database.query_into(DB_GuarRegLoanSpread, guar_loan_spread);
    assign(Bflr_, fixed_loan_rate    - guar_loan_spread);
    assign(Bvlr_, variable_loan_rate - guar_loan_spread);

    // If lmi someday implements VLR, then the current VLR rate on
    // the issue date constitutes a short-term guarantee that must be
    // reflected in the 7702 interest rates (excluding the GLP rate).

    // It just so happens that these loads are zero for all products
    // lmi supports, except for separate-account-only products. The
    // logic will soon be reworked to make that irrelevant.
    // 7702 !! DB_CurrAcctValLoad is sepacct only: change its name
    database.query_into(DB_CurrAcctValLoad, Dsep_);
    Dsep_ += stratified.minimum_tiered_sepacct_load_for_7702();

    // Eckley's 'ig' represents the interest rate by which death
    // benefit is discounted for calculating mortality charges,
    // as seen in his formula (1):
    //   [0V + P - Q(1/(1 + ig) - OV - P)] (1 + ic) = 1V
    // where it is the monthly (i upper 12 over 12) equivalent of
    // the annual 'Bgen_' rate above. Specifying a discount based on
    // any other rate is presumably an error.
    //
    // In lmi's product database, DB_GuarInt is i. DB_NaarDiscount is
    // (i upper 12)/12, rounded iff the contract specifies a rounded
    // numerical value. An exception is thrown if the absolute value
    // of the quantization error exceeds a small (though arbitrary)
    // tolerance.
    //
    // However, if the contract applies no such discount, then 'ig'
    // must be zero for formula (1) to apply. As of 2021-02, lmi
    // supports one ancient product that seems to have no such
    // discount. This is so extraordinary that it doesn't merit
    // a special database flag. Instead, the discount is deemed
    // to be absent iff the contractual discount according to the
    // product database is uniformly zero.

    database.query_into(DB_NaarDiscount, Em_);
    bool const no_naar_discount = zero == Em_;
    std::vector<double> theoretical_naar_discount(length_);
    theoretical_naar_discount +=
        apply_unary(i_upper_12_over_12_from_i<double>(), Bgen_);

    std::vector<double> diff(length_);
    diff += fabs(Em_ - theoretical_naar_discount);
    minmax<double> const mm(diff);
    constexpr double tolerance {0.0000001};
    LMI_ASSERT(no_naar_discount || mm < tolerance);

    initialize();
}
