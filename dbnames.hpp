// Product database entity names.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef dbnames_hpp
#define dbnames_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>
#include <vector>

/// See 'dbnames.xpp' for the definition of each entity.
///
/// Partial lexicon:
///  - Adb     Accidental death benefit
///  - Acct    Account
///  - Admin   Administration
///  - Amort   Amortization or amortized
///  - Amt     Amount
///  - Bft     Benefit
///  - Chg     Charge; more rarely, change (as in 'material change')
///  - Coi     Cost of insurance
///  - Comp    Compensation
///  - Conv    Conversion
///  - Curr    Current
///  - Cvat    Cash value accumulation test
///  - Dac     Deferred acquisition cost
///  - Db      Death benefit
///  - Dbo     Death benefit option
///  - Decr    Decrease
///  - Ded     Deduction
///  - Def     Deficiency (as in 'deficiency reserve')
///  - Defn    Definition
///  - Dev     Deviation (as in 'standard deviation')
///  - Dur     Duration
///  - Dyn     Dynamic
///  - Ea      Expense allowance (SNFL)
///  - Ee      Employee
///  - Endt    Endowment
///  - Equiv   Equivalent
///  - Er      Employer
///  - Exc     Excess (over target)
///  - Exch    Exchange
///  - Exp     Expense or experience
///  - Ext     Extended (as in 'extended endowment')
///  - Fit     Federal income tax
///  - Gdb     Guaranteed death benefit
///  - Gen     General (as in 'general account')
///  - Gpt     Guideline premium test
///  - Gsp     Guideline single premium
///  - Guar    Guaranteed
///  - Ibnr    Incurred but not reported (reserve)
///  - Imf     Investment management fee
///  - Incr    Increase
///  - Int     Interest; more rarely, internal
///  - Irc     Internal revenue code
///  - Iss     Issue
///  - Lic     Life insurance company
///  - MandE   Mortality and expense charge
///  - Mat     Material (as in 'material change')
///  - Max     Maximum
///  - Mdpt    Midpoint
///  - Min     Minimum
///  - Mort    Mortality
///  - Mult    Multiplier
///  - Naar    Net amount at risk
///  - Nlp     Net level premium
///  - Nsp     Net single premium
///  - Ny      New York
///  - Pmt     Payment
///  - Pol     Policy
///  - Pref    Preferred
///  - Prem    Premium
///  - Q       Death rate
///  - Rat     Rating (as in 'experience rating')
///  - Reg     Regular or regulation
///  - Renl    Renewal
///  - Req     Required (as in 'required surplus')
///  - Retal   Retaliation
///  - Rfd     Refund or refundable
///  - Sep     Separate (as in 'separate account')
///  - Simp    Simplified (as in 'simplified underwriting')
///  - Snfl    Standard nonforfeiture law
///  - Spec    Specified (as in 'specified amount')
///  - Std     Standard
///  - Substd  Substandard
///  - Surp    Surplus
///  - Surr    Surrender
///  - Tgt     Target
///  - Uw      Underwriting
///  - Val     Value (as in 'account value')
///  - Vlr     Variable loan rate
///  - Vx      Reserve
///  - Wd      Withdrawal
///  - Wp      Waiver of premium

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
        ,DB_SmokeOrTobacco
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

        ,DB_GroupIndivSelection

        ,DB_TxCallsGuarUwSubstd

        ,DB_Allowable

    ,DB_Topic_7702And7702A

        ,DB_AllowCvat
        ,DB_AllowGpt
        ,DB_AllowNo7702
        ,DB_AnnIntRate7702
        ,DB_AnnIntRateGspDelta
        ,DB_ShortTermIntGuar7702
        ,DB_IgnoreLoanRateFor7702
        ,DB_Irc7702Obreption

        ,DB_CorridorWhence
        ,DB_Irc7702NspWhence
        ,DB_SevenPayWhence
        ,DB_Irc7702QWhence
        ,DB_CorridorTable
        ,DB_Irc7702NspTable
        ,DB_SevenPayTable
        ,DB_Irc7702QTable
        ,DB_Irc7702QAxisGender
        ,DB_Irc7702QAxisSmoking

        ,DB_CvatMatChangeDefn
        ,DB_GptMatChangeDefn
        ,DB_Irc7702BftIsSpecAmt
        ,DB_Effective7702DboRop

        ,DB_TermIsQABOrDb7702
        ,DB_TermIsQABOrDb7702A
        ,DB_GioIsQAB
        ,DB_AdbIsQAB
        ,DB_SpouseRiderIsQAB
        ,DB_ChildRiderIsQAB
        ,DB_WpIsQAB

    ,DB_Topic_MortalityCharges

        ,DB_CsoEra
        ,DB_CsoMisprint
        ,DB_GuarCoiWhence
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
        ,DB_CoiResetMinDate
        ,DB_CoiResetMaxDate

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

        ,DB_CurrSepAcctLoad

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

        ,DB_AllowDboLvl
        ,DB_AllowDboInc
        ,DB_AllowDboRop
        ,DB_AllowDboMdb

        ,DB_DboLvlChangeToWhat
        ,DB_DboLvlChangeMethod
        ,DB_DboIncChangeToWhat
        ,DB_DboIncChangeMethod
        ,DB_DboRopChangeToWhat
        ,DB_DboRopChangeMethod
        ,DB_DboMdbChangeToWhat
        ,DB_DboMdbChangeMethod

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
        ,DB_TermIsNotRider
        ,DB_TermCanLapse
        ,DB_TermForcedConvAge
        ,DB_TermForcedConvDur
        ,DB_MaxTermProportion
        ,DB_TermCoiRate
        ,DB_TermPremRate

        ,DB_AllowWp
        ,DB_WpTable
        ,DB_WpMinIssAge
        ,DB_WpMaxIssAge
        ,DB_WpLimit
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
        ,DB_SpouseRiderMinAmt
        ,DB_SpouseRiderMaxAmt
        ,DB_SpouseRiderMinIssAge
        ,DB_SpouseRiderMaxIssAge
        ,DB_SpouseRiderGuarTable
        ,DB_SpouseRiderTable

        ,DB_AllowChildRider
        ,DB_ChildRiderMinAmt
        ,DB_ChildRiderMaxAmt
        ,DB_ChildRiderTable

    ,DB_Topic_Withdrawals

        ,DB_AllowWd
        ,DB_WdFee
        ,DB_WdFeeRate

        ,DB_FreeWdProportion
        ,DB_MinWd
        ,DB_MaxWdGenAcctValMult
        ,DB_MaxWdSepAcctValMult
        ,DB_MaxWdDed

        ,DB_WdDecrSpecAmtDboLvl
        ,DB_WdDecrSpecAmtDboInc
        ,DB_WdDecrSpecAmtDboRop

        ,DB_FirstWdMonth

    ,DB_Topic_Loans

        ,DB_AllowLoan
        ,DB_AllowPrefLoan
        ,DB_AllowFixedLoan
        ,DB_AllowVlr
        ,DB_FixedLoanRate
        ,DB_MaxVlrRate
        ,DB_MinVlrRate

        ,DB_MaxLoanAcctValMult
        ,DB_MaxLoanDed
        ,DB_FirstPrefLoanYear
        ,DB_PrefLoanRateDecr

        ,DB_GuarPrefLoanSpread
        ,DB_GuarRegLoanSpread

        ,DB_CurrPrefLoanSpread
        ,DB_CurrRegLoanSpread

        ,DB_FirstLoanMonth

    ,DB_Topic_Premiums

        ,DB_MinPremType
        ,DB_MinPremTable
        ,DB_MinPremIntSpread
        ,DB_SplitMinPrem
        ,DB_UnsplitSplitMinPrem
        ,DB_ErNotionallyPaysTerm
        ,DB_TgtPremType
        ,DB_TgtPremTable
        ,DB_TgtPremFixedAtIssue
        ,DB_TgtPremIgnoreSubstd
        ,DB_MinPmt
        ,DB_IsSinglePremium
        ,DB_MinSinglePremiumType
        ,DB_MinSinglePremiumMult

    ,DB_Topic_SecondaryGuarantees

        ,DB_NoLapseMinDur
        ,DB_NoLapseMinAge
        ,DB_NoLapseUnratedOnly
        ,DB_NoLapseDboLvlOnly
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

        ,DB_CalculateComp
        ,DB_CompTarget
        ,DB_CompExcess
        ,DB_CompChargeback
        ,DB_AssetComp
        ,DB_AllowExtraAssetComp
        ,DB_AllowExtraPremComp

    ,DB_Topic_Miscellanea

        ,DB_LedgerType
        ,DB_Nonillustrated
        ,DB_NoLongerIssued

        ,DB_AgeLastOrNearest
        ,DB_MaturityAge

        ,DB_AllowCashValueEnh
        ,DB_CashValueEnhMult
        ,DB_LapseIgnoresSurrChg

        ,DB_DefaultProcessOrder

        ,DB_GroupProxyRateTable
        ,DB_PartialMortTable
        ,DB_AllowGroupQuote

    ,DB_Topic_Lingo

        ,DB_PolicyForm
        ,DB_PolicyMktgName
        ,DB_PolicyLegalName
        ,DB_InsCoShortName
        ,DB_InsCoName
        ,DB_InsCoAddr
        ,DB_InsCoStreet
        ,DB_InsCoPhone
        ,DB_MainUnderwriter
        ,DB_MainUnderwriterAddress
        ,DB_CoUnderwriter
        ,DB_CoUnderwriterAddress
        ,DB_AvName
        ,DB_CsvName
        ,DB_CsvHeaderName
        ,DB_NoLapseProvisionName
        ,DB_ContractName
        ,DB_DboName
        ,DB_DboNameLevel
        ,DB_DboNameIncreasing
        ,DB_DboNameReturnOfPremium
        ,DB_DboNameMinDeathBenefit
        ,DB_GenAcctName
        ,DB_GenAcctNameElaborated
        ,DB_SepAcctName
        ,DB_SpecAmtName
        ,DB_SpecAmtNameElaborated
        ,DB_UwBasisMedical
        ,DB_UwBasisParamedical
        ,DB_UwBasisNonmedical
        ,DB_UwBasisSimplified
        ,DB_UwBasisGuaranteed
        ,DB_UwClassPreferred
        ,DB_UwClassStandard
        ,DB_UwClassRated
        ,DB_UwClassUltra
        ,DB_AccountValueFootnote
        ,DB_AttainedAgeFootnote
        ,DB_CashSurrValueFootnote
        ,DB_DeathBenefitFootnote
        ,DB_InitialPremiumFootnote
        ,DB_NetPremiumFootnote
        ,DB_GrossPremiumFootnote
        ,DB_OutlayFootnote
        ,DB_PolicyYearFootnote
        ,DB_ADDTerseName
        ,DB_InsurabilityTerseName
        ,DB_ChildTerseName
        ,DB_SpouseTerseName
        ,DB_TermTerseName
        ,DB_WaiverTerseName
        ,DB_AccelBftRiderTerseName
        ,DB_OverloanRiderTerseName
        ,DB_ADDFootnote
        ,DB_ChildFootnote
        ,DB_SpouseFootnote
        ,DB_TermFootnote
        ,DB_WaiverFootnote
        ,DB_AccelBftRiderFootnote
        ,DB_OverloanRiderFootnote
        ,DB_GroupQuoteShortProductName
        ,DB_GroupQuoteIsNotAnOffer
        ,DB_GroupQuoteRidersFooter
        ,DB_GroupQuotePolicyFormId
        ,DB_GroupQuoteStateVariations
        ,DB_GroupQuoteProspectus
        ,DB_GroupQuoteUnderwriter
        ,DB_GroupQuoteBrokerDealer
        ,DB_GroupQuoteRubricMandatory
        ,DB_GroupQuoteRubricVoluntary
        ,DB_GroupQuoteRubricFusion
        ,DB_GroupQuoteFooterMandatory
        ,DB_GroupQuoteFooterVoluntary
        ,DB_GroupQuoteFooterFusion
        ,DB_MinimumPremiumFootnote
        ,DB_PremAllocationFootnote
        ,DB_InterestDisclaimer
        ,DB_GuarMortalityFootnote
        ,DB_ProductDescription
        ,DB_StableValueFootnote
        ,DB_NoVanishPremiumFootnote
        ,DB_RejectPremiumFootnote
        ,DB_ExpRatingFootnote
        ,DB_MortalityBlendFootnote
        ,DB_HypotheticalRatesFootnote
        ,DB_SalesLoadRefundFootnote
        ,DB_NoLapseEverFootnote
        ,DB_NoLapseFootnote
        ,DB_CurrentValuesFootnote
        ,DB_DBOption1Footnote
        ,DB_DBOption2Footnote
        ,DB_DBOption3Footnote
        ,DB_MinDeathBenefitFootnote
        ,DB_ExpRatRiskChargeFootnote
        ,DB_ExchangeChargeFootnote1
        ,DB_FlexiblePremiumFootnote
        ,DB_GuaranteedValuesFootnote
        ,DB_CreditingRateFootnote
        ,DB_GrossRateFootnote
        ,DB_NetRateFootnote
        ,DB_MecFootnote
        ,DB_GptFootnote
        ,DB_MidpointValuesFootnote
        ,DB_SinglePremiumFootnote
        ,DB_MonthlyChargesFootnote
        ,DB_UltCreditingRateFootnote
        ,DB_UltCreditingRateHeader
        ,DB_MaxNaarFootnote
        ,DB_PremTaxSurrChgFootnote
        ,DB_PolicyFeeFootnote
        ,DB_AssetChargeFootnote
        ,DB_InvestmentIncomeFootnote
        ,DB_IrrDbFootnote
        ,DB_IrrCsvFootnote
        ,DB_MortalityChargesFootnote
        ,DB_LoanAndWithdrawalFootnote
        ,DB_LoanFootnote
        ,DB_ImprimaturPresale
        ,DB_ImprimaturPresaleComposite
        ,DB_ImprimaturInforce
        ,DB_ImprimaturInforceComposite
        ,DB_StateMarketingImprimatur
        ,DB_NonGuaranteedFootnote
        ,DB_NonGuaranteedFootnote1
        ,DB_NonGuaranteedFootnote1Tx
        ,DB_FnMonthlyDeductions
        ,DB_SurrenderFootnote
        ,DB_PortabilityFootnote
        ,DB_FundRateFootnote
        ,DB_IssuingCompanyFootnote
        ,DB_SubsidiaryFootnote
        ,DB_PlacementAgentFootnote
        ,DB_MarketingNameFootnote
        ,DB_GuarIssueDisclaimerNcSc
        ,DB_GuarIssueDisclaimerMd
        ,DB_GuarIssueDisclaimerTx
        ,DB_IllRegCertAgent
        ,DB_IllRegCertAgentIl
        ,DB_IllRegCertAgentTx
        ,DB_IllRegCertClient
        ,DB_IllRegCertClientIl
        ,DB_IllRegCertClientTx
        ,DB_FnMaturityAge
        ,DB_FnPartialMortality
        ,DB_FnProspectus
        ,DB_FnInitialSpecAmt
        ,DB_FnInforceAcctVal
        ,DB_FnInforceTaxBasis
        ,DB_Fn1035Charge
        ,DB_FnMecExtraWarning
        ,DB_FnNotTaxAdvice
        ,DB_FnNotTaxAdvice2
        ,DB_FnImf
        ,DB_FnCensus
        ,DB_FnDacTax
        ,DB_FnDefnLifeIns
        ,DB_FnBoyEoy
        ,DB_FnGeneralAccount
        ,DB_FnPpMemorandum
        ,DB_FnPpAccreditedInvestor
        ,DB_FnPpLoads
        ,DB_FnProposalUnderwriting
        ,DB_FnGuaranteedPremium
        ,DB_FnOmnibusDisclaimer
        ,DB_FnInitialDbo
        ,DB_DefnGuarGenAcctRate
        ,DB_DefnAV
        ,DB_DefnCSV
        ,DB_DefnMec
        ,DB_DefnOutlay
        ,DB_DefnSpecAmt

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
        ,DB_DeficVxQ
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

LMI_SO std::vector<db_names> const& GetDBNames();

LMI_SO int         db_key_from_name(std::string const&);
LMI_SO std::string db_name_from_key(int);

#endif // dbnames_hpp
