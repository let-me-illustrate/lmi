# Insert sortable keys before 'DB_'-prefixed enumerators.
#
# Copyright (C) 2019 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# Use this script to sort blocks of product-file code, e.g. with the
# following 'vim' commands:
#
#   '<,'>!sed -f db_sort.sed
#   '<,'>sort
#   '<,'>s/^[A-Z][0-9][0-9]//
#
# which may be combined thus:
#
#   :execute "'<,'>!sed -f db_sort.sed" | '<,'>sort | '<,'>s/^[A-Z][0-9][0-9]//

# DB_Topic_Underwriting
/DB_MinIssAge\>/           s/^/A01/
/DB_MaxIssAge\>/           s/^/A02/
/DB_MaxIncrAge\>/          s/^/A03/
/DB_AllowFullUw\>/         s/^/A04/
/DB_AllowParamedUw\>/      s/^/A05/
/DB_AllowNonmedUw\>/       s/^/A06/
/DB_AllowSimpUw\>/         s/^/A07/
/DB_AllowGuarUw\>/         s/^/A08/
/DB_SmokeOrTobacco\>/      s/^/A09/
/DB_AllowPreferredClass\>/ s/^/A10/
/DB_AllowUltraPrefClass\>/ s/^/A11/
/DB_AllowSubstdTable\>/    s/^/A12/
/DB_AllowFlatExtras\>/     s/^/A13/
/DB_AllowRatedWp\>/        s/^/A14/
/DB_AllowRatedAdb\>/       s/^/A15/
/DB_AllowRatedTerm\>/      s/^/A16/
/DB_AllowRetirees\>/       s/^/A17/
/DB_AllowUnisex\>/         s/^/A18/
/DB_AllowSexDistinct\>/    s/^/A19/
/DB_AllowUnismoke\>/       s/^/A20/
/DB_AllowSmokeDistinct\>/  s/^/A21/
/DB_StateApproved\>/       s/^/A22/
/DB_AllowStateXX\>/        s/^/A23/
/DB_AllowForeign\>/        s/^/A24/
/DB_GroupIndivSelection\>/ s/^/A25/
/DB_TxCallsGuarUwSubstd\>/ s/^/A26/
/DB_Allowable\>/           s/^/A27/
# DB_Topic_7702And7702A
/DB_AllowCvat\>/           s/^/B01/
/DB_AllowGpt\>/            s/^/B02/
/DB_AllowNo7702\>/         s/^/B03/
/DB_CorridorWhence\>/      s/^/B04/
/DB_Irc7702NspWhence\>/    s/^/B05/
/DB_SevenPayWhence\>/      s/^/B06/
/DB_CorridorTable\>/       s/^/B07/
/DB_Irc7702NspTable\>/     s/^/B08/
/DB_SevenPayTable\>/       s/^/B09/
/DB_Irc7702QTable\>/       s/^/B10/
/DB_RatingsAffect7702\>/   s/^/B11/
/DB_CvatMatChangeDefn\>/   s/^/B12/
/DB_GptMatChangeDefn\>/    s/^/B13/
/DB_Irc7702BftIsSpecAmt\>/ s/^/B14/
/DB_RiskyInitial7702Db\>/  s/^/B15/
/DB_Irc7702Endowment\>/    s/^/B16/
/DB_Effective7702DboRop\>/ s/^/B17/
/DB_TermIsQABOrDb7702\>/   s/^/B18/
/DB_TermIsQABOrDb7702A\>/  s/^/B19/
/DB_GioIsQAB\>/            s/^/B20/
/DB_AdbIsQAB\>/            s/^/B21/
/DB_SpouseRiderIsQAB\>/    s/^/B22/
/DB_ChildRiderIsQAB\>/     s/^/B23/
/DB_WpIsQAB\>/             s/^/B24/
# DB_Topic_MortalityCharges
/DB_GuarCoiTable\>/        s/^/C01/
/DB_GuarCoiIsAnnual\>/     s/^/C02/
/DB_GuarCoiMultiplier\>/   s/^/C03/
/DB_CurrCoiTable\>/        s/^/C04/
/DB_CurrCoiIsAnnual\>/     s/^/C05/
/DB_MinInputCoiMult\>/     s/^/C06/
/DB_CurrCoiMultiplier\>/   s/^/C07/
/DB_UnusualCoiBanding\>/   s/^/C08/
/DB_CurrCoiTable0Limit\>/  s/^/C09/
/DB_CurrCoiTable1\>/       s/^/C10/
/DB_CurrCoiTable1Limit\>/  s/^/C11/
/DB_CurrCoiTable2\>/       s/^/C12/
/DB_MdptCoiTable\>/        s/^/C13/
/DB_MdptCoiIsAnnual\>/     s/^/C14/
/DB_CoiNyMinTable\>/       s/^/C15/
/DB_UseNyCoiFloor\>/       s/^/C16/
/DB_MaxMonthlyCoiRate\>/   s/^/C17/
/DB_GuarCoiCeiling\>/      s/^/C18/
/DB_CoiGuarIsMin\>/        s/^/C19/
/DB_SubstdTableMult\>/     s/^/C20/
/DB_SubstdTableMultTable\>/s/^/C21/
/DB_CoiUpper12Method\>/    s/^/C22/
/DB_CoiInforceReentry\>/   s/^/C23/
/DB_CoiResetMinDate\>/     s/^/C24/
/DB_CoiResetMaxDate\>/     s/^/C25/
/DB_AllowMortBlendSex\>/   s/^/C26/
/DB_AllowMortBlendSmoke\>/ s/^/C27/
# DB_Topic_Interest
/DB_GuarInt\>/             s/^/D01/
/DB_NaarDiscount\>/        s/^/D02/
/DB_GuarIntSpread\>/       s/^/D03/
/DB_GuarMandE\>/           s/^/D04/
/DB_CurrIntSpread\>/       s/^/D05/
/DB_CurrMandE\>/           s/^/D06/
/DB_GenAcctIntBonus\>/     s/^/D07/
/DB_BonusInt\>/            s/^/D08/
/DB_IntFloor\>/            s/^/D09/
/DB_AllowGenAcct\>/        s/^/D10/
/DB_AllowSepAcct\>/        s/^/D11/
/DB_AllowGenAcctEarnRate\>/s/^/D12/
/DB_AllowSepAcctNetRate\>/ s/^/D13/
/DB_MaxGenAcctRate\>/      s/^/D14/
/DB_MaxSepAcctRate\>/      s/^/D15/
/DB_SepAcctSpreadMethod\>/ s/^/D16/
/DB_IntSpreadMode\>/       s/^/D17/
/DB_DynamicMandE\>/        s/^/D18/
# DB_Topic_AssetCharges
/DB_AllowAmortPremLoad\>/  s/^/E01/
/DB_LoadAmortFundCharge\>/ s/^/E02/
/DB_AllowImfOverride\>/    s/^/E03/
/DB_AssetChargeType\>/     s/^/E04/
/DB_StableValFundCharge\>/ s/^/E05/
/DB_GuarFundAdminChg\>/    s/^/E06/
/DB_CurrFundAdminChg\>/    s/^/E07/
/DB_FundCharge\>/          s/^/E08/
# DB_Topic_Loads
/DB_GuarMonthlyPolFee\>/   s/^/F01/
/DB_GuarAnnualPolFee\>/    s/^/F02/
/DB_GuarPremLoadTgt\>/     s/^/F03/
/DB_GuarPremLoadExc\>/     s/^/F04/
/DB_GuarPremLoadTgtRfd\>/  s/^/F05/
/DB_GuarPremLoadExcRfd\>/  s/^/F06/
/DB_GuarSpecAmtLoad\>/     s/^/F07/
/DB_GuarSpecAmtLoadTable\>/s/^/F08/
/DB_GuarAcctValLoad\>/     s/^/F09/
/DB_CurrMonthlyPolFee\>/   s/^/F10/
/DB_CurrAnnualPolFee\>/    s/^/F11/
/DB_CurrPremLoadTgt\>/     s/^/F12/
/DB_CurrPremLoadExc\>/     s/^/F13/
/DB_CurrPremLoadTgtRfd\>/  s/^/F14/
/DB_CurrPremLoadExcRfd\>/  s/^/F15/
/DB_CurrSpecAmtLoad\>/     s/^/F16/
/DB_CurrSpecAmtLoadTable\>/s/^/F17/
/DB_CurrAcctValLoad\>/     s/^/F18/
/DB_TgtPremMonthlyPolFee\>/s/^/F19/
/DB_LoadRfdProportion\>/   s/^/F20/
/DB_SpecAmtLoadLimit\>/    s/^/F21/
/DB_DynamicSepAcctLoad\>/  s/^/F22/
/DB_DynSepAcctLoadLimit\>/ s/^/F23/
# DB_Topic_DacAndPremiumTax
/DB_DacTaxFundCharge\>/    s/^/G01/
/DB_DacTaxPremLoad\>/      s/^/G02/
/DB_PremTaxFundCharge\>/   s/^/G03/
/DB_PremTaxLoad\>/         s/^/G04/
/DB_WaivePremTaxInt1035\>/ s/^/G05/
/DB_PremTaxRetalLimit\>/   s/^/G06/
/DB_PremTaxTierGroup\>/    s/^/G07/
/DB_PremTaxTierPeriod\>/   s/^/G08/
/DB_PremTaxTierNonDecr\>/  s/^/G09/
/DB_PremTaxAmortPeriod\>/  s/^/G10/
/DB_PremTaxAmortIntRate\>/ s/^/G11/
/DB_PremTaxRate\>/         s/^/G12/
/DB_PremTaxState\>/        s/^/G13/
# DB_Topic_SurrenderCharges
/DB_SurrChgOnIncr\>/       s/^/H01/
/DB_SurrChgOnDecr\>/       s/^/H02/
/DB_Has1035ExchCharge\>/   s/^/H03/
/DB_SnflQTable\>/          s/^/H04/
/DB_CoiSnflIsGuar\>/       s/^/H05/
/DB_SurrChgByFormula\>/    s/^/H06/
/DB_SurrChgPeriod\>/       s/^/H07/
/DB_SurrChgZeroDur\>/      s/^/H08/
/DB_SurrChgNlpMult\>/      s/^/H09/
/DB_SurrChgNlpMax\>/       s/^/H10/
/DB_SurrChgEaMax\>/        s/^/H11/
/DB_SurrChgAmort\>/        s/^/H12/
# DB_Topic_DeathBenefits
/DB_AllowSpecAmtIncr\>/    s/^/I01/
/DB_MinSpecAmtIncr\>/      s/^/I02/
/DB_EnforceNaarLimit\>/    s/^/I03/
/DB_MinSpecAmt\>/          s/^/I04/
/DB_MinIssSpecAmt\>/       s/^/I05/
/DB_MinIssBaseSpecAmt\>/   s/^/I06/
/DB_MinRenlSpecAmt\>/      s/^/I07/
/DB_MinRenlBaseSpecAmt\>/  s/^/I08/
/DB_MaxIssSpecAmt\>/       s/^/I09/
/DB_MaxRenlSpecAmt\>/      s/^/I10/
/DB_AllowDboLvl\>/         s/^/I11/
/DB_AllowDboInc\>/         s/^/I12/
/DB_AllowDboRop\>/         s/^/I13/
/DB_AllowDboMdb\>/         s/^/I14/
/DB_DboLvlChangeToWhat\>/  s/^/I15/
/DB_DboLvlChangeMethod\>/  s/^/I16/
/DB_DboIncChangeToWhat\>/  s/^/I17/
/DB_DboIncChangeMethod\>/  s/^/I18/
/DB_DboRopChangeToWhat\>/  s/^/I19/
/DB_DboRopChangeMethod\>/  s/^/I20/
/DB_DboMdbChangeToWhat\>/  s/^/I21/
/DB_DboMdbChangeMethod\>/  s/^/I22/
/DB_AllowChangeToDbo2\>/   s/^/I23/
/DB_DboChgCanIncrSpecAmt\>/s/^/I24/
/DB_DboChgCanDecrSpecAmt\>/s/^/I25/
/DB_AllowExtEndt\>/        s/^/I26/
# DB_Topic_Riders
/DB_AllowTerm\>/           s/^/J01/
/DB_GuarTermTable\>/       s/^/J02/
/DB_TermTable\>/           s/^/J03/
/DB_TermMinIssAge\>/       s/^/J04/
/DB_TermMaxIssAge\>/       s/^/J05/
/DB_TermIsNotRider\>/      s/^/J06/
/DB_TermCanLapse\>/        s/^/J07/
/DB_TermForcedConvAge\>/   s/^/J08/
/DB_TermForcedConvDur\>/   s/^/J09/
/DB_MaxTermProportion\>/   s/^/J10/
/DB_TermCoiRate\>/         s/^/J11/
/DB_TermPremRate\>/        s/^/J12/
/DB_AllowWp\>/             s/^/J13/
/DB_WpTable\>/             s/^/J14/
/DB_WpMinIssAge\>/         s/^/J15/
/DB_WpMaxIssAge\>/         s/^/J16/
/DB_WpLimit\>/             s/^/J17/
/DB_WpCoiRate\>/           s/^/J18/
/DB_WpPremRate\>/          s/^/J19/
/DB_WpChargeMethod\>/      s/^/J20/
/DB_AllowAdb\>/            s/^/J21/
/DB_AdbTable\>/            s/^/J22/
/DB_AdbMinIssAge\>/        s/^/J23/
/DB_AdbMaxIssAge\>/        s/^/J24/
/DB_AdbLimit\>/            s/^/J25/
/DB_AdbCoiRate\>/          s/^/J26/
/DB_AdbPremRate\>/         s/^/J27/
/DB_AllowSpouseRider\>/    s/^/J28/
/DB_SpouseRiderMinAmt\>/   s/^/J29/
/DB_SpouseRiderMaxAmt\>/   s/^/J30/
/DB_SpouseRiderMinIssAge\>/s/^/J31/
/DB_SpouseRiderMaxIssAge\>/s/^/J32/
/DB_SpouseRiderGuarTable\>/s/^/J33/
/DB_SpouseRiderTable\>/    s/^/J34/
/DB_AllowChildRider\>/     s/^/J35/
/DB_ChildRiderMinAmt\>/    s/^/J36/
/DB_ChildRiderMaxAmt\>/    s/^/J37/
/DB_ChildRiderTable\>/     s/^/J38/
# DB_Topic_Withdrawals
/DB_AllowWd\>/             s/^/K01/
/DB_WdFee\>/               s/^/K02/
/DB_WdFeeRate\>/           s/^/K03/
/DB_FreeWdProportion\>/    s/^/K04/
/DB_MinWd\>/               s/^/K05/
/DB_MaxWdGenAcctValMult\>/ s/^/K06/
/DB_MaxWdSepAcctValMult\>/ s/^/K07/
/DB_MaxWdDed\>/            s/^/K08/
/DB_WdDecrSpecAmtDboLvl\>/ s/^/K09/
/DB_WdDecrSpecAmtDboInc\>/ s/^/K10/
/DB_WdDecrSpecAmtDboRop\>/ s/^/K11/
/DB_FirstWdMonth\>/        s/^/K12/
# DB_Topic_Loans
/DB_AllowLoan\>/           s/^/L01/
/DB_AllowPrefLoan\>/       s/^/L02/
/DB_AllowFixedLoan\>/      s/^/L03/
/DB_AllowVlr\>/            s/^/L04/
/DB_FixedLoanRate\>/       s/^/L05/
/DB_MaxVlrRate\>/          s/^/L06/
/DB_MinVlrRate\>/          s/^/L07/
/DB_MaxLoanAcctValMult\>/  s/^/L08/
/DB_MaxLoanDed\>/          s/^/L09/
/DB_FirstPrefLoanYear\>/   s/^/L10/
/DB_PrefLoanRateDecr\>/    s/^/L11/
/DB_GuarPrefLoanSpread\>/  s/^/L12/
/DB_GuarRegLoanSpread\>/   s/^/L13/
/DB_CurrPrefLoanSpread\>/  s/^/L14/
/DB_CurrRegLoanSpread\>/   s/^/L15/
/DB_FirstLoanMonth\>/      s/^/L16/
# DB_Topic_Premiums
/DB_MinPremType\>/         s/^/M01/
/DB_MinPremIntSpread\>/    s/^/M02/
/DB_SplitMinPrem\>/        s/^/M03/
/DB_UnsplitSplitMinPrem\>/ s/^/M04/
/DB_ErNotionallyPaysTerm\>/s/^/M05/
/DB_TgtPremType\>/         s/^/M06/
/DB_TgtPremTable\>/        s/^/M07/
/DB_TgtPremFixedAtIssue\>/ s/^/M08/
/DB_TgtPremIgnoreSubstd\>/ s/^/M09/
/DB_MinPmt\>/              s/^/M10/
# DB_Topic_SecondaryGuarantees
/DB_NoLapseMinDur\>/       s/^/N01/
/DB_NoLapseMinAge\>/       s/^/N02/
/DB_NoLapseUnratedOnly\>/  s/^/N03/
/DB_NoLapseDboLvlOnly\>/   s/^/N04/
/DB_NoLapseAlwaysActive\>/ s/^/N05/
/DB_AllowHoneymoon\>/      s/^/N06/
# DB_Topic_Allocations
/DB_DeductionMethod\>/     s/^/O01/
/DB_DeductionAcct\>/       s/^/O02/
/DB_DistributionMethod\>/  s/^/O03/
/DB_DistributionAcct\>/    s/^/O04/
/DB_EePremMethod\>/        s/^/O05/
/DB_EePremAcct\>/          s/^/O06/
/DB_ErPremMethod\>/        s/^/O07/
/DB_ErPremAcct\>/          s/^/O08/
# DB_Topic_Compensation
/DB_CalculateComp\>/       s/^/P01/
/DB_CompTarget\>/          s/^/P02/
/DB_CompExcess\>/          s/^/P03/
/DB_CompChargeback\>/      s/^/P04/
/DB_AssetComp\>/           s/^/P05/
/DB_AllowExtraAssetComp\>/ s/^/P06/
/DB_AllowExtraPremComp\>/  s/^/P07/
# DB_Topic_ExperienceRating
/DB_AllowExpRating\>/      s/^/Q01/
/DB_ExpRatStdDevMult\>/    s/^/Q02/
/DB_ExpRatIbnrMult\>/      s/^/Q03/
/DB_ExpRatCoiRetention\>/  s/^/Q04/
/DB_ExpRatRiskCoiMult\>/   s/^/Q05/
/DB_ExpRatAmortPeriod\>/   s/^/Q06/
# DB_Topic_Miscellanea
/DB_LedgerType\>/          s/^/R01/
/DB_Nonillustrated\>/      s/^/R02/
/DB_NoLongerIssued\>/      s/^/R03/
/DB_AgeLastOrNearest\>/    s/^/R04/
/DB_MaturityAge\>/         s/^/R05/
/DB_CashValueEnhMult\>/    s/^/R06/
/DB_LapseIgnoresSurrChg\>/ s/^/R07/
/DB_DefaultProcessOrder\>/ s/^/R08/
/DB_GroupProxyRateTable\>/ s/^/R09/
/DB_PartialMortTable\>/    s/^/R10/
/DB_UsePolicyFormAlt\>/    s/^/R11/
# DB_Topic_Weights
/DB_WeightClass\>/         s/^/S01/
/DB_WeightGender\>/        s/^/S02/
/DB_WeightSmoking\>/       s/^/S03/
/DB_WeightAge\>/           s/^/S04/
/DB_WeightSpecAmt\>/       s/^/S05/
/DB_WeightState\>/         s/^/S06/
# DB_Topic_Expenses
/DB_FullExpPol\>/          s/^/T01/
/DB_FullExpPrem\>/         s/^/T02/
/DB_FullExpDumpin\>/       s/^/T03/
/DB_FullExpSpecAmt\>/      s/^/T04/
/DB_VarExpPol\>/           s/^/T05/
/DB_VarExpPrem\>/          s/^/T06/
/DB_VarExpDumpin\>/        s/^/T07/
/DB_VarExpSpecAmt\>/       s/^/T08/
/DB_ExpSpecAmtLimit\>/     s/^/T09/
/DB_MedicalProportion\>/   s/^/T10/
/DB_UwTestCost\>/          s/^/T11/
# DB_Topic_Reserves
/DB_VxBasicQTable\>/       s/^/U01/
/DB_VxDeficQTable\>/       s/^/U02/
/DB_VxTaxQTable\>/         s/^/U03/
/DB_StatVxInt\>/           s/^/U04/
/DB_TaxVxInt\>/            s/^/U05/
/DB_StatVxQ\>/             s/^/U06/
/DB_TaxVxQ\>/              s/^/U07/
/DB_DeficVxQ\>/            s/^/U08/
/DB_SnflQ\>/               s/^/U09/
# DB_Topic_OtherAssumptions
/DB_LapseRate\>/           s/^/V01/
/DB_ReqSurpNaar\>/         s/^/V02/
/DB_ReqSurpVx\>/           s/^/V03/
/DB_LicFitRate\>/          s/^/V04/
/DB_LicDacTaxRate\>/       s/^/V05/
/DB_GdbVxMethod\>/         s/^/V06/
/DB_PrimaryHurdle\>/       s/^/V07/
/DB_SecondaryHurdle\>/     s/^/V08/
