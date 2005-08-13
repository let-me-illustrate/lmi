// Life insurance illustration inputs.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: inputillus.cpp,v 1.8 2005-08-13 23:45:42 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputillus.hpp"

#include "alert.hpp"

#include <fstream>
#include <istream>
#include <iterator>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

//============================================================================
IllusInputParms::IllusInputParms()
    :InputParms()
    ,AddonMonthlyCustodialFee   ("0")
    ,AddonCompOnAssets          ("0")
    ,AddonCompOnPremium         ("0")
    ,NonUsCorridorFactor        ("1")
    ,PartialMortalityMultiplier ("1")
    ,CurrentCoiMultiplier       ("1")
    ,CurrentCoiGrading          ("0")
    ,CashValueEnhancementRate   ("0")
    ,CorpTaxBracket             ("0")
    ,IndvTaxBracket             ("0")
    ,ProjectedSalary            ("100000")
    ,SpecifiedAmount            ("1000000")
    ,DeathBenefitOption         ("a")
    ,IndvPayment                ("0")
    ,IndvPaymentMode            ("annual")
    ,CorpPayment                ("20000")
    ,CorpPaymentMode            ("annual")
    ,GenAcctIntRate             (current_credited_rate())
    ,SepAcctIntRate             (".08")
    ,NewLoan                    ("0")
    ,Withdrawal                 ("0")
    ,FlatExtra                  ("0")
    ,PolicyLevelFlatExtra       ("0")
    ,HoneymoonValueSpread       ("0")
    ,PremiumHistory             ("0")
    ,SpecamtHistory             ("0")
    ,FundAllocations            ("0")
    ,EffectiveDateToday               ("No")
    ,DeathBenefitOptionFromRetirement ("A")
    ,DeathBenefitOptionFromIssue      ("A")
    ,SpecifiedAmountFromRetirement    (1000000)
    ,SpecifiedAmountFromIssue         (1000000)
//    mce_                     SpecifiedAmountStrategyFromRetirement () // TODO ?? Rethink this.
    ,SpecifiedAmountStrategyFromIssue ("SAInputScalar")
    ,IndividualPaymentMode            ("Annual")
    ,IndividualPaymentToAlternative   (e_kludge_toend)
    ,IndividualPaymentToAge           (95)
    ,IndividualPaymentToDuration      (50)
    ,IndividualPaymentAmount          (0)
    ,IndividualPaymentStrategy        ("PmtInputScalar")
    ,LoanAmount                       (0)
    ,LoanFromAge                      (45)
    ,LoanFromAlternative              (e_kludge_fromret)
    ,LoanFromDuration                 (20)
    ,LoanToAge                        (95)
    ,LoanToAlternative                (e_kludge_toend)
    ,LoanToDuration                   (50)
    ,WithdrawalAmount                 (0)
    ,WithdrawalFromAge                (45)
    ,WithdrawalFromAlternative        (e_kludge_fromret)
    ,WithdrawalFromDuration           (20)
    ,WithdrawalToAge                  (95)
    ,WithdrawalToAlternative          (e_kludge_toend)
    ,WithdrawalToDuration             (50)

    ,sSpecAmount                (1000000.0)
    ,sDBOpt                     (e_option1)
    ,sRetDBOpt                  (e_option1)
    ,sEePremium                 (0.0)
    ,sEeMode                    (e_annual)
{
    propagate_changes_from_base_and_finalize();
    ascribe_members();
// TODO ?? Debugging--expunge
if(Status_Smoking != Status[0].Smoking)
  {
  hobsons_choice() << "Default constructor defective." << LMI_FLUSH;
  }
}

//============================================================================
// TODO ?? Class 'streamable' seems to have no such ctor:
IllusInputParms::IllusInputParms(IllusInputParms const& z)
    :streamable(z)
    ,InputParms(z)
    ,MemberSymbolTable<IllusInputParms>()
{
    AddonMonthlyCustodialFee   = z.AddonMonthlyCustodialFee   ;
    AddonCompOnAssets          = z.AddonCompOnAssets          ;
    AddonCompOnPremium         = z.AddonCompOnPremium         ;
    NonUsCorridorFactor        = z.NonUsCorridorFactor        ;
    PartialMortalityMultiplier = z.PartialMortalityMultiplier ;
    CurrentCoiMultiplier       = z.CurrentCoiMultiplier       ;
    CurrentCoiGrading          = z.CurrentCoiGrading          ;
    CashValueEnhancementRate   = z.CashValueEnhancementRate   ;
    CorpTaxBracket             = z.CorpTaxBracket             ;
    IndvTaxBracket             = z.IndvTaxBracket             ;
    ProjectedSalary            = z.ProjectedSalary            ;
    SpecifiedAmount            = z.SpecifiedAmount            ;
    DeathBenefitOption         = z.DeathBenefitOption         ;
    IndvPayment                = z.IndvPayment                ;
    IndvPaymentMode            = z.IndvPaymentMode            ;
    CorpPayment                = z.CorpPayment                ;
    CorpPaymentMode            = z.CorpPaymentMode            ;
    GenAcctIntRate             = z.GenAcctIntRate             ;
    SepAcctIntRate             = z.SepAcctIntRate             ;
    NewLoan                    = z.NewLoan                    ;
    Withdrawal                 = z.Withdrawal                 ;
    FlatExtra                  = z.FlatExtra                  ;
    PolicyLevelFlatExtra       = z.PolicyLevelFlatExtra       ;
    HoneymoonValueSpread       = z.HoneymoonValueSpread       ;
    PremiumHistory             = z.PremiumHistory             ;
    SpecamtHistory             = z.SpecamtHistory             ;
    FundAllocations            = z.FundAllocations            ;

    CreateSupplementalReport   = z.CreateSupplementalReport   ;
    SupplementalReportColumn00 = z.SupplementalReportColumn00 ;
    SupplementalReportColumn01 = z.SupplementalReportColumn01 ;
    SupplementalReportColumn02 = z.SupplementalReportColumn02 ;
    SupplementalReportColumn03 = z.SupplementalReportColumn03 ;
    SupplementalReportColumn04 = z.SupplementalReportColumn04 ;
    SupplementalReportColumn05 = z.SupplementalReportColumn05 ;
    SupplementalReportColumn06 = z.SupplementalReportColumn06 ;
    SupplementalReportColumn07 = z.SupplementalReportColumn07 ;
    SupplementalReportColumn08 = z.SupplementalReportColumn08 ;
    SupplementalReportColumn09 = z.SupplementalReportColumn09 ;
    SupplementalReportColumn10 = z.SupplementalReportColumn10 ;
    SupplementalReportColumn11 = z.SupplementalReportColumn11 ;

    EffectiveDateToday             = z.EffectiveDateToday                ;
    DeathBenefitOptionFromRetirement = z.DeathBenefitOptionFromRetirement ;
    DeathBenefitOptionFromIssue    = z.DeathBenefitOptionFromIssue      ;
    SpecifiedAmountFromRetirement  = z.SpecifiedAmountFromRetirement    ;
    SpecifiedAmountFromIssue       = z.SpecifiedAmountFromIssue         ;
//    SpecifiedAmountStrategyFromRetirement , &IllusInputParms:: // TODO ?? Rethink this.
    SpecifiedAmountStrategyFromIssue = z.IllusInputParms::SpecifiedAmountStrategyFromIssue ;
    IndividualPaymentMode          = z.IndividualPaymentMode            ;
    IndividualPaymentToAlternative = z.IndividualPaymentToAlternative   ;
    IndividualPaymentToAge         = z.IndividualPaymentToAge           ;
    IndividualPaymentToDuration    = z.IndividualPaymentToDuration      ;
    IndividualPaymentAmount        = z.IndividualPaymentAmount          ;
    IndividualPaymentStrategy      = z.IndividualPaymentStrategy        ;
    LoanAmount                     = z.LoanAmount                       ;
    LoanFromAge                    = z.LoanFromAge                      ;
    LoanFromAlternative            = z.LoanFromAlternative              ;
    LoanFromDuration               = z.LoanFromDuration                 ;
    LoanToAge                      = z.LoanToAge                        ;
    LoanToAlternative              = z.LoanToAlternative                ;
    LoanToDuration                 = z.LoanToDuration                   ;
    WithdrawalAmount               = z.WithdrawalAmount                 ;
    WithdrawalFromAge              = z.WithdrawalFromAge                ;
    WithdrawalFromAlternative      = z.WithdrawalFromAlternative        ;
    WithdrawalFromDuration         = z.WithdrawalFromDuration           ;
    WithdrawalToAge                = z.WithdrawalToAge                  ;
    WithdrawalToAlternative        = z.WithdrawalToAlternative          ;
    WithdrawalToDuration           = z.WithdrawalToDuration             ;

// TODO ?? Obsolescent.
    sSpecAmount                = z.sSpecAmount                ;
    sDBOpt                     = z.sDBOpt                     ;
    sRetDBOpt                  = z.sRetDBOpt                  ;
    sEePremium                 = z.sEePremium                 ;
    sEeMode                    = z.sEeMode                    ;
    sGenAcctRate               = z.sGenAcctRate               ;

    propagate_changes_from_base_and_finalize();
    ascribe_members();
// TODO ?? Debugging--expunge
if(Status[0].Smoking != z.Status[0].Smoking)
  {
  hobsons_choice() << "Copy constructor defective." << LMI_FLUSH;
  }
if(Status_Smoking != Status[0].Smoking)
  {
  hobsons_choice() << "Copy constructor defective." << LMI_FLUSH;
  }
if(Status_Smoking != z.Status_Smoking)
  {
  hobsons_choice() << "Copy constructor defective." << LMI_FLUSH;
  }
}

//============================================================================
IllusInputParms::~IllusInputParms()
{
}

//============================================================================
IllusInputParms& IllusInputParms::operator=(IllusInputParms const& z)
{
    dynamic_cast<InputParms&>(*this) = z;

    AddonMonthlyCustodialFee   = z.AddonMonthlyCustodialFee   ;
    AddonCompOnAssets          = z.AddonCompOnAssets          ;
    AddonCompOnPremium         = z.AddonCompOnPremium         ;
    NonUsCorridorFactor        = z.NonUsCorridorFactor        ;
    PartialMortalityMultiplier = z.PartialMortalityMultiplier ;
    CurrentCoiMultiplier       = z.CurrentCoiMultiplier       ;
    CurrentCoiGrading          = z.CurrentCoiGrading          ;
    CashValueEnhancementRate   = z.CashValueEnhancementRate   ;
    CorpTaxBracket             = z.CorpTaxBracket             ;
    IndvTaxBracket             = z.IndvTaxBracket             ;
    ProjectedSalary            = z.ProjectedSalary            ;
    SpecifiedAmount            = z.SpecifiedAmount            ;
    DeathBenefitOption         = z.DeathBenefitOption         ;
    IndvPayment                = z.IndvPayment                ;
    IndvPaymentMode            = z.IndvPaymentMode            ;
    CorpPayment                = z.CorpPayment                ;
    CorpPaymentMode            = z.CorpPaymentMode            ;
    GenAcctIntRate             = z.GenAcctIntRate             ;
    SepAcctIntRate             = z.SepAcctIntRate             ;
    NewLoan                    = z.NewLoan                    ;
    Withdrawal                 = z.Withdrawal                 ;
    FlatExtra                  = z.FlatExtra                  ;
    PolicyLevelFlatExtra       = z.PolicyLevelFlatExtra       ;
    HoneymoonValueSpread       = z.HoneymoonValueSpread       ;
    PremiumHistory             = z.PremiumHistory             ;
    SpecamtHistory             = z.SpecamtHistory             ;
    FundAllocations            = z.FundAllocations            ;

    CreateSupplementalReport   = z.CreateSupplementalReport   ;
    SupplementalReportColumn00 = z.SupplementalReportColumn00 ;
    SupplementalReportColumn01 = z.SupplementalReportColumn01 ;
    SupplementalReportColumn02 = z.SupplementalReportColumn02 ;
    SupplementalReportColumn03 = z.SupplementalReportColumn03 ;
    SupplementalReportColumn04 = z.SupplementalReportColumn04 ;
    SupplementalReportColumn05 = z.SupplementalReportColumn05 ;
    SupplementalReportColumn06 = z.SupplementalReportColumn06 ;
    SupplementalReportColumn07 = z.SupplementalReportColumn07 ;
    SupplementalReportColumn08 = z.SupplementalReportColumn08 ;
    SupplementalReportColumn09 = z.SupplementalReportColumn09 ;
    SupplementalReportColumn10 = z.SupplementalReportColumn10 ;
    SupplementalReportColumn11 = z.SupplementalReportColumn11 ;

    EffectiveDateToday             = z.EffectiveDateToday                ;
    DeathBenefitOptionFromRetirement = z.DeathBenefitOptionFromRetirement ;
    DeathBenefitOptionFromIssue    = z.DeathBenefitOptionFromIssue      ;
    SpecifiedAmountFromRetirement  = z.SpecifiedAmountFromRetirement    ;
    SpecifiedAmountFromIssue       = z.SpecifiedAmountFromIssue         ;
//    SpecifiedAmountStrategyFromRetirement , &IllusInputParms:: // TODO ?? Rethink this.
    SpecifiedAmountStrategyFromIssue = z.IllusInputParms::SpecifiedAmountStrategyFromIssue ;
    IndividualPaymentMode          = z.IndividualPaymentMode            ;
    IndividualPaymentToAlternative = z.IndividualPaymentToAlternative   ;
    IndividualPaymentToAge         = z.IndividualPaymentToAge           ;
    IndividualPaymentToDuration    = z.IndividualPaymentToDuration      ;
    IndividualPaymentAmount        = z.IndividualPaymentAmount          ;
    IndividualPaymentStrategy      = z.IndividualPaymentStrategy        ;
    LoanAmount                     = z.LoanAmount                       ;
    LoanFromAge                    = z.LoanFromAge                      ;
    LoanFromAlternative            = z.LoanFromAlternative              ;
    LoanFromDuration               = z.LoanFromDuration                 ;
    LoanToAge                      = z.LoanToAge                        ;
    LoanToAlternative              = z.LoanToAlternative                ;
    LoanToDuration                 = z.LoanToDuration                   ;
    WithdrawalAmount               = z.WithdrawalAmount                 ;
    WithdrawalFromAge              = z.WithdrawalFromAge                ;
    WithdrawalFromAlternative      = z.WithdrawalFromAlternative        ;
    WithdrawalFromDuration         = z.WithdrawalFromDuration           ;
    WithdrawalToAge                = z.WithdrawalToAge                  ;
    WithdrawalToAlternative        = z.WithdrawalToAlternative          ;
    WithdrawalToDuration           = z.WithdrawalToDuration             ;

    propagate_changes_from_base_and_finalize();

// TODO ?? Obsolescent.
    sSpecAmount                = z.sSpecAmount                ;
    sDBOpt                     = z.sDBOpt                     ;
    sRetDBOpt                  = z.sRetDBOpt                  ;
    sEePremium                 = z.sEePremium                 ;
    sEeMode                    = z.sEeMode                    ;
    sGenAcctRate               = z.sGenAcctRate               ;

// TODO ?? Debugging--expunge
if(Status_Smoking != z.Status_Smoking)
  {
  hobsons_choice() << "operator=() defective." << LMI_FLUSH;
  }
if(Status[0].Smoking != z.Status[0].Smoking)
  {
  hobsons_choice() << "operator=() defective." << LMI_FLUSH;
  }
if(Status_Smoking != Status[0].Smoking)
  {
  hobsons_choice() << "operator=() defective." << LMI_FLUSH;
  }
    return *this;
}

//============================================================================
bool IllusInputParms::operator==(IllusInputParms const& z) const
{
// TODO ?? Debugging--expunge
if(Status_Smoking != Status[0].Smoking)
  {
  hobsons_choice() << "operator==() defective." << LMI_FLUSH;
  }
if(z.Status_Smoking != z.Status[0].Smoking)
  {
  hobsons_choice() << "operator==() defective." << LMI_FLUSH;
  }

    if(!InputParms::operator==(z)) return false;

    if(AddonMonthlyCustodialFee   != z.AddonMonthlyCustodialFee  ) return false;
    if(AddonCompOnAssets          != z.AddonCompOnAssets         ) return false;
    if(AddonCompOnPremium         != z.AddonCompOnPremium        ) return false;
    if(NonUsCorridorFactor        != z.NonUsCorridorFactor       ) return false;
    if(PartialMortalityMultiplier != z.PartialMortalityMultiplier) return false;
    if(CurrentCoiMultiplier       != z.CurrentCoiMultiplier      ) return false;
    if(CurrentCoiGrading          != z.CurrentCoiGrading         ) return false;
    if(CashValueEnhancementRate   != z.CashValueEnhancementRate  ) return false;
    if(CorpTaxBracket             != z.CorpTaxBracket            ) return false;
    if(IndvTaxBracket             != z.IndvTaxBracket            ) return false;
    if(ProjectedSalary            != z.ProjectedSalary           ) return false;
    if(SpecifiedAmount            != z.SpecifiedAmount           ) return false;
    if(DeathBenefitOption         != z.DeathBenefitOption        ) return false;
    if(IndvPayment                != z.IndvPayment               ) return false;
    if(IndvPaymentMode            != z.IndvPaymentMode           ) return false;
    if(CorpPayment                != z.CorpPayment               ) return false;
    if(CorpPaymentMode            != z.CorpPaymentMode           ) return false;
    if(GenAcctIntRate             != z.GenAcctIntRate            ) return false;
    if(SepAcctIntRate             != z.SepAcctIntRate            ) return false;
    if(NewLoan                    != z.NewLoan                   ) return false;
    if(Withdrawal                 != z.Withdrawal                ) return false;
    if(FlatExtra                  != z.FlatExtra                 ) return false;
    if(PolicyLevelFlatExtra       != z.PolicyLevelFlatExtra      ) return false;
    if(HoneymoonValueSpread       != z.HoneymoonValueSpread      ) return false;
    if(PremiumHistory             != z.PremiumHistory            ) return false;
    if(SpecamtHistory             != z.SpecamtHistory            ) return false;
    if(FundAllocations            != z.FundAllocations           ) return false;

    if(CreateSupplementalReport   != z.CreateSupplementalReport  ) return false;
    if(SupplementalReportColumn00 != z.SupplementalReportColumn00) return false;
    if(SupplementalReportColumn01 != z.SupplementalReportColumn01) return false;
    if(SupplementalReportColumn02 != z.SupplementalReportColumn02) return false;
    if(SupplementalReportColumn03 != z.SupplementalReportColumn03) return false;
    if(SupplementalReportColumn04 != z.SupplementalReportColumn04) return false;
    if(SupplementalReportColumn05 != z.SupplementalReportColumn05) return false;
    if(SupplementalReportColumn06 != z.SupplementalReportColumn06) return false;
    if(SupplementalReportColumn07 != z.SupplementalReportColumn07) return false;
    if(SupplementalReportColumn08 != z.SupplementalReportColumn08) return false;
    if(SupplementalReportColumn09 != z.SupplementalReportColumn09) return false;
    if(SupplementalReportColumn10 != z.SupplementalReportColumn10) return false;
    if(SupplementalReportColumn11 != z.SupplementalReportColumn11) return false;

    if(EffectiveDateToday             != z.EffectiveDateToday                ) return false;
    if(DeathBenefitOptionFromRetirement != z.DeathBenefitOptionFromRetirement ) return false;
    if(DeathBenefitOptionFromIssue    != z.DeathBenefitOptionFromIssue      ) return false;
    if(SpecifiedAmountFromRetirement  != z.SpecifiedAmountFromRetirement    ) return false;
    if(SpecifiedAmountFromIssue       != z.SpecifiedAmountFromIssue         ) return false;
//    SpecifiedAmountStrategyFromRetirement , &IllusInputParms:: // TODO ?? Rethink this.
    if(SpecifiedAmountStrategyFromIssue != z.IllusInputParms::SpecifiedAmountStrategyFromIssue ) return false;
    if(IndividualPaymentMode          != z.IndividualPaymentMode            ) return false;
    if(IndividualPaymentToAlternative != z.IndividualPaymentToAlternative   ) return false;
    if(IndividualPaymentToAge         != z.IndividualPaymentToAge           ) return false;
    if(IndividualPaymentToDuration    != z.IndividualPaymentToDuration      ) return false;
    if(IndividualPaymentAmount        != z.IndividualPaymentAmount          ) return false;
    if(IndividualPaymentStrategy      != z.IndividualPaymentStrategy        ) return false;
    if(LoanAmount                     != z.LoanAmount                       ) return false;
    if(LoanFromAge                    != z.LoanFromAge                      ) return false;
    if(LoanFromAlternative            != z.LoanFromAlternative              ) return false;
    if(LoanFromDuration               != z.LoanFromDuration                 ) return false;
    if(LoanToAge                      != z.LoanToAge                        ) return false;
    if(LoanToAlternative              != z.LoanToAlternative                ) return false;
    if(LoanToDuration                 != z.LoanToDuration                   ) return false;
    if(WithdrawalAmount               != z.WithdrawalAmount                 ) return false;
    if(WithdrawalFromAge              != z.WithdrawalFromAge                ) return false;
    if(WithdrawalFromAlternative      != z.WithdrawalFromAlternative        ) return false;
    if(WithdrawalFromDuration         != z.WithdrawalFromDuration           ) return false;
    if(WithdrawalToAge                != z.WithdrawalToAge                  ) return false;
    if(WithdrawalToAlternative        != z.WithdrawalToAlternative          ) return false;
    if(WithdrawalToDuration           != z.WithdrawalToDuration             ) return false;

// TODO ?? Obsolescent.
    if(sSpecAmount                != z.sSpecAmount               ) return false;
    if(sDBOpt                     != z.sDBOpt                    ) return false;
    if(sRetDBOpt                  != z.sRetDBOpt                 ) return false;
    if(sEePremium                 != z.sEePremium                ) return false;
    if(sEeMode                    != z.sEeMode                   ) return false;
    if(sGenAcctRate               != z.sGenAcctRate              ) return false;

    if(Status_IssueAge            != z.Status_IssueAge           ) return false;
    if(Status_RetAge              != z.Status_RetAge             ) return false;
    if(Status_Gender              != z.Status_Gender             ) return false;
    if(Status_Smoking             != z.Status_Smoking            ) return false;
    if(Status_Class               != z.Status_Class              ) return false;
    if(Status_HasWP               != z.Status_HasWP              ) return false;
    if(Status_HasADD              != z.Status_HasADD             ) return false;
    if(Status_HasTerm             != z.Status_HasTerm            ) return false;
    if(Status_TermAmt             != z.Status_TermAmt            ) return false;
    if(Status_TotalSpecAmt        != z.Status_TotalSpecAmt       ) return false;
    if(Status_TermProportion      != z.Status_TermProportion     ) return false;
    if(Status_TermUseProportion   != z.Status_TermUseProportion  ) return false;
    if(Status_DOB                 != z.Status_DOB                ) return false;
    if(Status_UseDOB              != z.Status_UseDOB             ) return false;
    if(Status_DOR                 != z.Status_DOR                ) return false;
    if(Status_UseDOR              != z.Status_UseDOR             ) return false;
    if(Status_SubstdTable         != z.Status_SubstdTable        ) return false;

    return true;
}

//============================================================================
void IllusInputParms::propagate_changes_from_base_and_finalize
    (bool report_errors
    )
{
    propagate_status_to_alii();
    propagate_fund_allocations_to_string();
    realize_all_sequence_strings(report_errors);
    make_term_rider_consistent();
}

//============================================================================
void IllusInputParms::propagate_changes_to_base_and_finalize
    (bool report_errors
    )
{
    propagate_status_from_alii();
    propagate_fund_allocations_from_string();
    realize_all_sequence_strings(report_errors);
    make_term_rider_consistent();
}

//============================================================================
// TODO ?? Try to find a better way, like making class MemberSymbolTable
// accept reference members--except that there can be no pointer to a
// reference; can we instead use a function returning a reference?
void IllusInputParms::propagate_status_from_alii()
{
    Status[0].IssueAge          = Status_IssueAge          ;
    Status[0].RetAge            = Status_RetAge            ;
    Status[0].Gender            = Status_Gender            ;
    Status[0].Smoking           = Status_Smoking           ;
    Status[0].Class             = Status_Class             ;
    Status[0].HasWP             = Status_HasWP             ;
    Status[0].HasADD            = Status_HasADD            ;
    Status[0].HasTerm           = Status_HasTerm           ;
    Status[0].TermAmt           = Status_TermAmt           ;
    Status[0].TotalSpecAmt      = Status_TotalSpecAmt      ;
    Status[0].TermProportion    = Status_TermProportion    ;
    Status[0].TermUseProportion = Status_TermUseProportion ;
    Status[0].DOB               = Status_DOB               ;
    Status[0].UseDOB            = Status_UseDOB            ;
    Status[0].DOR               = Status_DOR               ;
    Status[0].UseDOR            = Status_UseDOR            ;
    Status[0].SubstdTable       = Status_SubstdTable       ;
}

//============================================================================
void IllusInputParms::propagate_status_to_alii()
{
    Status_IssueAge          = Status[0].IssueAge          ;
    Status_RetAge            = Status[0].RetAge            ;
    Status_Gender            = Status[0].Gender            ;
    Status_Smoking           = Status[0].Smoking           ;
    Status_Class             = Status[0].Class             ;
    Status_HasWP             = Status[0].HasWP             ;
    Status_HasADD            = Status[0].HasADD            ;
    Status_HasTerm           = Status[0].HasTerm           ;
    Status_TermAmt           = Status[0].TermAmt           ;
    Status_TotalSpecAmt      = Status[0].TotalSpecAmt      ;
    Status_TermProportion    = Status[0].TermProportion    ;
    Status_TermUseProportion = Status[0].TermUseProportion ;
    Status_DOB               = Status[0].DOB               ;
    Status_UseDOB            = Status[0].UseDOB            ;
    Status_DOR               = Status[0].DOR               ;
    Status_UseDOR            = Status[0].UseDOR            ;
    Status_SubstdTable       = Status[0].SubstdTable       ;
}

// TODO ?? Obviously these could be generalized, e.g. as templates.

//============================================================================
void IllusInputParms::propagate_fund_allocations_from_string()
{
    std::istringstream iss(FundAllocations);
    std::vector<r_fund> v;
    while(!!iss && !iss.eof())
        {
        int i;
        iss >> i;
        v.push_back(r_fund(i));
        }
    FundAllocs = v;

// TODO ?? The mainline code fails with gcc-3.4.2 and mpatrol.
#if 0
#ifndef BC_BEFORE_5_5
    // TODO ?? Here bc++5.5.1 warns:
    // template argument InputIterator passed to 'uninitialized_copy'
    // is a output iterator: input iterator required in function
    // __init_aux<istream_iterator<r_fund,char,char_traits<char>,int> >
    //  (istream_iterator<r_fund,char,char_traits<char>,int>
    //  ,istream_iterator<r_fund,char,char_traits<char>,int>
    //  ,_RW_is_not_integer)
    // Is borland right--is an istringstream not an InputIterator?
warning() << "FundAllocations: " << FundAllocations << LMI_FLUSH;
    FundAllocs = std::vector<r_fund>
        ((std::istream_iterator<r_fund>(iss))
        ,std::istream_iterator<r_fund>()
        );
std::ostringstream oss;
//std::ostream_iterator<std::string> osi(oss, "\r\n");
std::copy
        (FundAllocs.begin()
        ,FundAllocs.end()
        ,std::ostream_iterator<r_fund>(oss, " ")
        );
//std::copy(member_names.begin(), member_names.end(), osi);
warning() << "values: " << oss.str() << LMI_FLUSH;
#else // BC_BEFORE_5_5
/* Even this doesn't work with bc++5.02:
    FundAllocs = std::vector<r_fund>
        (std::istream_iterator<r_fund, std::ptrdiff_t>(iss)
        ,std::istream_iterator<r_fund, std::ptrdiff_t>()
        );
*/
/* ...or this either:
    FundAllocs.clear();
    std::copy
        (std::istream_iterator<r_fund, std::ptrdiff_t>(iss)
        ,std::istream_iterator<r_fund, std::ptrdiff_t>()
        ,std::back_inserter(FundAllocs)
        );
*/
/* ...or this:
    std::insert_iterator<std::vector<r_fund> > iv
        (FundAllocs
        ,FundAllocs.begin()
        );
    std::copy
        (std::istream_iterator<r_fund, std::ptrdiff_t>(iss)
        ,std::istream_iterator<r_fund, std::ptrdiff_t>()
        ,iv
        );
*/
// So we do it this silly way:
    std::vector<r_fund> v;
    while(iss)
        {
        if(iss.eof()) break;
        double d;
        iss >> d;
        if(iss.eof()) break; // TRICKY !! This compiler doesn't get eof right.
        v.push_back(r_fund(d));
        }
    FundAllocs = v;
#endif // BC_BEFORE_5_5
#endif // 0
}

//============================================================================
void IllusInputParms::propagate_fund_allocations_to_string()
{
    std::ostringstream oss;
    std::copy
        (FundAllocs.begin()
        ,FundAllocs.end()
        ,std::ostream_iterator<r_fund>(oss, " ")
        );
    FundAllocations = oss.str();
}

//============================================================================
void IllusInputParms::ascribe_members()
{
// TODO ?? Some of these names could be improved. Proposed improved names
// are given in comments. Names preceded by '!' would require changes to
// the help file; others need to be added to the help file. A comment
// consisting solely of '!' indicates a name already in the help file
// for which no change is proposed.

    // TRICKY !! Alii.
    ascribe("IssueAge"                     , &IllusInputParms::Status_IssueAge            ); //
    ascribe("RetirementAge"                , &IllusInputParms::Status_RetAge              ); // !
    ascribe("Gender"                       , &IllusInputParms::Status_Gender              ); //
    ascribe("Smoking"                      , &IllusInputParms::Status_Smoking             ); //
    ascribe("UnderwritingClass"            , &IllusInputParms::Status_Class               ); // !
    ascribe("WaiverOfPremiumBenefit"       , &IllusInputParms::Status_HasWP               ); // !
    ascribe("AccidentalDeathBenefit"       , &IllusInputParms::Status_HasADD              ); // !
    ascribe("TermRider"                    , &IllusInputParms::Status_HasTerm             );
    ascribe("TermRiderAmount"              , &IllusInputParms::Status_TermAmt             );
    ascribe("TotalSpecifiedAmount"         , &IllusInputParms::Status_TotalSpecAmt        );
    ascribe("TermRiderProportion"          , &IllusInputParms::Status_TermProportion      );
    ascribe("TermRiderUseProportion"       , &IllusInputParms::Status_TermUseProportion   );
    ascribe("DateOfBirth"                  , &IllusInputParms::Status_DOB                 );
    ascribe("DateOfRetirement"             , &IllusInputParms::Status_DOR                 );
    ascribe("SubstandardTable"             , &IllusInputParms::Status_SubstdTable         );
    ascribe("ProductName"                  , &IllusInputParms::ProductName                );
    ascribe("Dumpin"                       , &IllusInputParms::Dumpin                     ); // !
    ascribe("External1035ExchangeAmount"   , &IllusInputParms::External1035ExchangeAmount );
    ascribe("External1035ExchangeBasis"    , &IllusInputParms::External1035ExchangeBasis  );
    ascribe("External1035ExchangeFromMec"  , &IllusInputParms::External1035ExchangeFromMec);
    ascribe("Internal1035ExchangeAmount"   , &IllusInputParms::Internal1035ExchangeAmount );
    ascribe("Internal1035ExchangeBasis"    , &IllusInputParms::Internal1035ExchangeBasis  );
    ascribe("Internal1035ExchangeFromMec"  , &IllusInputParms::Internal1035ExchangeFromMec);
    ascribe("SolveTargetTime"              , &IllusInputParms::SolveTgtTime               ); // !
    ascribe("SolveBeginTime"               , &IllusInputParms::SolveBegTime               ); // !
    ascribe("SolveEndTime"                 , &IllusInputParms::SolveEndTime               ); // !
    ascribe("SolveType"                    , &IllusInputParms::SolveType                  ); // !
    ascribe("SolveBeginYear"               , &IllusInputParms::SolveBegYear               );
    ascribe("SolveEndYear"                 , &IllusInputParms::SolveEndYear               );
    ascribe("SolveTarget"                  , &IllusInputParms::SolveTarget                ); // !
    ascribe("SolveTargetCashSurrenderValue", &IllusInputParms::SolveTgtCSV                ); // !
    ascribe("SolveTargetYear"              , &IllusInputParms::SolveTgtYear               );
    ascribe("SolveBasis"                   , &IllusInputParms::SolveBasis                 ); // !
    ascribe("SolveSeparateAccountBasis"    , &IllusInputParms::SolveSABasis               );
    ascribe("GeneralAccountRateType"       , &IllusInputParms::IntRateTypeGA              );
    ascribe("SeparateAccountRateType"      , &IllusInputParms::IntRateTypeSA              );
    ascribe("LoanRate"                     , &IllusInputParms::LoanIntRate                ); // !
    ascribe("LoanRateType"                 , &IllusInputParms::LoanRateType               ); // !
    ascribe("ExperienceReserveRate"        , &IllusInputParms::ExperienceReserveRate      );
    ascribe("OverrideExperienceReserveRate", &IllusInputParms::OverrideExperienceReserveRate);
    ascribe("PayLoanInterestInCash"        , &IllusInputParms::PayLoanInt                 );
    ascribe("WithdrawToBasisThenLoan"      , &IllusInputParms::WDToBasisThenLoan          );
    ascribe("UseAverageOfAllFunds"         , &IllusInputParms::AvgFund                    );
    ascribe("OverrideFundManagementFee"    , &IllusInputParms::OverrideFundMgmtFee        );
    ascribe("FundChoiceType"               , &IllusInputParms::FundChoiceType             );
    ascribe("InputFundManagementFee"       , &IllusInputParms::InputFundMgmtFee           );
    ascribe("RunOrder"                     , &IllusInputParms::RunOrder                   );
    ascribe("NumberOfIdenticalLives"       , &IllusInputParms::NumIdenticalLives          );
    ascribe("UseExperienceRating"          , &IllusInputParms::UseExperienceRating        );
    ascribe("UsePartialMortality"          , &IllusInputParms::UsePartialMort             );
    ascribe("PartialMortalityTable"        , &IllusInputParms::PartialMortTable           );
// TODO ?? Duplicate name? Do we want this at all?
//    ascribe("PartialMortalityFactor"       , &IllusInputParms::PartialMortTableMult       );
    ascribe("InsuredName"                  , &IllusInputParms::InsdFirstName              ); // !
    ascribe("Address"                      , &IllusInputParms::InsdAddr1                  );
    ascribe("City"                         , &IllusInputParms::InsdCity                   );
    ascribe("State"                        , &IllusInputParms::InsdState                  ); // !
    ascribe("ZipCode"                      , &IllusInputParms::InsdZipCode                );
//    ascribe("TaxBracket"                   , &IllusInputParms::InsdTaxBracket             );
    ascribe("SocialSecurityNumber"         , &IllusInputParms::InsdSSN                    ); // !
    ascribe("EmployeeClass"                , &IllusInputParms::InsdEeClass                );
    ascribe("CorporationName"              , &IllusInputParms::SponsorFirstName           );
    ascribe("CorporationAddress"           , &IllusInputParms::SponsorAddr1               );
    ascribe("CorporationCity"              , &IllusInputParms::SponsorCity                );
    ascribe("CorporationState"             , &IllusInputParms::SponsorState               ); // !
    ascribe("CorporationZipCode"           , &IllusInputParms::SponsorZipCode             );
//    ascribe("CorporationTaxBracket"        , &IllusInputParms::SponsorTaxBracket          );
    ascribe("CorporationTaxpayerId"        , &IllusInputParms::SponsorTaxpayerID          );
    ascribe("AgentName"                    , &IllusInputParms::AgentFirstName             );
    ascribe("AgentAddress"                 , &IllusInputParms::AgentAddr1                 );
    ascribe("AgentCity"                    , &IllusInputParms::AgentCity                  );
    ascribe("AgentState"                   , &IllusInputParms::AgentState                 );
    ascribe("AgentZipCode"                 , &IllusInputParms::AgentZipCode               );
    ascribe("AgentPhone"                   , &IllusInputParms::AgentPhone                 );
    ascribe("AgentId"                      , &IllusInputParms::AgentID                    );
    ascribe("InsuredPremiumTableNumber"    , &IllusInputParms::EePremTableNum             );
    ascribe("InsuredPremiumTableFactor"    , &IllusInputParms::EePremTableMult            );
    ascribe("CorporationPremiumTableNumber", &IllusInputParms::ErPremTableNum             );
    ascribe("CorporationPremiumTableFactor", &IllusInputParms::ErPremTableMult            );
    ascribe("EffectiveDate"                , &IllusInputParms::EffDate                    );
    ascribe("DefinitionOfLifeInsurance"    , &IllusInputParms::DefnLifeIns                );
    ascribe("DefinitionOfMaterialChange"   , &IllusInputParms::DefnMaterialChange         );
    ascribe("AvoidMecMethod"               , &IllusInputParms::AvoidMec                   );
    ascribe("RetireesCanEnroll"            , &IllusInputParms::RetireesCanEnroll          );
    ascribe("GroupUnderwritingType"        , &IllusInputParms::GroupUWType                );
    ascribe("BlendGender"                  , &IllusInputParms::BlendMortGender            );
    ascribe("BlendSmoking"                 , &IllusInputParms::BlendMortSmoking           );
    ascribe("MaleProportion"               , &IllusInputParms::MaleProportion             );
    ascribe("NonsmokerProportion"          , &IllusInputParms::NonsmokerProportion        );
    ascribe("TermProportion"               , &IllusInputParms::InitTermProportion         );
    ascribe("TermAdjustmentMethod"         , &IllusInputParms::TermAdj                    );
    ascribe("IncludeInComposite"           , &IllusInputParms::IncludeInComposite         );
    ascribe("Comments"                     , &IllusInputParms::Comments                   );
    ascribe("AmortizePremiumLoad"          , &IllusInputParms::AmortizePremLoad           );
// TODO ?? Ugly name...and do we want this at all?
    ascribe("YearsOfZeroDeaths"            , &IllusInputParms::YrsPartMortYrsEqZero       );
    ascribe("InforceYear"                  , &IllusInputParms::InforceYear                );
    ascribe("InforceMonth"                 , &IllusInputParms::InforceMonth               );
    ascribe("InforceGeneralAccountValue"   , &IllusInputParms::InforceAVGenAcct           );
    ascribe("InforceSeparateAccountValue"  , &IllusInputParms::InforceAVSepAcct           );
    ascribe("InforceRegularLoanValue"      , &IllusInputParms::InforceAVRegLn             );
    ascribe("InforcePreferredLoanValue"    , &IllusInputParms::InforceAVPrfLn             );
    ascribe("InforceRegularLoanBalance"    , &IllusInputParms::InforceRegLnBal            );
    ascribe("InforcePreferredLoanBalance"  , &IllusInputParms::InforcePrfLnBal            );
    ascribe("InforceCumulativeNoLapsePremium" , &IllusInputParms::InforceCumNoLapsePrem      );
    ascribe("InforceCumulativePayments"    , &IllusInputParms::InforceCumPmts             );
    ascribe("Country"                      , &IllusInputParms::Country                    );
    ascribe("OverrideCoiMultiplier"        , &IllusInputParms::OverrideCOIMultiplier      );
    ascribe("CountryCoiMultiplier"         , &IllusInputParms::CountryCOIMultiplier       );
    ascribe("SurviveToType"                , &IllusInputParms::SurviveToType              );
    ascribe("SurviveToYear"                , &IllusInputParms::SurviveToYear              );
    ascribe("SurviveToAge"                 , &IllusInputParms::SurviveToAge               );
    ascribe("MaximumNaar"                  , &IllusInputParms::MaxNAAR                    );
    ascribe("UseOffshoreCorridorFactor"    , &IllusInputParms::NonUSCorridor              );
    ascribe("ChildRider"                   , &IllusInputParms::HasChildRider              );
    ascribe("ChildRiderAmount"             , &IllusInputParms::ChildRiderAmount           );
    ascribe("SpouseRider"                  , &IllusInputParms::HasSpouseRider             );
    ascribe("SpouseRiderAmount"            , &IllusInputParms::SpouseRiderAmount          );
    ascribe("SpouseIssueAge"               , &IllusInputParms::SpouseIssueAge             );
    ascribe("Franchise"                    , &IllusInputParms::Franchise                  );
    ascribe("PolicyNumber"                 , &IllusInputParms::PolicyNumber               );
    ascribe("PolicyDate"                   , &IllusInputParms::PolicyDate                 );
    ascribe("InforceTaxBasis"              , &IllusInputParms::InforceTaxBasis            );
    ascribe("InforceCumulativeGlp"         , &IllusInputParms::InforceCumGlp              );
    ascribe("InforceGlp"                   , &IllusInputParms::InforceGlp                 );
    ascribe("InforceGsp"                   , &IllusInputParms::InforceGsp                 );
    ascribe("InforceSevenPayPremium"       , &IllusInputParms::Inforce7pp                 );
    ascribe("InforceIsMec"                 , &IllusInputParms::InforceIsMec               );
    ascribe("LastMaterialChangeDate"       , &IllusInputParms::LastMatChgDate             );
    ascribe("InforceDcv"                   , &IllusInputParms::InforceDcv                 );
    ascribe("InforceDcvDeathBenefit"       , &IllusInputParms::InforceDcvDb               );
    ascribe("InforceAvBeforeLastMc"        , &IllusInputParms::InforceAvBeforeLastMc      );
    ascribe("InforceContractYear"          , &IllusInputParms::InforceContractYear        );
    ascribe("InforceContractMonth"         , &IllusInputParms::InforceContractMonth       );
    ascribe("InforceLeastDeathBenefit"     , &IllusInputParms::InforceLeastDeathBenefit   );
    ascribe("StateOfJurisdiction"          , &IllusInputParms::StateOfJurisdiction        );
    ascribe("SalarySpecifiedAmountFactor"  , &IllusInputParms::SalarySAPct                );
    ascribe("SalarySpecifiedAmountCap"     , &IllusInputParms::SalarySACap                );
    ascribe("SalarySpecifiedAmountOffset"  , &IllusInputParms::SalarySAOffset             );
    ascribe("HoneymoonEndorsement"         , &IllusInputParms::HasHoneymoon               );
    ascribe("PostHoneymoonSpread"          , &IllusInputParms::PostHoneymoonSpread        );
    ascribe("InforceHoneymoonValue"        , &IllusInputParms::InforceHoneymoonValue      );
    ascribe("ExtraMonthlyCustodialFee"     , &IllusInputParms::AddonMonthlyCustodialFee   );
    ascribe("ExtraCompensationOnAssets"    , &IllusInputParms::AddonCompOnAssets          );
    ascribe("ExtraCompensationOnPremium"   , &IllusInputParms::AddonCompOnPremium         );
    ascribe("OffshoreCorridorFactor"       , &IllusInputParms::NonUsCorridorFactor        );
    ascribe("PartialMortalityMultiplier"   , &IllusInputParms::PartialMortalityMultiplier );
    ascribe("CurrentCoiMultiplier"         , &IllusInputParms::CurrentCoiMultiplier       );
    ascribe("CurrentCoiGrading"            , &IllusInputParms::CurrentCoiGrading          );
    ascribe("CashValueEnhancementRate"     , &IllusInputParms::CashValueEnhancementRate   );
    ascribe("CorporationTaxBracket"        , &IllusInputParms::CorpTaxBracket             );
    ascribe("TaxBracket"                   , &IllusInputParms::IndvTaxBracket             );
    ascribe("ProjectedSalary"              , &IllusInputParms::ProjectedSalary            );
    ascribe("SpecifiedAmount"              , &IllusInputParms::SpecifiedAmount            ); // !
    ascribe("DeathBenefitOption"           , &IllusInputParms::DeathBenefitOption         ); // !
    ascribe("Payment"                      , &IllusInputParms::IndvPayment                ); // !
    ascribe("PaymentMode"                  , &IllusInputParms::IndvPaymentMode            ); // !
    ascribe("CorporationPayment"           , &IllusInputParms::CorpPayment                ); // !
    ascribe("CorporationPaymentMode"       , &IllusInputParms::CorpPaymentMode            ); // !
    ascribe("GeneralAccountRate"           , &IllusInputParms::GenAcctIntRate             ); // !
    ascribe("SeparateAccountRate"          , &IllusInputParms::SepAcctIntRate             ); // !
    ascribe("NewLoan"                      , &IllusInputParms::NewLoan                    ); // !
    ascribe("Withdrawal"                   , &IllusInputParms::Withdrawal                 ); // !
    ascribe("FlatExtra"                    , &IllusInputParms::FlatExtra                  ); // !
    ascribe("PolicyLevelFlatExtra"         , &IllusInputParms::PolicyLevelFlatExtra       );
    ascribe("HoneymoonValueSpread"         , &IllusInputParms::HoneymoonValueSpread       );
    ascribe("PremiumHistory"               , &IllusInputParms::PremiumHistory             );
    ascribe("SpecamtHistory"               , &IllusInputParms::SpecamtHistory             );
    ascribe("FundAllocations"              , &IllusInputParms::FundAllocations            );

    ascribe("CreateSupplementalReport"     , &IllusInputParms::CreateSupplementalReport   );
    ascribe("SupplementalReportColumn00"   , &IllusInputParms::SupplementalReportColumn00 );
    ascribe("SupplementalReportColumn01"   , &IllusInputParms::SupplementalReportColumn01 );
    ascribe("SupplementalReportColumn02"   , &IllusInputParms::SupplementalReportColumn02 );
    ascribe("SupplementalReportColumn03"   , &IllusInputParms::SupplementalReportColumn03 );
    ascribe("SupplementalReportColumn04"   , &IllusInputParms::SupplementalReportColumn04 );
    ascribe("SupplementalReportColumn05"   , &IllusInputParms::SupplementalReportColumn05 );
    ascribe("SupplementalReportColumn06"   , &IllusInputParms::SupplementalReportColumn06 );
    ascribe("SupplementalReportColumn07"   , &IllusInputParms::SupplementalReportColumn07 );
    ascribe("SupplementalReportColumn08"   , &IllusInputParms::SupplementalReportColumn08 );
    ascribe("SupplementalReportColumn09"   , &IllusInputParms::SupplementalReportColumn09 );
    ascribe("SupplementalReportColumn10"   , &IllusInputParms::SupplementalReportColumn10 );
    ascribe("SupplementalReportColumn11"   , &IllusInputParms::SupplementalReportColumn11 );

    ascribe("DeprecatedSolveTgtAtWhich"    , &IllusInputParms::SolveTgtAtWhich            );
    ascribe("DeprecatedSolveFromWhich"     , &IllusInputParms::SolveFromWhich             );
    ascribe("DeprecatedSolveToWhich"       , &IllusInputParms::SolveToWhich               );
    ascribe("DeprecatedUseDOB"             , &IllusInputParms::Status_UseDOB              );
    ascribe("DeprecatedUseDOR"             , &IllusInputParms::Status_UseDOR              );

    ascribe("EffectiveDateToday"              , &IllusInputParms::EffectiveDateToday                );
    ascribe("DeathBenefitOptionFromRetirement" , &IllusInputParms::DeathBenefitOptionFromRetirement );
    ascribe("DeathBenefitOptionFromIssue"      , &IllusInputParms::DeathBenefitOptionFromIssue      );
    ascribe("SpecifiedAmountFromRetirement"    , &IllusInputParms::SpecifiedAmountFromRetirement    );
    ascribe("SpecifiedAmountFromIssue"         , &IllusInputParms::SpecifiedAmountFromIssue         );
//    ascribe("SpecifiedAmountStrategyFromRetirement , &IllusInputParms:: // TODO ?? Rethink this.
    ascribe("SpecifiedAmountStrategyFromIssue" , &IllusInputParms::SpecifiedAmountStrategyFromIssue );
    ascribe("IndividualPaymentMode"            , &IllusInputParms::IndividualPaymentMode            );
    ascribe("IndividualPaymentToAlternative"   , &IllusInputParms::IndividualPaymentToAlternative   );
    ascribe("IndividualPaymentToAge"           , &IllusInputParms::IndividualPaymentToAge           );
    ascribe("IndividualPaymentToDuration"      , &IllusInputParms::IndividualPaymentToDuration      );
    ascribe("IndividualPaymentAmount"          , &IllusInputParms::IndividualPaymentAmount          );
    ascribe("IndividualPaymentStrategy"        , &IllusInputParms::IndividualPaymentStrategy        );
    ascribe("LoanAmount"                       , &IllusInputParms::LoanAmount                       );
    ascribe("LoanFromAge"                      , &IllusInputParms::LoanFromAge                      );
    ascribe("LoanFromAlternative"              , &IllusInputParms::LoanFromAlternative              );
    ascribe("LoanFromDuration"                 , &IllusInputParms::LoanFromDuration                 );
    ascribe("LoanToAge"                        , &IllusInputParms::LoanToAge                        );
    ascribe("LoanToAlternative"                , &IllusInputParms::LoanToAlternative                );
    ascribe("LoanToDuration"                   , &IllusInputParms::LoanToDuration                   );
    ascribe("WithdrawalAmount"                 , &IllusInputParms::WithdrawalAmount                 );
    ascribe("WithdrawalFromAge"                , &IllusInputParms::WithdrawalFromAge                );
    ascribe("WithdrawalFromAlternative"        , &IllusInputParms::WithdrawalFromAlternative        );
    ascribe("WithdrawalFromDuration"           , &IllusInputParms::WithdrawalFromDuration           );
    ascribe("WithdrawalToAge"                  , &IllusInputParms::WithdrawalToAge                  );
    ascribe("WithdrawalToAlternative"          , &IllusInputParms::WithdrawalToAlternative          );
    ascribe("WithdrawalToDuration"             , &IllusInputParms::WithdrawalToDuration             );
}

