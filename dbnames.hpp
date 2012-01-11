// Product database entity names.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#ifndef dbnames_hpp
#define dbnames_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>
#include <vector>

/// See 'dbnames.xpp' for the definition of each entity.
///
/// Partial lexicon:
///  - Adb   Accidental death benefit
///  - Acct  Account
///  - Amort Amortization
///  - Amt   Amount
///  - Chg   Charge
///  - Coi   Cost of insurance
///  - Comp  Compensation
///  - Conv  Conversion
///  - Curr  Current
///  - Dac   Deferred acquisition cost
///  - Dbo   Death benefit option
///  - Decr  Decrease
///  - Ded   Deduction
///  - Dur   Duration
///  - Dyn   Dynamic
///  - Ea    Expense allowance (SNFL)
///  - Ee    Employee
///  - Endt  Endowment
///  - Er    Employer
///  - Exc   Excess (over target)
///  - Exch  Exchange
///  - Exp   Expense or experience
///  - Ext   Extended (as in 'extended endowment')
///  - Fit   Federal income tax
///  - Gdb   Guaranteed death benefit
///  - Gen   General (as in 'general account')
///  - Guar  Guaranteed
///  - Ibnr  Incurred but not reported (reserve)
///  - Imf   Investment management fee
///  - Incr  Increase
///  - Int   Interest; more rarely, internal
///  - Irc   Internal revenue code
///  - Iss   Issue
///  - Lic   Life insurance company
///  - MandE Mortality and expense charge
///  - Max   Maximum
///  - Mdpt  Midpoint
///  - Min   Minimum
///  - Mort  Mortality
///  - Mult  Multiplier
///  - Naar  Net amount at risk
///  - Nlp   Net level premium
///  - Ny    New York
///  - Pmt   Payment
///  - Pol   Policy
///  - Pref  Preferred
///  - Prem  Premium
///  - Q     Death rate
///  - Reg   Regular or regulation
///  - Renl  Renewal
///  - Retal Retaliation
///  - Rfd   Refund or refundable
///  - Sep   Separate (as in 'separate account')
///  - Snfl  Standard nonforfeiture law
///  - Spec  Specified (as in 'specified amount')
///  - Tgt   Target
///  - Uw    Underwriting
///  - Val   Value
///  - Vlr   Variable loan rate
///  - Vx    Reserve
///  - Wd    Withdrawal
///  - Wp    Waiver of premium

enum e_database_key
    {DB_FIRST

    ,DB_Topic_Underwriting

        ,DB_MinIssAge
        ,DB_MaxIssAge
        ,DB_MaxIncrAge

        ,DB_AllowFullUw
        ,DB_AllowParamedUw
        ,DB_AllowNonmedUw
        ,DB_AllowSimpUw
        ,DB_AllowGuarUw
        ,DB_SmokeOrTobacco       // DATABASE !! Move to '.policy'
        ,DB_PrefOrSelect         // DATABASE !! Move to '.policy'
        ,DB_AllowPreferredClass
        ,DB_AllowUltraPrefClass

        ,DB_AllowSubstdTable
        ,DB_AllowFlatExtras
        ,DB_AllowRatedWp
        ,DB_AllowRatedAdb
        ,DB_AllowRatedTerm
        ,DB_AllowRetirees

        ,DB_AllowUnisex
        ,DB_AllowSexDistinct
        ,DB_AllowUnismoke
        ,DB_AllowSmokeDistinct

        ,DB_StateApproved
        ,DB_AllowStateXX
        ,DB_AllowForeign

        ,DB_Allowable

    ,DB_Topic_7702And7702A

        ,DB_AllowCvat
        ,DB_AllowGpt
        ,DB_AllowNo7702

        ,DB_CorridorTable
        ,DB_SevenPayTable
        ,DB_Irc7702QTable

        ,DB_PremLoad7702
        ,DB_Equiv7702Dbo3

    ,DB_Topic_MortalityCharges

        ,DB_GuarCoiTable
        ,DB_GuarCoiIsAnnual
        ,DB_GuarCoiMultiplier

        ,DB_CurrCoiTable
        ,DB_CurrCoiIsAnnual
        ,DB_MinInputCoiMult
        ,DB_CurrCoiMultiplier
        ,DB_UnusualCoiBanding
        ,DB_CurrCoiTable0Limit
        ,DB_CurrCoiTable1
        ,DB_CurrCoiTable1Limit
        ,DB_CurrCoiTable2

        ,DB_MdptCoiTable
        ,DB_MdptCoiIsAnnual

        ,DB_CoiNyMinTable
        ,DB_UseNyCoiFloor
        ,DB_MaxMonthlyCoiRate
        ,DB_GuarCoiCeiling
        ,DB_CoiGuarIsMin

        ,DB_SubstdTableMult
        ,DB_SubstdTableMultTable

        ,DB_CoiUpper12Method
        ,DB_CoiInforceReentry

        ,DB_AllowMortBlendSex
        ,DB_AllowMortBlendSmoke

    ,DB_Topic_Interest

        ,DB_GuarInt
        ,DB_NaarDiscount

        ,DB_GuarIntSpread
        ,DB_GuarMandE

        ,DB_CurrIntSpread
        ,DB_CurrMandE

        ,DB_GenAcctIntBonus
        ,DB_BonusInt
        ,DB_IntFloor
        ,DB_AllowGenAcct
        ,DB_AllowSepAcct
        ,DB_AllowGenAcctEarnRate
        ,DB_AllowSepAcctNetRate
        ,DB_MaxGenAcctRate
        ,DB_MaxSepAcctRate

        ,DB_SepAcctSpreadMethod
        ,DB_IntSpreadMode
        ,DB_DynamicMandE

    ,DB_Topic_AssetCharges

        ,DB_AllowAmortPremLoad
        ,DB_LoadAmortFundCharge

        ,DB_AllowImfOverride
        ,DB_AssetChargeType
        ,DB_StableValFundCharge

        ,DB_GuarFundAdminChg
        ,DB_CurrFundAdminChg
        ,DB_FundCharge

    ,DB_Topic_Loads

        ,DB_GuarMonthlyPolFee
        ,DB_GuarAnnualPolFee

        ,DB_GuarPremLoadTgt
        ,DB_GuarPremLoadExc
        ,DB_GuarPremLoadTgtRfd
        ,DB_GuarPremLoadExcRfd

        ,DB_GuarSpecAmtLoad
        ,DB_GuarSpecAmtLoadTable

        ,DB_GuarAcctValLoad

        ,DB_CurrMonthlyPolFee
        ,DB_CurrAnnualPolFee

        ,DB_CurrPremLoadTgt
        ,DB_CurrPremLoadExc
        ,DB_CurrPremLoadTgtRfd
        ,DB_CurrPremLoadExcRfd

        ,DB_CurrSpecAmtLoad
        ,DB_CurrSpecAmtLoadTable

        ,DB_CurrAcctValLoad

        ,DB_TgtPremMonthlyPolFee
        ,DB_LoadRfdProportion
        ,DB_SpecAmtLoadLimit
        ,DB_DynamicSepAcctLoad
        ,DB_DynSepAcctLoadLimit

    ,DB_Topic_DacAndPremiumTax

        ,DB_DacTaxFundCharge
        ,DB_DacTaxPremLoad

        ,DB_PremTaxFundCharge
        ,DB_PremTaxLoad
        ,DB_WaivePremTaxInt1035
        ,DB_PremTaxRetalLimit
        ,DB_PremTaxTierGroup
        ,DB_PremTaxTierPeriod
        ,DB_PremTaxTierNonDecr

        ,DB_PremTaxAmortPeriod
        ,DB_PremTaxAmortIntRate

        ,DB_PremTaxRate
        ,DB_PremTaxState

    ,DB_Topic_SurrenderCharges

        ,DB_SurrChgAcctValMult
        ,DB_SurrChgAcctValSlope
        ,DB_SurrChgSpecAmtMult
        ,DB_SurrChgSpecAmtSlope
        ,DB_SurrChgPremMult
        ,DB_SurrChgOnIncr
        ,DB_SurrChgOnDecr
        ,DB_Has1035ExchCharge

        ,DB_SnflQTable
        ,DB_CoiSnflIsGuar
        ,DB_SurrChgByFormula
        ,DB_SurrChgPeriod
        ,DB_SurrChgZeroDur
        ,DB_SurrChgNlpMult
        ,DB_SurrChgNlpMax
        ,DB_SurrChgEaMax
        ,DB_SurrChgAmort

    ,DB_Topic_DeathBenefits

        ,DB_AllowSpecAmtIncr
        ,DB_MinSpecAmtIncr
        ,DB_EnforceNaarLimit

        ,DB_MinSpecAmt
        ,DB_MinIssSpecAmt
        ,DB_MinIssBaseSpecAmt
        ,DB_MinRenlSpecAmt
        ,DB_MinRenlBaseSpecAmt
        ,DB_MaxIssSpecAmt
        ,DB_MaxRenlSpecAmt

        ,DB_AllowDbo1
        ,DB_AllowDbo2
        ,DB_AllowDbo3
        ,DB_AllowChangeToDbo2
        ,DB_DboChgCanIncrSpecAmt
        ,DB_DboChgCanDecrSpecAmt

        ,DB_AllowExtEndt

    ,DB_Topic_Riders

        ,DB_AllowTerm
        ,DB_GuarTermTable
        ,DB_TermTable
        ,DB_TermMinIssAge
        ,DB_TermMaxIssAge
        ,DB_TermForcedConvAge
        ,DB_MaxTermProportion
        ,DB_TermCoiRate
        ,DB_TermPremRate
        ,DB_TermIsDbFor7702
        ,DB_TermIsDbFor7702A

        ,DB_AllowWp
        ,DB_WpTable
        ,DB_WpMinIssAge
        ,DB_WpMaxIssAge
        ,DB_WpMax
        ,DB_WpCoiRate
        ,DB_WpPremRate
        ,DB_WpChargeMethod

        ,DB_AllowAdb
        ,DB_AdbTable
        ,DB_AdbMinIssAge
        ,DB_AdbMaxIssAge
        ,DB_AdbLimit
        ,DB_AdbCoiRate
        ,DB_AdbPremRate

        ,DB_AllowSpouseRider
        ,DB_SpouseRiderGuarTable
        ,DB_SpouseRiderTable

        ,DB_AllowChildRider
        ,DB_ChildRiderTable

    ,DB_Topic_Withdrawals

        ,DB_AllowWd
        ,DB_WdFee
        ,DB_WdFeeRate

        ,DB_FreeWdProportion
        ,DB_MinWd
        ,DB_MaxWdAcctValMult
        ,DB_MaxWdDed

        ,DB_WdCanDecrSpecAmtDbo1
        ,DB_WdCanDecrSpecAmtDbo2
        ,DB_WdCanDecrSpecAmtDbo3

        ,DB_FirstWdYear

    ,DB_Topic_Loans

        ,DB_AllowLoan
        ,DB_AllowPrefLoan
        ,DB_AllowFixedLoan
        ,DB_AllowVlr
        ,DB_FixedLoanRate
        ,DB_MaxVlrRate

        ,DB_MaxLoanAcctValMult
        ,DB_MaxLoanDed

        ,DB_GuarPrefLoanSpread
        ,DB_GuarRegLoanSpread

        ,DB_CurrPrefLoanSpread
        ,DB_CurrRegLoanSpread

        ,DB_FirstLoanYear

    ,DB_Topic_Premiums

        ,DB_MinPremType
        ,DB_MinPremIntSpread
        ,DB_TgtPremType
        ,DB_TgtPremTable
        ,DB_TgtPremFixedAtIssue
        ,DB_TgtPremIgnoreSubstd
        ,DB_MinPmt

    ,DB_Topic_SecondaryGuarantees

        ,DB_NoLapseMinDur
        ,DB_NoLapseMinAge
        ,DB_NoLapseUnratedOnly
        ,DB_NoLapseDbo1Only
        ,DB_NoLapseAlwaysActive

        ,DB_AllowHoneymoon

    ,DB_Topic_Allocations

        ,DB_DeductionMethod
        ,DB_DeductionAcct
        ,DB_DistributionMethod
        ,DB_DistributionAcct
        ,DB_EePremMethod
        ,DB_EePremAcct
        ,DB_ErPremMethod
        ,DB_ErPremAcct

    ,DB_Topic_Compensation

        ,DB_CompTarget
        ,DB_CompExcess
        ,DB_CompChargeBack
        ,DB_AssetComp
        ,DB_AllowExtraAssetComp
        ,DB_AllowExtraPremComp

    ,DB_Topic_ExperienceRating

        ,DB_AllowExpRating
        ,DB_ExpRatStdDevMult
        ,DB_ExpRatIbnrMult
        ,DB_ExpRatCoiRetention
        ,DB_ExpRatRiskCoiMult
        ,DB_ExpRatAmortPeriod

    ,DB_Topic_Miscellanea

        ,DB_LedgerType

        ,DB_AgeLastOrNearest     // DATABASE !! Add more options...
        ,DB_MaxIllusAge
        ,DB_MaturityAge

        ,DB_LapseIgnoresSurrChg
        ,DB_DefaultProcessOrder
        ,DB_NominallyPar         // DATABASE !! Use a footnote instead.

        ,DB_TableYTable
        ,DB_Gam83Table

    ,DB_Topic_Weights

        ,DB_WeightClass
        ,DB_WeightGender
        ,DB_WeightSmoking
        ,DB_WeightAge
        ,DB_WeightSpecAmt
        ,DB_WeightState

    ,DB_Topic_Expenses

        ,DB_FullExpPol
        ,DB_FullExpPrem
        ,DB_FullExpDumpin
        ,DB_FullExpSpecAmt

        ,DB_VarExpPol
        ,DB_VarExpPrem
        ,DB_VarExpDumpin
        ,DB_VarExpSpecAmt

        ,DB_ExpSpecAmtLimit

        ,DB_MedicalProportion
        ,DB_UwTestCost

    ,DB_Topic_Reserves

        ,DB_VxBasicQTable
        ,DB_VxDeficQTable
        ,DB_VxTaxQTable

        ,DB_StatVxInt
        ,DB_TaxVxInt

        ,DB_StatVxQ
        ,DB_TaxVxQ
        ,DB_DefVxQ
        ,DB_SnflQ

    ,DB_Topic_OtherAssumptions

        ,DB_LapseRate
        ,DB_ReqSurpNaar
        ,DB_ReqSurpVx
        ,DB_LicFitRate
        ,DB_LicDacTaxRate
        ,DB_GdbVxMethod
        ,DB_PrimaryHurdle
        ,DB_SecondaryHurdle

    ,DB_LAST
    };

struct db_names
{
    e_database_key      Idx;
    e_database_key      ParentIdx;
    char const*         ShortName;
    char const*         LongName;
};

std::vector<db_names> const& LMI_SO GetDBNames();

int         LMI_SO db_key_from_name(std::string const&);
std::string LMI_SO db_name_from_key(int);

#endif // dbnames_hpp

