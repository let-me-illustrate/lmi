// Account value: monthly detail.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_avdebug.cpp,v 1.23 2006-09-05 13:59:22 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "account_value.hpp"

#include "calendar_date.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "death_benefits.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_proddata.hpp"
#include "ihs_rnddata.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "loads.hpp"
#include "outlay.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>

namespace
{
    inline std::string const& not_applicable()
        {
        static char const* n_a = "---";
        static std::string const s(n_a);
        return s;
        }

    // Columns can be rearranged by changing the order of enumerators.

    enum DebugColNames
        {eYear
        ,eMonth
        ,eRateBasis
        ,eAge
        ,eGenAcctBOMAV
        ,eSepAcctBOMAV
        ,eUnloanedBOMAV
        ,eRegularLoanBOMAV
        ,ePrefLoanBOMAV
        ,eTotalBOMAV
        ,eRegLoanBal
        ,ePrefLoanBal
        ,eDBOption
        ,eSpecAmt
        ,eCorridorFactor
        ,eDeathBft
        ,eForceout
        ,eEePrem
        ,eErPrem
        ,eTotalPrem
        ,eTargetPrem
        ,ePremLoad
        ,eNetPrem
        ,ePolicyFees
        ,eSpecAmtLoad
        ,eNAAR
        ,eCoiRate
        ,eCoiCharge
        ,eAdbRate
        ,eAdbCharge
        ,eWpRate
        ,eWpCharge
        ,eTermAmount
        ,eTermRate
        ,eTermCharge
        ,eTotalRiderCharge
        ,eTotalMonthlyDeds
        ,eGenAcctIntRate
        ,eGenAcctIntCred
        ,eSepAcctIntRate
        ,eSepAcctIntCred
        ,eAssetsPostBom
        ,eCumPmtsPostBom
        ,eSepAcctLoad
        ,eRegLnIntRate
        ,eRegLnIntCred
        ,ePrfLnIntRate
        ,ePrfLnIntCred
        ,eYearsHMValueRate
        ,eYearsPostHMRate
        ,eRequestedWD
        ,eMaxWD
        ,eGrossWD
        ,eNetWD
        ,eRequestedLoan
        ,eMaxLoan
        ,eNewLoan
        ,eTaxBasis
        ,eCumNoLapsePrem
        ,eNoLapseActive
        ,eEOMAV
        ,eHMValue
        ,eSurrChg
        ,eEOMCSVNet
        ,eEOMCV7702
        ,eInforceFactor
        ,eClaimsPaid
        ,e7702ATestDur
        ,e7702A7ppRate
        ,e7702ANsp
        ,e7702ALowestDb
        ,e7702ADeemedCv
        ,e7702ANetMaxNecPm
        ,e7702AGrossMaxNecPm
        ,e7702AUnnecPm
        ,e7702ADbAdj
        ,e7702A7pp
        ,e7702ACumPmts
        ,e7702ACum7pp
        ,e7702AIsMatChg
        ,e7702AIsMec
        ,eGSP
        ,eGLP
        // Insert new enumerators above
        ,eLast
        };

    // Headers for monthly output.

    inline std::vector<std::string> const& DebugColHeadersHelper()
    {
        static std::vector<std::string> v(eLast);
        v[eYear]                = "Year";
        v[eMonth]               = "Month";
        v[eRateBasis]           = "Rate basis";
        v[eAge]                 = "Age";
        v[eGenAcctBOMAV]        = "Unloaned BOM GA AV";
        v[eSepAcctBOMAV]        = "Unloaned BOM SA AV";
        v[eUnloanedBOMAV]       = "Unloaned BOM Tot AV";
        v[eRegularLoanBOMAV]    = "Regular loan BOM AV";
        v[ePrefLoanBOMAV]       = "Pref loan BOM AV";
        v[eTotalBOMAV]          = "Total BOM AV";
        v[eRegLoanBal]          = "Reg loan bal";
        v[ePrefLoanBal]         = "Pref loan bal";
        v[eDBOption]            = "DB option";
        v[eSpecAmt]             = "Spec amt";
        v[eCorridorFactor]      = "Corridor factor";
        v[eDeathBft]            = "Death benefit";
        v[eForceout]            = "Forceout";
        v[eEePrem]              = "Ee prem";
        v[eErPrem]              = "Er prem";
        v[eTotalPrem]           = "Total prem";
        v[eTargetPrem]          = "Target prem";
        v[ePremLoad]            = "Prem load";
        v[eNetPrem]             = "Net prem";
        v[ePolicyFees]          = "Policy fees";
        v[eSpecAmtLoad]         = "Spec amt load";
        v[eNAAR]                = "NAAR";
        v[eCoiRate]             = "COI rate";
        v[eCoiCharge]           = "COI charge";
        v[eAdbRate]             = "ADD rate";
        v[eAdbCharge]           = "ADD charge";
        v[eWpRate]              = "WP rate";
        v[eWpCharge]            = "WP charge";
        v[eTermAmount]          = "Term amount";
        v[eTermRate]            = "Term rate";
        v[eTermCharge]          = "Term charge";
        v[eTotalRiderCharge]    = "Total rider charge";
        v[eTotalMonthlyDeds]    = "Total monthly deductions";
        v[eGenAcctIntRate]      = "Unloaned GA interest rate";
        v[eGenAcctIntCred]      = "Unloaned GA interest credited";
        v[eSepAcctIntRate]      = "Unloaned SA interest rate";
        v[eSepAcctIntCred]      = "Unloaned SA interest credited";
        v[eAssetsPostBom]       = "Assets post BOM";
        v[eCumPmtsPostBom]      = "Cumulative payments post BOM";
        v[eSepAcctLoad]         = "Separate account load";
        v[eRegLnIntRate]        = "Regular loan interest rate";
        v[eRegLnIntCred]        = "Regular loan interest credited";
        v[ePrfLnIntRate]        = "Pref loan interest rate";
        v[ePrfLnIntCred]        = "Pref loan interest credited";
        v[eYearsHMValueRate]    = "Honeymoon value rate";
        v[eYearsPostHMRate]     = "Post honeymoon rate";
        v[eRequestedWD]         = "Requested wd";
        v[eMaxWD]               = "Max wd";
        v[eGrossWD]             = "Gross wd";
        v[eNetWD]               = "Net wd";
        v[eRequestedLoan]       = "Requested loan";
        v[eMaxLoan]             = "Max loan";
        v[eNewLoan]             = "New loan";
        v[eTaxBasis]            = "Tax basis";
        v[eCumNoLapsePrem]      = "Cum no lapse prem";
        v[eNoLapseActive]       = "No lapse active";
        v[eEOMAV]               = "EOM AV";
        v[eHMValue]             = "Honeymoon value";
        v[eSurrChg]             = "EOM surrender charge";
        v[eEOMCSVNet]           = "EOM CSV net";
        v[eEOMCV7702]           = "EOM CV for 7702";
        v[eInforceFactor]       = "Inforce factor";
        v[eClaimsPaid]          = "Partial mort claims paid";
        v[e7702ATestDur]        = "7702A test duration";
        v[e7702A7ppRate]        = "7702A 7pp rate";
        v[e7702ANsp]            = "7702A NSP";
        v[e7702ALowestDb]       = "7702A lowest DB";
        v[e7702ADeemedCv]       = "7702A deemed CV";
        v[e7702ANetMaxNecPm]    = "7702A net max nec prem";
        v[e7702AGrossMaxNecPm]  = "7702A gross max nec prem";
        v[e7702AUnnecPm]        = "7702A unnec prem";
        v[e7702ADbAdj]          = "7702A DB adjustment";
        v[e7702A7pp]            = "7702A 7pp";
        v[e7702ACumPmts]        = "7702A cum pmts";
        v[e7702ACum7pp]         = "7702A cum 7pp";
        v[e7702AIsMatChg]       = "Is material change";
        v[e7702AIsMec]          = "Is MEC";
        v[eGSP]                 = "GSP";
        v[eGLP]                 = "GLP";

        return v;
    }
} // Unnamed namespace.

inline std::vector<std::string> const& DebugColHeaders()
{
    static std::vector<std::string> v(DebugColHeadersHelper());
    return v;
}

//============================================================================
inline void AccountValue::SetMonthlyDetail(int enumerator, std::string const& s)
{
    DebugRecord[enumerator] = s;
}

//============================================================================
inline void AccountValue::SetMonthlyDetail(int enumerator, double d)
{
    DebugRecord[enumerator] = value_cast<std::string>(d);
}

//============================================================================
void AccountValue::SetDebugFilename(std::string const& s)
{
    DebugFilename = s;
}

//============================================================================
void AccountValue::DebugPrintInit()
{
    DebugRecord.assign(eLast, "EMPTY");

    if(!Debugging)
        {
        return;
        }

    DebugStream.open
        (DebugFilename.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    std::copy
        (DebugColHeaders().begin()
        ,DebugColHeaders().end()
        ,std::ostream_iterator<std::string>(DebugStream, "\t")
        );
    DebugStream << '\n';
}

//============================================================================
void AccountValue::DebugEndBasis()
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return;
        }
    DebugStream << '\n';
}

//============================================================================
void AccountValue::DebugRestart(std::string const& reason)
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return;
        }
    DebugStream << reason << '\n';
}

//============================================================================
// To add a new column, see ihs_dbughdr.hpp .
void AccountValue::DebugPrint()
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return; // Show detail on final run, not every solve iteration.
        }

    DebugRecord.assign(eLast, "EMPTY");

    SetMonthlyDetail(eYear               ,Year);
    SetMonthlyDetail(eMonth              ,Month);
    SetMonthlyDetail(eRateBasis          ,RateBasis.str());
    SetMonthlyDetail(eAge                ,InvariantValues().Age + Year);

    // Initial values at beginning of run, reflecting inforce if applicable.
    if(InforceYear == Year && InforceMonth == Month)
        {
        PriorAVGenAcct  = InforceAVGenAcct;
        PriorAVSepAcct  = InforceAVSepAcct;
        PriorAVRegLn    = InforceAVRegLn;
        PriorAVPrfLn    = InforceAVPrfLn;
        PriorRegLnBal   = InforceRegLnBal;
        PriorPrfLnBal   = InforcePrfLnBal;
        }

    // Beginning of month values.
    SetMonthlyDetail(eGenAcctBOMAV       ,PriorAVGenAcct);
    SetMonthlyDetail(eSepAcctBOMAV       ,PriorAVSepAcct);
    SetMonthlyDetail(eUnloanedBOMAV      ,PriorAVGenAcct + PriorAVSepAcct);
    SetMonthlyDetail(eRegularLoanBOMAV   ,PriorAVRegLn);
    SetMonthlyDetail(ePrefLoanBOMAV      ,PriorAVPrfLn);
    SetMonthlyDetail
        (eTotalBOMAV
        ,PriorAVGenAcct + PriorAVSepAcct + PriorAVRegLn + PriorAVPrfLn
        );

    PriorAVGenAcct  = AVGenAcct;
    PriorAVSepAcct  = AVSepAcct;
    PriorAVRegLn    = AVRegLn;
    PriorAVPrfLn    = AVPrfLn;

    // TODO ?? Should loan balance columns be similarly offset?

    SetMonthlyDetail(eRegLoanBal         ,RegLnBal                         );
    SetMonthlyDetail(ePrefLoanBal        ,PrfLnBal                         );
    SetMonthlyDetail(eDBOption           ,YearsDBOpt.str()                 );
    SetMonthlyDetail(eSpecAmt            ,ActualSpecAmt                    );
    SetMonthlyDetail(eCorridorFactor     ,YearsCorridorFactor              );
    SetMonthlyDetail(eDeathBft           ,DBReflectingCorr                 );
    SetMonthlyDetail(eForceout           ,GptForceout                      );
    SetMonthlyDetail(eEePrem             ,EeGrossPmts[Month]               );
    SetMonthlyDetail(eErPrem             ,ErGrossPmts[Month]               );
    SetMonthlyDetail(eTotalPrem          ,GrossPmts[Month]                 );
    SetMonthlyDetail(eTargetPrem         ,AnnualTargetPrem                 );
    SetMonthlyDetail(ePremLoad           ,GrossPmts[Month] - NetPmts[Month]);
    SetMonthlyDetail(eNetPrem            ,NetPmts[Month]                   );
    SetMonthlyDetail(ePolicyFees         ,MonthsPolicyFees                 );
    SetMonthlyDetail(eSpecAmtLoad        ,SpecAmtLoad                      );
    SetMonthlyDetail(eNAAR               ,NAAR                             );
    SetMonthlyDetail(eCoiRate            ,ActualCoiRate                    );
    SetMonthlyDetail(eCoiCharge          ,CoiCharge                        );
    SetMonthlyDetail(eAdbRate            ,YearsAdbRate                     );
    SetMonthlyDetail(eAdbCharge          ,AdbCharge                        );
    SetMonthlyDetail(eWpRate             ,YearsWpRate                      );
    SetMonthlyDetail(eWpCharge           ,WpCharge                         );
    SetMonthlyDetail(eTermAmount         ,TermDB                           );
    SetMonthlyDetail(eTermRate           ,YearsTermRate                    );
    SetMonthlyDetail(eTermCharge         ,TermCharge                       );
    SetMonthlyDetail(eTotalRiderCharge   ,RiderDeductions                  );
    SetMonthlyDetail(eTotalMonthlyDeds   ,MlyDed                           );

    SetMonthlyDetail(eGenAcctIntRate     ,ActualMonthlyRate(YearsGenAcctIntRate              ));
    SetMonthlyDetail(eGenAcctIntCred     ,GenAcctIntCred                   );
    SetMonthlyDetail(eSepAcctIntRate     ,ActualMonthlyRate(YearsSepAcctIntRate              ));
    SetMonthlyDetail(eSepAcctIntCred     ,SepAcctIntCred                   );

    SetMonthlyDetail(eAssetsPostBom      ,AssetsPostBom                    );
    SetMonthlyDetail(eCumPmtsPostBom     ,CumPmtsPostBom                   );
    SetMonthlyDetail(eSepAcctLoad        ,SepAcctLoad                      );
    SetMonthlyDetail(eRegLnIntRate       ,ActualMonthlyRate(YearsRegLnIntCredRate            ));
    SetMonthlyDetail(eRegLnIntCred       ,RegLnIntCred                     );
    SetMonthlyDetail(ePrfLnIntRate       ,ActualMonthlyRate(YearsPrfLnIntCredRate            ));
    SetMonthlyDetail(ePrfLnIntCred       ,PrfLnIntCred                     );
    SetMonthlyDetail(eYearsHMValueRate   ,ActualMonthlyRate(YearsHoneymoonValueRate          ));
    SetMonthlyDetail(eYearsPostHMRate    ,ActualMonthlyRate(YearsPostHoneymoonGenAcctIntRate ));

    // Items that are used only on anniversary.
    if(0 == Month)
        {
        SetMonthlyDetail(eRequestedWD    ,RequestedWD                      );
        SetMonthlyDetail(eMaxWD          ,MaxWD                            );
        SetMonthlyDetail(eGrossWD        ,GrossWD                          );
        SetMonthlyDetail(eNetWD          ,NetWD                            );
        SetMonthlyDetail(eRequestedLoan  ,RequestedLoan                    );
        SetMonthlyDetail(eMaxLoan        ,MaxLoan                          );
        SetMonthlyDetail(eNewLoan        ,ActualLoan                       );
        }
    else
        {
        SetMonthlyDetail(eRequestedWD    ,not_applicable()                 );
        SetMonthlyDetail(eMaxWD          ,not_applicable()                 );
        SetMonthlyDetail(eGrossWD        ,not_applicable()                 );
        SetMonthlyDetail(eNetWD          ,not_applicable()                 );
        SetMonthlyDetail(eRequestedLoan  ,not_applicable()                 );
        SetMonthlyDetail(eMaxLoan        ,not_applicable()                 );
        SetMonthlyDetail(eNewLoan        ,not_applicable()                 );
        }

    SetMonthlyDetail(eTaxBasis           ,TaxBasis                         );
    SetMonthlyDetail(eCumNoLapsePrem     ,CumNoLapsePrem                   );
    SetMonthlyDetail(eNoLapseActive      ,NoLapseActive                    );
    SetMonthlyDetail(eEOMAV              ,TotalAccountValue()              );
    SetMonthlyDetail(eHMValue            ,std::max(HoneymoonValue, 0.0)    );
    SetMonthlyDetail(eSurrChg            ,SurrChg()                        );

// TODO ?? Unfortunately duplicated from AccountValue::FinalizeYear().
    double total_av = TotalAccountValue();
    double csv_net =
            total_av
        -   SurrChg()
        -   RegLnBal
        +   GetRefundableSalesLoad()
//        +   std::max(0.0, ExpRatReserve) // This would be added if it existed.
        ;
    csv_net = std::max(HoneymoonValue, csv_net);

    SetMonthlyDetail(eEOMCSVNet          ,csv_net                          );
    SetMonthlyDetail(eEOMCV7702          ,CashValueFor7702()               );

    LMI_ASSERT(0 != Input_->NumIdenticalLives); // Make sure division is safe.
    SetMonthlyDetail
        (eInforceFactor
        ,ItLapsed
            ? 0.0
            : InvariantValues().InforceLives[Year] / Input_->NumIdenticalLives
        );

    // TODO ?? Claims appear as zero because SetClaims() is called not
    // at the end of each month (before DebugPrint() is called), but
    // at the end of each year (after DebugPrint() has been called).
    // The monthly-detail facility should be redesigned anyway to be
    // useful for composites. And it should show 'YearsNetClaims' and
    // 'YearsDeathProceeds' as well as 'YearsGrossClaims', but adding
    // the others is pointless as long as they would be zero.
    SetMonthlyDetail(eClaimsPaid         ,YearsGrossClaims                 );

    bool irc7702a_data_irrelevant =
           InvariantValues().MecYear < Year
        ||    InvariantValues().MecYear == Year
           && InvariantValues().MecMonth < Month
        || e_run_curr_basis != RateBasis
        ;

    if(!irc7702a_data_irrelevant)
        {
        SetMonthlyDetail(e7702ATestDur       ,Irc7702A_->DebugGetTestDur  ());
        SetMonthlyDetail(e7702A7ppRate       ,Irc7702A_->DebugGet7ppRate  ());
        SetMonthlyDetail(e7702ANsp           ,Irc7702A_->DebugGetNsp      ());
        SetMonthlyDetail(e7702ALowestDb      ,Irc7702A_->DebugGetLowestBft());
        // This one's a little tricky. We show the DCV actually used in
        // material change calculations, iff there is a material change
        // in the current month. Otherwise, we show the DCV at the end
        // of the month; we could have shown zero instead, but that
        // wouldn't be useful. We could always have shown DCV at the end
        // of the month, but that wouldn't show what was actually used in
        // material change processing.
        SetMonthlyDetail
            (e7702ADeemedCv
            ,Irc7702A_->DebugGetIsMatChg()
                ? Irc7702A_->DebugGetSavedDCV()
                : Dcv
            );
        SetMonthlyDetail(e7702ANetMaxNecPm   ,NetMaxNecessaryPremium       );
        SetMonthlyDetail(e7702AGrossMaxNecPm ,GrossMaxNecessaryPremium     );

        SetMonthlyDetail(e7702AUnnecPm       ,UnnecessaryPremium           );
        SetMonthlyDetail(e7702ADbAdj         ,Irc7702A_->DebugGetDbAdj   ());
        SetMonthlyDetail(e7702A7pp           ,Irc7702A_->DebugGet7pp     ());
        SetMonthlyDetail(e7702ACumPmts       ,Irc7702A_->DebugGetCumPmts ());
        SetMonthlyDetail(e7702ACum7pp        ,Irc7702A_->DebugGetCum7pp  ());
        SetMonthlyDetail(e7702AIsMatChg      ,Irc7702A_->DebugGetIsMatChg());
        }
    else
        {
        SetMonthlyDetail(e7702ATestDur       ,not_applicable()             );
        SetMonthlyDetail(e7702A7ppRate       ,not_applicable()             );
        SetMonthlyDetail(e7702ANsp           ,not_applicable()             );
        SetMonthlyDetail(e7702ALowestDb      ,not_applicable()             );
        SetMonthlyDetail(e7702ADeemedCv      ,not_applicable()             );
        SetMonthlyDetail(e7702ANetMaxNecPm   ,not_applicable()             );
        SetMonthlyDetail(e7702AGrossMaxNecPm ,not_applicable()             );
        SetMonthlyDetail(e7702AUnnecPm       ,not_applicable()             );
        SetMonthlyDetail(e7702ADbAdj         ,not_applicable()             );
        SetMonthlyDetail(e7702A7pp           ,not_applicable()             );
        SetMonthlyDetail(e7702ACumPmts       ,not_applicable()             );
        SetMonthlyDetail(e7702ACum7pp        ,not_applicable()             );
        SetMonthlyDetail(e7702AIsMatChg      ,not_applicable()             );
        }
    SetMonthlyDetail(e7702AIsMec         ,InvariantValues().IsMec          );

    SetMonthlyDetail(eGSP                ,Irc7702_->RoundedGSP            ());
    SetMonthlyDetail(eGLP                ,Irc7702_->RoundedGLP            ());

    std::copy
        (DebugRecord.begin()
        ,DebugRecord.end()
        ,std::ostream_iterator<std::string>(DebugStream, "\t")
        );
    DebugStream << '\n';
    DebugRecord.assign(eLast, "EMPTY");
}

