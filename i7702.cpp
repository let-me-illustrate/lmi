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
/// that statutory rates are always scalar.
///
/// Several rates must be considered:
///
/// A: statutory rates (concrete values A0 and A1 described below)
/// B: contractual guarantees
/// C: initial short-term guarantees
/// D: asset-based charges
/// E: NAAR discount
///
/// and several account-paths for each of {B,C,D}:
///
/// gen: general account  (unloaned)
/// sep: separate account (unloaned)
/// flr: fixed loan rate
/// vlr: variable loan rate
///
/// Let "max" mean the year-by-year maximum of vectors: e.g.,
///   max({1 2 3}, {0 2 4}) is {1 2 4}
///
/// Then the general formula is the maximum of
///   max(A, B, C) - D
/// along each account-path, i.e.
///   max
///     (max(A, Bgen, Cgen) - Dgen
///     ,max(A, Bsep, Csep) - Dsep
///     ,max(A, Bflr, Cflr) - Dflr
///     ,max(A, Bvlr, Cvlr) - Dvlr
///     )
/// where A is chosen from {A0, A1} as appropriate,
/// and the particular formulas are
///   ic_usual  max(A0, B, C)
///   ic_glp    max(A0, B   ) - D
///   ic_gsp    max(A1, B, C) - D
/// where C and D are taken as zero if omitted.
///
/// All ig are zero iff E uniformly equals zero; otherwise each is
/// the greater of its ic counterpart and E:
///   ig_usual  max(ic_usual, E)
///   ig_glp    max(ic_glp,   E)
///   ig_gsp    max(ic_gsp,   E)
///
/// Exhaustive list of use cases:
///   {GLP; GSP; CVAT NSP and corridor; §7702A NSP; 7PP; DCV}
/// All but {GLP; GSP} use "usual" rates.
///
/// Notes on input rates
///
/// A: statutory rates (always scalar)
///   A0: all but GSP (e.g., 4% as of 1984)
///   A1: GSP only (A0 + 2% in current statute)
///
/// B: contractual guarantees
///   for loans: charged - spread
///   usually no guarantee for separate account, making Bsep zero
///
/// C: initial short-term guarantees
///   usually altogether avoided by careful product design
///   variable loan rate may cause Cvlr to be nonzero
///   always zero in practice for lmi, which doesn't yet implement VLR
///
/// D: asset-based charges
///   lowest value each year, if dependent on assets, premiums, etc.
///   separate-account-only charges affect only Dsep
///
/// E: NAAR discount (given here as i, the annual rate of interest)
///   often specified in contract as Bgen upper 12 / 12
///     if monthly contract factor rounded down, Bgen governs instead
///     (slightly better 7702 outcome in that case)
///   but some policies do not discount NAAR
///     in which case E uniformly equals zero
///   an assertion checks that either E=0 or E materially equals Bgen
///
/// Monthly and annual rates
///
/// For consistency, the formulas above are expressed in terms of
/// annual rates only. Most of the calculations use only annual rates,
/// transforming them (i --> i upper 12 / 12) to monthly as a final
/// step for actual use with monthly UL commutation functions.
///
/// However, to preserve accuracy, the ig* calculations are performed
/// on a monthly basis. The rate E above is normally expressed on a
/// monthly basis, often with rounding; it makes no sense to convert
/// it to annual in a formula like this:
///   ig_usual  max(ic_usual, E)
/// and then convert the result back to monthly, especially since the
/// corresponding ic* rate must be converted to monthly anyway.
///
/// Therefore, lmi actually retrieves a monthly E (call it Em) from
/// the product database, and returns only monthly ig* and ic* rates.
///
/// Discussion
///
/// 7702 interest rates should be rounded up, if at all; lmi doesn't
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
/// the ic_usual rate, which ignores D because those charges must not
/// be double-counted; thus, DCV correctly reflects any dependence of
/// such charges on asset or premium tiers, which D cannot do.
///
/// For contracts that require an irrevocable choice, before delivery,
/// between a fixed and a variable loan rate, the path not chosen need
/// not be considered.
///
/// Idea not implemented: optionally set all ig* equal to E. The SOA
/// textbook (page 52) supports treating it as "a contractual element
/// that is not an interest rate guaranteed on issue of the contract"
/// for §7702 purposes. Yet it's simple to follow lmi's more careful
/// interpretation, which most often produces materially the same
/// result. If a contract specifies E as the monthly equivalent of
/// any rate other than Bgen, that's presumably just a mistake.
///
/// Present shortcomings of the code in this file:
///   [first change names to follow scheme above, adding new ones]
///   implementation is too simplistic:
///     igross: max(A0, Bmax)
///     iglp:   max(A0, Bmax) - D
///     igsp:   max(A1, Bmax) - D
///     ig:     max(A0, E)
///   ig problems:
///     A1 ignored: must reflect +2% for GSP
///       and that 2% extra should be parameterized
///     use contractual rates if greater

i7702::i7702
    (double                     A0
    ,double                     A1
    ,std::vector<double> const& Bgen
    ,std::vector<double> const& Bsep
    ,std::vector<double> const& Bflr
    ,std::vector<double> const& Bvlr
    ,std::vector<double> const& Cgen
    ,std::vector<double> const& Csep
    ,std::vector<double> const& Cflr
    ,std::vector<double> const& Cvlr
    ,std::vector<double> const& Dgen
    ,std::vector<double> const& Dsep
    ,std::vector<double> const& Dflr
    ,std::vector<double> const& Dvlr
    ,std::vector<double> const& Em
    )
    :A0_   {A0  }
    ,A1_   {A1  }
    ,Bgen_ {Bgen}
    ,Bsep_ {Bsep}
    ,Bflr_ {Bflr}
    ,Bvlr_ {Bvlr}
    ,Cgen_ {Cgen}
    ,Csep_ {Csep}
    ,Cflr_ {Cflr}
    ,Cvlr_ {Cvlr}
    ,Dgen_ {Dgen}
    ,Dsep_ {Dsep}
    ,Dflr_ {Dflr}
    ,Dvlr_ {Dvlr}
    ,Em_   {Em  }
{
    initialize();
}

i7702::i7702
    (product_database   const& database
    ,stratified_charges const& stratified
    )
    :A0_   {}
    ,A1_   {}
    ,Bgen_ (database.length())
    ,Bsep_ (database.length())
    ,Bflr_ (database.length())
    ,Bvlr_ (database.length())
    ,Cgen_ (database.length())
    ,Csep_ (database.length())
    ,Cflr_ (database.length())
    ,Cvlr_ (database.length())
    ,Dgen_ (database.length())
    ,Dsep_ (database.length())
    ,Dflr_ (database.length())
    ,Dvlr_ (database.length())
    ,Em_   (database.length())
{
    // Monthly guar net int for 7702 is
    //   greater of {iglp(), igsp()} and annual guar int rate
    //   less AV load
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    A0_ = database.query<double>(DB_AnnInterestRate7702);

    database.query_into(DB_GuarInt, Bgen_);

    // For 7702 purposes, the rate guaranteed by the contract is the
    // highest rate on any potential path, at each duration; thus,
    // it is no less than the guaranteed fixed loan rate, i.e.:
    //   (fixed rate charged on loans) - (guaranteed loan spread)
    if(!database.query<bool>(DB_IgnoreLoanRateFor7702))
        {
        std::vector<double> allow_fixed_loan;
        database.query_into(DB_AllowFixedLoan, allow_fixed_loan);
        if(!each_equal(allow_fixed_loan, false))
            {
            std::vector<double> gross_loan_rate;
            database.query_into(DB_FixedLoanRate    , gross_loan_rate);
            std::vector<double> guar_loan_spread;
            database.query_into(DB_GuarRegLoanSpread, guar_loan_spread);
            assign(Bflr_, gross_loan_rate - guar_loan_spread);
            }
        }

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
    //
    // For 7702, 'ig' should generally equal Eckley's 'ic'.

    std::vector<double> const zero(database.length(), 0.0);
    database.query_into(DB_NaarDiscount, Em_);
    bool const no_naar_discount = zero == Em_;
    std::vector<double> theoretical_naar_discount(database.length(), 0.0);
    theoretical_naar_discount +=
        apply_unary(i_upper_12_over_12_from_i<double>(), Bgen_);

    std::vector<double> diff(database.length(), 0.0);
    diff += fabs(Em_ - theoretical_naar_discount);
    minmax<double> const mm(diff);
    constexpr double tolerance {0.0000001};
    LMI_ASSERT(no_naar_discount || mm < tolerance);

    // Here begin the dubious calculations that will be replaced.
    std::vector<double> guar_int = Bgen_;
    assign(guar_int, Max(guar_int, Bflr_));

    gross_.assign(database.length(), 0.0);
    assign
        (gross_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(A0_, guar_int)
            )
        );

    net_glp_.assign(database.length(), 0.0);
    assign
        (net_glp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(A0_, guar_int) - Dsep_
            )
        );

    net_gsp_.assign(database.length(), 0.0);
    assign
        (net_gsp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,Max(0.02 + A0_, guar_int) - Dsep_
            )
        );

    std::vector<double> operative_naar_discount(database.length(), 0.0);
    operative_naar_discount +=
        Max
            (i_upper_12_over_12_from_i<double>()(A0_)
            ,Max(Em_, theoretical_naar_discount)
            );
    ig_ = no_naar_discount ? zero : operative_naar_discount;
}

void i7702::initialize()
{
}
