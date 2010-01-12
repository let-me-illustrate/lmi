// Product database map.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "dbdict.hpp"

#include "dbnames.hpp"
#include "mc_enum_type_enums.hpp"

#include <cstring>

std::string DBDictionary::CachedFilename;

//============================================================================
DBDictionary& DBDictionary::instance()
{
    static DBDictionary z;
    return z;
}

//============================================================================
DBDictionary::DBDictionary()
{
    dictionary = new TDBDictionary;
}

//============================================================================
DBDictionary::~DBDictionary()
{
    (*dictionary).clear();
    delete dictionary;
    dictionary = 0;
}

//============================================================================
void DBDictionary::Init(std::string const& NewFilename)
{
    // We don't need to perform the expensive operation of reading the
    // dictionary from file if it's the same dictionary we last read.
    // We don't address the problem that someone might have modified
    // that file in the meantime.

    // TODO ?? Consider rewriting this as
    // if(!condition)
    //   {
    //   [do something]
    //   }

    if
        (
                // But we must read the dictionary from file if we've
                // never yet read in any dictionary.
                ("" != CachedFilename)
            && (CachedFilename == NewFilename)
        )
        {
        // do nothing
        }
    else
        {
        CachedFilename = NewFilename;
        delete dictionary;
        dictionary = new TDBDictionary;
        // stream from file
// TODO ?? HERE WE NEED TO ADD A STREAMING METHOD
// TODO ?? CachedDBDictionary = Dictionary = new TDBDictionary(CachedFilename);
        Init(); // TODO ?? KLUDGE
        }
}

//============================================================================
void DBDictionary::Init()
{
    static int const n = 1 + TDBIndex::MaxIndex;
    int dims[n] = {1, 1, 1, 1, 1, 1, 1};

    dictionary->erase(dictionary->begin(), dictionary->end());

    double zilch[1] = {0.0};
    // Zero is inappropriate for some entities ("DB_CCOIMultiplier",
    // e.g.), but the antediluvian branch doesn't actually use most
    // database entities.
    for(int j = DB_FIRST; j < DB_LAST; ++j)
        {
        AddEntry(TDBEntry(j, TDBValue(j, n, dims, zilch)));
        }

    double guar_int_rate[1] = {0.03};
    AddEntry(TDBEntry(DB_GuarInt, TDBValue(DB_GuarInt, n, dims, guar_int_rate)));

    double fixed_loan_rate[1] = {0.06};
    AddEntry(TDBEntry(DB_FixedLoanRate, TDBValue(DB_FixedLoanRate, n, dims, fixed_loan_rate)));

    double loan_spread[1] = {0.0};

    AddEntry(TDBEntry(DB_GuarRegLoanSpread, TDBValue(DB_GuarRegLoanSpread, n, dims, loan_spread)));
    AddEntry(TDBEntry(DB_CurrRegLoanSpread, TDBValue(DB_CurrRegLoanSpread, n, dims, loan_spread)));
    AddEntry(TDBEntry(DB_GuarPrefLoanSpread, TDBValue(DB_GuarPrefLoanSpread, n, dims, loan_spread)));
    AddEntry(TDBEntry(DB_CurrPrefLoanSpread, TDBValue(DB_CurrPrefLoanSpread, n, dims, loan_spread)));

    double affirmative[1] = {1.0};

    AddEntry(TDBEntry(DB_AllowGenAcct, TDBValue(DB_AllowGenAcct, n, dims, affirmative)));
    AddEntry(TDBEntry(DB_AllowPreferredClass, TDBValue(DB_AllowPreferredClass, n, dims, affirmative)));

    // premium loads

    double load[1] = {0.025};
    double cfee[1] = {5.00};
    double gfee[1] = {12.00};
    double zero[1] = {0.0};

    AddEntry(TDBEntry(DB_GuarPolFee, TDBValue(DB_GuarPolFee, n, dims, gfee)));
    AddEntry(TDBEntry(DB_GuarSpecAmtLoad, TDBValue(DB_GuarSpecAmtLoad, n, dims, zero)));
    AddEntry(TDBEntry(DB_GuarPremLoadTgt, TDBValue(DB_GuarPremLoadTgt, n, dims, load)));
    AddEntry(TDBEntry(DB_GuarPremLoadExc, TDBValue(DB_GuarPremLoadExc, n, dims, load)));
    AddEntry(TDBEntry(DB_CurrPolFee, TDBValue(DB_CurrPolFee, n, dims, cfee)));
    AddEntry(TDBEntry(DB_CurrSpecAmtLoad, TDBValue(DB_CurrSpecAmtLoad, n, dims, zero)));
    AddEntry(TDBEntry(DB_CurrPremLoadTgt, TDBValue(DB_CurrPremLoadTgt, n, dims, load)));
    AddEntry(TDBEntry(DB_CurrPremLoadExc, TDBValue(DB_CurrPremLoadExc, n, dims, load)));

    double minwd[1] = {100.0};
    AddEntry(TDBEntry(DB_MinWD, TDBValue(DB_MinWD, n, dims, minwd)));
    double wdfee[1] = {5.0};
    AddEntry(TDBEntry(DB_WDFee, TDBValue(DB_WDFee, n, dims, wdfee)));
    double wdfeerate[1] = {0.01};
    AddEntry(TDBEntry(DB_WDFeeRate, TDBValue(DB_WDFeeRate, n, dims, wdfeerate)));

    int guar_coi_dims[n] = {1, 1, 3, 1, 1, 1, 1};
    // smoker, nonsmoker, unismoke
    double guar_coi_tables[] = {111, 109, 107};
    AddEntry
        (TDBEntry
            (DB_GuarCOITable
            ,TDBValue(DB_GuarCOITable, n, guar_coi_dims, guar_coi_tables)
            )
        );

    int curr_coi_dims[n] = {1, 4, 3, 1, 1, 1, 1};
    // preferred, standard, rated, ultrapreferred by smoker, nonsmoker, unismoke
    double curr_coi_tables[] =
        {
        2, 3, 1, // pref  sm ns us
        5, 6, 4, // std   sm ns us
        5, 6, 4, // rated sm ns us
        0, 0, 0, // ultra sm ns us
        };
    AddEntry
        (TDBEntry
            (DB_CurrCOITable
            ,TDBValue(DB_CurrCOITable, n, curr_coi_dims, curr_coi_tables)
            )
        );

    double corr_table[1] = {7};
    AddEntry(TDBEntry(DB_CorridorTable, TDBValue(DB_CorridorTable, n, dims, corr_table)));

    double wp_table[1] = {8};
    AddEntry(TDBEntry(DB_WPTable, TDBValue(DB_WPTable, n, dims, wp_table)));

    double add_table[1] = {9};
    AddEntry(TDBEntry(DB_ADDTable, TDBValue(DB_ADDTable, n, dims, add_table)));

    double endtage[1] = {100};
    AddEntry(TDBEntry(DB_EndtAge, TDBValue(DB_EndtAge, n, dims, endtage)));

    double anb[1] = {1.0};
    AddEntry(TDBEntry(DB_AgeLastOrNearest, TDBValue(DB_AgeLastOrNearest, n, dims, anb)));

    double minspecamt[1] = {10000.0};
    AddEntry(TDBEntry(DB_MinSpecAmt, TDBValue(DB_MinSpecAmt, n, dims, minspecamt)));

    double max_gen_acct_rate[1] = {0.12};
    AddEntry(TDBEntry(DB_MaxGenAcctRate, TDBValue(DB_MaxGenAcctRate, n, dims, max_gen_acct_rate)));
    double max_sep_acct_rate[1] = {0.12};
    AddEntry(TDBEntry(DB_MaxSepAcctRate, TDBValue(DB_MaxSepAcctRate, n, dims, max_sep_acct_rate)));

    double allow_loan[1] = {1.0};
    AddEntry(TDBEntry(DB_AllowLoan, TDBValue(DB_AllowLoan, n, dims, allow_loan)));
    double allow_wd[1] = {1.0};
    AddEntry(TDBEntry(DB_AllowWD, TDBValue(DB_AllowWD, n, dims, allow_wd)));
    double allow_flat_extras[1] = {1.0};
    AddEntry(TDBEntry(DB_AllowFlatExtras, TDBValue(DB_AllowFlatExtras, n, dims, allow_flat_extras)));
    double allow_change_to_dbo2[1] = {1.0};
    AddEntry(TDBEntry(DB_AllowChangeToDBO2, TDBValue(DB_AllowChangeToDBO2, n, dims, allow_change_to_dbo2)));
    double allow_dbo3[1] = {1.0};
    AddEntry(TDBEntry(DB_AllowDBO3, TDBValue(DB_AllowDBO3, n, dims, allow_dbo3)));

    double surrchg_prem_mult[1] = {0.0};
    AddEntry(TDBEntry(DB_SurrChgPremMult, TDBValue(DB_SurrChgPremMult, n, dims, surrchg_prem_mult)));
    double surrchg_av_mult[1] = {0.0};
    AddEntry(TDBEntry(DB_SurrChgAVMult, TDBValue(DB_SurrChgAVMult, n, dims, surrchg_av_mult)));
    double surrchg_sa_mult[1] = {0.0};
    AddEntry(TDBEntry(DB_SurrChgSAMult, TDBValue(DB_SurrChgSAMult, n, dims, surrchg_sa_mult)));
    double surrchg_av_dur_factor[1] = {1.0};
    AddEntry(TDBEntry(DB_SurrChgAVDurFactor, TDBValue(DB_SurrChgAVDurFactor, n, dims, surrchg_av_dur_factor)));
    double surrchg_sa_dur_factor[1] = {1.0};
    AddEntry(TDBEntry(DB_SurrChgSADurFactor, TDBValue(DB_SurrChgSADurFactor, n, dims, surrchg_sa_dur_factor)));

    double ledger_type[1] = {mce_ill_reg};
    AddEntry(TDBEntry(DB_LedgerType, TDBValue(DB_LedgerType, n, dims, ledger_type)));

    double no_lapse_always_active[1] = {0.0};
    AddEntry(TDBEntry(DB_NoLapseAlwaysActive, TDBValue(DB_NoLapseAlwaysActive, n, dims, no_lapse_always_active)));
    double no_lapse_min_dur[1] = {0.0};
    AddEntry(TDBEntry(DB_NoLapseMinDur, TDBValue(DB_NoLapseMinDur, n, dims, no_lapse_min_dur)));
    double no_lapse_min_age[1] = {0.0};
    AddEntry(TDBEntry(DB_NoLapseMinAge, TDBValue(DB_NoLapseMinAge, n, dims, no_lapse_min_age)));

    AddEntry(TDBEntry(DB_NominallyPar, TDBValue(DB_NominallyPar, n, dims, zero)));
    AddEntry(TDBEntry(DB_Has1035ExchCharge, TDBValue(DB_Has1035ExchCharge, n, dims, zero)));
    AddEntry(TDBEntry(DB_SmokeOrTobacco, TDBValue(DB_SmokeOrTobacco, n, dims, zero)));
    AddEntry(TDBEntry(DB_DACTaxFundCharge, TDBValue(DB_DACTaxFundCharge, n, dims, zero)));
    AddEntry(TDBEntry(DB_AllowWP, TDBValue(DB_AllowWP, n, dims, zero)));
    AddEntry(TDBEntry(DB_AllowADD, TDBValue(DB_AllowADD, n, dims, zero)));
    AddEntry(TDBEntry(DB_AllowSpouse, TDBValue(DB_AllowSpouse, n, dims, zero)));
    AddEntry(TDBEntry(DB_AllowChild, TDBValue(DB_AllowChild, n, dims, zero)));

    double exp_rat_amort_period[1] = {4.0};
    AddEntry(TDBEntry(DB_ExpRatAmortPeriod, TDBValue(DB_ExpRatAmortPeriod, n, dims, exp_rat_amort_period)));
}

//===========================================================================
void DBDictionary::AddEntry(TDBEntry const& e)
{
    (*dictionary)[e.first] = e.second;
}

//===========================================================================
// Unimplemented in this branch.
void print_databases()
{
}

