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

#include "database.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "miscellany.hpp"               // each_equal()
#include "stratified_charges.hpp"

i7702::i7702
    (product_database   const& database
    ,stratified_charges const& stratified
    )
    :database_   {database}
    ,stratified_ {stratified}
{
    spread_.assign
        (database_.length()
        ,stratified_.minimum_tiered_spread_for_7702()
        );

    // Monthly guar net int for 7702 is
    //   greater of {iglp(), igsp()} and annual guar int rate
    //   less 7702 spread
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> statutory7702i;
    database_.query_into(DB_AnnInterestRate7702, statutory7702i);

    std::vector<double> guar_int;
    database_.query_into(DB_GuarInt, guar_int);

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
                ,apply_binary
                    (greater_of<double>()
                    ,guar_int
                    ,gross_loan_rate - guar_loan_spread
                    )
                );
            }
        }

    // If lmi someday implements VLR, then the current VLR rate on
    // the issue date constitutes a short-term guarantee that must be
    // reflected in the 7702 interest rates (excluding the GLP rate).

    gross_.assign(database_.length(), 0.0);
    assign
        (gross_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), statutory7702i, guar_int)
            )
        );

    net_glp_.assign(database_.length(), 0.0);
    assign
        (net_glp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), statutory7702i, guar_int) - spread_
            )
        );

    net_gsp_.assign(database_.length(), 0.0);
    assign
        (net_gsp_
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), 0.02 + statutory7702i, guar_int) - spread_
            )
        );

    // Eckley's 'ig' represents the interest rate by which death
    // benefit is discounted for calculating mortality charges,
    // as seen in his formula (1):
    //   [0V + P - Q(1/(1 + ig) - OV - P)] (1 + ic) = 1V
    // For 7702, that 'ig' should generally be 'gross_' above.
    //
    // However, if the contract applies no such discount, then 'ig'
    // must be zero for formula (1) to apply. As of 2021-02, lmi
    // supports one ancient product that seems to have no such
    // discount. This is so extraordinary that it doesn't merit
    // a special database flag. Instead, the discount is deemed
    // to be absent iff the contractual discount according to the
    // product database is uniformly zero.

    std::vector<double> const zero(database_.length(), 0.0);
    std::vector<double> contractual_naar_discount;
    database_.query_into(DB_NaarDiscount, contractual_naar_discount);
    ig_ = (zero == contractual_naar_discount) ? zero : gross_;
}
