// Life-insurance illustration input.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: input.cpp,v 1.5 2005-05-29 21:32:31 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "alert.hpp"
#include "inputillus.hpp"
#include "timer.hpp"

// TODO ?? Initialize values in types, not initializer-list.
// This probably requires two distinct yes-no types.

Input::Input()
//    :MemberSymbolTable<Input>() // TODO ?? Is this desirable?
    :CreateSupplementalReport("No")
{
    ascribe_members();
}

Input::~Input()
{
}

Input::Input(Input const& z)
    :MemberSymbolTable<Input>()
{
    ascribe_members();
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        // This would be wrong:
        //   operator[](*i) = z[*i];
        // because it would swap in a copy of z's *members*.
        //
        // TODO ?? Would we be better off without the operator=() that
        // does that? Using str() here, passim, seems distateful.
        operator[](*i) = z[*i].str();
        }
}

Input& Input::operator=(Input const& z)
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = z[*i].str();
        }
    return *this;
}

// TODO ?? Can this be put into class MemberSymbolTable?
bool Input::operator==(Input const& z) const
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
// TODO ?? Provide operator!=(). Done yet?
//        if(operator[](*i) != z[*i])
// TODO ?? Wait--this wouldn't work, at least not yet...
// ...it tests *identity*, not *equivalence*.
//        if(!(operator[](*i) == z[*i]))
//
// TODO ?? Then why doesn't even this work?
//        if(!(operator[](*i).str() == z[*i].str()))
        std::string const s0 = operator[](*i).str();
        std::string const s1 = z[*i].str();
        if(s0 != s1)
            {
            return false;
            }
        }
    return true;
}

std::string Input::differing_fields(Input const& z) const
{
    std::ostringstream oss;
    oss << "Fields that differ:\n";
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const s0 = operator[](*i).str();
        std::string const s1 = z[*i].str();
        if(s0 != s1)
            {
            oss
                << "  name " << *i
                << ": '" << s0
                << "[" << s0.size() << "]"
                << "' versus '" << s1
                << "[" << s1.size() << "]"
                << "'\n"
                ;
            }
        }
    return oss.str();
}

// TODO ?? Decide what to do about commented-out members.
void Input::ascribe_members()
{
    ascribe("IssueAge"                              , &Input::IssueAge                              );
    ascribe("RetirementAge"                         , &Input::RetirementAge                         );
    ascribe("Gender"                                , &Input::Gender                                );
    ascribe("Smoking"                               , &Input::Smoking                               );
    ascribe("UnderwritingClass"                     , &Input::UnderwritingClass                     );
    ascribe("WaiverOfPremiumBenefit"                , &Input::WaiverOfPremiumBenefit                );
    ascribe("AccidentalDeathBenefit"                , &Input::AccidentalDeathBenefit                );
    ascribe("TermRider"                             , &Input::TermRider                             );
    ascribe("TermRiderAmount"                       , &Input::TermRiderAmount                       );
    ascribe("TotalSpecifiedAmount"                  , &Input::TotalSpecifiedAmount                  );
    ascribe("TermRiderProportion"                   , &Input::TermRiderProportion                   );
    ascribe("TermRiderUseProportion"                , &Input::TermRiderUseProportion                );
    ascribe("DateOfBirth"                           , &Input::DateOfBirth                           );
    ascribe("DateOfRetirement"                      , &Input::DateOfRetirement                      );
    ascribe("SubstandardTable"                      , &Input::SubstandardTable                      );
    ascribe("ProductName"                           , &Input::ProductName                           ); // TODO ?? Required.
    ascribe("Dumpin"                                , &Input::Dumpin                                );
    ascribe("External1035ExchangeAmount"            , &Input::External1035ExchangeAmount            );
    ascribe("External1035ExchangeBasis"             , &Input::External1035ExchangeBasis             );
    ascribe("External1035ExchangeFromMec"           , &Input::External1035ExchangeFromMec           );
    ascribe("Internal1035ExchangeAmount"            , &Input::Internal1035ExchangeAmount            );
    ascribe("Internal1035ExchangeBasis"             , &Input::Internal1035ExchangeBasis             );
    ascribe("Internal1035ExchangeFromMec"           , &Input::Internal1035ExchangeFromMec           );
    ascribe("SolveTargetTime"                       , &Input::SolveTargetTime                       );
    ascribe("SolveBeginTime"                        , &Input::SolveBeginTime                        );
    ascribe("SolveEndTime"                          , &Input::SolveEndTime                          );
    ascribe("SolveType"                             , &Input::SolveType                             );
    ascribe("SolveBeginYear"                        , &Input::SolveBeginYear                        );
    ascribe("SolveEndYear"                          , &Input::SolveEndYear                          );
    ascribe("SolveTarget"                           , &Input::SolveTarget                           );
    ascribe("SolveTargetCashSurrenderValue"         , &Input::SolveTargetCashSurrenderValue         );
    ascribe("SolveTargetYear"                       , &Input::SolveTargetYear                       );
    ascribe("SolveBasis"                            , &Input::SolveBasis                            );
    ascribe("SolveSeparateAccountBasis"             , &Input::SolveSeparateAccountBasis             );
    ascribe("GeneralAccountRateType"                , &Input::GeneralAccountRateType                );
    ascribe("SeparateAccountRateType"               , &Input::SeparateAccountRateType               );
    ascribe("LoanRate"                              , &Input::LoanRate                              );
    ascribe("LoanRateType"                          , &Input::LoanRateType                          );
    ascribe("ExperienceReserveRate"                 , &Input::ExperienceReserveRate                 );
    ascribe("OverrideExperienceReserveRate"         , &Input::OverrideExperienceReserveRate         );
    ascribe("WithdrawToBasisThenLoan"               , &Input::WithdrawToBasisThenLoan               );
    ascribe("UseAverageOfAllFunds"                  , &Input::UseAverageOfAllFunds                  );
    ascribe("OverrideFundManagementFee"             , &Input::OverrideFundManagementFee             );
    ascribe("FundChoiceType"                        , &Input::FundChoiceType                        );
    ascribe("InputFundManagementFee"                , &Input::InputFundManagementFee                );
    ascribe("RunOrder"                              , &Input::RunOrder                              );
    ascribe("NumberOfIdenticalLives"                , &Input::NumberOfIdenticalLives                );
    ascribe("UseExperienceRating"                   , &Input::UseExperienceRating                   );
    ascribe("UsePartialMortality"                   , &Input::UsePartialMortality                   );
////    ascribe("PartialMortalityTable"                 , &Input::PartialMortalityTable                 ); // TODO ?? Not implemented.
    ascribe("InsuredName"                           , &Input::InsuredName                           );
    ascribe("Address"                               , &Input::Address                               );
    ascribe("City"                                  , &Input::City                                  );
    ascribe("State"                                 , &Input::State                                 );
    ascribe("ZipCode"                               , &Input::ZipCode                               );
    ascribe("EmployeeClass"                         , &Input::EmployeeClass                         );
    ascribe("CorporationName"                       , &Input::CorporationName                       );
    ascribe("CorporationAddress"                    , &Input::CorporationAddress                    );
    ascribe("CorporationCity"                       , &Input::CorporationCity                       );
    ascribe("CorporationState"                      , &Input::CorporationState                      );
    ascribe("CorporationZipCode"                    , &Input::CorporationZipCode                    );
    ascribe("AgentName"                             , &Input::AgentName                             );
    ascribe("AgentAddress"                          , &Input::AgentAddress                          );
    ascribe("AgentCity"                             , &Input::AgentCity                             );
    ascribe("AgentState"                            , &Input::AgentState                            );
    ascribe("AgentZipCode"                          , &Input::AgentZipCode                          );
    ascribe("AgentPhone"                            , &Input::AgentPhone                            );
    ascribe("AgentId"                               , &Input::AgentId                               );
//    ascribe("InsuredPremiumTableNumber"             , &Input::InsuredPremiumTableNumber             ); // TODO ?? Not implemented.
    ascribe("InsuredPremiumTableFactor"             , &Input::InsuredPremiumTableFactor             );
//    ascribe("CorporationPremiumTableNumber"         , &Input::CorporationPremiumTableNumber         ); // TODO ?? Not implemented.
//    ascribe("CorporationPremiumTableFactor"         , &Input::CorporationPremiumTableFactor         ); // TODO ?? Not implemented.
    ascribe("EffectiveDate"                         , &Input::EffectiveDate                         );
    ascribe("DefinitionOfLifeInsurance"             , &Input::DefinitionOfLifeInsurance             );
    ascribe("DefinitionOfMaterialChange"            , &Input::DefinitionOfMaterialChange            );
    ascribe("AvoidMecMethod"                        , &Input::AvoidMecMethod                        );
    ascribe("RetireesCanEnroll"                     , &Input::RetireesCanEnroll                     );
    ascribe("GroupUnderwritingType"                 , &Input::GroupUnderwritingType                 );
    ascribe("BlendGender"                           , &Input::BlendGender                           );
    ascribe("BlendSmoking"                          , &Input::BlendSmoking                          );
    ascribe("MaleProportion"                        , &Input::MaleProportion                        );
    ascribe("NonsmokerProportion"                   , &Input::NonsmokerProportion                   );
    ascribe("TermProportion"                        , &Input::TermProportion                        );
    ascribe("TermAdjustmentMethod"                  , &Input::TermAdjustmentMethod                  );
    ascribe("IncludeInComposite"                    , &Input::IncludeInComposite                    );
    ascribe("Comments"                              , &Input::Comments                              );
    ascribe("AmortizePremiumLoad"                   , &Input::AmortizePremiumLoad                   );
    ascribe("InforceYear"                           , &Input::InforceYear                           );
    ascribe("InforceMonth"                          , &Input::InforceMonth                          );
    ascribe("InforceGeneralAccountValue"            , &Input::InforceGeneralAccountValue            );
    ascribe("InforceSeparateAccountValue"           , &Input::InforceSeparateAccountValue           );
    ascribe("InforceRegularLoanValue"               , &Input::InforceRegularLoanValue               );
    ascribe("InforcePreferredLoanValue"             , &Input::InforcePreferredLoanValue             );
    ascribe("InforceRegularLoanBalance"             , &Input::InforceRegularLoanBalance             );
    ascribe("InforcePreferredLoanBalance"           , &Input::InforcePreferredLoanBalance           );
    ascribe("InforceCumulativeNoLapsePremium"       , &Input::InforceCumulativeNoLapsePremium       );
    ascribe("InforceCumulativePayments"             , &Input::InforceCumulativePayments             );
    ascribe("Country"                               , &Input::Country                               );
    ascribe("OverrideCoiMultiplier"                 , &Input::OverrideCoiMultiplier                 );
    ascribe("CountryCoiMultiplier"                  , &Input::CountryCoiMultiplier                  );
    ascribe("SurviveToType"                         , &Input::SurviveToType                         );
    ascribe("SurviveToYear"                         , &Input::SurviveToYear                         );
    ascribe("SurviveToAge"                          , &Input::SurviveToAge                          );
    ascribe("MaximumNaar"                           , &Input::MaximumNaar                           );
    ascribe("UseOffshoreCorridorFactor"             , &Input::UseOffshoreCorridorFactor             );
    ascribe("ChildRider"                            , &Input::ChildRider                            );
    ascribe("ChildRiderAmount"                      , &Input::ChildRiderAmount                      );
    ascribe("SpouseRider"                           , &Input::SpouseRider                           );
    ascribe("SpouseRiderAmount"                     , &Input::SpouseRiderAmount                     );
    ascribe("SpouseIssueAge"                        , &Input::SpouseIssueAge                        );
    ascribe("Franchise"                             , &Input::Franchise                             );
    ascribe("PolicyNumber"                          , &Input::PolicyNumber                          );
    ascribe("PolicyDate"                            , &Input::PolicyDate                            );
    ascribe("InforceTaxBasis"                       , &Input::InforceTaxBasis                       );
    ascribe("InforceCumulativeGlp"                  , &Input::InforceCumulativeGlp                  );
    ascribe("InforceGlp"                            , &Input::InforceGlp                            );
    ascribe("InforceGsp"                            , &Input::InforceGsp                            );
    ascribe("InforceSevenPayPremium"                , &Input::InforceSevenPayPremium                );
    ascribe("InforceIsMec"                          , &Input::InforceIsMec                          );
    ascribe("LastMaterialChangeDate"                , &Input::LastMaterialChangeDate                );
    ascribe("InforceDcv"                            , &Input::InforceDcv                            );
    ascribe("InforceAvBeforeLastMc"                 , &Input::InforceAvBeforeLastMc                 );
    ascribe("InforceContractYear"                   , &Input::InforceContractYear                   );
    ascribe("InforceContractMonth"                  , &Input::InforceContractMonth                  );
    ascribe("InforceLeastDeathBenefit"              , &Input::InforceLeastDeathBenefit              );
    ascribe("StateOfJurisdiction"                   , &Input::StateOfJurisdiction                   );
    ascribe("SalarySpecifiedAmountFactor"           , &Input::SalarySpecifiedAmountFactor           );
    ascribe("SalarySpecifiedAmountCap"              , &Input::SalarySpecifiedAmountCap              );
    ascribe("SalarySpecifiedAmountOffset"           , &Input::SalarySpecifiedAmountOffset           );
    ascribe("HoneymoonEndorsement"                  , &Input::HoneymoonEndorsement                  );
    ascribe("PostHoneymoonSpread"                   , &Input::PostHoneymoonSpread                   );
    ascribe("InforceHoneymoonValue"                 , &Input::InforceHoneymoonValue                 );
    ascribe("ExtraMonthlyCustodialFee"              , &Input::ExtraMonthlyCustodialFee              );
    ascribe("ExtraCompensationOnAssets"             , &Input::ExtraCompensationOnAssets             );
    ascribe("ExtraCompensationOnPremium"            , &Input::ExtraCompensationOnPremium            );
    ascribe("OffshoreCorridorFactor"                , &Input::OffshoreCorridorFactor                );
    ascribe("PartialMortalityMultiplier"            , &Input::PartialMortalityMultiplier            );
    ascribe("CurrentCoiMultiplier"                  , &Input::CurrentCoiMultiplier                  );
    ascribe("CurrentCoiGrading"                     , &Input::CurrentCoiGrading                     );
//    ascribe("CorporationTaxBracket"                 , &Input::CorporationTaxBracket                 ); // TODO ?? Not implemented.
//    ascribe("TaxBracket"                            , &Input::TaxBracket                            ); // TODO ?? Not implemented.
    ascribe("ProjectedSalary"                       , &Input::ProjectedSalary                       );
    ascribe("SpecifiedAmount"                       , &Input::SpecifiedAmount                       );
    ascribe("DeathBenefitOption"                    , &Input::DeathBenefitOption                    );
    ascribe("Payment"                               , &Input::Payment                               );
    ascribe("PaymentMode"                           , &Input::PaymentMode                           );
    ascribe("CorporationPayment"                    , &Input::CorporationPayment                    );
    ascribe("CorporationPaymentMode"                , &Input::CorporationPaymentMode                );
    ascribe("GeneralAccountRate"                    , &Input::GeneralAccountRate                    );
    ascribe("SeparateAccountRate"                   , &Input::SeparateAccountRate                   );
    ascribe("NewLoan"                               , &Input::NewLoan                               );
    ascribe("Withdrawal"                            , &Input::Withdrawal                            );
    ascribe("FlatExtra"                             , &Input::FlatExtra                             );
//    ascribe("PolicyLevelFlatExtra"                  , &Input::PolicyLevelFlatExtra                  ); // TODO ?? Not implemented.
    ascribe("HoneymoonValueSpread"                  , &Input::HoneymoonValueSpread                  );
    ascribe("PremiumHistory"                        , &Input::PremiumHistory                        );
    ascribe("SpecamtHistory"                        , &Input::SpecamtHistory                        );
    ascribe("FundAllocations"                       , &Input::FundAllocations                       );
    ascribe("CashValueEnhancementRate"              , &Input::CashValueEnhancementRate              );

    ascribe("CreateSupplementalReport"              , &Input::CreateSupplementalReport              );
    ascribe("SupplementalReportColumn00"            , &Input::SupplementalReportColumn00            );
    ascribe("SupplementalReportColumn01"            , &Input::SupplementalReportColumn01            );
    ascribe("SupplementalReportColumn02"            , &Input::SupplementalReportColumn02            );
    ascribe("SupplementalReportColumn03"            , &Input::SupplementalReportColumn03            );
    ascribe("SupplementalReportColumn04"            , &Input::SupplementalReportColumn04            );
    ascribe("SupplementalReportColumn05"            , &Input::SupplementalReportColumn05            );
    ascribe("SupplementalReportColumn06"            , &Input::SupplementalReportColumn06            );
    ascribe("SupplementalReportColumn07"            , &Input::SupplementalReportColumn07            );
    ascribe("SupplementalReportColumn08"            , &Input::SupplementalReportColumn08            );
    ascribe("SupplementalReportColumn09"            , &Input::SupplementalReportColumn09            );
    ascribe("SupplementalReportColumn10"            , &Input::SupplementalReportColumn10            );
    ascribe("SupplementalReportColumn11"            , &Input::SupplementalReportColumn11            );

    ascribe("DeprecatedSolveTgtAtWhich"             , &Input::DeprecatedSolveTgtAtWhich             );
    ascribe("DeprecatedSolveFromWhich"              , &Input::DeprecatedSolveFromWhich              );
    ascribe("DeprecatedSolveToWhich"                , &Input::DeprecatedSolveToWhich                );
    ascribe("DeprecatedUseDOB"                      , &Input::DeprecatedUseDOB                      );
    ascribe("DeprecatedUseDOR"                      , &Input::DeprecatedUseDOR                      );
    ascribe("EffectiveDateToday"                    , &Input::EffectiveDateToday                    );

    ascribe("DeathBenefitOptionFromRetirement"      , &Input::DeathBenefitOptionFromRetirement      );
    ascribe("DeathBenefitOptionFromIssue"           , &Input::DeathBenefitOptionFromIssue           );
    ascribe("SpecifiedAmountFromRetirement"         , &Input::SpecifiedAmountFromRetirement         );
    ascribe("SpecifiedAmountFromIssue"              , &Input::SpecifiedAmountFromIssue              );
//    ascribe("SpecifiedAmountStrategyFromRetirement" , &Input::SpecifiedAmountStrategyFromRetirement ); // TODO ?? Not implemented.
    ascribe("SpecifiedAmountStrategyFromIssue"      , &Input::SpecifiedAmountStrategyFromIssue      );
    ascribe("IndividualPaymentMode"                 , &Input::IndividualPaymentMode                 );
    ascribe("IndividualPaymentToAlternative"        , &Input::IndividualPaymentToAlternative        );
    ascribe("IndividualPaymentToAge"                , &Input::IndividualPaymentToAge                );
    ascribe("IndividualPaymentToDuration"           , &Input::IndividualPaymentToDuration           );
    ascribe("IndividualPaymentAmount"               , &Input::IndividualPaymentAmount               );
    ascribe("IndividualPaymentStrategy"             , &Input::IndividualPaymentStrategy             );
    ascribe("LoanAmount"                            , &Input::LoanAmount                            );
    ascribe("LoanFromAge"                           , &Input::LoanFromAge                           );
    ascribe("LoanFromAlternative"                   , &Input::LoanFromAlternative                   );
    ascribe("LoanFromDuration"                      , &Input::LoanFromDuration                      );
    ascribe("LoanToAge"                             , &Input::LoanToAge                             );
    ascribe("LoanToAlternative"                     , &Input::LoanToAlternative                     );
    ascribe("LoanToDuration"                        , &Input::LoanToDuration                        );
    ascribe("WithdrawalAmount"                      , &Input::WithdrawalAmount                      );
    ascribe("WithdrawalFromAge"                     , &Input::WithdrawalFromAge                     );
    ascribe("WithdrawalFromAlternative"             , &Input::WithdrawalFromAlternative             );
    ascribe("WithdrawalFromDuration"                , &Input::WithdrawalFromDuration                );
    ascribe("WithdrawalToAge"                       , &Input::WithdrawalToAge                       );
    ascribe("WithdrawalToAlternative"               , &Input::WithdrawalToAlternative               );
    ascribe("WithdrawalToDuration"                  , &Input::WithdrawalToDuration                  );
}

void convert_to_ihs(IllusInputParms& ihs, Input const& lmi)
{
    std::vector<std::string>::const_iterator i;
    for(i = lmi.member_names().begin(); i != lmi.member_names().end(); ++i)
        {
        ihs[*i] = lmi[*i].str();
        }
// TODO ?? This is wrong, no?    ihs.propagate_changes_from_base_and_finalize();
    ihs.propagate_changes_to_base_and_finalize();
}

void convert_to_ihs(std::vector<IllusInputParms>& ihs, std::vector<Input> const& lmi)
{
    Timer timer;
    ihs.resize(lmi.size());
    for(unsigned int j = 0; j < lmi.size(); ++j)
        {
        convert_to_ihs(ihs[j], lmi[j]);
        }
    status() << "Convert to ihs: " << timer.Stop().Report() << std::flush;
}

void convert_from_ihs(IllusInputParms const& ihs, Input& lmi)
{
    std::vector<std::string>::const_iterator i;
    for(i = lmi.member_names().begin(); i != lmi.member_names().end(); ++i)
        {
        lmi[*i] = ihs[*i].str();
        }
    // Repair a known problem in the legacy implementation, where
    // these two possibilities were originally treated as independent
    // boolean states (which is wrong, because they're mutually
    // exclusive), and later unified into a single enumerative state
    // (but defectively, so that only the boolean state is actually
    // reliable).
    if(ihs.AvgFund)
        {
        lmi["FundChoiceType"] = "Average fund";
        }
    if(ihs.OverrideFundMgmtFee)
        {
        lmi["FundChoiceType"] = "Override fund";
        }
}

void convert_from_ihs(std::vector<IllusInputParms> const& ihs, std::vector<Input>& lmi)
{
    Timer timer;
    lmi.resize(ihs.size());
    for(unsigned int j = 0; j < ihs.size(); ++j)
        {
        convert_from_ihs(ihs[j], lmi[j]);
        }
    status() << "Convert to lmi: " << timer.Stop().Report() << std::flush;
}

