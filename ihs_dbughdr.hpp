// Display monthly values.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_dbughdr.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef dbughdr_hpp
#define dbughdr_hpp

// Headers for debug output.

// We can rearrange columns by just changing the order of enumerators.

// To add a new column, insert an enumerator here and a header in
// DebugColHeadersHelper(), and print the value in AccountValue::DebugPrint().

#include "config.hpp"

#include <string>
#include <vector>

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
    ,eMlyPolicyFee
    ,eAnnPolicyFee
    ,eSpecAmtLoad
    ,eAcctValLoadBOM
    ,eNAAR
    ,eCOIRate
    ,eCOICharge
    ,eADDRate
    ,eADDCharge
    ,eWPRate
    ,eWPCharge
    ,eTermAmount
    ,eTermRate
    ,eTermCharge
    ,eTotalRiderCharge
    ,eTotalMonthlyDeds
    ,eGenAcctIntRate
    ,eGenAcctIntCred
    ,eSepAcctIntRate
    ,eSepAcctIntCred
    ,eAcctValLoadAMD
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
    ,eExpRatReserve
    ,eExpRatStabReserve
    ,eExpRatIBNRReserve
    ,eExpRatRfd
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
    v[eDBOption]            = "DB Option";
    v[eSpecAmt]             = "Spec amt";
    v[eCorridorFactor]      = "Corridor factor";
    v[eDeathBft]            = "Death bft";
    v[eForceout]            = "Forceout";
    v[eEePrem]              = "Ee prem";
    v[eErPrem]              = "Er prem";
    v[eTotalPrem]           = "Total prem";
    v[eTargetPrem]          = "Target prem";
    v[ePremLoad]            = "Prem load";
    v[eNetPrem]             = "Net prem";
    v[eMlyPolicyFee]        = "Monthly policy fee";
    v[eAnnPolicyFee]        = "Annual policy fee";
    v[eSpecAmtLoad]         = "Spec amt load";
    v[eAcctValLoadBOM]      = "Sep acct load BOM";
    v[eNAAR]                = "NAAR";
    v[eCOIRate]             = "COI Rate";
    v[eCOICharge]           = "COI charge";
    v[eADDRate]             = "ADD Rate";
    v[eADDCharge]           = "ADD charge";
    v[eWPRate]              = "WP Rate";
    v[eWPCharge]            = "WP charge";
    v[eTermAmount]          = "Term amount";
    v[eTermRate]            = "Term Rate";
    v[eTermCharge]          = "Term charge";
    v[eTotalRiderCharge]    = "Total rider charge";
    v[eTotalMonthlyDeds]    = "Total monthly deds";
    v[eGenAcctIntRate]      = "Unloaned GA interest rate";
    v[eGenAcctIntCred]      = "Unloaned GA interest credited";
    v[eSepAcctIntRate]      = "Unloaned SA interest rate";
    v[eSepAcctIntCred]      = "Unloaned SA interest credited";
    v[eAcctValLoadAMD]      = "Sep acct load after mly ded";
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
    v[eSurrChg]             = "EOM SurrChg";
    v[eEOMCSVNet]           = "EOM CSV net";
    v[eEOMCV7702]           = "EOM CV for 7702";
    v[eInforceFactor]       = "Inforce factor";
    v[eExpRatReserve]       = "Exp rating reserve";
    v[eExpRatStabReserve]   = "Exp rating stabilization reserve";
    v[eExpRatIBNRReserve]   = "Exp rating IBNR reserve";
    v[eExpRatRfd]           = "Exp rating refund";
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

inline std::vector<std::string> const& DebugColHeaders()
{
    static std::vector<std::string> v(DebugColHeadersHelper());
    return v;
}

#endif // dbughdr_hpp

