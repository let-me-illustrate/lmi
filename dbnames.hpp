// Product database entity names.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: dbnames.hpp,v 1.3 2005-05-06 17:20:54 chicares Exp $

#ifndef dbnames_hpp
#define dbnames_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <vector>

enum DatabaseNames
    {DB_FIRST

    ,DB_Topic_Loads

        ,DB_GuarPolFee
        ,DB_GuarSpecAmtLoad
        ,DB_GuarIssueFee
        ,DB_GuarFundAdminChg

        ,DB_GuarPremLoadTgt
        ,DB_GuarPremLoadExc
        ,DB_GuarPremLoadTgtRfd
        ,DB_GuarPremLoadExcRfd

        ,DB_CurrPolFee
        ,DB_CurrSpecAmtLoad
        ,DB_CurrIssueFee
        ,DB_CurrFundAdminChg

        ,DB_CurrPremLoadTgt
        ,DB_CurrPremLoadExc
        ,DB_CurrPremLoadTgtRfd
        ,DB_CurrPremLoadExcRfd

        ,DB_DACTaxPremLoad

        ,DB_FundCharge
        ,DB_PremTaxFundCharge
        ,DB_DACTaxFundCharge

        ,DB_WaivePmTxInt1035

    ,DB_Topic_WD

        ,DB_FirstWDYear

        ,DB_MaxWDAVMult
        ,DB_MaxWDDed
        ,DB_MinWD

        ,DB_WDFee
        ,DB_WDFeeRate

        ,DB_WDCanDecrSADBO1
        ,DB_WDCanDecrSADBO2
        ,DB_WDCanDecrSADBO3

    ,DB_Topic_Loans

        ,DB_FirstLoanYear

        ,DB_AllowPrefLoan
        ,DB_AllowFixedLoan
        ,DB_FixedLoanRate
        ,DB_AllowVLR

        ,DB_MaxLoanAVMult
        ,DB_MaxLoanDed

        ,DB_GuarPrefLoanSpread
        ,DB_GuarRegLoanSpread

        ,DB_CurrPrefLoanSpread
        ,DB_CurrRegLoanSpread

    ,DB_Topic_Interest

        ,DB_GuarInt
        ,DB_NAARDiscount

        ,DB_GuarIntSpread
        ,DB_GuarMandE

        ,DB_CurrIntSpread
        ,DB_CurrMandE

        ,DB_BonusInt
        ,DB_IntFloor

        ,DB_SepAcctSpreadMethod
        ,DB_DynamicMandE
        ,DB_DynamicCOI

    ,DB_Topic_Mortality

        ,DB_CurrCOITable
        ,DB_MdptCOITable
        ,DB_GuarCOITable
        ,DB_COINYMinTable

        ,DB_CCOIMultiplier
        ,DB_UseNYCOIFloor
        ,DB_GuarCOICeiling
        ,DB_COIGuarIsMin
        ,DB_COINonforfIsGuar

        ,DB_SubstdTblMult
        ,DB_CCoiIsAnnual
        ,DB_GCoiIsAnnual
        ,DB_MCoiIsAnnual

    ,DB_Topic_Issue_Rules

        ,DB_AgeLastOrNearest
        ,DB_AllowRetirees
        ,DB_MinSpecAmt
        ,DB_AllowSubstdTable
        ,DB_AllowFlatExtras

        ,DB_MinIssAge
        ,DB_MaxIssAge
        ,DB_MinIssFace
        ,DB_MaxIssFace
        ,DB_MinRenlBaseFace
        ,DB_MinRenlFace
        ,DB_MaxRenlFace
        ,DB_MinFaceIncr
        ,DB_MaxIncrAge
        ,DB_MinPmt
        ,DB_SmokeOrTobacco

        ,DB_AllowUnisex
        ,DB_AllowSexDistinct
        ,DB_AllowUnismoke
        ,DB_AllowSmokeDistinct
        ,DB_AllowFullUW
        ,DB_AllowSimpUW
        ,DB_AllowGuarUW
        ,DB_AllowMortBlendSex
        ,DB_AllowMortBlendSmoke
        ,DB_AllowRatedWP
        ,DB_AllowRatedADD
        ,DB_AllowRatedTerm
        ,DB_Allowable
        ,DB_AllowPreferredClass

    ,DB_Topic_7702

        ,DB_AllowCVAT
        ,DB_AllowGPT

        ,DB_CorridorTable
        ,DB_TAMRA7PayTable
        ,DB_IRC7702QTable

        ,DB_PremLoad7702

    ,DB_Topic_DeathBfts

        ,DB_AllowDBO1
        ,DB_AllowDBO2
        ,DB_AllowDBO3
        ,DB_OptChgCanIncrSA
        ,DB_OptChgCanDecrSA

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

    ,DB_Topic_Misc

        ,DB_PremTaxRate
        ,DB_PremTaxState

        ,DB_EndtAge
        ,DB_AllowExtEndt

        ,DB_AllowGenAcct
        ,DB_AllowSepAcct

        ,DB_MinPremType
        ,DB_TgtPremType
        ,DB_TgtPmFixedAtIssue
        ,DB_TgtPmIgnoreSubstd

        ,DB_NoLapseMinDur
        ,DB_NoLapseMinAge
        ,DB_NoLapseUnratedOnly
        ,DB_NoLapseOpt1Only

        ,DB_PremRefund

    ,DB_Topic_Riders

        ,DB_TermTable
        ,DB_AllowTerm
        ,DB_TermMinIssAge
        ,DB_TermMaxIssAge
        ,DB_TermForcedConvAge
        ,DB_MaxTermProportion
        ,DB_TermCOIRate
        ,DB_TermPremRate

        ,DB_WPTable
        ,DB_AllowWP
        ,DB_WPMinIssAge
        ,DB_WPMaxIssAge
        ,DB_WPMax
        ,DB_WPCOIRate
        ,DB_WPPremRate

        ,DB_ADDTable
        ,DB_AllowADD
        ,DB_ADDMinIssAge
        ,DB_ADDMaxIssAge
        ,DB_ADDLimit
        ,DB_ADDCOIRate
        ,DB_ADDPremRate

    ,DB_Topic_Weights

        ,DB_WeightClass
        ,DB_WeightGender
        ,DB_WeightSmoking
        ,DB_WeightAge
        ,DB_WeightFace
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

    ,DB_Topic_Comp

        ,DB_CompTarget
        ,DB_CompExcess
        ,DB_CompChargeBack

    ,DB_Topic_MiscAsspts

        ,DB_LapseRate
        ,DB_ReqSurpNAAR
        ,DB_ReqSurpVx
        ,DB_LICFitRate
        ,DB_LicDacTaxRate
        ,DB_GDBVxMethod
        ,DB_PrimaryHurdle
        ,DB_SecondaryHurdle

// Other stuff

        ,DB_LedgerType
        ,DB_AllowExpRating

    ,DB_Topic_Tables

        ,DB_PremTaxTable
        ,DB_TableYTable
        ,DB_83GamTable

        ,DB_AllowWD
        ,DB_AllowLoan
        ,DB_AllowChangeToDBO2
        ,DB_AllowSAIncr
        ,DB_NoLapseAlwaysActive
        ,DB_PrefOrSelect

    ,DB_Topic_ExpRating

        ,DB_ExpRatStdDevMult
        ,DB_ExpRatIBNRMult
        ,DB_ExpRatCOIRetention

        ,DB_UsePMQOnCurrCOI

    ,DB_Topic_AssetCharges

        ,DB_MiscFundCharge
        ,DB_StableValFundCharge
        ,DB_MgmtFeeFundCharge
        ,DB_AmortPmLdFundCharge
        ,DB_AllowAmortPremLoad

        ,DB_GuarAcctValLoadBOM
        ,DB_CurrAcctValLoadBOM

        ,DB_PmTxAmortPeriod
        ,DB_PmTxAmortIntRate
        ,DB_PremTaxLoad
        ,DB_GCOIMultiplier

        ,DB_AllowHoneymoon
        ,DB_HCVSpread // TODO ?? Obsolete?
        ,DB_ExtraHMSpreadonCV // TODO ?? Obsolete?
        ,DB_TgtPremTable
        ,DB_TgtPremPolFee
        ,DB_AllowExtraAssetComp
        ,DB_AllowExtraPremComp
        ,DB_AssetChargeType
        ,DB_AllowUltraPrefClass
        ,DB_MaxGenAcctRate
        ,DB_MaxSepAcctRate
        ,DB_MaxVLRRate
        ,DB_SurrChgAVMult
        ,DB_IntSpreadFreq
        ,DB_StateApproved
        ,DB_AllowStateXX
        ,DB_AllowForeign
        ,DB_AllowCustomFund
        ,DB_AllowNo7702
        ,DB_EnforceNAARLimit
        ,DB_GuarAcctValLoadAMD
        ,DB_CurrAcctValLoadAMD
        ,DB_DynamicSepAcctLoad
        ,DB_SpecAmtLoadLimit
        ,DB_Equiv7702DBO3
        ,DB_NonUSCorridorPivot
        ,DB_ExpRatRiskCOIMult
        ,DB_SurrChgSAMult
        ,DB_AllowChild
        ,DB_AllowSpouse
        ,DB_GAIntBonus
        ,DB_DeductionMethod
        ,DB_DeductionAcct
        ,DB_DistributionMethod
        ,DB_DistributionAcct
        ,DB_EePremMethod
        ,DB_EePremAcct
        ,DB_ErPremMethod
        ,DB_ErPremAcct
        ,DB_PremTaxRetalLimit
        ,DB_PremTaxTierGroup
        ,DB_PremTaxTierPeriod
        ,DB_PremTaxTierNonDecr
        ,DB_UnusualCOIBanding
        ,DB_CurrCOITable0Limit
        ,DB_CurrCOITable1
        ,DB_CurrCOITable1Limit
        ,DB_CurrCOITable2
        ,DB_SurrChgOnIncr
        ,DB_FreeWDProportion
        ,DB_SpouseRiderTable
        ,DB_ChildRiderTable
        ,DB_WPChargeMethod
        ,DB_DefaultProcessOrder
        ,DB_SurrChgSADurFactor
        ,DB_SurrChgAVDurFactor
        ,DB_SurrChgOnDecr
        ,DB_MinPremIntSpread
        ,DB_CoiUpper12Method
        ,DB_MaxMonthlyCoiRate
        ,DB_ExpRatCoiMultGuar
        ,DB_ExpRatCoiMultAlt
        ,DB_ExpRatCoiMultCurr0
        ,DB_ExpRatCoiMultCurr1
        ,DB_SpousRiderGuarTable
        ,DB_GuarTermTable
        ,DB_NominallyPar
        ,DB_Has1035ExchCharge
        ,DB_LapseIgnoresSurrChg
        ,DB_CoiInforceReentry
        ,DB_AssetComp
        ,DB_ExpRatAmortPeriod

    ,DB_LAST    // Adding a new item? Insert directly above *only*.
    // That way, old databases remain compatible with new code, and any
    // error in using a new item will occur when the new item is used
    // instead of where a renumbered old item is used--that should make
    // such errors much easier to track down and repair. We can re-sort
    // additions into categories when preparing a major release.

    // TODO ?? We ought to find a way to make this file change whenever
    // dbnames.xpp changes. See that file for definition of each entity.
    };

struct db_names
{
/* TODO ?? Do we want such a ctor?
    db_names
        (DatabaseNames  a_Idx
        ,DatabaseNames  a_ParentIdx
        ,char*  a_ShortName
        ,char*  a_LongName
        )
        :Idx        (a_Idx)
        ,ParentIdx  (a_ParentIdx)
        ,ShortName  (a_ShortName)
        ,LongName   (a_LongName)
        {}
*/
    DatabaseNames       Idx;
    DatabaseNames       ParentIdx;
    char const*         ShortName;
    char const*         LongName;
};

std::vector<db_names> const& LMI_EXPIMP GetDBNames();

#endif // dbnames_hpp

