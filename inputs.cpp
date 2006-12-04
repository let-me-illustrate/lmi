// Life insurance illustration inputs.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: inputs.cpp,v 1.15 2006-12-04 07:16:15 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputs.hpp"

#include "alert.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "inputillus.hpp"
#include "inputstatus.hpp"
#include "is_sorted_.hpp"
#include "product_names.hpp"

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>

#include <algorithm>
#include <functional>
#include <istream>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <string>

namespace
{
    int KludgeLength = 100; // TODO ?? KLUDGE
}

//============================================================================
InputParms::InputParms()
    :ProductName                   (default_product_name())
    ,Plan                          (e_sample1)
    ,NumberOfLives                 (1)
    ,Dumpin                        (0.0)
    ,External1035ExchangeAmount    (0.0)
    ,External1035ExchangeBasis     (0.0)
    ,External1035ExchangeFromMec   ("No")
    ,Internal1035ExchangeAmount    (0.0)
    ,Internal1035ExchangeBasis     (0.0)
    ,Internal1035ExchangeFromMec   ("No")
    ,SolveTgtAtWhich               (e_tgtatend)
    ,SolveTgtTime                  (95)
    ,SolveFromWhich                (e_fromyear)
    ,SolveBegTime                  (0)
    ,SolveToWhich                  (e_toend)
    ,SolveEndTime                  (95)
    ,SolveType                     (e_solve_none)
    ,SolveBegYear                  (0)
    ,SolveEndYear                  (95)
    ,SolveTarget                   (e_solve_for_endt)
    ,SolveTgtCSV                   (0.0)
    ,SolveTgtYear                  (0)
    ,SolveBasis                    (e_currbasis)
    ,SolveSABasis                  (e_sep_acct_full)
    ,EePmtStrategy                 (e_pmtinputscalar)
    ,ErPmtStrategy                 (e_pmtinputscalar)
    ,SAStrategy                    (e_sainputscalar)
    ,PostRetType                   (e_obsolete_same_as_initial)
    ,PostRetAmt                    (0.0)
    ,PostRetPct                    (1.0)
    ,IntRateTypeGA                 (e_netrate)
    ,IntRateTypeSA                 (e_grossrate)
    ,LoanIntRate                   (0.06)
    ,LoanRateType                  (e_fixed_loan_rate)
    ,ExperienceRatingInitialKFactor(1.0)
    ,ExperienceReserveRate         (0.02)
    ,InforceExperienceReserve      (0.0)
    ,OverrideExperienceReserveRate ("Yes")
    ,PayLoanInt                    ("No")
    ,WDToBasisThenLoan             ("No")
    ,AvgFund                       ("No")
    ,OverrideFundMgmtFee           ("No")
    ,FundChoiceType                (e_fund_selection)
    ,InputFundMgmtFee              (0.0)
    ,RunOrder                      (e_life_by_life)
    ,NumIdenticalLives             (1)
    ,UseExperienceRating           ("No")
    ,UsePartialMort                ("No")
    ,PartialMortTable              (e_default_part_mort_table)
    ,PartialMortTableMult          (1.0)
    ,InsdState                     (e_s_CT)
    ,SponsorState                  (e_s_CT)
    ,AgentFirstName                ("*** REQUIRED FIELD MISSING ***")
    ,AgentLastName                 ("") // Not meaningfully used.
    ,AgentAddr1                    ("*** REQUIRED FIELD MISSING ***")
    ,AgentCity                     ("*** REQUIRED FIELD MISSING ***")
    ,AgentState                    (e_s_CT)
    ,AgentID                       ("*** REQUIRED FIELD MISSING ***")
    ,EePremTableNum                (e_default_premium_table)
    ,EePremTableMult               (1.0)
    ,ErPremTableNum                (e_default_premium_table)
    ,ErPremTableMult               (1.0)
    ,WDBegYear                     (0)
    ,WDEndYear                     (0)
    ,LoanBegYear                   (0)
    ,LoanEndYear                   (0)
    ,DefnLifeIns                   (e_cvat)
    ,DefnMaterialChange            (e_earlier_of_increase_or_unnecessary_premium)
    ,AvoidMec                      (e_allow_mec)
    ,RetireesCanEnroll             ("No")
    ,GroupUWType                   (e_medical)
    ,BlendMortGender               ("No")
    ,BlendMortSmoking              ("No")
    ,MaleProportion                (1.0)
    ,NonsmokerProportion           (1.0)
    ,InitTermProportion            (0.0)
    ,TermAdj                       (e_adjust_term)
    ,EePremEndYear                 (0)
    ,ErPremEndYear                 (0)
    ,IncludeInComposite            ("Yes")
    ,AmortizePremLoad              ("No")
    ,InforceYear                   (0)
    ,InforceMonth                  (0)
    ,InforceAVGenAcct              (0.0)
    ,InforceAVSepAcct              (0.0)
    ,InforceAVRegLn                (0.0)
    ,InforceAVPrfLn                (0.0)
    ,InforceRegLnBal               (0.0)
    ,InforcePrfLnBal               (0.0)
    ,InforceCumNoLapsePrem         (0.0)
    ,InforceCumPmts                (0.0)
    ,ExtraAssetComp                (0.0)
    ,ExtraAssetCompDur             (0)
    ,ExtraPremComp                 (0.0)
    ,ExtraPremCompDur              (0)
    ,ExtraPolFee                   (0.0)
    ,Country                       (e_c_US)
    ,OverrideCOIMultiplier         ("No")
    ,CountryCOIMultiplier          (1.0)
    ,SurviveToType                 (e_survive_to_age)
    ,SurviveToYear                 (100)
    ,SurviveToAge                  (99)
    ,MaxNAAR                       (10000000.0)
    ,NonUSCorridor                 (1.0)
    ,HasChildRider                 ("No")
    ,ChildRiderAmount              (0.0)
    ,HasSpouseRider                ("No")
    ,SpouseRiderAmount             (0.0)
    ,SpouseIssueAge                (45)
    ,InforceTaxBasis               (0.0)
    ,InforceCumGlp                 (0.0)
    ,InforceGlp                    (0.0)
    ,InforceGsp                    (0.0)
    ,Inforce7pp                    (0.0)
    ,InforceIsMec                  ("No")
    ,InforceDcv                    (0.0)
    ,InforceDcvDb                  (0.0)
    ,InforceAvBeforeLastMc         (0.0)
    ,InforceHoneymoonValue         (0.0)
    ,InforceContractYear           (0)
    ,InforceContractMonth          (0)
    ,InforceLeastDeathBenefit      (0.0)
    ,StateOfJurisdiction           (e_s_CT)
    ,SalarySAPct                   (1.0)
    ,SalarySACap                   (100000.0)
    ,SalarySAOffset                (50000.0)
    ,HasHoneymoon                  ("No")
    ,PostHoneymoonSpread           (0.0)
    ,CreateSupplementalReport      ("No")
    ,SupplementalReportColumn00    ("[none]")
    ,SupplementalReportColumn01    ("[none]")
    ,SupplementalReportColumn02    ("[none]")
    ,SupplementalReportColumn03    ("[none]")
    ,SupplementalReportColumn04    ("[none]")
    ,SupplementalReportColumn05    ("[none]")
    ,SupplementalReportColumn06    ("[none]")
    ,SupplementalReportColumn07    ("[none]")
    ,SupplementalReportColumn08    ("[none]")
    ,SupplementalReportColumn09    ("[none]")
    ,SupplementalReportColumn10    ("[none]")
    ,SupplementalReportColumn11    ("[none]")
{
    Status.resize(NumberOfLives.value());
    Length = YearsToMaturity();

    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState
            )
        );
    GenAcctRate.resize(KludgeLength);
    std::vector<double> general_account_max_rate;
    temp_database->Query(general_account_max_rate, DB_MaxGenAcctRate);
    for(unsigned int j = 0; j < general_account_max_rate.size(); ++j)
        {
        GenAcctRate[j] = general_account_max_rate[j];
        }

    SepAcctRate    .assign(KludgeLength, r_curr_int_rate(      0.08 ));
    EePremium      .assign(KludgeLength, r_pmt          (      0.0  ));
    ErPremium      .assign(KludgeLength, r_pmt          (      0.0  ));
    EeMode         .assign(KludgeLength, e_mode         ( e_annual  ));
    ErMode         .assign(KludgeLength, e_mode         ( e_annual  ));
    Loan           .assign(KludgeLength, r_loan         (      0.0  ));
    WD             .assign(KludgeLength, r_wd           (      0.0  ));
    SpecAmt        .assign(KludgeLength, r_spec_amt     (      0.0  ));
    DBOpt          .assign(KludgeLength, e_dbopt        (e_option1  ));
    Salary         .assign(KludgeLength,                       0.0   );
    IntegralTerm   .assign(KludgeLength,                       0.0   );
    VectorAddonMonthlyCustodialFee   .assign(KludgeLength,     0.0   );
    VectorAddonCompOnAssets          .assign(KludgeLength,     0.0   );
    VectorAddonCompOnPremium         .assign(KludgeLength,     0.0   );
    VectorNonUsCorridorFactor        .assign(KludgeLength,     1.0   );
    VectorPartialMortalityMultiplier .assign(KludgeLength,     1.0   );
    VectorCurrentCoiMultiplier       .assign(KludgeLength,     1.0   );
    VectorCurrentCoiGrading          .assign(KludgeLength,     0.0   );
    VectorCashValueEnhancementRate   .assign(KludgeLength,     0.0   );
    VectorCorpTaxBracket             .assign(KludgeLength,     0.0   );
    VectorIndvTaxBracket             .assign(KludgeLength,     0.0   );
    VectorPolicyLevelFlatExtra       .assign(KludgeLength,     0.0   );

    VectorSpecifiedAmountStrategy .assign(KludgeLength, e_sa_strategy   ( e_sainputscalar ));
    VectorIndvPaymentStrategy     .assign(KludgeLength, e_pmt_strategy  ( e_pmtinputscalar));
    VectorCorpPaymentStrategy     .assign(KludgeLength, e_pmt_strategy  ( e_pmtinputscalar));
    VectorNewLoanStrategy         .assign(KludgeLength, e_loan_strategy ( e_loanasinput   ));
    VectorWithdrawalStrategy      .assign(KludgeLength, e_wd_strategy   ( e_wdasinput     ));

    VectorHoneymoonValueSpread       .assign(KludgeLength,     0.0   );

    VectorPremiumHistory          .assign(KludgeLength, r_pmt          (      0.0  ));
    VectorSpecamtHistory          .assign(KludgeLength, r_spec_amt     (      0.0  ));

    // FundAllocs has a different number of elements than other vectors.
    FundAllocs.assign(InputParms::NumberOfFunds, r_fund(0));

    // TODO ?? It seems terribly lame to do this in a default ctor.
    // Probably something like this should be done when a case is saved.
    // Reproducible problem:
    //   File | New | Illustration
    //   increase the issue age
    //   hit OK without focusing the Solve tab
    //   save the case
    //   close the document
    //   File | 1 (open most recently used file--the one just saved
    //   now the solve-to age is over 100
    switch(SolveTgtAtWhich.value())
        {
        case e_tgtatyear:
            {
            SolveTgtYear = SolveTgtTime.value();
            }
            break;
        case e_tgtatage:
            {
            SolveTgtYear = SolveTgtTime.value() - Status[0].IssueAge.value();
            }
            break;
        case e_tgtatret:
            {
            SolveTgtYear = Status[0].YearsToRetirement();
            }
            break;
        case e_tgtatend:
            {
            SolveTgtYear = YearsToMaturity();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveTgtAtWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(SolveFromWhich.value())
        {
        case e_fromyear:
            {
            SolveBegYear = SolveBegTime.value();
            }
            break;
        case e_fromage:
            {
            SolveBegYear =
                SolveBegTime.value()
              - Status[0].IssueAge.value()
              ;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveFromWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(SolveToWhich.value())
        {
        case e_toyear:
            {
int z = SolveEndTime.value();
if(0 <= z && z <= 100)
            SolveEndYear = SolveEndTime.value();
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toage:
            {
int z =
                SolveEndTime.value()
              - Status[0].IssueAge.value()
              ;
if(0 <= z && z <= 100)
            SolveEndYear =
                SolveEndTime.value()
              - Status[0].IssueAge.value()
              ;
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toret:
            {
int z = Status[0].YearsToRetirement();
if(0 <= z && z <= 100)
            SolveEndYear = Status[0].YearsToRetirement();
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toend:
            {
int z = YearsToMaturity();
if(0 <= z && z <= 100)
            SolveEndYear = YearsToMaturity();
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveToWhich.value()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    EnforceConsistency();
}

//============================================================================
InputParms::~InputParms()
{
}

//============================================================================
bool InputParms::operator==(InputParms const& z) const
{
    if(ProductName                      != z.ProductName                     ) return false;
    if(Plan                             != z.Plan                            ) return false;
    if(NumberOfLives                    != z.NumberOfLives                   ) return false;
    if(Status                           != z.Status                          ) return false;
    if(FundAllocs                       != z.FundAllocs                      ) return false;
    if(GenAcctRate                      != z.GenAcctRate                     ) return false;
    if(SepAcctRate                      != z.SepAcctRate                     ) return false;
    if(EePremium                        != z.EePremium                       ) return false;
    if(ErPremium                        != z.ErPremium                       ) return false;
    if(EeMode                           != z.EeMode                          ) return false;
    if(ErMode                           != z.ErMode                          ) return false;
    if(Loan                             != z.Loan                            ) return false;
    if(WD                               != z.WD                              ) return false;
    if(SpecAmt                          != z.SpecAmt                         ) return false;
    if(DBOpt                            != z.DBOpt                           ) return false;
    if(IntegralTerm                     != z.IntegralTerm                    ) return false;
    if(VectorAddonMonthlyCustodialFee   != z.VectorAddonMonthlyCustodialFee  ) return false;
    if(VectorAddonCompOnAssets          != z.VectorAddonCompOnAssets         ) return false;
    if(VectorAddonCompOnPremium         != z.VectorAddonCompOnPremium        ) return false;
    if(VectorNonUsCorridorFactor        != z.VectorNonUsCorridorFactor       ) return false;
    if(VectorPartialMortalityMultiplier != z.VectorPartialMortalityMultiplier) return false;
    if(VectorCurrentCoiMultiplier       != z.VectorCurrentCoiMultiplier      ) return false;
    if(VectorCurrentCoiGrading          != z.VectorCurrentCoiGrading         ) return false;
    if(VectorCashValueEnhancementRate   != z.VectorCashValueEnhancementRate  ) return false;
    if(VectorCorpTaxBracket             != z.VectorCorpTaxBracket            ) return false;
    if(VectorIndvTaxBracket             != z.VectorIndvTaxBracket            ) return false;
    if(VectorPolicyLevelFlatExtra       != z.VectorPolicyLevelFlatExtra      ) return false;
    if(VectorSpecifiedAmountStrategy    != z.VectorSpecifiedAmountStrategy   ) return false;
    if(VectorIndvPaymentStrategy        != z.VectorIndvPaymentStrategy       ) return false;
    if(VectorCorpPaymentStrategy        != z.VectorCorpPaymentStrategy       ) return false;
    if(VectorNewLoanStrategy            != z.VectorNewLoanStrategy           ) return false;
    if(VectorWithdrawalStrategy         != z.VectorWithdrawalStrategy        ) return false;
    if(VectorHoneymoonValueSpread       != z.VectorHoneymoonValueSpread      ) return false;
    if(VectorPremiumHistory             != z.VectorPremiumHistory            ) return false;
    if(VectorSpecamtHistory             != z.VectorSpecamtHistory            ) return false;

    if(Dumpin                           != z.Dumpin                          ) return false;
    if(External1035ExchangeAmount       != z.External1035ExchangeAmount      ) return false;
    if(External1035ExchangeBasis        != z.External1035ExchangeBasis       ) return false;
    if(External1035ExchangeFromMec      != z.External1035ExchangeFromMec     ) return false;
    if(Internal1035ExchangeAmount       != z.Internal1035ExchangeAmount      ) return false;
    if(Internal1035ExchangeBasis        != z.Internal1035ExchangeBasis       ) return false;
    if(Internal1035ExchangeFromMec      != z.Internal1035ExchangeFromMec     ) return false;

    if(SolveTgtAtWhich                  != z.SolveTgtAtWhich                 ) return false;
    if(SolveTgtTime                     != z.SolveTgtTime                    ) return false;
    if(SolveFromWhich                   != z.SolveFromWhich                  ) return false;
    if(SolveBegTime                     != z.SolveBegTime                    ) return false;
    if(SolveToWhich                     != z.SolveToWhich                    ) return false;
    if(SolveEndTime                     != z.SolveEndTime                    ) return false;
    if(SolveType                        != z.SolveType                       ) return false;
    if(SolveBegYear                     != z.SolveBegYear                    ) return false;
    if(SolveEndYear                     != z.SolveEndYear                    ) return false;
    if(SolveTarget                      != z.SolveTarget                     ) return false;
    if(SolveTgtCSV                      != z.SolveTgtCSV                     ) return false;
    if(SolveTgtYear                     != z.SolveTgtYear                    ) return false;
    if(SolveBasis                       != z.SolveBasis                      ) return false;
    if(SolveSABasis                     != z.SolveSABasis                    ) return false;
    if(EePmtStrategy                    != z.EePmtStrategy                   ) return false;
    if(ErPmtStrategy                    != z.ErPmtStrategy                   ) return false;
    if(SAStrategy                       != z.SAStrategy                      ) return false;
    if(PostRetType                      != z.PostRetType                     ) return false;
    if(PostRetAmt                       != z.PostRetAmt                      ) return false;
    if(PostRetPct                       != z.PostRetPct                      ) return false;
    if(IntRateTypeGA                    != z.IntRateTypeGA                   ) return false;
    if(IntRateTypeSA                    != z.IntRateTypeSA                   ) return false;
    if(LoanIntRate                      != z.LoanIntRate                     ) return false;
    if(LoanRateType                     != z.LoanRateType                    ) return false;
    if(ExperienceRatingInitialKFactor   != z.ExperienceRatingInitialKFactor  ) return false;
    if(ExperienceReserveRate            != z.ExperienceReserveRate           ) return false;
    if(InforceExperienceReserve         != z.InforceExperienceReserve        ) return false;
    if(OverrideExperienceReserveRate    != z.OverrideExperienceReserveRate   ) return false;
    if(PayLoanInt                       != z.PayLoanInt                      ) return false;
    if(WDToBasisThenLoan                != z.WDToBasisThenLoan               ) return false;
    if(AvgFund                          != z.AvgFund                         ) return false;
    if(OverrideFundMgmtFee              != z.OverrideFundMgmtFee             ) return false;
    if(FundChoiceType                   != z.FundChoiceType                  ) return false;
    if(InputFundMgmtFee                 != z.InputFundMgmtFee                ) return false;
    if(RunOrder                         != z.RunOrder                        ) return false;
    if(NumIdenticalLives                != z.NumIdenticalLives               ) return false;
    if(UseExperienceRating              != z.UseExperienceRating             ) return false;
    if(UsePartialMort                   != z.UsePartialMort                  ) return false;
    if(PartialMortTable                 != z.PartialMortTable                ) return false;
    if(PartialMortTableMult             != z.PartialMortTableMult            ) return false;
    if(InsdFirstName                    != z.InsdFirstName                   ) return false;
    if(InsdMiddleName                   != z.InsdMiddleName                  ) return false;
    if(InsdLastName                     != z.InsdLastName                    ) return false;
    if(InsdAddr1                        != z.InsdAddr1                       ) return false;
    if(InsdCity                         != z.InsdCity                        ) return false;
    if(InsdState                        != z.InsdState                       ) return false;
    if(InsdZipCode                      != z.InsdZipCode                     ) return false;
    if(InsdEeClass                      != z.InsdEeClass                     ) return false;
    if(SponsorFirstName                 != z.SponsorFirstName                ) return false;
    if(SponsorAddr1                     != z.SponsorAddr1                    ) return false;
    if(SponsorCity                      != z.SponsorCity                     ) return false;
    if(SponsorState                     != z.SponsorState                    ) return false;
    if(SponsorZipCode                   != z.SponsorZipCode                  ) return false;
    if(SponsorTaxpayerID                != z.SponsorTaxpayerID               ) return false;
    if(AgentFirstName                   != z.AgentFirstName                  ) return false;
    if(AgentMiddleName                  != z.AgentMiddleName                 ) return false;
    if(AgentLastName                    != z.AgentLastName                   ) return false;
    if(AgentAddr1                       != z.AgentAddr1                      ) return false;
    if(AgentCity                        != z.AgentCity                       ) return false;
    if(AgentState                       != z.AgentState                      ) return false;
    if(AgentZipCode                     != z.AgentZipCode                    ) return false;
    if(AgentPhone                       != z.AgentPhone                      ) return false;
    if(AgentID                          != z.AgentID                         ) return false;
    if(EePremTableNum                   != z.EePremTableNum                  ) return false;
    if(EePremTableMult                  != z.EePremTableMult                 ) return false;
    if(ErPremTableNum                   != z.ErPremTableNum                  ) return false;
    if(ErPremTableMult                  != z.ErPremTableMult                 ) return false;
    if(WDBegYear                        != z.WDBegYear                       ) return false;
    if(WDEndYear                        != z.WDEndYear                       ) return false;
    if(LoanBegYear                      != z.LoanBegYear                     ) return false;
    if(LoanEndYear                      != z.LoanEndYear                     ) return false;
    if(EffDate                          != z.EffDate                         ) return false;
    if(DefnLifeIns                      != z.DefnLifeIns                     ) return false;
    if(DefnMaterialChange               != z.DefnMaterialChange              ) return false;
    if(AvoidMec                         != z.AvoidMec                        ) return false;
    if(RetireesCanEnroll                != z.RetireesCanEnroll               ) return false;
    if(GroupUWType                      != z.GroupUWType                     ) return false;
    if(BlendMortGender                  != z.BlendMortGender                 ) return false;
    if(BlendMortSmoking                 != z.BlendMortSmoking                ) return false;
    if(MaleProportion                   != z.MaleProportion                  ) return false;
    if(NonsmokerProportion              != z.NonsmokerProportion             ) return false;
    if(InitTermProportion               != z.InitTermProportion              ) return false;
    if(TermAdj                          != z.TermAdj                         ) return false;
    if(EePremEndYear                    != z.EePremEndYear                   ) return false;
    if(ErPremEndYear                    != z.ErPremEndYear                   ) return false;
    if(IncludeInComposite               != z.IncludeInComposite              ) return false;
    if(Comments                         != z.Comments                        ) return false;
    if(AmortizePremLoad                 != z.AmortizePremLoad                ) return false;
    if(InforceYear                      != z.InforceYear                     ) return false;
    if(InforceMonth                     != z.InforceMonth                    ) return false;
    if(InforceAVGenAcct                 != z.InforceAVGenAcct                ) return false;
    if(InforceAVSepAcct                 != z.InforceAVSepAcct                ) return false;
    if(InforceAVRegLn                   != z.InforceAVRegLn                  ) return false;
    if(InforceAVPrfLn                   != z.InforceAVPrfLn                  ) return false;
    if(InforceRegLnBal                  != z.InforceRegLnBal                 ) return false;
    if(InforcePrfLnBal                  != z.InforcePrfLnBal                 ) return false;
    if(InforceCumNoLapsePrem            != z.InforceCumNoLapsePrem           ) return false;
    if(InforceCumPmts                   != z.InforceCumPmts                  ) return false;
    if(ExtraAssetComp                   != z.ExtraAssetComp                  ) return false;
    if(ExtraAssetCompDur                != z.ExtraAssetCompDur               ) return false;
    if(ExtraPremComp                    != z.ExtraPremComp                   ) return false;
    if(ExtraPremCompDur                 != z.ExtraPremCompDur                ) return false;
    if(ExtraPolFee                      != z.ExtraPolFee                     ) return false;
    if(Country                          != z.Country                         ) return false;
    if(OverrideCOIMultiplier            != z.OverrideCOIMultiplier           ) return false;
    if(CountryCOIMultiplier             != z.CountryCOIMultiplier            ) return false;
    if(SurviveToType                    != z.SurviveToType                   ) return false;
    if(SurviveToYear                    != z.SurviveToYear                   ) return false;
    if(SurviveToAge                     != z.SurviveToAge                    ) return false;
    if(MaxNAAR                          != z.MaxNAAR                         ) return false;
    if(NonUSCorridor                    != z.NonUSCorridor                   ) return false;
    if(HasChildRider                    != z.HasChildRider                   ) return false;
    if(ChildRiderAmount                 != z.ChildRiderAmount                ) return false;
    if(HasSpouseRider                   != z.HasSpouseRider                  ) return false;
    if(SpouseRiderAmount                != z.SpouseRiderAmount               ) return false;
    if(SpouseIssueAge                   != z.SpouseIssueAge                  ) return false;
    if(Franchise                        != z.Franchise                       ) return false;
    if(PolicyNumber                     != z.PolicyNumber                    ) return false;
    if(PolicyDate                       != z.PolicyDate                      ) return false;
    if(InforceTaxBasis                  != z.InforceTaxBasis                 ) return false;
    if(InforceCumGlp                    != z.InforceCumGlp                   ) return false;
    if(InforceGlp                       != z.InforceGlp                      ) return false;
    if(InforceGsp                       != z.InforceGsp                      ) return false;
    if(Inforce7pp                       != z.Inforce7pp                      ) return false;
    if(InforceIsMec                     != z.InforceIsMec                    ) return false;
    if(LastMatChgDate                   != z.LastMatChgDate                  ) return false;
    if(InforceDcv                       != z.InforceDcv                      ) return false;
    if(InforceDcvDb                     != z.InforceDcvDb                    ) return false;
    if(InforceAvBeforeLastMc            != z.InforceAvBeforeLastMc           ) return false;
    if(InforceHoneymoonValue            != z.InforceHoneymoonValue           ) return false;
    if(InforceContractYear              != z.InforceContractYear             ) return false;
    if(InforceContractMonth             != z.InforceContractMonth            ) return false;
    if(InforceLeastDeathBenefit         != z.InforceLeastDeathBenefit        ) return false;
    if(StateOfJurisdiction              != z.StateOfJurisdiction             ) return false;
    if(SalarySAPct                      != z.SalarySAPct                     ) return false;
    if(SalarySACap                      != z.SalarySACap                     ) return false;
    if(SalarySAOffset                   != z.SalarySAOffset                  ) return false;
    if(HasHoneymoon                     != z.HasHoneymoon                    ) return false;
    if(PostHoneymoonSpread              != z.PostHoneymoonSpread             ) return false;

    return true;
}

//============================================================================
int InputParms::YearsToMaturity() const
{
    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState
            )
        );
    // The database class constrains endowment age to be scalar.
    int endt_age = static_cast<int>(temp_database->Query(DB_EndtAge));

    return endt_age - Status[0].IssueAge.value();
}

//============================================================================
int InputParms::SumOfSepAcctFundAllocs() const
{
    return std::accumulate(FundAllocs.begin(), FundAllocs.end(), 0);
}

//============================================================================
void InputParms::ResetAllFunds(bool GeneralAccountAllowed)
{
    FundAllocs.assign(InputParms::NumberOfFunds, r_fund(0));
    // If it is not allowed to allocate funds to the general account,
    // then allocate all funds to the first separate account by default.
    //
    // TODO ?? But if it is allowed to allocate funds to the general account,
    // then what happens? I suppose the code (elsewhere) does the right
    // thing, but the above comment suggests that there's a defect here.
    if(!GeneralAccountAllowed)
        {
        FundAllocs[0] = 100;
        }
}

//============================================================================
// A const Set- function requires explanation. The dialog has one overloaded
// field for input age or duration in several cases. This function resolves
// the input to a specific age or duration variable based on another variable
// indicating which was intended. It therefore needs to modify these variables
// even when they are called on a const object. This indicates a regrettable
// design. TODO ?? Separate input fields would be simpler in this and other
// ways. TODO ?? The name lies about what this function does.
void InputParms::SetSolveDurations() const
{
    // TODO ?? Strictly speaking, these should be mutable? or const_cast?
    r_solve_tgt_year& mutable_solve_tgt_year = const_cast<r_solve_tgt_year&>(SolveTgtYear);
    r_solve_beg_year& mutable_solve_beg_year = const_cast<r_solve_beg_year&>(SolveBegYear);
    r_solve_end_year& mutable_solve_end_year = const_cast<r_solve_end_year&>(SolveEndYear);

    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState // TODO ?? This is wrong if endt age varies by state.
            )
        );
    int endt_age = static_cast<int>(temp_database->Query(DB_EndtAge));

    switch(SolveTgtAtWhich)
        {
        case e_tgtatyear:
            {
            mutable_solve_tgt_year = SolveTgtTime;
            }
            break;
        case e_tgtatage:
            {
            mutable_solve_tgt_year = SolveTgtTime - Status[0].IssueAge;
            }
            break;
        case e_tgtatret:
            {
            mutable_solve_tgt_year = Status[0].YearsToRetirement();
            }
            break;
        case e_tgtatend:
            {
            mutable_solve_tgt_year = endt_age - Status[0].IssueAge;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveTgtAtWhich
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    if(endt_age < Status[0].IssueAge + mutable_solve_tgt_year)
        {
        fatal_error()
            << "Solve target year exceeds maturity year."
            << LMI_FLUSH
            ;
        }

    switch(SolveFromWhich)
        {
        case e_fromyear:
            {
            mutable_solve_beg_year = SolveBegTime;
            }
            break;
        case e_fromage:
            {
            mutable_solve_beg_year = SolveBegTime - Status[0].IssueAge;
            }
            break;
        case e_fromissue:
            {
            mutable_solve_beg_year = 0;
            }
            break;
        case e_fromret:
            {
            mutable_solve_beg_year = Status[0].YearsToRetirement();
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveFromWhich
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    switch(SolveToWhich)
        {
        case e_toyear:
            {
int z = SolveEndTime;
if(0 <= z && z <= 100)
            mutable_solve_end_year = SolveEndTime;
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toage:
            {
int z = SolveEndTime - Status[0].IssueAge;
if(0 <= z && z <= 100)
            mutable_solve_end_year = SolveEndTime - Status[0].IssueAge;
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toret:
            {
int z = Status[0].YearsToRetirement();
if(0 <= z && z <= 100)
            mutable_solve_end_year = Status[0].YearsToRetirement();
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        case e_toend:
            {
int z = endt_age - Status[0].IssueAge;
if(0 <= z && z <= 100)
            mutable_solve_end_year = endt_age - Status[0].IssueAge;
else warning() << "Solve end year out of range." << LMI_FLUSH;
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << SolveToWhich
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
}

// TODO ?? This is a crock. We should assign 'ProductName' early so that
// we can query the database. We want to do that before we set 'Length',
// which ought to reflect maturity duration (even though it doesn't, it
// really ought to), and also to find out whether the product is ALB or
// ANB before we make age consistent with DOB.
//============================================================================
void InputParms::EnforceConsistency()
{
    if(0 == ProductName.size())
        {
        throw std::runtime_error("Internal error: ProductName is empty.");
        }

    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState // TODO ?? This is wrong if endt age varies by state.
            )
        );
    bool use_anb = temp_database->Query(DB_AgeLastOrNearest);
    Status[0].MakeAgesAndDatesConsistent(EffDate, use_anb);

    Length = YearsToMaturity();
}

// Obsolete, but not harmful if the unit tests are powerful enough.

//============================================================================
std::string InputParms::AgentFullName() const
{
    std::string s(AgentFirstName);
    if(!s.empty() && !AgentMiddleName.empty())
        {
        s += " ";
        }
    s += AgentMiddleName;
    if(!s.empty() && !AgentLastName.empty())
        {
        s += " ";
        }
    s += AgentLastName;
    return s;
}

//============================================================================
std::string InputParms::InsdFullName() const
{
    std::string s(InsdFirstName);
    if(!s.empty() && !InsdMiddleName.empty())
        {
        s += " ";
        }
    s += InsdMiddleName;
    if(!s.empty() && !InsdLastName.empty())
        {
        s += " ";
        }
    s += InsdLastName;
    return s;
}

//============================================================================
bool InputParms::NeedLoanRates() const
{
    bool need_loan_rates =
            e_solve_loan         == SolveType
        ||  e_solve_wd_then_loan == SolveType
        ||  true                 == WDToBasisThenLoan
        ||  0.0                  != InforceAVRegLn
        ||  0.0                  != InforceAVPrfLn
        ||  0.0                  != InforceRegLnBal
        ||  0.0                  != InforcePrfLnBal
        ;
    for(int j = 0; !need_loan_rates && j < Length; j++)
        {
        need_loan_rates = need_loan_rates || Loan[j];
        }
    return need_loan_rates;
}

//============================================================================
// Make sure death benefit option obeys constraint:
// change to option B conditionally not allowed.
bool InputParms::CheckAllowChangeToDBO2() const
{
    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState // TODO ?? This is wrong if endt age varies by state.
            )
        );
    if(temp_database->Query(DB_AllowChangeToDBO2))
        {
        return true;
        }
    else if
        (nonstd::is_sorted_
            (DBOpt.begin()
            ,DBOpt.end()
            ,boost::bind
                (std::logical_and<bool>()
                ,boost::bind(std::equal_to<e_dbopt>(), _1, e_dbopt("B"))
                ,boost::bind(std::not_equal_to<e_dbopt>(), _2, e_dbopt("B"))
                )
            )
        )
        {
        return true;
        }
    else
        {
        return false;
        }
}

//============================================================================
// Make sure death benefit option obeys constraint:
// ROP conditionally not allowed.
bool InputParms::CheckAllowDBO3() const
{
    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState // TODO ?? This is wrong if endt age varies by state.
            )
        );
    if(temp_database->Query(DB_AllowDBO3))
        {
        return true;
        }
    else if
        (DBOpt.end() == std::find
            (DBOpt.begin()
            ,DBOpt.end()
            ,e_dbopt("ROP")
            )
        )
        {
        return true;
        }
    else
        {
        return false;
        }
}

e_ledger_type InputParms::LedgerType() const
{
// TODO ?? This class should use a smart-pointer member instead of
// creating objects like this repeatedly.
    boost::scoped_ptr<TDatabase> temp_database
        (new TDatabase
            (ProductName
            ,Status[0].Gender
            ,Status[0].Class
            ,Status[0].Smoking
            ,Status[0].IssueAge
            ,GroupUWType
            ,InsdState // TODO ?? This is wrong if endt age varies by state.
            )
        );
    return e_ledger_type
        (static_cast<enum_ledger_type>
            (static_cast<int>
                (temp_database->Query(DB_LedgerType))
            )
        );
}

