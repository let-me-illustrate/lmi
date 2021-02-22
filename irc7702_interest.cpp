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

#include "irc7702_interest.hpp"

#include "contains.hpp"                 // 7702 !! obsolescent
#include "database.hpp"
#include "global_settings.hpp"          // 7702 !! obsolescent
#include "stratified_charges.hpp"

/// Statutory interest rate for 7702 and 7702A (except GSP).
///
/// Conditionally use the H.R. 6800 transitional rates. See:
///   https://www.congress.gov/bill/116th-congress/house-bill/6800/text

double iglp()
{
    static bool const hr6800 =
           global_settings::instance().ash_nazg()
        && contains(global_settings::instance().pyx(), "HR6800")
        ;
    return hr6800 ? 0.02 : 0.04;
}

/// Statutory interest rate for 7702 (GSP only).
///
/// Implemented in terms of iglp(), q.v.

double igsp()
{
    return 0.02 + iglp();
}

i7702::i7702
    (product_database   const& database
    ,stratified_charges const& stratified
    )
    :database_   {database}
    ,stratified_ {stratified}
{
#if 0
    SpreadFor7702_.assign
        (Length
        ,StratifiedCharges_->minimum_tiered_spread_for_7702()
        );

    // Monthly guar net int for 7702 is
    //   greater of {iglp(), igsp()} and annual guar int rate
    //   less 7702 spread
    //   transformed to monthly (simple subtraction?).
    // These interest rates belong here because they're used by
    // DCV calculations in the account value class as well as
    // GPT calculations in the 7702 class.

    std::vector<double> statutory7702i;
    database().query_into(DB_AnnInterestRate7702, statutory7702i);

    std::vector<double> guar_int;
    database().query_into(DB_GuarInt, guar_int);

    // For 7702 purposes, the rate guaranteed by the contract is the
    // highest rate on any potential path, at each duration; thus,
    // it is no less than the guaranteed fixed loan rate, i.e.:
    //   (fixed rate charged on loans) - (guaranteed loan spread)
    if(!database().query<bool>(DB_IgnoreLoanRateFor7702))
        {
        std::vector<double> allow_fixed_loan;
        database().query_into(DB_AllowFixedLoan, allow_fixed_loan);
        if(!each_equal(allow_fixed_loan, false))
            {
            std::vector<double> gross_loan_rate;
            database().query_into(DB_FixedLoanRate    , gross_loan_rate);
            std::vector<double> guar_loan_spread;
            database().query_into(DB_GuarRegLoanSpread, guar_loan_spread);
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

    Mly7702iGlp.assign(Length, 0.0);
    assign
        (Mly7702iGlp
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), statutory7702i, guar_int) - SpreadFor7702_
            )
        );

    Mly7702iGsp.assign(Length, 0.0);
    assign
        (Mly7702iGsp
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), 0.02 + statutory7702i, guar_int) - SpreadFor7702_
            )
        );

    database().query_into(DB_NaarDiscount, Mly7702ig);
#endif // 0
}
