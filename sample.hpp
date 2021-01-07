// Parameters for a company's entire portfolio of products.
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

#ifndef sample_hpp
#define sample_hpp

#include "config.hpp"

// For now, this file contains only an enumeration, but someday it may
// include other information that applies to an entire portfolio.

/// For the fictional Superior Life Insurance Company of Superior, WI.

namespace superior
{
/// Enumerate lingo strings.
///
/// This is deliberately defined with enum-key 'enum' rather than
/// 'enum class' or 'enum struct'. Because it is defined inside a
/// namespace, with an enum-base, it is the same as an 'enum class'
/// except that its enumerators decay to int as nature intended.
/// Feature comparison:
///
///   this enum  enum class   desirable properties
///   ---------  ----------   --------------------
///      yes        yes       avoids polluting global namespace
///      yes        yes       specifies underlying type
///      yes         no       implicitly converts to int
///
/// Enumerator zero is reserved for an empty string because zero is
/// the default value for database entities.

enum lingo : int
    {empty_string = 0

    // Essential strings describing the policy and company.
    ,policy_form_term
    ,policy_form
    ,policy_form_KS_KY
    // LINGO !! reconsider this...
    // These MixedCase terms seem better--it's easier to grep for a
    // string that has no lower_case_with_underscores variant:
//  ,PolicyForm_term
//  ,PolicyForm
    ,PolicyMktgName_term
    ,PolicyMktgName
    ,PolicyLegalName_term
    ,PolicyLegalName
    ,InsCoShortName_term
    ,InsCoShortName
    ,InsCoName_term
    ,InsCoName
    ,InsCoAddr_term
    ,InsCoAddr
    ,InsCoStreet_term
    ,InsCoStreet
    ,InsCoPhone_term
    ,InsCoPhone
    ,MainUnderwriter_term
    ,MainUnderwriter
    ,MainUnderwriterAddress_term
    ,MainUnderwriterAddress
    ,CoUnderwriter_term
    ,CoUnderwriter
    ,CoUnderwriterAddress_term
    ,CoUnderwriterAddress

    // Terms defined in the contract, which must be used for column
    // headers, footnotes, etc. according to the illustration reg.
    ,AvName_term
    ,AvName
    ,CsvName_term
    ,CsvName
    ,CsvHeaderName_term
    ,CsvHeaderName
    ,NoLapseProvisionName_term
    ,NoLapseProvisionName
    ,ContractName_term
    ,ContractName
    ,DboName_term
    ,DboName
    ,DboNameLevel_term
    ,DboNameLevel
    ,DboNameIncreasing_term
    ,DboNameIncreasing
    ,DboNameReturnOfPremium_term
    ,DboNameReturnOfPremium
    ,DboNameMinDeathBenefit_term
    ,DboNameMinDeathBenefit
    ,GenAcctName_term
    ,GenAcctName
    ,GenAcctNameElaborated_term
    ,GenAcctNameElaborated
    ,SepAcctName_term
    ,SepAcctName
    ,SpecAmtName_term
    ,SpecAmtName
    ,SpecAmtNameElaborated_term
    ,SpecAmtNameElaborated

    // Underwriting terms.
    ,UwBasisMedical_term
    ,UwBasisMedical
    ,UwBasisParamedical_term
    ,UwBasisParamedical
    ,UwBasisNonmedical_term
    ,UwBasisNonmedical
    ,UwBasisSimplified_term
    ,UwBasisSimplified
    ,UwBasisGuaranteed_term
    ,UwBasisGuaranteed
    ,UwClassPreferred_term
    ,UwClassPreferred
    ,UwClassStandard_term
    ,UwClassStandard
    ,UwClassRated_term
    ,UwClassRated
    ,UwClassUltra_term
    ,UwClassUltra

    // Ledger column definitions.
    ,AccountValueFootnote_term
    ,AccountValueFootnote
    ,AttainedAgeFootnote_term
    ,AttainedAgeFootnote
    ,CashSurrValueFootnote_term
    ,CashSurrValueFootnote
    ,DeathBenefitFootnote_term
    ,DeathBenefitFootnote
    ,InitialPremiumFootnote_term
    ,InitialPremiumFootnote
    ,NetPremiumFootnote_term
    ,NetPremiumFootnote
    ,GrossPremiumFootnote_term
    ,GrossPremiumFootnote
    ,OutlayFootnote_term
    ,OutlayFootnote
    ,PolicyYearFootnote_term
    ,PolicyYearFootnote

    // Terse rider names.
    ,ADDTerseName_term
    ,ADDTerseName
    ,InsurabilityTerseName_term
    ,InsurabilityTerseName
    ,ChildTerseName_term
    ,ChildTerseName
    ,SpouseTerseName_term
    ,SpouseTerseName
    ,TermTerseName_term
    ,TermTerseName
    ,WaiverTerseName_term
    ,WaiverTerseName
    ,AccelBftRiderTerseName_term
    ,AccelBftRiderTerseName
    ,OverloanRiderTerseName_term
    ,OverloanRiderTerseName

    // Rider footnotes.
    ,ADDFootnote_term
    ,ADDFootnote
    ,ChildFootnote_term
    ,ChildFootnote
    ,SpouseFootnote_term
    ,SpouseFootnote
    ,TermFootnote_term
    ,TermFootnote
    ,WaiverFootnote_term
    ,WaiverFootnote
    ,AccelBftRiderFootnote_term
    ,AccelBftRiderFootnote
    ,OverloanRiderFootnote_term
    ,OverloanRiderFootnote

    // Group quotes.
    ,GroupQuoteShortProductName_term
    ,GroupQuoteShortProductName
    ,GroupQuoteIsNotAnOffer_term
    ,GroupQuoteIsNotAnOffer
    ,GroupQuoteRidersFooter_term
    ,GroupQuoteRidersFooter
    ,GroupQuotePolicyFormId_term
    ,GroupQuotePolicyFormId
    ,GroupQuoteStateVariations_term
    ,GroupQuoteStateVariations
    ,GroupQuoteProspectus_term
    ,GroupQuoteProspectus
    ,GroupQuoteUnderwriter_term
    ,GroupQuoteUnderwriter
    ,GroupQuoteBrokerDealer_term
    ,GroupQuoteBrokerDealer
    // Group plan type is one of:
    //   -Mandatory: no individual selection of amounts; typically,
    //     the employer pays the entire premium
    //   -Voluntary: individual selection of amounts; typically, the
    //      employee pays the premium; may be called "supplemental"
    //      when it complements a (separate) "mandatory" plan
    //   -Fusion: mandatory and supplemental combined; typically, the
    //      employer and employee pay their respective premiums
    ,GroupQuoteRubricMandatory_term
    ,GroupQuoteRubricMandatory
    ,GroupQuoteRubricVoluntary_term
    ,GroupQuoteRubricVoluntary
    ,GroupQuoteRubricFusion_term
    ,GroupQuoteRubricFusion
    ,GroupQuoteFooterMandatory_term
    ,GroupQuoteFooterMandatory
    ,GroupQuoteFooterVoluntary_term
    ,GroupQuoteFooterVoluntary
    ,GroupQuoteFooterFusion_term
    ,GroupQuoteFooterFusion

    // Premium-specific footnotes.
    ,MinimumPremiumFootnote_term
    ,MinimumPremiumFootnote
    ,PremAllocationFootnote_term
    ,PremAllocationFootnote

    // Miscellaneous other footnotes.
    ,InterestDisclaimer_term
    ,InterestDisclaimer
    ,GuarMortalityFootnote_term
    ,GuarMortalityFootnote
    ,ProductDescription_term
    ,ProductDescription
    ,StableValueFootnote_term
    ,StableValueFootnote
    ,NoVanishPremiumFootnote_term
    ,NoVanishPremiumFootnote
    ,RejectPremiumFootnote_term
    ,RejectPremiumFootnote
    ,ExpRatingFootnote_term
    ,ExpRatingFootnote
    ,MortalityBlendFootnote_term
    ,MortalityBlendFootnote
    ,HypotheticalRatesFootnote_term
    ,HypotheticalRatesFootnote
    ,SalesLoadRefundFootnote_term
    ,SalesLoadRefundFootnote
    ,NoLapseEverFootnote_term
    ,NoLapseEverFootnote
    ,NoLapseFootnote_term
    ,NoLapseFootnote
    ,CurrentValuesFootnote_term
    ,CurrentValuesFootnote
    ,DBOption1Footnote_term
    ,DBOption1Footnote
    ,DBOption2Footnote_term
    ,DBOption2Footnote
    ,DBOption3Footnote_term
    ,DBOption3Footnote
    ,MinDeathBenefitFootnote_term
    ,MinDeathBenefitFootnote
    ,ExpRatRiskChargeFootnote_term
    ,ExpRatRiskChargeFootnote
    ,ExchangeChargeFootnote1_term
    ,ExchangeChargeFootnote1
    ,FlexiblePremiumFootnote_term
    ,FlexiblePremiumFootnote
    ,GuaranteedValuesFootnote_term
    ,GuaranteedValuesFootnote
    ,CreditingRateFootnote_term
    ,CreditingRateFootnote
    ,GrossRateFootnote_term
    ,GrossRateFootnote
    ,NetRateFootnote_term
    ,NetRateFootnote
    ,MecFootnote_term
    ,MecFootnote
    ,GptFootnote_term
    ,GptFootnote
    ,MidpointValuesFootnote_term
    ,MidpointValuesFootnote
    ,SinglePremiumFootnote_term
    ,SinglePremiumFootnote
    ,MonthlyChargesFootnote_term
    ,MonthlyChargesFootnote
    ,UltCreditingRateFootnote_term
    ,UltCreditingRateFootnote
    ,UltCreditingRateHeader_term
    ,UltCreditingRateHeader
    ,MaxNaarFootnote_term
    ,MaxNaarFootnote
    ,PremTaxSurrChgFootnote_term
    ,PremTaxSurrChgFootnote
    ,PolicyFeeFootnote_term
    ,PolicyFeeFootnote
    ,AssetChargeFootnote_term
    ,AssetChargeFootnote
    ,InvestmentIncomeFootnote_term
    ,InvestmentIncomeFootnote
    ,IrrDbFootnote_term
    ,IrrDbFootnote
    ,IrrCsvFootnote_term
    ,IrrCsvFootnote
    ,MortalityChargesFootnote_term
    ,MortalityChargesFootnote
    ,LoanAndWithdrawalFootnote_term
    ,LoanAndWithdrawalFootnote
    ,LoanFootnote_term
    ,LoanFootnote
    ,ImprimaturPresale_term
    ,ImprimaturPresale
    ,ImprimaturPresaleComposite_term
    ,ImprimaturPresaleComposite
    ,ImprimaturInforce_term
    ,ImprimaturInforce
    ,ImprimaturInforceComposite_term
    ,ImprimaturInforceComposite
    ,StateMarketingImprimatur_term
    ,StateMarketingImprimatur
    ,NonGuaranteedFootnote_term
    ,NonGuaranteedFootnote
    ,NonGuaranteedFootnote1_term
    ,NonGuaranteedFootnote1
    ,NonGuaranteedFootnote1Tx_term
    ,NonGuaranteedFootnote1Tx
    ,FnMonthlyDeductions_term
    ,FnMonthlyDeductions
    ,SurrenderFootnote_term
    ,SurrenderFootnote
    ,PortabilityFootnote_term
    ,PortabilityFootnote
    ,FundRateFootnote_term
    ,FundRateFootnote
    ,IssuingCompanyFootnote_term
    ,IssuingCompanyFootnote
    ,SubsidiaryFootnote_term
    ,SubsidiaryFootnote
    ,PlacementAgentFootnote_term
    ,PlacementAgentFootnote
    ,MarketingNameFootnote_term
    ,MarketingNameFootnote
    ,GuarIssueDisclaimerNcSc_term
    ,GuarIssueDisclaimerNcSc
    ,GuarIssueDisclaimerMd_term
    ,GuarIssueDisclaimerMd
    ,GuarIssueDisclaimerTx_term
    ,GuarIssueDisclaimerTx
    ,IllRegCertAgent_term
    ,IllRegCertAgent
    ,IllRegCertAgentIl_term
    ,IllRegCertAgentIl
    ,IllRegCertAgentTx_term
    ,IllRegCertAgentTx
    ,IllRegCertClient_term
    ,IllRegCertClient
    ,IllRegCertClientIl_term
    ,IllRegCertClientIl
    ,IllRegCertClientTx_term
    ,IllRegCertClientTx
    ,FnMaturityAge_term
    ,FnMaturityAge
    ,FnPartialMortality_term
    ,FnPartialMortality
    ,FnProspectus_term
    ,FnProspectus
    ,FnInitialSpecAmt_term
    ,FnInitialSpecAmt
    ,FnInforceAcctVal_term
    ,FnInforceAcctVal
    ,FnInforceTaxBasis_term
    ,FnInforceTaxBasis
    ,Fn1035Charge_term
    ,Fn1035Charge
    ,FnMecExtraWarning_term
    ,FnMecExtraWarning
    ,FnNotTaxAdvice_term
    ,FnNotTaxAdvice
    ,FnNotTaxAdvice2_term
    ,FnNotTaxAdvice2
    ,FnImf_term
    ,FnImf
    ,FnCensus_term
    ,FnCensus
    ,FnDacTax_term
    ,FnDacTax
    ,FnDefnLifeIns_term
    ,FnDefnLifeIns
    ,FnBoyEoy_term
    ,FnBoyEoy
    ,FnGeneralAccount_term
    ,FnGeneralAccount
    ,FnPpMemorandum_term
    ,FnPpMemorandum
    ,FnPpAccreditedInvestor_term
    ,FnPpAccreditedInvestor
    ,FnPpLoads_term
    ,FnPpLoads
    ,FnProposalUnderwriting_term
    ,FnProposalUnderwriting
    ,FnGuaranteedPremium_term
    ,FnGuaranteedPremium
    ,FnOmnibusDisclaimer_term
    ,FnOmnibusDisclaimer
    ,FnInitialDbo_term
    ,FnInitialDbo
    ,DefnGuarGenAcctRate_term
    ,DefnGuarGenAcctRate
    ,DefnAV_term
    ,DefnAV
    ,DefnCSV_term
    ,DefnCSV
    ,DefnMec_term
    ,DefnMec
    ,DefnOutlay_term
    ,DefnOutlay
    ,DefnSpecAmt_term
    ,DefnSpecAmt

    // Esoterica.
    ,Poe0
    ,Poe1
    ,Poe2
    ,Poe3
    };
} // namespace superior

#endif // sample_hpp
