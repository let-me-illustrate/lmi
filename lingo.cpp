// Text to be dropped into report templates.
//
// Copyright (C) 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "lingo.hpp"

#include "alert.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "map_lookup.hpp"
#include "my_proem.hpp"                 // ::write_proem()
#include "sample.hpp"                   // superior::lingo
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

/// Construct from filename.

lingo::lingo(fs::path const& filename)
{
    xml_lmi::dom_parser parser(filename.string());
    xml::element const& root = parser.root_node(xml_root_name());
    int file_version = 0;
    if(!xml_lmi::get_attr(root, "version", file_version))
        {
        alarum()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    xml_serialize::from_xml(root, map_);
}

std::string const& lingo::lookup(int index) const
{
    return map_lookup(map_, index);
}

namespace
{
static std::string const S_FnMonthlyDeductions =
  "Monthly charges are deducted from the account value; if it is depleted,"
  " additional premiums may be required.";

// These two certifications are copied verbatim et literatim from the
// illustration reg.
static std::string const S_IllRegCertAgent =
  "I certify that this illustration has been presented to the applicant and"
  " that I have explained that any non-guaranteed elements illustrated are"
  " subject to change. I have made no statements that are inconsistent with"
  " the illustration.";
static std::string const S_IllRegCertClient =
  "I have received a copy of this illustration and understand that any"
  " non-guaranteed elements illustrated are subject to change and could be"
  " either higher or lower. The agent has told me they are not guaranteed.";

static std::string const S_FnMaturityAge =
  "¶¶Maturity age: {{EndtAge}}.";

static std::string const S_FnPartialMortality =
  "¶¶Columns reflect mortality, beginning at {{PartMortTableMult[0]}}"
  " of the {{PartMortTableName}} table,"
  " with all deaths at the end of each year"
  "{{#SurviveToExpectancy}}"
  " and survival limited to life expectancy"
  "{{/SurviveToExpectancy}}"
  "{{#SurviveToYear}}"
  " and survival limited to {{SurvivalMaxYear}} years"
  "{{/SurviveToYear}}"
  "{{#SurviveToAge}}"
  " and survival limited to age {{SurvivalMaxAge}}"
  "{{/SurviveToAge}}"
  ".";

static std::string const S_FnProspectus =
  "Must be preceded or accompanied by a prospectus.";
static std::string const S_FnInitialSpecAmt =
  "The initial specified amount is ${{InitTotalSA}}.";
static std::string const S_FnInforceAcctVal =
  "The inforce account value is ${{InforceTotalAV}}.";
static std::string const S_FnInforceTaxBasis =
  "The inforce tax basis is ${{InforceTaxBasis}}.";
static std::string const S_Fn1035Charge =
  "A charge may be deducted from the proceeds of a 1035 exchange.";
static std::string const S_FnMecExtraWarning =
  "{{#IsMec}}¶¶This is a Modified Endowment Contract.{{/IsMec}}";
static std::string const S_FnNotTaxAdvice =
  "{{InsCoShortName}} cannot give tax advice. Consult your own advisors.";
static std::string const S_FnImf =
  "Initial investment management fee: {{TotalIMF[0]}}.";
static std::string const S_FnCensus =
  ""; // There is no census attached to a composite.
static std::string const S_FnDacTax =
  "There is no explicit charge for DAC tax.";

static std::string const S_FnDefnLifeIns =
  "This policy is intended to qualify as life insurance under the IRC §7702"
  "{{#DefnLifeInsIsGPT}}"
  " guideline premium test. ${{InitGSP}} is the guideline single premium,"
  " and ${{InitGLP}} is the guideline level premium."
  "{{/DefnLifeInsIsGPT}}"
  "{{^DefnLifeInsIsGPT}}"
  "cash value accumulation test."
  "{{/DefnLifeInsIsGPT}}";

static std::string const S_FnBoyEoy =
  "Premiums are payable in advance. Benefits are as of year end.";
static std::string const S_FnGeneralAccount =
  "The general account credits interest of at least {{InitAnnGenAcctInt_Guaranteed}}.";
static std::string const S_FnPpMemorandum =
  "Must be preceded or accompanied by a prospectus.";
static std::string const S_FnPpAccreditedInvestor =
  "Available only to accredited investors.";
static std::string const S_FnPpLoads =
  ""; // Explanation of any special loads.
static std::string const S_FnProposalUnderwriting =
  ""; // Explanation of group underwriting.
static std::string const S_FnGuaranteedPremium =
  "An outlay of ${{GuarPrem}} ({{InitEeMode}}) will guarantee coverage"
  " to age {{EndtAge}}"
  "{{#DefnLifeInsIsGPT}}"
  ", subject to guideline premium test limits"
  "{{/DefnLifeInsIsGPT}}"
  ".";
static std::string const S_FnOmnibusDisclaimer =
  "Non-guaranteed values are based on current assumptions, which are"
  " subject to change. Actual results may be more or less favorable.";
static std::string const S_FnInitialDbo =
  "The initial death benefit option is {{InitDBOpt}}.";
static std::string const S_DefnGuarGenAcctRate =
  "¶¶«Guaranteed Crediting Rate:»"
  " The minimum annual interest rate credited on unloaned funds."
  ;
static std::string const S_DefnAV =
  "Account value is the accumulation of payments less charges and disbursements.";
static std::string const S_DefnCSV =
  "Cash surrender value is account value less any surrender charge.";
static std::string const S_DefnMec =
  "A Modified Endowment Contract is a contract that does not qualify"
  " for favorable tax treatment under IRC §7702A.";
static std::string const S_DefnOutlay =
  "Outlay is premium paid out of pocket.";
static std::string const S_DefnSpecAmt =
  "Specified amount is the nominal face amount.";
static std::string const S_Poe0 =
  "The \"Red Death\" had long devastated the country. No pestilence"
  " had ever been so fatal, or so hideous. Blood was its Avatar and"
  " its seal--the redness and the horror of blood. There were sharp"
  " pains, and sudden dizziness, and then profuse bleeding at the"
  " pores, with dissolution. The scarlet stains upon the body and"
  " especially upon the face of the victim, were the pest ban which"
  " shut him out from the aid and from the sympathy of his fellow-men."
  " And the whole seizure, progress and termination of the disease,"
  " were the incidents of half an hour.";
static std::string const S_Poe1 =
  "But the Prince Prospero was happy and dauntless and sagacious. When"
  " his dominions were half depopulated, he summoned to his presence a"
  " thousand hale and light-hearted friends from among the knights and"
  " dames of his court, and with these retired to the deep seclusion"
  " of one of his castellated abbeys. This was an extensive and"
  " magnificent structure, the creation of the prince's own eccentric"
  " yet august taste. A strong and lofty wall girdled it in. This wall"
  " had gates of iron. The courtiers, having entered, brought furnaces"
  " and massy hammers and welded the bolts. They resolved to leave"
  " means neither of ingress nor egress to the sudden impulses of"
  " despair or of frenzy from within. The abbey was amply provisioned."
  " With such precautions the courtiers might bid defiance to contagion."
  " The external world could take care of itself. In the meantime it"
  " was folly to grieve, or to think. The prince had provided all the"
  " appliances of pleasure. There were buffoons, there were"
  " improvisatori, there were ballet-dancers, there were musicians,"
  " there was Beauty, there was wine. All these and security were"
  " within. Without was the \"Red Death\".";
static std::string const S_Poe2 =
  "It was towards the close of the fifth or sixth month of his"
  " seclusion, and while the pestilence raged most furiously abroad,"
  " that the Prince Prospero entertained his thousand friends at a"
  " masked ball of the most unusual magnificence.";
static std::string const S_Poe3 =
  "It was a voluptuous scene, that masquerade. But first let me tell"
  " of the rooms in which it was held. These were seven--an imperial"
  " suite. In many palaces, however, such suites form a long and"
  " straight vista, while the folding doors slide back nearly to the"
  " walls on either hand, so that the view of the whole extent is"
  " scarcely impeded. Here the case was very different, as might have"
  " been expected from the duke's love of the _bizarre_. The apartments"
  " were so irregularly disposed that the vision embraced but little"
  " more than one at a time. There was a sharp turn at every twenty or"
  " thirty yards, and at each turn a novel effect. To the right and"
  " left, in the middle of each wall, a tall and narrow Gothic window"
  " looked out upon a closed corridor which pursued the windings of the"
  " suite. These windows were of stained glass whose color varied in"
  " accordance with the prevailing hue of the decorations of the"
  " chamber into which it opened. That at the eastern extremity was"
  " hung, for example in blue--and vividly blue were its windows. The"
  " second chamber was purple in its ornaments and tapestries, and here"
  " the panes were purple. The third was green throughout, and so were"
  " the casements. The fourth was furnished and lighted with orange--the"
  " fifth with white--the sixth with violet. The seventh apartment was"
  " closely shrouded in black velvet tapestries that hung all over the"
  " ceiling and down the walls, falling in heavy folds upon a carpet of"
  " the same material and hue. But in this chamber only, the color of"
  " the windows failed to correspond with the decorations. The panes"
  " here were scarlet--a deep blood color. Now in no one of the seven"
  " apartments was there any lamp or candelabrum, amid the profusion of"
  " golden ornaments that lay scattered to and fro or depended from the"
  " roof. There was no light of any kind emanating from lamp or candle"
  " within the suite of chambers. But in the corridors that followed the"
  " suite, there stood, opposite to each window, a heavy tripod, bearing"
  " a brazier of fire, that projected its rays through the tinted glass"
  " and so glaringly illumined the room. And thus were produced a"
  " multitude of gaudy and fantastic appearances. But in the western or"
  " black chamber the effect of the fire-light that streamed upon the"
  " dark hangings through the blood-tinted panes, was ghastly in the"
  " extreme, and produced so wild a look upon the countenances of those"
  " who entered, that there were few of the company bold enough to set"
  " foot within its precincts at all.";
} // Unnamed namespace.

// 'sample2*' products generally use this pattern:
//   ,{superior::TelephoneNumber_term, "{TelephoneNumber}"}
// so that an MST template containing
//   "Call {{TelephoneNumber}} for service."
// is rendered to PDF as
//   "Call {TelephoneNumber} for service."
// which is useful for testing.
//
// LINGO !! reconsider this...
// It would probably make more sense to write a separate map (perhaps
// with separate enumerators) for 'sample' and 'sample2', instead of
// combining them (as here) and distinguishing the 'sample2' elements
// with a "_term" suffix. For the nonce, combining them into one large
// '.lingo' file makes the overhead (and the need for caching) more
// obvious: suppressing the '#if 1' block materially affects speed.

void lingo::write_lingo_files()
{
    // superior::lingo enumerators are used for clarity in specifying
    // this map. They decay to integers in the resulting file, which
    // can therefore be read without the enumerators being visible.
    //
    // For clarity of presentation, every enumerator-string pair is
    // explicitly initialized here, even if the string is empty; but
    // that is not necessary, because superior::empty_string is used
    // for database entities with the default value of zero.
    //
    static std::unordered_map<superior::lingo,std::string> const enumerative_map
        {{superior::empty_string                        , ""}
        // Essential strings describing the policy and company.
        ,{superior::policy_form_term                    , "{PolicyForm}"}
        ,{superior::policy_form                         , "UL32768-NY"}
        ,{superior::policy_form_KS_KY                   , "UL32768-X"}
#if 1
        // LINGO !! reconsider this...
        // These MixedCase terms seem better--it's easier to grep for a
        // string that has no lower_case_with_underscores variant:
//      ,{superior::PolicyForm_term                     , "{PolicyForm}"}
//      ,{superior::PolicyForm                          , "UL32768-NY"}
        ,{superior::PolicyMktgName_term                 , "{PolicyMktgName}"}
        ,{superior::PolicyMktgName                      , "UL Supreme"}
        ,{superior::PolicyLegalName_term                , "{PolicyLegalName}"}
        ,{superior::PolicyLegalName                     , "Flexible Premium Adjustable Life Insurance Policy"}
        ,{superior::InsCoShortName_term                 , "{InsCoShortName}"}
        ,{superior::InsCoShortName                      , "Superior Life"}
        ,{superior::InsCoName_term                      , "{InsCoName}"}
        ,{superior::InsCoName                           , "Superior Life Insurance Company"}
        ,{superior::InsCoAddr_term                      , "{InsCoAddr}"}
        ,{superior::InsCoAddr                           , "Superior, WI 12345"}
        ,{superior::InsCoStreet_term                    , "{InsCoStreet}"}
        ,{superior::InsCoStreet                         , "246 Main Street"}
        ,{superior::InsCoPhone_term                     , "{InsCoPhone}"}
        ,{superior::InsCoPhone                          , "(800) 555-1212"}
        ,{superior::MainUnderwriter_term                , "{MainUnderwriter}"}
        ,{superior::MainUnderwriter                     , "Superior Securities"}
        ,{superior::MainUnderwriterAddress_term         , "{MainUnderwriterAddress}"}
        ,{superior::MainUnderwriterAddress              , "246-M Main Street, Superior, WI 12345"}
        ,{superior::CoUnderwriter_term                  , "{CoUnderwriter}"}
        ,{superior::CoUnderwriter                       , "Superior Investors"}
        ,{superior::CoUnderwriterAddress_term           , "{CoUnderwriterAddress}"}
        ,{superior::CoUnderwriterAddress                , "246-C Main Street, Superior, WI 12345"}

        // Terms defined in the contract, which must be used for column
        // headers, footnotes, etc. according to the illustration reg.
        ,{superior::AvName_term                         , "{AvName}"}
        ,{superior::AvName                              , "Account"}
        ,{superior::CsvName_term                        , "{CsvName}"}
        ,{superior::CsvName                             , "Cash Surrender"}
        ,{superior::CsvHeaderName_term                  , "{CsvHeaderName}"}
        ,{superior::CsvHeaderName                       , "Cash Surr"}
        ,{superior::NoLapseProvisionName_term           , "{NoLapseProvisionName}"}
        ,{superior::NoLapseProvisionName                , "No-lapse Provision"}
        ,{superior::ContractName_term                   , "{ContractName}"}
        ,{superior::ContractName                        , "contract"} // Alternatively, "policy" or "certificate".
        ,{superior::DboName_term                        , "{DboName}"}
        ,{superior::DboName                             , "Death Benefit Option"}
        ,{superior::DboNameLevel_term                   , "{DboNameLevel}"}
        ,{superior::DboNameLevel                        , "A"}
        ,{superior::DboNameIncreasing_term              , "{DboNameIncreasing}"}
        ,{superior::DboNameIncreasing                   , "B"}
        ,{superior::DboNameReturnOfPremium_term         , "{DboNameReturnOfPremium}"}
        ,{superior::DboNameReturnOfPremium              , "ROP"}
        ,{superior::DboNameMinDeathBenefit_term         , "{DboNameMinDeathBenefit}"}
        ,{superior::DboNameMinDeathBenefit              , "MDB"}
        ,{superior::GenAcctName_term                    , "{GenAcctName}"}
        ,{superior::GenAcctName                         , "General Account"}
        ,{superior::GenAcctNameElaborated_term          , "{GenAcctNameElaborated}"}
        ,{superior::GenAcctNameElaborated               , "General Account (GA)"}
        ,{superior::SepAcctName_term                    , "{SepAcctName}"}
        ,{superior::SepAcctName                         , "Separate Account"}
        ,{superior::SpecAmtName_term                    , "{SpecAmtName}"}
        ,{superior::SpecAmtName                         , "Specified Amount"}
        ,{superior::SpecAmtNameElaborated_term          , "{SpecAmtNameElaborated}"}
        ,{superior::SpecAmtNameElaborated               , "Specified (Face) Amount"}

        // Underwriting terms.
        ,{superior::UwBasisMedical_term                 , "{UwBasisMedical}"}
        ,{superior::UwBasisMedical                      , "Medical"}
        ,{superior::UwBasisParamedical_term             , "{UwBasisParamedical}"}
        ,{superior::UwBasisParamedical                  , "Paramedical"}
        ,{superior::UwBasisNonmedical_term              , "{UwBasisNonmedical}"}
        ,{superior::UwBasisNonmedical                   , "Nonmedical"}
        ,{superior::UwBasisSimplified_term              , "{UwBasisSimplified}"}
        ,{superior::UwBasisSimplified                   , "Simplified Issue"}
        ,{superior::UwBasisGuaranteed_term              , "{UwBasisGuaranteed}"}
        ,{superior::UwBasisGuaranteed                   , "Guaranteed Issue"}
        ,{superior::UwClassPreferred_term               , "{UwClassPreferred}"}
        ,{superior::UwClassPreferred                    , "Preferred"}
        ,{superior::UwClassStandard_term                , "{UwClassStandard}"}
        ,{superior::UwClassStandard                     , "Standard"}
        ,{superior::UwClassRated_term                   , "{UwClassRated}"}
        ,{superior::UwClassRated                        , "Rated"}
        ,{superior::UwClassUltra_term                   , "{UwClassUltra}"}
        ,{superior::UwClassUltra                        , "Ultrapreferred"}

        // Ledger column definitions.
        ,{superior::AccountValueFootnote_term           , "{AccountValueFootnote}"}
        ,{superior::AccountValueFootnote                , ""}
        ,{superior::AttainedAgeFootnote_term            , "{AttainedAgeFootnote}"}
        ,{superior::AttainedAgeFootnote                 , ""}
        ,{superior::CashSurrValueFootnote_term          , "{CashSurrValueFootnote}"}
        ,{superior::CashSurrValueFootnote               , ""}
        ,{superior::DeathBenefitFootnote_term           , "{DeathBenefitFootnote}"}
        ,{superior::DeathBenefitFootnote                , ""}
        ,{superior::InitialPremiumFootnote_term         , "{InitialPremiumFootnote}"}
        ,{superior::InitialPremiumFootnote              , ""}
        ,{superior::NetPremiumFootnote_term             , "{NetPremiumFootnote}"}
        ,{superior::NetPremiumFootnote                  , ""}
        ,{superior::GrossPremiumFootnote_term           , "{GrossPremiumFootnote}"}
        ,{superior::GrossPremiumFootnote                , ""}
        ,{superior::OutlayFootnote_term                 , "{OutlayFootnote}"}
        ,{superior::OutlayFootnote                      , ""}
        ,{superior::PolicyYearFootnote_term             , "{PolicyYearFootnote}"}
        ,{superior::PolicyYearFootnote                  , ""}

        // Terse rider names.
        ,{superior::ADDTerseName_term                   , "{ADDTerseName}"}
        ,{superior::ADDTerseName                        , "Accident"}
        ,{superior::InsurabilityTerseName_term          , "{InsurabilityTerseName}"}
        ,{superior::InsurabilityTerseName               , "Insurability"}
        ,{superior::ChildTerseName_term                 , "{ChildTerseName}"}
        ,{superior::ChildTerseName                      , "Child"}
        ,{superior::SpouseTerseName_term                , "{SpouseTerseName}"}
        ,{superior::SpouseTerseName                     , "Spouse"}
        ,{superior::TermTerseName_term                  , "{TermTerseName}"}
        ,{superior::TermTerseName                       , "Term"}
        ,{superior::WaiverTerseName_term                , "{WaiverTerseName}"}
        ,{superior::WaiverTerseName                     , "Waiver"}
        ,{superior::AccelBftRiderTerseName_term         , "{AccelBftRiderTerseName}"}
        ,{superior::AccelBftRiderTerseName              , "Acceleration"}
        ,{superior::OverloanRiderTerseName_term         , "{OverloanRiderTerseName}"}
        ,{superior::OverloanRiderTerseName              , "Overloan"}

        // Rider footnotes.
        ,{superior::ADDFootnote_term                    , "{ADDFootnote}"}
        ,{superior::ADDFootnote                         , ""}
        ,{superior::ChildFootnote_term                  , "{ChildFootnote}"}
        ,{superior::ChildFootnote                       , ""}
        ,{superior::SpouseFootnote_term                 , "{SpouseFootnote}"}
        ,{superior::SpouseFootnote                      , ""}
        ,{superior::TermFootnote_term                   , "{TermFootnote}"}
        ,{superior::TermFootnote                        , ""}
        ,{superior::WaiverFootnote_term                 , "{WaiverFootnote}"}
        ,{superior::WaiverFootnote                      , ""}
        ,{superior::AccelBftRiderFootnote_term          , "{AccelBftRiderFootnote}"}
        ,{superior::AccelBftRiderFootnote               , ""}
        ,{superior::OverloanRiderFootnote_term          , "{OverloanRiderFootnote}"}
        ,{superior::OverloanRiderFootnote               , ""}

        // Group quotes.
        ,{superior::GroupQuoteShortProductName_term     , "{GroupQuoteShortProductName}"}
        ,{superior::GroupQuoteShortProductName          , "UL SUPREME®"}
        ,{superior::GroupQuoteIsNotAnOffer_term         , "{GroupQuoteIsNotAnOffer}"}
        ,{superior::GroupQuoteIsNotAnOffer              , "This is not an offer of insurance."}
        ,{superior::GroupQuoteRidersFooter_term         , "{GroupQuoteRidersFooter}"}
        ,{superior::GroupQuoteRidersFooter              , "Available riders: accident and waiver."}
        ,{superior::GroupQuotePolicyFormId_term         , "{GroupQuotePolicyFormId}"}
        ,{superior::GroupQuotePolicyFormId              , "Policy form UL32768-NY is a flexible premium contract."}
        ,{superior::GroupQuoteStateVariations_term      , "{GroupQuoteStateVariations}"}
        ,{superior::GroupQuoteStateVariations           , "Not available in all states."}
        ,{superior::GroupQuoteProspectus_term           , "{GroupQuoteProspectus}"}
        ,{superior::GroupQuoteProspectus                , "Read the prospectus carefully."}
        ,{superior::GroupQuoteUnderwriter_term          , "{GroupQuoteUnderwriter}"}
        ,{superior::GroupQuoteUnderwriter               , "Securities underwritten by Superior Securities."}
        ,{superior::GroupQuoteBrokerDealer_term         , "{GroupQuoteBrokerDealer}"}
        ,{superior::GroupQuoteBrokerDealer              , "Securities offered through Superior Brokerage."}
        // Group plan type is one of:
        //   -Mandatory: no individual selection of amounts; typically,
        //     the employer pays the entire premium
        //   -Voluntary: individual selection of amounts; typically, the
        //      employee pays the premium; may be called "supplemental"
        //      when it complements a (separate) "mandatory" plan
        //   -Fusion: mandatory and supplemental combined; typically, the
        //      employer and employee pay their respective premiums
        ,{superior::GroupQuoteRubricMandatory_term      , "{GroupQuoteRubricMandatory}"}
        ,{superior::GroupQuoteRubricMandatory           , "Mandatory"}
        ,{superior::GroupQuoteRubricVoluntary_term      , "{GroupQuoteRubricVoluntary}"}
        ,{superior::GroupQuoteRubricVoluntary           , "Voluntary"}
        ,{superior::GroupQuoteRubricFusion_term         , "{GroupQuoteRubricFusion}"}
        ,{superior::GroupQuoteRubricFusion              , "Fusion"}
        ,{superior::GroupQuoteFooterMandatory_term      , "{GroupQuoteFooterMandatory}"}
        ,{superior::GroupQuoteFooterMandatory           , "The employer pays all premiums."}
        ,{superior::GroupQuoteFooterVoluntary_term      , "{GroupQuoteFooterVoluntary}"}
        ,{superior::GroupQuoteFooterVoluntary           , "The employee pays all premiums."}
        ,{superior::GroupQuoteFooterFusion_term         , "{GroupQuoteFooterFusion}"}
        ,{superior::GroupQuoteFooterFusion              , "The employer and employee pay their respective premiums."}

        // Premium-specific footnotes.
        ,{superior::MinimumPremiumFootnote_term         , "{MinimumPremiumFootnote}"}
        ,{superior::MinimumPremiumFootnote              , ""}
        ,{superior::PremAllocationFootnote_term         , "{PremAllocationFootnote}"}
        ,{superior::PremAllocationFootnote              , ""}

        // Miscellaneous other footnotes.
        ,{superior::InterestDisclaimer_term             , "{InterestDisclaimer}"}
        ,{superior::InterestDisclaimer                  , ""}
        ,{superior::GuarMortalityFootnote_term          , "{GuarMortalityFootnote}"}
        ,{superior::GuarMortalityFootnote               , "Guaranteed mortality basis: {{CsoEra}} CSO."}
        ,{superior::ProductDescription_term             , "{ProductDescription}"}
        ,{superior::ProductDescription                  , ""}
        ,{superior::StableValueFootnote_term            , "{StableValueFootnote}"}
        ,{superior::StableValueFootnote                 , ""}
        ,{superior::NoVanishPremiumFootnote_term        , "{NoVanishPremiumFootnote}"}
        ,{superior::NoVanishPremiumFootnote             , ""}
        ,{superior::RejectPremiumFootnote_term          , "{RejectPremiumFootnote}"}
        ,{superior::RejectPremiumFootnote               , ""}
        ,{superior::ExpRatingFootnote_term              , "{ExpRatingFootnote}"}
        ,{superior::ExpRatingFootnote                   , ""}
        ,{superior::MortalityBlendFootnote_term         , "{MortalityBlendFootnote}"}
        ,{superior::MortalityBlendFootnote              , ""}
        ,{superior::HypotheticalRatesFootnote_term      , "{HypotheticalRatesFootnote}"}
        ,{superior::HypotheticalRatesFootnote           , ""}
        ,{superior::SalesLoadRefundFootnote_term        , "{SalesLoadRefundFootnote}"}
        ,{superior::SalesLoadRefundFootnote             , ""}
        ,{superior::NoLapseEverFootnote_term            , "{NoLapseEverFootnote}"}
        ,{superior::NoLapseEverFootnote                 , ""}
        ,{superior::NoLapseFootnote_term                , "{NoLapseFootnote}"}
        ,{superior::NoLapseFootnote                     , ""}
        ,{superior::CurrentValuesFootnote_term          , "{CurrentValuesFootnote}"}
        ,{superior::CurrentValuesFootnote               , ""}
        ,{superior::DBOption1Footnote_term              , "{DBOption1Footnote}"}
        ,{superior::DBOption1Footnote                   , ""}
        ,{superior::DBOption2Footnote_term              , "{DBOption2Footnote}"}
        ,{superior::DBOption2Footnote                   , ""}
        ,{superior::DBOption3Footnote_term              , "{DBOption3Footnote}"}
        ,{superior::DBOption3Footnote                   , ""}
        ,{superior::MinDeathBenefitFootnote_term        , "{MinDeathBenefitFootnote}"}
        ,{superior::MinDeathBenefitFootnote             , ""}
        ,{superior::ExpRatRiskChargeFootnote_term       , "{ExpRatRiskChargeFootnote}"}
        ,{superior::ExpRatRiskChargeFootnote            , ""}
        ,{superior::ExchangeChargeFootnote1_term        , "{ExchangeChargeFootnote1}"}
        ,{superior::ExchangeChargeFootnote1             , ""}
        ,{superior::FlexiblePremiumFootnote_term        , "{FlexiblePremiumFootnote}"}
        ,{superior::FlexiblePremiumFootnote             , ""}
        ,{superior::GuaranteedValuesFootnote_term       , "{GuaranteedValuesFootnote}"}
        ,{superior::GuaranteedValuesFootnote            , ""}
        ,{superior::CreditingRateFootnote_term          , "{CreditingRateFootnote}"}
        ,{superior::CreditingRateFootnote               , ""}
        ,{superior::GrossRateFootnote_term              , "{GrossRateFootnote}"}
        ,{superior::GrossRateFootnote                   , ""}
        ,{superior::NetRateFootnote_term                , "{NetRateFootnote}"}
        ,{superior::NetRateFootnote                     , ""}
        ,{superior::MecFootnote_term                    , "{MecFootnote}"}
        ,{superior::MecFootnote                         , ""}
        ,{superior::GptFootnote_term                    , "{GptFootnote}"}
        ,{superior::GptFootnote                         , ""}
        ,{superior::MidpointValuesFootnote_term         , "{MidpointValuesFootnote}"}
        ,{superior::MidpointValuesFootnote              , ""}
        ,{superior::SinglePremiumFootnote_term          , "{SinglePremiumFootnote}"}
        ,{superior::SinglePremiumFootnote               , ""}
        ,{superior::MonthlyChargesFootnote_term         , "{MonthlyChargesFootnote}"}
        ,{superior::MonthlyChargesFootnote              , ""}
        ,{superior::UltCreditingRateFootnote_term       , "{UltCreditingRateFootnote}"}
        ,{superior::UltCreditingRateFootnote            , ""}
        ,{superior::UltCreditingRateHeader_term         , "{UltCreditingRateHeader}"}
        ,{superior::UltCreditingRateHeader              , ""}
        ,{superior::MaxNaarFootnote_term                , "{MaxNaarFootnote}"}
        ,{superior::MaxNaarFootnote                     , ""}
        ,{superior::PremTaxSurrChgFootnote_term         , "{PremTaxSurrChgFootnote}"}
        ,{superior::PremTaxSurrChgFootnote              , ""}
        ,{superior::PolicyFeeFootnote_term              , "{PolicyFeeFootnote}"}
        ,{superior::PolicyFeeFootnote                   , ""}
        ,{superior::AssetChargeFootnote_term            , "{AssetChargeFootnote}"}
        ,{superior::AssetChargeFootnote                 , ""}
        ,{superior::InvestmentIncomeFootnote_term       , "{InvestmentIncomeFootnote}"}
        ,{superior::InvestmentIncomeFootnote            , ""}
        ,{superior::IrrDbFootnote_term                  , "{IrrDbFootnote}"}
        ,{superior::IrrDbFootnote                       , ""}
        ,{superior::IrrCsvFootnote_term                 , "{IrrCsvFootnote}"}
        ,{superior::IrrCsvFootnote                      , ""}
        ,{superior::MortalityChargesFootnote_term       , "{MortalityChargesFootnote}"}
        ,{superior::MortalityChargesFootnote            , ""}
        ,{superior::LoanAndWithdrawalFootnote_term      , "{LoanAndWithdrawalFootnote}"}
        ,{superior::LoanAndWithdrawalFootnote           , ""}
        ,{superior::LoanFootnote_term                   , "{LoanFootnote}"}
        ,{superior::LoanFootnote                        , ""}
        ,{superior::ImprimaturPresale_term              , "{ImprimaturPresale}"}
        ,{superior::ImprimaturPresale                   , ""}
        ,{superior::ImprimaturPresaleComposite_term     , "{ImprimaturPresaleComposite}"}
        ,{superior::ImprimaturPresaleComposite          , ""}
        ,{superior::ImprimaturInforce_term              , "{ImprimaturInforce}"}
        ,{superior::ImprimaturInforce                   , ""}
        ,{superior::ImprimaturInforceComposite_term     , "{ImprimaturInforceComposite}"}
        ,{superior::ImprimaturInforceComposite          , ""}
        ,{superior::StateMarketingImprimatur_term       , "{StateMarketingImprimatur}"}
        ,{superior::StateMarketingImprimatur            , ""}
        ,{superior::NonGuaranteedFootnote_term          , "{NonGuaranteedFootnote}"}
        ,{superior::NonGuaranteedFootnote               , ""}
        ,{superior::NonGuaranteedFootnote1_term         , "{NonGuaranteedFootnote1}"}
        ,{superior::NonGuaranteedFootnote1              , ""}
        ,{superior::NonGuaranteedFootnote1Tx_term       , "{NonGuaranteedFootnote1Tx}"}
        ,{superior::NonGuaranteedFootnote1Tx            , ""}
        ,{superior::FnMonthlyDeductions_term            , "{FnMonthlyDeductions}"}
        ,{superior::FnMonthlyDeductions                 , S_FnMonthlyDeductions}
        ,{superior::SurrenderFootnote_term              , "{SurrenderFootnote}"}
        ,{superior::SurrenderFootnote                   , ""}
        ,{superior::PortabilityFootnote_term            , "{PortabilityFootnote}"}
        ,{superior::PortabilityFootnote                 , ""}
        ,{superior::FundRateFootnote_term               , "{FundRateFootnote}"}
        ,{superior::FundRateFootnote                    , ""}
        ,{superior::IssuingCompanyFootnote_term         , "{IssuingCompanyFootnote}"}
        ,{superior::IssuingCompanyFootnote              , ""}
        ,{superior::SubsidiaryFootnote_term             , "{SubsidiaryFootnote}"}
        ,{superior::SubsidiaryFootnote                  , ""}
        ,{superior::PlacementAgentFootnote_term         , "{PlacementAgentFootnote}"}
        ,{superior::PlacementAgentFootnote              , ""}
        ,{superior::MarketingNameFootnote_term          , "{MarketingNameFootnote}"}
        ,{superior::MarketingNameFootnote               , "Policy form UL32768-NY is marketed as 'UL Supreme'."}
        ,{superior::GuarIssueDisclaimerNcSc_term        , "{GuarIssueDisclaimerNcSc}"}
        ,{superior::GuarIssueDisclaimerNcSc             , ""}
        ,{superior::GuarIssueDisclaimerMd_term          , "{GuarIssueDisclaimerMd}"}
        ,{superior::GuarIssueDisclaimerMd               , ""}
        ,{superior::GuarIssueDisclaimerTx_term          , "{GuarIssueDisclaimerTx}"}
        ,{superior::GuarIssueDisclaimerTx               , ""}
        ,{superior::IllRegCertAgent_term                , "{IllRegCertAgent}"}
        ,{superior::IllRegCertAgent                     , S_IllRegCertAgent}
        ,{superior::IllRegCertAgentIl_term              , "{IllRegCertAgentIl}"}
        ,{superior::IllRegCertAgentIl                   , S_IllRegCertAgent}
        ,{superior::IllRegCertAgentTx_term              , "{IllRegCertAgentTx}"}
        ,{superior::IllRegCertAgentTx                   , S_IllRegCertAgent}
        ,{superior::IllRegCertClient_term               , "{IllRegCertClient}"}
        ,{superior::IllRegCertClient                    , S_IllRegCertClient}
        ,{superior::IllRegCertClientIl_term             , "{IllRegCertClientIl}"}
        ,{superior::IllRegCertClientIl                  , S_IllRegCertClient}
        ,{superior::IllRegCertClientTx_term             , "{IllRegCertClientTx}"}
        ,{superior::IllRegCertClientTx                  , S_IllRegCertClient}
        ,{superior::FnMaturityAge_term                  , "{FnMaturityAge}"}
        ,{superior::FnMaturityAge                       , S_FnMaturityAge}
        ,{superior::FnPartialMortality_term             , "{FnPartialMortality}"}
        ,{superior::FnPartialMortality                  , S_FnPartialMortality}
        ,{superior::FnProspectus_term                   , "{FnProspectus}"}
        ,{superior::FnProspectus                        , S_FnProspectus}
        ,{superior::FnInitialSpecAmt_term               , "{FnInitialSpecAmt}"}
        ,{superior::FnInitialSpecAmt                    , S_FnInitialSpecAmt}
        ,{superior::FnInforceAcctVal_term               , "{FnInforceAcctVal}"}
        ,{superior::FnInforceAcctVal                    , S_FnInforceAcctVal}
        ,{superior::FnInforceTaxBasis_term              , "{FnInforceTaxBasis}"}
        ,{superior::FnInforceTaxBasis                   , S_FnInforceTaxBasis}
        ,{superior::Fn1035Charge_term                   , "{Fn1035Charge}"}
        ,{superior::Fn1035Charge                        , S_Fn1035Charge}
        ,{superior::FnMecExtraWarning_term              , "{FnMecExtraWarning}"}
        ,{superior::FnMecExtraWarning                   , S_FnMecExtraWarning}
        ,{superior::FnNotTaxAdvice_term                 , "{FnNotTaxAdvice}"}
        ,{superior::FnNotTaxAdvice                      , S_FnNotTaxAdvice}
        ,{superior::FnNotTaxAdvice2_term                , "{FnNotTaxAdvice2}"}
        ,{superior::FnNotTaxAdvice2                     , ""} // Deliberately empty: shouldn't even exist.
        ,{superior::FnImf_term                          , "{FnImf}"}
        ,{superior::FnImf                               , S_FnImf}
        ,{superior::FnCensus_term                       , "{FnCensus}"}
        ,{superior::FnCensus                            , S_FnCensus}
        ,{superior::FnDacTax_term                       , "{FnDacTax}"}
        ,{superior::FnDacTax                            , S_FnDacTax}
        ,{superior::FnDefnLifeIns_term                  , "{FnDefnLifeIns}"}
        ,{superior::FnDefnLifeIns                       , S_FnDefnLifeIns}
        ,{superior::FnBoyEoy_term                       , "{FnBoyEoy}"}
        ,{superior::FnBoyEoy                            , S_FnBoyEoy}
        ,{superior::FnGeneralAccount_term               , "{FnGeneralAccount}"}
        ,{superior::FnGeneralAccount                    , S_FnGeneralAccount}
        ,{superior::FnPpMemorandum_term                 , "{FnPpMemorandum}"}
        ,{superior::FnPpMemorandum                      , S_FnPpMemorandum}
        ,{superior::FnPpAccreditedInvestor_term         , "{FnPpAccreditedInvestor}"}
        ,{superior::FnPpAccreditedInvestor              , S_FnPpAccreditedInvestor}
        ,{superior::FnPpLoads_term                      , "{FnPpLoads}"}
        ,{superior::FnPpLoads                           , S_FnPpLoads}
        ,{superior::FnProposalUnderwriting_term         , "{FnProposalUnderwriting}"}
        ,{superior::FnProposalUnderwriting              , S_FnProposalUnderwriting}
        ,{superior::FnGuaranteedPremium_term            , "{FnGuaranteedPremium}"}
        ,{superior::FnGuaranteedPremium                 , S_FnGuaranteedPremium}
        ,{superior::FnOmnibusDisclaimer_term            , "{FnOmnibusDisclaimer}"}
        ,{superior::FnOmnibusDisclaimer                 , S_FnOmnibusDisclaimer}
        ,{superior::FnInitialDbo_term                   , "{FnInitialDbo}"}
        ,{superior::FnInitialDbo                        , S_FnInitialDbo}
        ,{superior::DefnGuarGenAcctRate_term            , "{DefnGuarGenAcctRate}"}
        ,{superior::DefnGuarGenAcctRate                 , S_DefnGuarGenAcctRate}
        ,{superior::DefnAV_term                         , "{DefnAV}"}
        ,{superior::DefnAV                              , S_DefnAV}
        ,{superior::DefnCSV_term                        , "{DefnCSV}"}
        ,{superior::DefnCSV                             , S_DefnCSV}
        ,{superior::DefnMec_term                        , "{DefnMec}"}
        ,{superior::DefnMec                             , S_DefnMec}
        ,{superior::DefnOutlay_term                     , "{DefnOutlay}"}
        ,{superior::DefnOutlay                          , S_DefnOutlay}
        ,{superior::DefnSpecAmt_term                    , "{DefnSpecAmt}"}
        ,{superior::DefnSpecAmt                         , S_DefnSpecAmt}

        // Esoterica.
        ,{superior::Poe0                                , S_Poe0}
        ,{superior::Poe1                                , S_Poe1}
        ,{superior::Poe2                                , S_Poe2}
        ,{superior::Poe3                                , S_Poe3}
#endif // 1
        };

    fs::path const path(AddDataDir("sample.lingo"));
    xml_lmi::xml_document document(xml_root_name());
    write_proem(document, path.stem().string());
    xml::element& root = document.root_node();
    xml_lmi::set_attr(root, "version", class_version());
    xml_serialize::to_xml(root, enumerative_map);
    document.save(path.string());
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20201109T1600Z

int lingo::class_version()
{
    return 0;
}

std::string const& lingo::xml_root_name()
{
    static std::string const s("lingo");
    return s;
}

void lingo::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_basename
    )
{
    ::write_proem(document, file_basename);
}
