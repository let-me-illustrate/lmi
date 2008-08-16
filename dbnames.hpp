// Product database entity names.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: dbnames.hpp,v 1.15 2008-08-16 17:36:13 chicares Exp $

#ifndef dbnames_hpp
#define dbnames_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <vector>

enum DatabaseNames
    {DB_FIRST

    ,DB_Topic_Issue_Rules

        ,DB_AgeLastOrNearest
        ,DB_MinIssAge
        ,DB_MaxIssAge
        ,DB_MaxIncrAge
        ,DB_AllowRetirees

        ,DB_AllowFullUW
        ,DB_AllowSimpUW
        ,DB_AllowGuarUW
        ,DB_PrefOrSelect
        ,DB_AllowPreferredClass
        ,DB_AllowUltraPrefClass
        ,DB_AllowSubstdTable
        ,DB_AllowFlatExtras
        ,DB_AllowRatedWP
        ,DB_AllowRatedADD
        ,DB_AllowRatedTerm

        ,DB_MinPmt
        ,DB_MinSpecAmt
        ,DB_MinIssSpecAmt
        ,DB_MinRenlSpecAmt
        ,DB_MinRenlBaseSpecAmt
        ,DB_MaxIssSpecAmt
        ,DB_MaxRenlSpecAmt

        ,DB_AllowUnisex
        ,DB_AllowSexDistinct
        ,DB_AllowUnismoke
        ,DB_AllowSmokeDistinct
        ,DB_SmokeOrTobacco

        ,DB_AllowMortBlendSex
        ,DB_AllowMortBlendSmoke

        ,DB_StateApproved
        ,DB_AllowStateXX
        ,DB_AllowForeign

        ,DB_Allowable

    ,DB_Topic_7702

        ,DB_AllowCVAT
        ,DB_AllowGPT
        ,DB_AllowNo7702

        ,DB_CorridorTable
        ,DB_TAMRA7PayTable
        ,DB_IRC7702QTable

        ,DB_PremLoad7702
        ,DB_Equiv7702DBO3

    ,DB_Topic_Mortality

        ,DB_GuarCOITable
        ,DB_GCoiIsAnnual
        ,DB_GCOIMultiplier

        ,DB_CurrCOITable
        ,DB_CCoiIsAnnual
        ,DB_CCOIMultiplier
        ,DB_UnusualCOIBanding
        ,DB_CurrCOITable0Limit
        ,DB_CurrCOITable1
        ,DB_CurrCOITable1Limit
        ,DB_CurrCOITable2

        ,DB_MdptCOITable
        ,DB_MCoiIsAnnual

        ,DB_COINYMinTable
        ,DB_UseNYCOIFloor
        ,DB_MaxMonthlyCoiRate
        ,DB_GuarCOICeiling
        ,DB_COIGuarIsMin
        ,DB_COINonforfIsGuar

        ,DB_SubstdTblMult

        ,DB_CoiUpper12Method
        ,DB_CoiInforceReentry

    ,DB_Topic_Interest

        ,DB_GuarInt
        ,DB_NAARDiscount

        ,DB_GuarIntSpread
        ,DB_GuarMandE

        ,DB_CurrIntSpread
        ,DB_CurrMandE

        ,DB_GAIntBonus
        ,DB_BonusInt
        ,DB_IntFloor
        ,DB_MaxGenAcctRate
        ,DB_MaxSepAcctRate

        ,DB_SepAcctSpreadMethod
        ,DB_IntSpreadFreq
        ,DB_DynamicMandE
        ,DB_AllowHoneymoon

    ,DB_Topic_AssetCharges

        ,DB_AllowAmortPremLoad
        ,DB_AmortPmLdFundCharge

        ,DB_PmTxAmortPeriod
        ,DB_PmTxAmortIntRate

        ,DB_AllowCustomFund
        ,DB_AssetChargeType
        ,DB_StableValFundCharge

    ,DB_Topic_Loads

        ,DB_GuarPolFee
        ,DB_GuarIssueFee

        ,DB_GuarPremLoadTgt
        ,DB_GuarPremLoadExc
        ,DB_GuarPremLoadTgtRfd
        ,DB_GuarPremLoadExcRfd

        ,DB_GuarSpecAmtLoad
        ,DB_GuarFundAdminChg
        ,DB_GuarAcctValLoadAMD

        ,DB_CurrPolFee
        ,DB_CurrIssueFee

        ,DB_CurrPremLoadTgt
        ,DB_CurrPremLoadExc
        ,DB_CurrPremLoadTgtRfd
        ,DB_CurrPremLoadExcRfd

        ,DB_CurrSpecAmtLoad
        ,DB_CurrFundAdminChg
        ,DB_CurrAcctValLoadAMD

        ,DB_PremRefund
        ,DB_TgtPremPolFee
        ,DB_DynamicSepAcctLoad
        ,DB_SpecAmtLoadLimit
        ,DB_FundCharge

        ,DB_DACTaxPremLoad
        ,DB_DACTaxFundCharge

        ,DB_PremTaxLoad
        ,DB_PremTaxFundCharge
        ,DB_WaivePmTxInt1035
        ,DB_PremTaxRetalLimit
        ,DB_PremTaxTierGroup
        ,DB_PremTaxTierPeriod
        ,DB_PremTaxTierNonDecr

    ,DB_Topic_SurrChgs

        ,DB_NonforfQTable
        ,DB_SurrChgByFormula
        ,DB_SurrChgPeriod
        ,DB_SurrChgZeroDur
        ,DB_SurrChgNLPMult
        ,DB_SurrChgNLPMax
        ,DB_SurrChgEAMax
        ,DB_SurrChgPremMult
        ,DB_SurrChgIsMly

        ,DB_SurrChgAVMult
        ,DB_SurrChgSAMult
        ,DB_SurrChgOnIncr
        ,DB_SurrChgSADurFactor
        ,DB_SurrChgAVDurFactor
        ,DB_SurrChgOnDecr
        ,DB_Has1035ExchCharge

    ,DB_Topic_DeathBfts

        ,DB_AllowSAIncr
        ,DB_MinSpecAmtIncr
        ,DB_EnforceNAARLimit

        ,DB_AllowDBO1
        ,DB_AllowDBO2
        ,DB_AllowDBO3
        ,DB_AllowChangeToDBO2
        ,DB_OptChgCanIncrSA
        ,DB_OptChgCanDecrSA

    ,DB_Topic_Riders

        ,DB_AllowTerm
        ,DB_GuarTermTable
        ,DB_TermTable
        ,DB_TermMinIssAge
        ,DB_TermMaxIssAge
        ,DB_TermForcedConvAge
        ,DB_MaxTermProportion
        ,DB_TermCOIRate
        ,DB_TermPremRate

        ,DB_AllowWP
        ,DB_WPTable
        ,DB_WPMinIssAge
        ,DB_WPMaxIssAge
        ,DB_WPMax
        ,DB_WPCOIRate
        ,DB_WPPremRate
        ,DB_WPChargeMethod

        ,DB_AllowADD
        ,DB_ADDTable
        ,DB_ADDMinIssAge
        ,DB_ADDMaxIssAge
        ,DB_ADDLimit
        ,DB_ADDCOIRate
        ,DB_ADDPremRate

        ,DB_AllowSpouse
        ,DB_SpousRiderGuarTable
        ,DB_SpouseRiderTable

        ,DB_AllowChild
        ,DB_ChildRiderTable

    ,DB_Topic_WD

        ,DB_AllowWD
        ,DB_WDFee
        ,DB_WDFeeRate

        ,DB_FreeWDProportion
        ,DB_MinWD
        ,DB_MaxWDAVMult
        ,DB_MaxWDDed

        ,DB_WDCanDecrSADBO1
        ,DB_WDCanDecrSADBO2
        ,DB_WDCanDecrSADBO3

        ,DB_FirstWDYear

    ,DB_Topic_Loans

        ,DB_AllowLoan
        ,DB_AllowPrefLoan
        ,DB_AllowFixedLoan
        ,DB_AllowVLR
        ,DB_FixedLoanRate
        ,DB_MaxVLRRate

        ,DB_MaxLoanAVMult
        ,DB_MaxLoanDed

        ,DB_GuarPrefLoanSpread
        ,DB_GuarRegLoanSpread

        ,DB_CurrPrefLoanSpread
        ,DB_CurrRegLoanSpread

        ,DB_FirstLoanYear

    ,DB_Topic_Comp

        ,DB_CompTarget
        ,DB_CompExcess
        ,DB_CompChargeBack
        ,DB_AllowExtraPremComp
        ,DB_AllowExtraAssetComp
        ,DB_AssetComp

    ,DB_Topic_ExpRating

        ,DB_AllowExpRating
        ,DB_ExpRatStdDevMult
        ,DB_ExpRatIBNRMult
        ,DB_ExpRatCOIRetention
        ,DB_ExpRatRiskCOIMult
        ,DB_ExpRatAmortPeriod

    ,DB_Topic_Tables

        ,DB_TgtPremTable
        ,DB_PremTaxTable
        ,DB_TableYTable
        ,DB_83GamTable

    ,DB_Topic_Miscellanea

        ,DB_LedgerType
        ,DB_AllowGenAcct
        ,DB_AllowSepAcct

        ,DB_EndtAge
        ,DB_AllowExtEndt

        ,DB_MinPremType
        ,DB_MinPremIntSpread
        ,DB_TgtPremType
        ,DB_TgtPmFixedAtIssue
        ,DB_TgtPmIgnoreSubstd

        ,DB_DeductionMethod
        ,DB_DeductionAcct
        ,DB_DistributionMethod
        ,DB_DistributionAcct
        ,DB_EePremMethod
        ,DB_EePremAcct
        ,DB_ErPremMethod
        ,DB_ErPremAcct

        ,DB_LapseIgnoresSurrChg
        ,DB_NoLapseMinDur
        ,DB_NoLapseMinAge
        ,DB_NoLapseUnratedOnly
        ,DB_NoLapseOpt1Only
        ,DB_NoLapseAlwaysActive

        ,DB_DefaultProcessOrder
        ,DB_NominallyPar

        ,DB_PremTaxRate
        ,DB_PremTaxState

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
        ,DB_FullExpPerK

        ,DB_VarExpPol
        ,DB_VarExpPrem
        ,DB_VarExpDumpin
        ,DB_VarExpPerK

        ,DB_ExpPerKLimit

        ,DB_MedicalProportion
        ,DB_UWTestCost

    ,DB_Topic_Reserves

        ,DB_VxBasicQTable
        ,DB_VxDeficQTable
        ,DB_VxTaxQTable

        ,DB_StatVxInt
        ,DB_TaxVxInt

        ,DB_StatVxQ
        ,DB_TaxVxQ
        ,DB_DefVxQ
        ,DB_NonforfQ

    ,DB_Topic_OtherAsspts

        ,DB_LapseRate
        ,DB_ReqSurpNAAR
        ,DB_ReqSurpVx
        ,DB_LICFitRate
        ,DB_LicDacTaxRate
        ,DB_GDBVxMethod
        ,DB_PrimaryHurdle
        ,DB_SecondaryHurdle

    ,DB_LAST    // Adding a new item? Insert directly above *only*.
    // That way, old databases remain compatible with new code, and any
    // error in using a new item will occur when the new item is used
    // instead of where a renumbered old item is used--that should make
    // such errors much easier to track down and repair. We can re-sort
    // additions into categories when preparing a major release.
    //
    // See 'dbnames.xpp' for the definition of each entity.
    };

struct db_names
{
    DatabaseNames       Idx;
    DatabaseNames       ParentIdx;
    char const*         ShortName;
    char const*         LongName;
};

std::vector<db_names> const& LMI_SO GetDBNames();

#endif // dbnames_hpp

