// MVC Model for life-insurance illustrations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "input.hpp"

#include "alert.hpp"
#include "database.hpp"                 // product_database::~product_database()
#include "timer.hpp"

/// Values are initialized by UDT defaults where appropriate, and here
/// in the initializer-list otherwise. All "ascribed" data members are
/// listed here for clarity and maintainability, and commented out if
/// UDT defaults are presently appropriate.

Input::Input()
    :IssueAge                         {"45"}
    ,RetirementAge                    {"65"}
    ,Gender                           {"Male"}
    ,Smoking                          {"Nonsmoker"}
    ,UnderwritingClass                {"Standard"}
//    ,WaiverOfPremiumBenefit           {""}
//    ,AccidentalDeathBenefit           {""}
//    ,TermRider                        {""}
//    ,TermRiderAmount                  {""}
    ,TotalSpecifiedAmount             {"1000000"}
//    ,TermRiderProportion              {""}
//    ,TermRiderUseProportion           {""}
//    ,DateOfBirth                      {""}
//    ,SubstandardTable                 {""}
//    ,ProductName                      {""}
//    ,Dumpin                           {""}
//    ,External1035ExchangeAmount       {""}
//    ,External1035ExchangeTaxBasis     {""}
//    ,External1035ExchangeFromMec      {""}
//    ,Internal1035ExchangeAmount       {""}
//    ,Internal1035ExchangeTaxBasis     {""}
//    ,Internal1035ExchangeFromMec      {""}
//    ,SolveTargetAge                   {"95"}
//    ,SolveBeginAge                    {""}
//    ,SolveEndAge                      {"95"}
//    ,SolveType                        {""}
//    ,SolveBeginYear                   {""}
//    ,SolveEndYear                     {"50"}
//    ,SolveTarget                      {""}
//    ,SolveTargetValue                 {""}
//    ,SolveTargetYear                  {"50"}
//    ,SolveExpenseGeneralAccountBasis  {""}
//    ,SolveSeparateAccountBasis        {""}
    ,UseCurrentDeclaredRate           {"Yes"}
//    ,GeneralAccountRateType           {"Credited rate"}
    ,SeparateAccountRateType          {"Gross rate"}
    ,LoanRate                         {"0.06"}
//    ,LoanRateType                     {""}
//    ,WithdrawToBasisThenLoan          {""}
//    ,UseAverageOfAllFunds             {""}
//    ,OverrideFundManagementFee        {""}
    ,FundChoiceType                   {"Choose funds"}
//    ,InputFundManagementFee           {""}
//    ,RunOrder                         {""}
    ,NumberOfIdenticalLives           {"1"}
//    ,UsePartialMortality              {""}
//    ,PartialMortalityTable            {""}
//    ,InsuredName                      {""}
//    ,Address                          {""}
//    ,City                             {""}
    ,State                            {"CT"}
//    ,ZipCode                          {""}
//    ,EmployeeClass                    {""}
//    ,CorporationName                  {""}
//    ,CorporationAddress               {""}
//    ,CorporationCity                  {""}
    ,CorporationState                 {"CT"}
//    ,CorporationZipCode               {""}
    ,AgentName                        {"*** REQUIRED FIELD MISSING ***"}
    ,AgentAddress                     {"*** REQUIRED FIELD MISSING ***"}
    ,AgentCity                        {"*** REQUIRED FIELD MISSING ***"}
    ,AgentState                       {"CT"}
//    ,AgentZipCode                     {""}
    ,AgentPhone                       {"*** REQUIRED FIELD MISSING ***"}
    ,AgentId                          {"*** REQUIRED FIELD MISSING ***"}
//    ,InsuredPremiumTableNumber        {""}
    ,InsuredPremiumTableFactor        {"1"}
//    ,CorporationPremiumTableNumber    {""}
    ,CorporationPremiumTableFactor    {"1"}
//    ,EffectiveDate                    {""}
//    ,DefinitionOfLifeInsurance        {""}
//    ,DefinitionOfMaterialChange       {""}
//    ,AvoidMecMethod                   {""}
//    ,RetireesCanEnroll                {""}
//    ,GroupUnderwritingType            {""}
//    ,LastCoiReentryDate               {""}
      ,ListBillDate                   {"2440588"} // Assume no inforce <19700101
//    ,BlendGender                      {""}
//    ,BlendSmoking                     {""}
    ,MaleProportion                   {"1"}
    ,NonsmokerProportion              {"1"}
    ,TermAdjustmentMethod             {"Adjust base first"}
    ,IncludeInComposite               {"Yes"}
//    ,Comments                         {""}
//    ,AmortizePremiumLoad              {""}
//    ,ContractNumber                   {""}
//    ,MasterContractNumber             {""}
//    ,IsInforce                        {""}
//    ,InforceAsOfDate                  {""}
//    ,InforceYear                      {""}
//    ,InforceMonth                     {""}
//    ,InforceAnnualTargetPremium       {""}
//    ,InforceYtdGrossPremium           {""}
//    ,InforceGeneralAccountValue       {""}
//    ,InforceSeparateAccountValue      {""}
//    ,InforceRegularLoanValue          {""}
//    ,InforcePreferredLoanValue        {""}
//    ,InforceRegularLoanBalance        {""}
//    ,InforcePreferredLoanBalance      {""}
//    ,InforceNoLapseActive             {""}
//    ,InforceMonthlyNoLapsePremium     {""}
//    ,InforceCumulativeNoLapsePremium  {""}
//    ,InforceCumulativeNoLapsePayments {""}
//    ,InforceCumulativeRopPayments     {""}
//    ,InforceYtdTaxablePremium         {""}
//    ,InforceCumulativeSalesLoad       {""}
//    ,InforceSpecAmtLoadBase           {""}
//    ,InforceHoneymoonValue            {""}
//    ,InforceCorporationStake          {""}
//    ,InforceTaxBasis                  {""}
//    ,InforceGlp                       {""}
//    ,InforceCumulativeGlp             {""}
//    ,InforceGsp                       {""}
//    ,InforceCumulativeGptPremiumsPaid {""}
//    ,InforceIsMec                     {""}
//    ,InforceSevenPayPremium           {""}
//    ,LastMaterialChangeDate           {""}
//    ,InforceContractYear              {""}
//    ,InforceContractMonth             {""}
//    ,InforceAvBeforeLastMc            {""}
//    ,InforceDcv                       {""}
//    ,InforceLeastDeathBenefit         {""}
    ,Inforce7702AAmountsPaidHistory   {"0"}
    ,Country                          {"US"}
//    ,OverrideCoiMultiplier            {""}
    ,CountryCoiMultiplier             {"1"}
    ,SurviveToType                    {"Survive to age limit"}
    ,SurviveToYear                    {"100"}
    ,SurviveToAge                     {"99"}
    ,MaximumNaar                      {"1000000000"}
//    ,ChildRider                       {""}
//    ,ChildRiderAmount                 {""}
//    ,SpouseRider                      {""}
//    ,SpouseRiderAmount                {""}
    ,SpouseIssueAge                   {"45"}
    ,StateOfJurisdiction              {"CT"}
    ,PremiumTaxState                  {"CT"}
    ,SalarySpecifiedAmountFactor      {"1"}
    ,SalarySpecifiedAmountCap         {"100000"}
    ,SalarySpecifiedAmountOffset      {"50000"}
//    ,HoneymoonEndorsement             {""}
//    ,PostHoneymoonSpread              {""}
    ,ExtraMonthlyCustodialFee         {"0"}
    ,ExtraCompensationOnAssets        {"0"}
    ,ExtraCompensationOnPremium       {"0"}
    ,PartialMortalityMultiplier       {"1"}
    ,CurrentCoiMultiplier             {"1"}
//    ,CorporationTaxBracket            {""}
//    ,TaxBracket                       {""}
    ,ProjectedSalary                  {"100000"}
    ,SpecifiedAmount                  {"1000000"}
    ,SupplementalAmount               {"0"}
    ,DeathBenefitOption               {"a"}
    ,Payment                          {"20000"}
    ,PaymentMode                      {"annual"}
    ,CorporationPayment               {"0"}
    ,CorporationPaymentMode           {"annual"}
//    ,GeneralAccountRate               {""} // See DoCustomizeInitialValues().
    ,SeparateAccountRate              {"0.08"}
    ,NewLoan                          {"0"}
    ,Withdrawal                       {"0"}
    ,FlatExtra                        {"0"}
//    ,PolicyLevelFlatExtra             {""}
    ,HoneymoonValueSpread             {"0"}
    ,FundAllocations                  {"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0"}
    ,CashValueEnhancementRate         {"0"}
//    ,AdditionalReports
//    ,SupplementalIllustrationType
//    ,SplitDollarAccumulateInterest
    ,SplitDollarLoanRate              {"0"}
//    ,SplitDollarRolloutAge
//    ,SplitDollarRolloutAtWhich
//    ,SplitDollarRolloutYear
//    ,CreateSupplementalReport         {""}
//    ,SupplementalReportColumn00       {""}
//    ,SupplementalReportColumn01       {""}
//    ,SupplementalReportColumn02       {""}
//    ,SupplementalReportColumn03       {""}
//    ,SupplementalReportColumn04       {""}
//    ,SupplementalReportColumn05       {""}
//    ,SupplementalReportColumn06       {""}
//    ,SupplementalReportColumn07       {""}
//    ,SupplementalReportColumn08       {""}
//    ,SupplementalReportColumn09       {""}
//    ,SupplementalReportColumn10       {""}
//    ,SupplementalReportColumn11       {""}
    ,SolveTgtAtWhich                  {"Maturity"}
    ,SolveFromWhich                   {"Issue"}
    ,SolveToWhich                     {"Retirement"}
//    ,UseDOB                           {""}
//    ,EffectiveDateToday               {""}
//    ,SpecifiedAmountStrategyFromIssue {""}
//    ,IndividualPaymentStrategy        {""}
{
    AscribeMembers();
    DoAdaptExternalities(); // Initialize database, e.g.
    DoTransmogrify();       // Make DOB and age consistent, e.g.
}

Input::Input(Input const& z)
    :xml_serializable  <Input> {}
    ,MvcModel                  {}
    ,MemberSymbolTable <Input> {}
{
    AscribeMembers();
    MemberSymbolTable<Input>::assign(z);
    DoAdaptExternalities();
}

/// Destructor.
///
/// Although it is explicitly defaulted, this destructor cannot be
/// implemented inside the class definition, where a class type that
/// it depends upon is incomplete.

Input::~Input() = default;

Input& Input::operator=(Input const& z)
{
    MemberSymbolTable<Input>::assign(z);
    DoAdaptExternalities();
    return *this;
}

bool Input::operator==(Input const& z) const
{
    return MemberSymbolTable<Input>::equals(z);
}

mcenum_ledger_type Input::ledger_type () const {return GleanedLedgerType_;}
int                Input::maturity_age() const {return GleanedMaturityAge_;}

int Input::years_to_retirement() const {return retirement_age() - issue_age();}
int Input::years_to_maturity  () const {return maturity_age() - issue_age();}
int Input::issue_age          () const {return IssueAge     .value();}
int Input::retirement_age     () const {return RetirementAge.value();}
int Input::inforce_year       () const {return InforceYear  .value();}
int Input::effective_year     () const {return EffectiveDate.value().year();}

void Input::AscribeMembers()
{
    ascribe("IssueAge"                        , &Input::IssueAge                        );
    ascribe("RetirementAge"                   , &Input::RetirementAge                   );
    ascribe("Gender"                          , &Input::Gender                          );
    ascribe("Smoking"                         , &Input::Smoking                         );
    ascribe("UnderwritingClass"               , &Input::UnderwritingClass               );
    ascribe("WaiverOfPremiumBenefit"          , &Input::WaiverOfPremiumBenefit          );
    ascribe("AccidentalDeathBenefit"          , &Input::AccidentalDeathBenefit          );
    ascribe("TermRider"                       , &Input::TermRider                       );
    ascribe("TermRiderAmount"                 , &Input::TermRiderAmount                 );
    ascribe("TotalSpecifiedAmount"            , &Input::TotalSpecifiedAmount            );
    ascribe("TermRiderProportion"             , &Input::TermRiderProportion             );
    ascribe("TermRiderUseProportion"          , &Input::TermRiderUseProportion          );
    ascribe("DateOfBirth"                     , &Input::DateOfBirth                     );
    ascribe("SubstandardTable"                , &Input::SubstandardTable                );
    ascribe("ProductName"                     , &Input::ProductName                     );
    ascribe("Dumpin"                          , &Input::Dumpin                          );
    ascribe("External1035ExchangeAmount"      , &Input::External1035ExchangeAmount      );
    ascribe("External1035ExchangeTaxBasis"    , &Input::External1035ExchangeTaxBasis    );
    ascribe("External1035ExchangeFromMec"     , &Input::External1035ExchangeFromMec     );
    ascribe("Internal1035ExchangeAmount"      , &Input::Internal1035ExchangeAmount      );
    ascribe("Internal1035ExchangeTaxBasis"    , &Input::Internal1035ExchangeTaxBasis    );
    ascribe("Internal1035ExchangeFromMec"     , &Input::Internal1035ExchangeFromMec     );
    ascribe("SolveTargetAge"                  , &Input::SolveTargetAge                  );
    ascribe("SolveBeginAge"                   , &Input::SolveBeginAge                   );
    ascribe("SolveEndAge"                     , &Input::SolveEndAge                     );
    ascribe("SolveType"                       , &Input::SolveType                       );
    ascribe("SolveBeginYear"                  , &Input::SolveBeginYear                  );
    ascribe("SolveEndYear"                    , &Input::SolveEndYear                    );
    ascribe("SolveTarget"                     , &Input::SolveTarget                     );
    ascribe("SolveTargetValue"                , &Input::SolveTargetValue                );
    ascribe("SolveTargetYear"                 , &Input::SolveTargetYear                 );
    ascribe("SolveExpenseGeneralAccountBasis" , &Input::SolveExpenseGeneralAccountBasis );
    ascribe("SolveSeparateAccountBasis"       , &Input::SolveSeparateAccountBasis       );
    ascribe("UseCurrentDeclaredRate"          , &Input::UseCurrentDeclaredRate          );
    ascribe("GeneralAccountRateType"          , &Input::GeneralAccountRateType          );
    ascribe("SeparateAccountRateType"         , &Input::SeparateAccountRateType         );
    ascribe("LoanRate"                        , &Input::LoanRate                        );
    ascribe("LoanRateType"                    , &Input::LoanRateType                    );
    ascribe("WithdrawToBasisThenLoan"         , &Input::WithdrawToBasisThenLoan         );
    ascribe("UseAverageOfAllFunds"            , &Input::UseAverageOfAllFunds            );
    ascribe("OverrideFundManagementFee"       , &Input::OverrideFundManagementFee       );
    ascribe("FundChoiceType"                  , &Input::FundChoiceType                  );
    ascribe("InputFundManagementFee"          , &Input::InputFundManagementFee          );
    ascribe("RunOrder"                        , &Input::RunOrder                        );
    ascribe("NumberOfIdenticalLives"          , &Input::NumberOfIdenticalLives          );
    ascribe("UsePartialMortality"             , &Input::UsePartialMortality             );
////    ascribe("PartialMortalityTable"           , &Input::PartialMortalityTable           ); // INPUT !! Not yet implemented.
    ascribe("InsuredName"                     , &Input::InsuredName                     );
    ascribe("Address"                         , &Input::Address                         );
    ascribe("City"                            , &Input::City                            );
    ascribe("State"                           , &Input::State                           );
    ascribe("ZipCode"                         , &Input::ZipCode                         );
    ascribe("EmployeeClass"                   , &Input::EmployeeClass                   );
    ascribe("CorporationName"                 , &Input::CorporationName                 );
    ascribe("CorporationAddress"              , &Input::CorporationAddress              );
    ascribe("CorporationCity"                 , &Input::CorporationCity                 );
    ascribe("CorporationState"                , &Input::CorporationState                );
    ascribe("CorporationZipCode"              , &Input::CorporationZipCode              );
    ascribe("AgentName"                       , &Input::AgentName                       );
    ascribe("AgentAddress"                    , &Input::AgentAddress                    );
    ascribe("AgentCity"                       , &Input::AgentCity                       );
    ascribe("AgentState"                      , &Input::AgentState                      );
    ascribe("AgentZipCode"                    , &Input::AgentZipCode                    );
    ascribe("AgentPhone"                      , &Input::AgentPhone                      );
    ascribe("AgentId"                         , &Input::AgentId                         );
//    ascribe("InsuredPremiumTableNumber"       , &Input::InsuredPremiumTableNumber       ); // INPUT !! Not yet implemented.
    ascribe("InsuredPremiumTableFactor"       , &Input::InsuredPremiumTableFactor       );
//    ascribe("CorporationPremiumTableNumber"   , &Input::CorporationPremiumTableNumber   ); // INPUT !! Not yet implemented.
    ascribe("CorporationPremiumTableFactor"   , &Input::CorporationPremiumTableFactor   );
    ascribe("EffectiveDate"                   , &Input::EffectiveDate                   );
    ascribe("DefinitionOfLifeInsurance"       , &Input::DefinitionOfLifeInsurance       );
    ascribe("DefinitionOfMaterialChange"      , &Input::DefinitionOfMaterialChange      );
    ascribe("AvoidMecMethod"                  , &Input::AvoidMecMethod                  );
    ascribe("RetireesCanEnroll"               , &Input::RetireesCanEnroll               );
    ascribe("GroupUnderwritingType"           , &Input::GroupUnderwritingType           );
    ascribe("LastCoiReentryDate"              , &Input::LastCoiReentryDate              );
    ascribe("ListBillDate"                    , &Input::ListBillDate                    );
    ascribe("BlendGender"                     , &Input::BlendGender                     );
    ascribe("BlendSmoking"                    , &Input::BlendSmoking                    );
    ascribe("MaleProportion"                  , &Input::MaleProportion                  );
    ascribe("NonsmokerProportion"             , &Input::NonsmokerProportion             );
    ascribe("TermAdjustmentMethod"            , &Input::TermAdjustmentMethod            );
    ascribe("IncludeInComposite"              , &Input::IncludeInComposite              );
    ascribe("Comments"                        , &Input::Comments                        );
    ascribe("AmortizePremiumLoad"             , &Input::AmortizePremiumLoad             );
    ascribe("ContractNumber"                  , &Input::ContractNumber                  );
    ascribe("MasterContractNumber"            , &Input::MasterContractNumber            );
    ascribe("IsInforce"                       , &Input::IsInforce                       );
    ascribe("InforceAsOfDate"                 , &Input::InforceAsOfDate                 );
    ascribe("InforceYear"                     , &Input::InforceYear                     );
    ascribe("InforceMonth"                    , &Input::InforceMonth                    );
    ascribe("InforceAnnualTargetPremium"      , &Input::InforceAnnualTargetPremium      );
    ascribe("InforceYtdGrossPremium"          , &Input::InforceYtdGrossPremium          );
    ascribe("InforceGeneralAccountValue"      , &Input::InforceGeneralAccountValue      );
    ascribe("InforceSeparateAccountValue"     , &Input::InforceSeparateAccountValue     );
    ascribe("InforceRegularLoanValue"         , &Input::InforceRegularLoanValue         );
    ascribe("InforcePreferredLoanValue"       , &Input::InforcePreferredLoanValue       );
    ascribe("InforceRegularLoanBalance"       , &Input::InforceRegularLoanBalance       );
    ascribe("InforcePreferredLoanBalance"     , &Input::InforcePreferredLoanBalance     );
    ascribe("InforceNoLapseActive"            , &Input::InforceNoLapseActive            );
    ascribe("InforceMonthlyNoLapsePremium"    , &Input::InforceMonthlyNoLapsePremium    );
    ascribe("InforceCumulativeNoLapsePremium" , &Input::InforceCumulativeNoLapsePremium );
    ascribe("InforceCumulativeNoLapsePayments", &Input::InforceCumulativeNoLapsePayments);
    ascribe("InforceCumulativeRopPayments"    , &Input::InforceCumulativeRopPayments    );
    ascribe("InforceYtdTaxablePremium"        , &Input::InforceYtdTaxablePremium        );
    ascribe("InforceCumulativeSalesLoad"      , &Input::InforceCumulativeSalesLoad      );
    ascribe("InforceSpecAmtLoadBase"          , &Input::InforceSpecAmtLoadBase          );
    ascribe("InforceHoneymoonValue"           , &Input::InforceHoneymoonValue           );
    ascribe("InforceCorporationStake"         , &Input::InforceCorporationStake         );
    ascribe("InforceTaxBasis"                 , &Input::InforceTaxBasis                 );
    ascribe("InforceGlp"                      , &Input::InforceGlp                      );
    ascribe("InforceCumulativeGlp"            , &Input::InforceCumulativeGlp            );
    ascribe("InforceGsp"                      , &Input::InforceGsp                      );
    ascribe("InforceCumulativeGptPremiumsPaid", &Input::InforceCumulativeGptPremiumsPaid);
    ascribe("InforceIsMec"                    , &Input::InforceIsMec                    );
    ascribe("InforceSevenPayPremium"          , &Input::InforceSevenPayPremium          );
    ascribe("LastMaterialChangeDate"          , &Input::LastMaterialChangeDate          );
    ascribe("InforceContractYear"             , &Input::InforceContractYear             );
    ascribe("InforceContractMonth"            , &Input::InforceContractMonth            );
    ascribe("InforceAvBeforeLastMc"           , &Input::InforceAvBeforeLastMc           );
    ascribe("InforceDcv"                      , &Input::InforceDcv                      );
    ascribe("InforceLeastDeathBenefit"        , &Input::InforceLeastDeathBenefit        );
    ascribe("Inforce7702AAmountsPaidHistory"  , &Input::Inforce7702AAmountsPaidHistory  );
    ascribe("Country"                         , &Input::Country                         );
    ascribe("OverrideCoiMultiplier"           , &Input::OverrideCoiMultiplier           );
    ascribe("CountryCoiMultiplier"            , &Input::CountryCoiMultiplier            );
    ascribe("SurviveToType"                   , &Input::SurviveToType                   );
    ascribe("SurviveToYear"                   , &Input::SurviveToYear                   );
    ascribe("SurviveToAge"                    , &Input::SurviveToAge                    );
    ascribe("MaximumNaar"                     , &Input::MaximumNaar                     );
    ascribe("ChildRider"                      , &Input::ChildRider                      );
    ascribe("ChildRiderAmount"                , &Input::ChildRiderAmount                );
    ascribe("SpouseRider"                     , &Input::SpouseRider                     );
    ascribe("SpouseRiderAmount"               , &Input::SpouseRiderAmount               );
    ascribe("SpouseIssueAge"                  , &Input::SpouseIssueAge                  );
    ascribe("StateOfJurisdiction"             , &Input::StateOfJurisdiction             );
    ascribe("PremiumTaxState"                 , &Input::PremiumTaxState                 );
    ascribe("SalarySpecifiedAmountFactor"     , &Input::SalarySpecifiedAmountFactor     );
    ascribe("SalarySpecifiedAmountCap"        , &Input::SalarySpecifiedAmountCap        );
    ascribe("SalarySpecifiedAmountOffset"     , &Input::SalarySpecifiedAmountOffset     );
    ascribe("HoneymoonEndorsement"            , &Input::HoneymoonEndorsement            );
    ascribe("PostHoneymoonSpread"             , &Input::PostHoneymoonSpread             );
    ascribe("ExtraMonthlyCustodialFee"        , &Input::ExtraMonthlyCustodialFee        );
    ascribe("ExtraCompensationOnAssets"       , &Input::ExtraCompensationOnAssets       );
    ascribe("ExtraCompensationOnPremium"      , &Input::ExtraCompensationOnPremium      );
    ascribe("PartialMortalityMultiplier"      , &Input::PartialMortalityMultiplier      );
    ascribe("CurrentCoiMultiplier"            , &Input::CurrentCoiMultiplier            );
    ascribe("CorporationTaxBracket"           , &Input::CorporationTaxBracket           );
    ascribe("TaxBracket"                      , &Input::TaxBracket                      );
    ascribe("ProjectedSalary"                 , &Input::ProjectedSalary                 );
    ascribe("SpecifiedAmount"                 , &Input::SpecifiedAmount                 );
    ascribe("SupplementalAmount"              , &Input::SupplementalAmount              );
    ascribe("DeathBenefitOption"              , &Input::DeathBenefitOption              );
    ascribe("Payment"                         , &Input::Payment                         );
    ascribe("PaymentMode"                     , &Input::PaymentMode                     );
    ascribe("CorporationPayment"              , &Input::CorporationPayment              );
    ascribe("CorporationPaymentMode"          , &Input::CorporationPaymentMode          );
    ascribe("GeneralAccountRate"              , &Input::GeneralAccountRate              );
    ascribe("SeparateAccountRate"             , &Input::SeparateAccountRate             );
    ascribe("NewLoan"                         , &Input::NewLoan                         );
    ascribe("Withdrawal"                      , &Input::Withdrawal                      );
    ascribe("FlatExtra"                       , &Input::FlatExtra                       );
//    ascribe("PolicyLevelFlatExtra"            , &Input::PolicyLevelFlatExtra            ); // INPUT !! Not yet implemented.
    ascribe("HoneymoonValueSpread"            , &Input::HoneymoonValueSpread            );
    ascribe("FundAllocations"                 , &Input::FundAllocations                 );
    ascribe("CashValueEnhancementRate"        , &Input::CashValueEnhancementRate        );

    ascribe("AdditionalReports"               , &Input::AdditionalReports               );
    ascribe("SupplementalIllustrationType"    , &Input::SupplementalIllustrationType    );
    ascribe("SplitDollarAccumulateInterest"   , &Input::SplitDollarAccumulateInterest   );
    ascribe("SplitDollarLoanRate"             , &Input::SplitDollarLoanRate             );
    ascribe("SplitDollarRolloutAge"           , &Input::SplitDollarRolloutAge           );
    ascribe("SplitDollarRolloutAtWhich"       , &Input::SplitDollarRolloutAtWhich       );
    ascribe("SplitDollarRolloutYear"          , &Input::SplitDollarRolloutYear          );

    ascribe("CreateSupplementalReport"        , &Input::CreateSupplementalReport        );
    ascribe("SupplementalReportColumn00"      , &Input::SupplementalReportColumn00      );
    ascribe("SupplementalReportColumn01"      , &Input::SupplementalReportColumn01      );
    ascribe("SupplementalReportColumn02"      , &Input::SupplementalReportColumn02      );
    ascribe("SupplementalReportColumn03"      , &Input::SupplementalReportColumn03      );
    ascribe("SupplementalReportColumn04"      , &Input::SupplementalReportColumn04      );
    ascribe("SupplementalReportColumn05"      , &Input::SupplementalReportColumn05      );
    ascribe("SupplementalReportColumn06"      , &Input::SupplementalReportColumn06      );
    ascribe("SupplementalReportColumn07"      , &Input::SupplementalReportColumn07      );
    ascribe("SupplementalReportColumn08"      , &Input::SupplementalReportColumn08      );
    ascribe("SupplementalReportColumn09"      , &Input::SupplementalReportColumn09      );
    ascribe("SupplementalReportColumn10"      , &Input::SupplementalReportColumn10      );
    ascribe("SupplementalReportColumn11"      , &Input::SupplementalReportColumn11      );

    ascribe("SolveTgtAtWhich"                 , &Input::SolveTgtAtWhich                 );
    ascribe("SolveFromWhich"                  , &Input::SolveFromWhich                  );
    ascribe("SolveToWhich"                    , &Input::SolveToWhich                    );
    ascribe("UseDOB"                          , &Input::UseDOB                          );
    ascribe("EffectiveDateToday"              , &Input::EffectiveDateToday              );

    ascribe("SpecifiedAmountStrategyFromIssue", &Input::SpecifiedAmountStrategyFromIssue);
    ascribe("IndividualPaymentStrategy"       , &Input::IndividualPaymentStrategy       );
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

/// Make sure input is consistent before using it.
///
/// Classes derived from MvcModel permit changing any of their member
/// data at any time. They provide member functions for maintaining
/// consistency among such values. When should those member functions
/// be called, and by whom?
///
/// One possible answer is that MvcModel should maintain its own
/// consistency, by calling those member functions synchronously. But
/// consider this case:
///   Input i;
///   i["ProductName"] = some_product_with_no_term_rider; // initially
///   i["TermRider"] = "Yes";
///   i["ProductName"] = some_product_that_offers_a_term_rider;
/// In the synchronous case, the middle assignment would be rejected;
/// there would have to be some mechanism for reporting that, because
/// silent failure is clearly undesirable. It would not be possible to
/// read values from an input file without sorting them carefully in
/// advance; but that would be a daunting task.
///
/// In the GUI, MvcController calls those member functions frequently
/// enough to maintain consistency synchronously. That's appropriate
/// for a GUI: in the example above, an end user would not expect to
/// elect a term rider without first selecting a product that permits
/// it. What would be a daunting task for the program is natural and
/// obvious for an end user familiar with the problem domain who is
/// guided by active, visual control enablement.
///
/// In the asynchronous, non-GUI case, consistency must be ensured by
/// calling those member functions after the last change to member
/// data. That has to be the client's responsibility. For example,
/// AccountValue::AccountValue() calls this function just before it
/// creates an illustration.
///
/// Consider running a "custom_io_1" input file with DOB 1961-12-31,
/// specifying <AutoClose>X</AutoClose> to generate an equivalent
/// '.ill' file. For "custom_io_1" input, DOB governs. In this case,
/// the corresponding issue age has exceeded 45 for as long as lmi
/// has existed, yet the '.ill' file contains <IssueAge>45</IssueAge>
/// (as of 20151102T2351Z, revision 6384) because the default age is
/// inconsistent with the DOB. That's fixed when an illustration is
/// run: AccountValue::AccountValue() calls this function, which calls
/// MvcModel::Reconcile(), which calls MvcModel::Transmogrify(), which
/// calls Input::DoTransmogrify(), which then sets <IssueAge>, so the
/// illustration is correct--only because this function was called.
///
/// That specific issue in "custom_io_1" code was addressed by calling
/// this function explicitly (20151104T0416Z, revision 6385). However,
/// similar issues may exist or may arise elsewhere, and are already
/// present in some old regression-testing files, so the only way to
/// be sure this function is called before producing an illustration
/// is to call it at an appropriate chokepoint; that's why it's called
/// explicitly by AccountValue::AccountValue().

Input Input::consummate(Input const& original)
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

    z.Reconcile();
    z.RealizeAllSequenceInput();
    z.make_term_rider_consistent();

    return z;
}
