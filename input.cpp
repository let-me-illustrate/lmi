// MVC Model for life-insurance illustrations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "input.hpp"

#include "alert.hpp"
#include "database.hpp" // Needed only for database_'s dtor.
#include "timer.hpp"

/// Values are initialized by UDT defaults where appropriate, and here
/// in the initializer-list otherwise. All "ascribed" data members are
/// listed here for clarity and maintainability, and commented out if
/// UDT defaults are presently appropriate.

Input::Input()
    :IssueAge                         ("45")
    ,RetirementAge                    ("65")
    ,Gender                           ("Male")
    ,Smoking                          ("Nonsmoker")
    ,UnderwritingClass                ("Standard")
//    ,WaiverOfPremiumBenefit           ("")
//    ,AccidentalDeathBenefit           ("")
//    ,TermRider                        ("")
//    ,TermRiderAmount                  ("")
    ,TotalSpecifiedAmount             ("1000000")
//    ,TermRiderProportion              ("")
//    ,TermRiderUseProportion           ("")
//    ,DateOfBirth                      ("")
//    ,SubstandardTable                 ("")
//    ,ProductName                      ("")
//    ,Dumpin                           ("")
//    ,External1035ExchangeAmount       ("")
//    ,External1035ExchangeBasis        ("")
//    ,External1035ExchangeFromMec      ("")
//    ,Internal1035ExchangeAmount       ("")
//    ,Internal1035ExchangeBasis        ("")
//    ,Internal1035ExchangeFromMec      ("")
//    ,SolveTargetTime                  ("95")
//    ,SolveBeginTime                   ("")
//    ,SolveEndTime                     ("95")
//    ,SolveType                        ("")
//    ,SolveBeginYear                   ("")
//    ,SolveEndYear                     ("50")
//    ,SolveTarget                      ("")
//    ,SolveTargetCashSurrenderValue    ("")
//    ,SolveTargetYear                  ("50")
//    ,SolveBasis                       ("")
//    ,SolveSeparateAccountBasis        ("")
    ,UseCurrentDeclaredRate           ("Yes")
//    ,GeneralAccountRateType           ("Credited rate")
    ,SeparateAccountRateType          ("Gross rate")
    ,LoanRate                         ("0.06")
//    ,LoanRateType                     ("")
    ,OverrideExperienceReserveRate    ("Yes")
    ,ExperienceReserveRate            ("0.02")
    ,ExperienceRatingInitialKFactor   ("1")
//    ,InforceNetExperienceReserve      ("")
//    ,InforceYtdNetCoiCharge           ("")
//    ,WithdrawToBasisThenLoan          ("")
//    ,UseAverageOfAllFunds             ("")
//    ,OverrideFundManagementFee        ("")
    ,FundChoiceType                   ("Choose funds")
//    ,InputFundManagementFee           ("")
//    ,RunOrder                         ("")
    ,NumberOfIdenticalLives           ("1")
//    ,UseExperienceRating              ("")
//    ,UsePartialMortality              ("")
//    ,PartialMortalityTable            ("")
//    ,InsuredName                      ("")
//    ,Address                          ("")
//    ,City                             ("")
    ,State                            ("CT")
//    ,ZipCode                          ("")
//    ,EmployeeClass                    ("")
//    ,CorporationName                  ("")
//    ,CorporationAddress               ("")
//    ,CorporationCity                  ("")
    ,CorporationState                 ("CT")
//    ,CorporationZipCode               ("")
    ,AgentName                        ("*** REQUIRED FIELD MISSING ***")
    ,AgentAddress                     ("*** REQUIRED FIELD MISSING ***")
    ,AgentCity                        ("*** REQUIRED FIELD MISSING ***")
    ,AgentState                       ("CT")
//    ,AgentZipCode                     ("")
//    ,AgentPhone                       ("")
    ,AgentId                          ("*** REQUIRED FIELD MISSING ***")
//    ,InsuredPremiumTableNumber        ("")
    ,InsuredPremiumTableFactor        ("1")
//    ,CorporationPremiumTableNumber    ("")
    ,CorporationPremiumTableFactor    ("1")
//    ,EffectiveDate                    ("")
//    ,DefinitionOfLifeInsurance        ("")
    ,DefinitionOfMaterialChange       ("Earlier of increase or unnecessary premium")
//    ,AvoidMecMethod                   ("")
//    ,RetireesCanEnroll                ("")
//    ,GroupUnderwritingType            ("")
//    ,LastCoiReentryDate               ("")
//    ,BlendGender                      ("")
//    ,BlendSmoking                     ("")
    ,MaleProportion                   ("1")
    ,NonsmokerProportion              ("1")
    ,TermAdjustmentMethod             ("Adjust base first")
    ,IncludeInComposite               ("Yes")
//    ,Comments                         ("")
//    ,AmortizePremiumLoad              ("")
//    ,InforceYear                      ("")
//    ,InforceMonth                     ("")
//    ,InforceGeneralAccountValue       ("")
//    ,InforceSeparateAccountValue      ("")
//    ,InforceRegularLoanValue          ("")
//    ,InforcePreferredLoanValue        ("")
//    ,InforceRegularLoanBalance        ("")
//    ,InforcePreferredLoanBalance      ("")
//    ,InforceCumulativeNoLapsePremium  ("")
//    ,InforceCumulativePayments        ("")
    ,Country                          ("US")
//    ,OverrideCoiMultiplier            ("")
    ,CountryCoiMultiplier             ("1")
    ,SurviveToType                    ("Survive to age limit")
    ,SurviveToYear                    ("100")
    ,SurviveToAge                     ("99")
    ,MaximumNaar                      ("10000000")
//    ,ChildRider                       ("")
//    ,ChildRiderAmount                 ("")
//    ,SpouseRider                      ("")
//    ,SpouseRiderAmount                ("")
    ,SpouseIssueAge                   ("45")
//    ,Franchise                        ("")
//    ,PolicyNumber                     ("")
//    ,InforceAsOfDate                  ("")
//    ,InforceTaxBasis                  ("")
//    ,InforceCumulativeGlp             ("")
//    ,InforceGlp                       ("")
//    ,InforceGsp                       ("")
//    ,InforceSevenPayPremium           ("")
//    ,InforceIsMec                     ("")
//    ,LastMaterialChangeDate           ("")
//    ,InforceDcv                       ("")
//    ,InforceAvBeforeLastMc            ("")
//    ,InforceContractYear              ("")
//    ,InforceContractMonth             ("")
//    ,InforceLeastDeathBenefit         ("")
    ,StateOfJurisdiction              ("CT")
    ,PremiumTaxState                  ("CT")
    ,SalarySpecifiedAmountFactor      ("1")
    ,SalarySpecifiedAmountCap         ("100000")
    ,SalarySpecifiedAmountOffset      ("50000")
//    ,HoneymoonEndorsement             ("")
//    ,PostHoneymoonSpread              ("")
//    ,InforceHoneymoonValue            ("")
    ,ExtraMonthlyCustodialFee         ("0")
    ,ExtraCompensationOnAssets        ("0")
    ,ExtraCompensationOnPremium       ("0")
    ,PartialMortalityMultiplier       ("1")
    ,CurrentCoiMultiplier             ("1")
//    ,CorporationTaxBracket            ("")
//    ,TaxBracket                       ("")
    ,ProjectedSalary                  ("100000")
    ,SpecifiedAmount                  ("1000000")
    ,DeathBenefitOption               ("a")
    ,Payment                          ("20000")
    ,PaymentMode                      ("annual")
    ,CorporationPayment               ("0")
    ,CorporationPaymentMode           ("annual")
//    ,GeneralAccountRate               ("") // See DoCustomizeInitialValues().
    ,SeparateAccountRate              ("0.08")
    ,NewLoan                          ("0")
    ,Withdrawal                       ("0")
    ,FlatExtra                        ("0")
//    ,PolicyLevelFlatExtra             ("")
    ,HoneymoonValueSpread             ("0")
    ,PremiumHistory                   ("0")
    ,SpecamtHistory                   ("0")
    ,FundAllocations                  ("0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0")
    ,CashValueEnhancementRate         ("0")
//    ,CreateSupplementalReport         ("")
//    ,SupplementalReportColumn00       ("")
//    ,SupplementalReportColumn01       ("")
//    ,SupplementalReportColumn02       ("")
//    ,SupplementalReportColumn03       ("")
//    ,SupplementalReportColumn04       ("")
//    ,SupplementalReportColumn05       ("")
//    ,SupplementalReportColumn06       ("")
//    ,SupplementalReportColumn07       ("")
//    ,SupplementalReportColumn08       ("")
//    ,SupplementalReportColumn09       ("")
//    ,SupplementalReportColumn10       ("")
//    ,SupplementalReportColumn11       ("")
    ,SolveTgtAtWhich                  ("Maturity")
    ,SolveFromWhich                   ("Issue")
    ,SolveToWhich                     ("Retirement")
//    ,UseDOB                           ("")
//    ,EffectiveDateToday               ("")
//    ,DeathBenefitOptionFromRetirement ("")
//    ,DeathBenefitOptionFromIssue      ("")
    ,SpecifiedAmountFromRetirement    ("1000000")
    ,SpecifiedAmountFromIssue         ("1000000")
//    ,SpecifiedAmountStrategyFromIssue ("")
//    ,IndividualPaymentMode            ("")
    ,IndividualPaymentToAlternative   ("Maturity")
    ,IndividualPaymentToAge           ("95")
    ,IndividualPaymentToDuration      ("50")
//    ,IndividualPaymentAmount          ("")
//    ,IndividualPaymentStrategy        ("")
//    ,LoanAmount                       ("")
    ,LoanFromAge                      ("45")
    ,LoanFromAlternative              ("Retirement")
    ,LoanFromDuration                 ("20")
    ,LoanToAge                        ("95")
    ,LoanToAlternative                ("Maturity")
    ,LoanToDuration                   ("50")
//    ,WithdrawalAmount                 ("")
    ,WithdrawalFromAge                ("45")
    ,WithdrawalFromAlternative        ("Retirement")
    ,WithdrawalFromDuration           ("20")
    ,WithdrawalToAge                  ("95")
    ,WithdrawalToAlternative          ("Maturity")
    ,WithdrawalToDuration             ("50")
{
    AscribeMembers();
    DoAdaptExternalities(); // Initialize database, e.g.
    DoTransmogrify();       // Make DOB and age consistent, e.g.
}

Input::Input(Input const& z)
    :obstruct_slicing  <Input>()
    ,xml_serializable  <Input>()
    ,MvcModel                 ()
    ,MemberSymbolTable <Input>()
{
    AscribeMembers();
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
    DoAdaptExternalities();
}

Input::~Input()
{
}

Input& Input::operator=(Input const& z)
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = z[*i].str();
        }
    DoAdaptExternalities();
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

void Input::AscribeMembers()
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
    ascribe("SubstandardTable"                      , &Input::SubstandardTable                      );
    ascribe("ProductName"                           , &Input::ProductName                           );
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
    ascribe("UseCurrentDeclaredRate"                , &Input::UseCurrentDeclaredRate                );
    ascribe("GeneralAccountRateType"                , &Input::GeneralAccountRateType                );
    ascribe("SeparateAccountRateType"               , &Input::SeparateAccountRateType               );
    ascribe("LoanRate"                              , &Input::LoanRate                              );
    ascribe("LoanRateType"                          , &Input::LoanRateType                          );
    ascribe("OverrideExperienceReserveRate"         , &Input::OverrideExperienceReserveRate         );
    ascribe("ExperienceReserveRate"                 , &Input::ExperienceReserveRate                 );
    ascribe("ExperienceRatingInitialKFactor"        , &Input::ExperienceRatingInitialKFactor        );
    ascribe("InforceNetExperienceReserve"           , &Input::InforceNetExperienceReserve           );
    ascribe("InforceYtdNetCoiCharge"                , &Input::InforceYtdNetCoiCharge                );
    ascribe("WithdrawToBasisThenLoan"               , &Input::WithdrawToBasisThenLoan               );
    ascribe("UseAverageOfAllFunds"                  , &Input::UseAverageOfAllFunds                  );
    ascribe("OverrideFundManagementFee"             , &Input::OverrideFundManagementFee             );
    ascribe("FundChoiceType"                        , &Input::FundChoiceType                        );
    ascribe("InputFundManagementFee"                , &Input::InputFundManagementFee                );
    ascribe("RunOrder"                              , &Input::RunOrder                              );
    ascribe("NumberOfIdenticalLives"                , &Input::NumberOfIdenticalLives                );
    ascribe("UseExperienceRating"                   , &Input::UseExperienceRating                   );
    ascribe("UsePartialMortality"                   , &Input::UsePartialMortality                   );
////    ascribe("PartialMortalityTable"                 , &Input::PartialMortalityTable                 ); // INPUT !! Not yet implemented.
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
//    ascribe("InsuredPremiumTableNumber"             , &Input::InsuredPremiumTableNumber             ); // INPUT !! Not yet implemented.
    ascribe("InsuredPremiumTableFactor"             , &Input::InsuredPremiumTableFactor             );
//    ascribe("CorporationPremiumTableNumber"         , &Input::CorporationPremiumTableNumber         ); // INPUT !! Not yet implemented.
    ascribe("CorporationPremiumTableFactor"         , &Input::CorporationPremiumTableFactor         );
    ascribe("EffectiveDate"                         , &Input::EffectiveDate                         );
    ascribe("DefinitionOfLifeInsurance"             , &Input::DefinitionOfLifeInsurance             );
    ascribe("DefinitionOfMaterialChange"            , &Input::DefinitionOfMaterialChange            );
    ascribe("AvoidMecMethod"                        , &Input::AvoidMecMethod                        );
    ascribe("RetireesCanEnroll"                     , &Input::RetireesCanEnroll                     );
    ascribe("GroupUnderwritingType"                 , &Input::GroupUnderwritingType                 );
    ascribe("LastCoiReentryDate"                    , &Input::LastCoiReentryDate                    );
    ascribe("BlendGender"                           , &Input::BlendGender                           );
    ascribe("BlendSmoking"                          , &Input::BlendSmoking                          );
    ascribe("MaleProportion"                        , &Input::MaleProportion                        );
    ascribe("NonsmokerProportion"                   , &Input::NonsmokerProportion                   );
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
    ascribe("ChildRider"                            , &Input::ChildRider                            );
    ascribe("ChildRiderAmount"                      , &Input::ChildRiderAmount                      );
    ascribe("SpouseRider"                           , &Input::SpouseRider                           );
    ascribe("SpouseRiderAmount"                     , &Input::SpouseRiderAmount                     );
    ascribe("SpouseIssueAge"                        , &Input::SpouseIssueAge                        );
    ascribe("Franchise"                             , &Input::Franchise                             );
    ascribe("PolicyNumber"                          , &Input::PolicyNumber                          );
    ascribe("InforceAsOfDate"                       , &Input::InforceAsOfDate                       );
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
    ascribe("PremiumTaxState"                       , &Input::PremiumTaxState                       );
    ascribe("SalarySpecifiedAmountFactor"           , &Input::SalarySpecifiedAmountFactor           );
    ascribe("SalarySpecifiedAmountCap"              , &Input::SalarySpecifiedAmountCap              );
    ascribe("SalarySpecifiedAmountOffset"           , &Input::SalarySpecifiedAmountOffset           );
    ascribe("HoneymoonEndorsement"                  , &Input::HoneymoonEndorsement                  );
    ascribe("PostHoneymoonSpread"                   , &Input::PostHoneymoonSpread                   );
    ascribe("InforceHoneymoonValue"                 , &Input::InforceHoneymoonValue                 );
    ascribe("ExtraMonthlyCustodialFee"              , &Input::ExtraMonthlyCustodialFee              );
    ascribe("ExtraCompensationOnAssets"             , &Input::ExtraCompensationOnAssets             );
    ascribe("ExtraCompensationOnPremium"            , &Input::ExtraCompensationOnPremium            );
    ascribe("PartialMortalityMultiplier"            , &Input::PartialMortalityMultiplier            );
    ascribe("CurrentCoiMultiplier"                  , &Input::CurrentCoiMultiplier                  );
    ascribe("CorporationTaxBracket"                 , &Input::CorporationTaxBracket                 );
    ascribe("TaxBracket"                            , &Input::TaxBracket                            );
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
//    ascribe("PolicyLevelFlatExtra"                  , &Input::PolicyLevelFlatExtra                  ); // INPUT !! Not yet implemented.
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

    ascribe("SolveTgtAtWhich"                       , &Input::SolveTgtAtWhich                       );
    ascribe("SolveFromWhich"                        , &Input::SolveFromWhich                        );
    ascribe("SolveToWhich"                          , &Input::SolveToWhich                          );
    ascribe("UseDOB"                                , &Input::UseDOB                                );
    ascribe("EffectiveDateToday"                    , &Input::EffectiveDateToday                    );

    ascribe("DeathBenefitOptionFromRetirement"      , &Input::DeathBenefitOptionFromRetirement      );
    ascribe("DeathBenefitOptionFromIssue"           , &Input::DeathBenefitOptionFromIssue           );
    ascribe("SpecifiedAmountFromRetirement"         , &Input::SpecifiedAmountFromRetirement         );
    ascribe("SpecifiedAmountFromIssue"              , &Input::SpecifiedAmountFromIssue              );
//    ascribe("SpecifiedAmountStrategyFromRetirement" , &Input::SpecifiedAmountStrategyFromRetirement ); // INPUT !! Obsolete scalar alternative controls.
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

datum_base const* Input::DoBaseDatumPointer
    (std::string const& name
    ) const
{
    return member_cast<datum_base>(operator[](name));
}

any_entity& Input::DoEntity(std::string const& name)
{
    return MemberSymbolTable<Input>::operator[](name);
}

any_entity const& Input::DoEntity(std::string const& name) const
{
    return MemberSymbolTable<Input>::operator[](name);
}

MvcModel::NamesType const& Input::DoNames() const
{
    return member_names();
}

MvcModel::StateType Input::DoState() const
{
    return member_state(*this);
}

void Input::DoEnforceCircumscription(std::string const& name)
{
    datum_base* base_datum = member_cast<datum_base>(operator[](name));
    tn_range_base* datum = dynamic_cast<tn_range_base*>(base_datum);
    if(datum)
        {
        datum->enforce_circumscription();
        }
}

void Input::DoEnforceProscription(std::string const& name)
{
    // Here one could handle special cases for which the generic
    // behavior is not wanted.

    datum_base* base_datum = member_cast<datum_base>(operator[](name));
    mc_enum_base* datum = dynamic_cast<mc_enum_base*>(base_datum);
    if(datum)
        {
        datum->enforce_proscription();
        }
}

Input Input::magically_rectify(Input const& original)
{
    Input z(original);

    // Repair a known problem in the legacy implementation, where
    // these two possibilities were originally treated as independent
    // boolean states (which is wrong, because they're mutually
    // exclusive), and later unified into a single enumerative state
    // (but defectively, so that only the boolean state is actually
    // reliable).
    if("Yes" == z["UseAverageOfAllFunds"].str())
        {
        z["FundChoiceType"] = "Average fund";
        }
    if("Yes" == z["OverrideFundManagementFee"].str())
        {
        z["FundChoiceType"] = "Override fund";
        }

    z.Reconcile(); // TODO ?? Necessary only for problematic old cases.
    z.RealizeAllSequenceInput();
    z.make_term_rider_consistent();

    return z;
}

