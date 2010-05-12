// Product database entity names.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

enum e_database_key
    {DB_FIRST

    ,DB_Topic_Underwriting

        ,DB_MinIssAge
        ,DB_MaxIssAge
        ,DB_MaxIncrAge

        ,DB_AllowFullUW
        ,DB_AllowSimpUW
        ,DB_AllowGuarUW
        ,DB_SmokeOrTobacco       // DATABASE !! Move to '.policy'
        ,DB_PrefOrSelect         // DATABASE !! Move to '.policy'
        ,DB_AllowPreferredClass
        ,DB_AllowUltraPrefClass

        ,DB_AllowSubstdTable
        ,DB_AllowFlatExtras
        ,DB_AllowRatedWP
        ,DB_AllowRatedADD
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

        ,DB_AllowCVAT
        ,DB_AllowGPT
        ,DB_AllowNo7702

        ,DB_CorridorTable
        ,DB_TAMRA7PayTable
        ,DB_IRC7702QTable

        ,DB_PremLoad7702
        ,DB_Equiv7702DBO3

    ,DB_Topic_MortalityCharges

        ,DB_GuarCOITable         // DATABASE !! s/COI/Coi/ (passim)
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

        ,DB_SubstdTblMult
        ,DB_SubstdTblMultTable

        ,DB_CoiUpper12Method
        ,DB_CoiInforceReentry

        ,DB_AllowMortBlendSex
        ,DB_AllowMortBlendSmoke

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

    ,DB_Topic_AssetCharges

        ,DB_AllowAmortPremLoad
        ,DB_AmortPmLdFundCharge

        ,DB_AllowCustomFund      // DATABASE !! DB_AllowImfOverride
        ,DB_AssetChargeType
        ,DB_StableValFundCharge

        ,DB_GuarFundAdminChg
        ,DB_CurrFundAdminChg
        ,DB_FundCharge

    ,DB_Topic_Loads

        ,DB_GuarPolFee           // DATABASE !! DB_GuarMonthlyPolFee
        ,DB_GuarIssueFee         // DATABASE !! DB_GuarAnnualPolFee

        ,DB_GuarPremLoadTgt
        ,DB_GuarPremLoadExc
        ,DB_GuarPremLoadTgtRfd
        ,DB_GuarPremLoadExcRfd

        ,DB_GuarSpecAmtLoad
        ,DB_GuarSpecAmtLoadTable

        ,DB_GuarAcctValLoadAMD   // DATABASE !! Lose the "AMD" suffix.

        ,DB_CurrPolFee           // DATABASE !! DB_CurrMonthlyPolFee
        ,DB_CurrIssueFee         // DATABASE !! DB_CurrAnnualPolFee

        ,DB_CurrPremLoadTgt
        ,DB_CurrPremLoadExc
        ,DB_CurrPremLoadTgtRfd
        ,DB_CurrPremLoadExcRfd

        ,DB_CurrSpecAmtLoad
        ,DB_CurrSpecAmtLoadTable

        ,DB_CurrAcctValLoadAMD   // DATABASE !! Lose the "AMD" suffix.

        ,DB_TgtPremPolFee
        ,DB_PremRefund           // DATABASE !! DB_RefundablePremLoad
        ,DB_SpecAmtLoadLimit
        ,DB_DynamicSepAcctLoad
        ,DB_DynSepAcctLoadLimit

    ,DB_Topic_DacAndPremiumTax

        ,DB_DACTaxFundCharge
        ,DB_DACTaxPremLoad

        ,DB_PremTaxFundCharge
        ,DB_PremTaxLoad
        ,DB_WaivePmTxInt1035
        ,DB_PremTaxRetalLimit
        ,DB_PremTaxTierGroup
        ,DB_PremTaxTierPeriod
        ,DB_PremTaxTierNonDecr

        ,DB_PmTxAmortPeriod
        ,DB_PmTxAmortIntRate

        ,DB_PremTaxRate
        ,DB_PremTaxState
        ,DB_PremTaxTable

    ,DB_Topic_SurrenderCharges

        ,DB_SurrChgAVMult
        ,DB_SurrChgAVDurFactor
        ,DB_SurrChgSAMult
        ,DB_SurrChgSADurFactor
        ,DB_SurrChgPremMult
        ,DB_SurrChgOnIncr
        ,DB_SurrChgOnDecr
        ,DB_Has1035ExchCharge    // DATABASE !! DB_1035ExchCharge

        ,DB_NonforfQTable        // DATABASE !! s/Nonforf/Snfl/ (passim)
        ,DB_COINonforfIsGuar
        ,DB_SurrChgByFormula
        ,DB_SurrChgPeriod
        ,DB_SurrChgZeroDur
        ,DB_SurrChgNLPMult
        ,DB_SurrChgNLPMax
        ,DB_SurrChgEAMax
        ,DB_SurrChgIsMly

    ,DB_Topic_DeathBenefits

        ,DB_AllowSAIncr
        ,DB_MinSpecAmtIncr
        ,DB_EnforceNAARLimit

        ,DB_MinSpecAmt
        ,DB_MinIssSpecAmt
        ,DB_MinRenlSpecAmt
        ,DB_MinRenlBaseSpecAmt
        ,DB_MaxIssSpecAmt
        ,DB_MaxRenlSpecAmt

        ,DB_AllowDBO1
        ,DB_AllowDBO2
        ,DB_AllowDBO3
        ,DB_AllowChangeToDBO2
        ,DB_OptChgCanIncrSA
        ,DB_OptChgCanDecrSA

        ,DB_AllowExtEndt         // DATABASE !! DB_CovgBeyondMaturity

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
        ,DB_SpouseRiderGuarTable
        ,DB_SpouseRiderTable

        ,DB_AllowChild
        ,DB_ChildRiderTable

    ,DB_Topic_Withdrawals

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

    ,DB_Topic_Premiums

        ,DB_MinPremType
        ,DB_MinPremIntSpread
        ,DB_TgtPremType
        ,DB_TgtPremTable
        ,DB_TgtPmFixedAtIssue
        ,DB_TgtPmIgnoreSubstd
        ,DB_MinPmt

    ,DB_Topic_SecondaryGuarantees

        ,DB_NoLapseMinDur
        ,DB_NoLapseMinAge
        ,DB_NoLapseUnratedOnly
        ,DB_NoLapseOpt1Only
        ,DB_NoLapseAlwaysActive

        ,DB_AllowHoneymoon

    ,DB_Topic_Allocations

        ,DB_AllowGenAcct
        ,DB_AllowSepAcct
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
        ,DB_ExpRatIBNRMult
        ,DB_ExpRatCOIRetention
        ,DB_ExpRatRiskCOIMult
        ,DB_ExpRatAmortPeriod

    ,DB_Topic_Miscellanea

        ,DB_LedgerType

        ,DB_AgeLastOrNearest     // DATABASE !! Add more options...
        ,DB_MaxIllusAge
        ,DB_EndtAge              // DATABASE !! DB_MaturityAge

        ,DB_LapseIgnoresSurrChg
        ,DB_DefaultProcessOrder
        ,DB_NominallyPar         // DATABASE !! Use a footnote instead.

        ,DB_TableYTable
        ,DB_83GamTable

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

    ,DB_Topic_OtherAssumptions

        ,DB_LapseRate
        ,DB_ReqSurpNAAR
        ,DB_ReqSurpVx
        ,DB_LICFitRate
        ,DB_LicDacTaxRate
        ,DB_GDBVxMethod
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

int         db_key_from_name(std::string const&);
std::string db_name_from_key(int);

#endif // dbnames_hpp

