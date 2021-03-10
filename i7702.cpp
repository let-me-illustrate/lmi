// 7702 (and 7702A) interest rates.
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
#include "database.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "miscellany.hpp"               // each_equal(), minmax
#include "stratified_charges.hpp"

/// Here's how lmi determines §7702 and §7702A interest rates.
///
/// All these variables are vectors that may vary by year, except
/// statutory rates marked as scalar.
///
/// "max" means the year-by-year maximum: e.g.,
///   max({1 2 3}, {0 2 4}) is {1 2 4}
///
/// A: statutory rates (always scalar)
///   A0: all but GSP (e.g., 4% as of 1984)
///   A1: GSP only (A0 + 2% in current statute)
///
/// B: contractual guarantees
///   B0: unloaned gen acct (specified by contract)
///   B1: loaned, fixed rate (max net = charged - credited)
///   B2: loaned, variable rate (max net, ascertainable from contract)
///   B3: sep acct (generally 0%, so no effect on Bmax)
///   Bmax: max(B0, B1, B2, B3)
///
/// C: initial short-term guarantees
///   good product design generally creates none
///   therefore, always zero in practice for lmi
///
/// D: asset-based charges
///   lowest value each year, if dependent on assets, premiums, etc.
///
/// E: NAAR discount (given here as i, the annual rate of interest)
///   almost always specified in contract as B0 upper 12 / 12
///     which should be rounded up, if at all
///     if it was rounded down, B0 governs instead
///   but some policies do not discount NAAR, in which case
///     but E uniformly equals zero
///   therefore, assert that either E=0 or E materially equals B0
///
/// ic_usual  max(A0, Bmax, C   )
/// ig_usual  max(A0, B0,   C, E)
/// ic_glp    max(A0, Bmax      ) - D
/// ig_glp    max(A0, B0,      E) - D
/// ic_gsp    max(A1, Bmax, C   ) - D
/// ig_gsp    max(A1, B0,   C, E) - D
///   but all ig* are zero if E uniformly equals zero
///
/// Exhaustive list of use cases:
///   {GLP, GSP, CVAT NSP, §7702A NSP, 7PP, and DCV}
/// All but {GLP, GSP} use "usual" rates.
///
/// Discussion
///
/// Interest rates should be rounded up, if at all; lmi doesn't
/// round them at all.
///
/// ig may actually exceed ic; for example:
///   ic = 4% guaranteed rate
///   ig = 4.00000001% implied by contractual NAAR discount
/// The contractual NAAR discount affects only ig. Effect:
///   lower  ig --> higher guideline
///   higher ig --> lower guideline
/// so this could be considered "conservative", but really it just
/// follows from §7702(b)(2)(A)'s "greater of" prescription.
///
/// Asset-based charges (D above) affect the interest rate only for
/// calculation of guideline premiums. They're expense charges, which
/// must be ignored for 7PP and for the §7702 as well as §7702A NSPs
/// because those quantities are net premiums. DCV calculations deduct
/// actual charges during monthly processing, and credit interest at
/// the ic_usual rate, which ignores D, because those charges must not
/// be double-counted; thus, DCV correctly reflects any dependence of
/// such charges on asset or premium tiers, which D cannot do.
///
/// Idea not implemented: optionally set all ig* equal to E. The SOA
/// textbook (page 52) supports treating it as "a contractual element
/// that is not an interest rate guaranteed on issue of the contract"
/// for §7702 purposes. Yet it's simple to follow lmi's more careful
/// interpretation, which most often produces materially the same
/// result. If a contract specifies E as the monthly equivalent of
/// any rate other than B0, that's presumably just a mistake.
///
/// Present shortcomings of the code in this file:
///   [first change names to follow scheme above, adding new ones]
///   formulas are too simplistic:
///     igross: max(A0, Bmax)
///     iglp:   max(A0, Bmax) - D
///     igsp:   max(A1, Bmax) - D
///     ig:     max(A0, E)
///   ig problems:
///     A1 ignored: must reflect +2% for GSP
///       and that 2% extra should be parameterized
///     use contractual rates if greater
///   separate-account-only charges: deduct only from B3?

i7702::i7702
    (product_database   const& database
    ,stratified_charges const& stratified
    )
    :database_   {database}
    ,stratified_ {stratified}
{
    // Monthly guar net int for 7702 is
    //   greater of {iglp(), igsp()} and annual guar int rate
    //   less AV load
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> statutory7702i;
    database_.query_into(DB_AnnInterestRate7702, statutory7702i);

    std::vector<double> guar_int;
    database_.query_into(DB_GuarInt, guar_int);
    std::vector<double> const contractual_guar_int = guar_int;

    // For 7702 purposes, the rate guaranteed by the contract is the
    // highest rate on any potential path, at each duration; thus,
    // it is no less than the guaranteed fixed loan rate, i.e.:
    //   (fixed rate charged on loans) - (guaranteed loan spread)
    if(!database_.query<bool>(DB_IgnoreLoanRateFor7702))
        {
        std::vector<double> allow_fixed_loan;
        database_.query_into(DB_AllowFixedLoan, allow_fixed_loan);
        if(!each_equal(allow_fixed_loan, false))
            {
            std::vector<double> gross_loan_rate;
            database_.query_into(DB_FixedLoanRate    , gross_loan_rate);
            std::vector<double> guar_loan_spread;
            database_.query_into(DB_GuarRegLoanSpread, guar_loan_spread);
            assign
                (guar_int
                ,Max(guar_int, gross_loan_rate - guar_loan_spread)
                );
            }
        }

    // If lmi someday implements VLR, then the current VLR rate on
    // the issue date constitutes a short-term guarantee that must be
    // reflected in the 7702 interest rates (excluding the GLP rate).

    std::vector<double> av_load;
    database_.query_into(DB_CurrAcctValLoad, av_load);
    if
        (   database_.query<bool>(DB_AllowSepAcct)
        && !database_.query<bool>(DB_AllowGenAcct)
        )
        {
        av_load += stratified_.minimum_tiered_sepacct_load_for_7702();
        }

    gross_.assign(database_.length(), 0.0);
    assign
        (gross_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(statutory7702i, guar_int)
            )
        );

    net_glp_.assign(database_.length(), 0.0);
    assign
        (net_glp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(statutory7702i, guar_int) - av_load
            )
        );

    net_gsp_.assign(database_.length(), 0.0);
    assign
        (net_gsp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(0.02 + statutory7702i, guar_int) - av_load
            )
        );

    // Eckley's 'ig' represents the interest rate by which death
    // benefit is discounted for calculating mortality charges,
    // as seen in his formula (1):
    //   [0V + P - Q(1/(1 + ig) - OV - P)] (1 + ic) = 1V
    // where it is the monthly (i upper 12 over 12) equivalent of
    // the annual 'contractual_guar_int' rate above. Specifying a
    // discount based on any other rate is presumably an error.
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
    //
    // For 7702, 'ig' should generally equal Eckley's 'ic'.

    std::vector<double> const zero(database_.length(), 0.0);
    std::vector<double> contractual_naar_discount;
    database_.query_into(DB_NaarDiscount, contractual_naar_discount);
    bool const no_naar_discount = zero == contractual_naar_discount;
    std::vector<double> theoretical_naar_discount(database_.length(), 0.0);
    theoretical_naar_discount +=
        apply_unary(i_upper_12_over_12_from_i<double>(), contractual_guar_int);

    std::vector<double> diff(database_.length(), 0.0);
    diff += fabs(contractual_naar_discount - theoretical_naar_discount);
    minmax<double> const mm(diff);
    constexpr double tolerance {0.0000001};
    LMI_ASSERT(no_naar_discount || mm < tolerance);

    std::vector<double> operative_naar_discount(database_.length(), 0.0);
    operative_naar_discount +=
        Max
            (apply_unary(i_upper_12_over_12_from_i<double>(), statutory7702i)
            ,Max(contractual_naar_discount, theoretical_naar_discount)
            );
    ig_ = no_naar_discount ? zero : operative_naar_discount;
}
