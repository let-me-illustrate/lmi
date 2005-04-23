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

// $Id: inputillus.cpp,v 1.4 2005-04-23 21:42:57 chicares Exp $

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
    ,CaseAssumedAssets          ("0")
    ,CorpTaxBracket             ("0")
    ,IndvTaxBracket             ("0")
    ,ProjectedSalary            ("100000")
    ,SpecifiedAmount            ("1000000")
    ,DeathBenefitOption         ("a")
    ,IndvPayment                ("20000")
    ,IndvPaymentMode            ("annual")
    ,CorpPayment                ("0")
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
    ,IndividualPaymentAmount          (20000)
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
    ,sEePremium                 (20000.0)
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
#ifndef BC_BEFORE_5_5
    ,MemberSymbolTable_ihs<IllusInputParms>()
#endif // not old borland compiler
{
    AddonMonthlyCustodialFee   = z.AddonMonthlyCustodialFee   ;
    AddonCompOnAssets          = z.AddonCompOnAssets          ;
    AddonCompOnPremium         = z.AddonCompOnPremium         ;
    NonUsCorridorFactor        = z.NonUsCorridorFactor        ;
    PartialMortalityMultiplier = z.PartialMortalityMultiplier ;
    CurrentCoiMultiplier       = z.CurrentCoiMultiplier       ;
    CurrentCoiGrading          = z.CurrentCoiGrading          ;
    CashValueEnhancementRate   = z.CashValueEnhancementRate   ;
    CaseAssumedAssets          = z.CaseAssumedAssets          ;
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
    CaseAssumedAssets          = z.CaseAssumedAssets          ;
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
    if(CaseAssumedAssets          != z.CaseAssumedAssets         ) return false;
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
// TODO ?? Try to find a better way, like making class MemberSymbolTable_ihs
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
#ifndef BC_BEFORE_5_5

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
    ascribe("AssumedCaseNumberOfLives"     , &IllusInputParms::AssumedCaseNumLives        );
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
    ascribe("CaseAssumedAssets"            , &IllusInputParms::CaseAssumedAssets          );
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
#endif // not old borland compiler
}

// Workarounds for old compilers that can't handle class MemberSymbolTable_ihs.

#ifdef BC_BEFORE_5_5

// SOMEDAY !! It would be nifty to create this map with the code in
// ascribe_members, with a suitable alternative ascribe() like
//    void ascribe(char const* s...)
// But that would remove only part of the maintenance problem:
// set_value_by_name() and get_value_by_name() still require manual maintenance.

std::vector<std::string> IllusInputParms::map_keys() const
{
    std::vector<std::string> keys;
    keys.push_back("IssueAge"                     );
    keys.push_back("RetirementAge"                );
    keys.push_back("Gender"                       );
    keys.push_back("Smoking"                      );
    keys.push_back("UnderwritingClass"            );
    keys.push_back("WaiverOfPremiumBenefit"       );
    keys.push_back("AccidentalDeathBenefit"       );
    keys.push_back("TermRider"                    );
    keys.push_back("TermRiderAmount"              );
    keys.push_back("TotalSpecifiedAmount"         );
    keys.push_back("TermRiderProportion"          );
    keys.push_back("TermRiderUseProportion"       );
    keys.push_back("DateOfBirth"                  );
    keys.push_back("DateOfRetirement"             );
    keys.push_back("SubstandardTable"             );
    keys.push_back("ProductName"                  );
    keys.push_back("Dumpin"                       );
    keys.push_back("External1035ExchangeAmount"   );
    keys.push_back("External1035ExchangeBasis"    );
    keys.push_back("External1035ExchangeFromMec"  );
    keys.push_back("Internal1035ExchangeAmount"   );
    keys.push_back("Internal1035ExchangeBasis"    );
    keys.push_back("Internal1035ExchangeFromMec"  );
    keys.push_back("SolveTargetTime"              );
    keys.push_back("SolveBeginTime"               );
    keys.push_back("SolveEndTime"                 );
    keys.push_back("SolveType"                    );
    keys.push_back("SolveBeginYear"               );
    keys.push_back("SolveEndYear"                 );
    keys.push_back("SolveTarget"                  );
    keys.push_back("SolveTargetCashSurrenderValue");
    keys.push_back("SolveTargetYear"              );
    keys.push_back("SolveBasis"                   );
    keys.push_back("SolveSeparateAccountBasis"    );
    keys.push_back("GeneralAccountRateType"       );
    keys.push_back("SeparateAccountRateType"      );
    keys.push_back("LoanRate"                     );
    keys.push_back("LoanRateType"                 );
    keys.push_back("ExperienceReserveRate"        );
    keys.push_back("OverrideExperienceReserveRate");
    keys.push_back("PayLoanInterestInCash"        );
    keys.push_back("WithdrawToBasisThenLoan"      );
    keys.push_back("UseAverageOfAllFunds"         );
    keys.push_back("OverrideFundManagementFee"    );
    keys.push_back("FundChoiceType"               );
    keys.push_back("InputFundManagementFee"       );
    keys.push_back("RunOrder"                     );
    keys.push_back("AssumedCaseNumberOfLives"     );
    keys.push_back("NumberOfIdenticalLives"       );
    keys.push_back("UseExperienceRating"          );
    keys.push_back("UsePartialMortality"          );
    keys.push_back("PartialMortalityTable"        );
    keys.push_back("InsuredName"                  );
    keys.push_back("Address"                      );
    keys.push_back("City"                         );
    keys.push_back("State"                        );
    keys.push_back("ZipCode"                      );
    keys.push_back("SocialSecurityNumber"         );
    keys.push_back("EmployeeClass"                );
    keys.push_back("CorporationName"              );
    keys.push_back("CorporationAddress"           );
    keys.push_back("CorporationCity"              );
    keys.push_back("CorporationState"             );
    keys.push_back("CorporationZipCode"           );
    keys.push_back("CorporationTaxpayerId"        );
    keys.push_back("AgentName"                    );
    keys.push_back("AgentAddress"                 );
    keys.push_back("AgentCity"                    );
    keys.push_back("AgentState"                   );
    keys.push_back("AgentZipCode"                 );
    keys.push_back("AgentPhone"                   );
    keys.push_back("AgentId"                      );
    keys.push_back("InsuredPremiumTableNumber"    );
    keys.push_back("InsuredPremiumTableFactor"    );
    keys.push_back("CorporationPremiumTableNumber");
    keys.push_back("CorporationPremiumTableFactor");
    keys.push_back("EffectiveDate"                );
    keys.push_back("DefinitionOfLifeInsurance"    );
    keys.push_back("AvoidMecMethod"               );
    keys.push_back("DefinitionOfMaterialChange"   );
    keys.push_back("RetireesCanEnroll"            );
    keys.push_back("GroupUnderwritingType"        );
    keys.push_back("BlendGender"                  );
    keys.push_back("BlendSmoking"                 );
    keys.push_back("MaleProportion"               );
    keys.push_back("NonsmokerProportion"          );
    keys.push_back("TermProportion"               );
    keys.push_back("TermAdjustmentMethod"         );
    keys.push_back("IncludeInComposite"           );
    keys.push_back("Comments"                     );
    keys.push_back("AmortizePremiumLoad"          );
    keys.push_back("YearsOfZeroDeaths"            );
    keys.push_back("InforceYear"                  );
    keys.push_back("InforceMonth"                 );
    keys.push_back("InforceGeneralAccountValue"   );
    keys.push_back("InforceSeparateAccountValue"  );
    keys.push_back("InforceRegularLoanValue"      );
    keys.push_back("InforcePreferredLoanValue"    );
    keys.push_back("InforceRegularLoanBalance"    );
    keys.push_back("InforcePreferredLoanBalance"  );
    keys.push_back("InforceCumulativeNoLapsePremium" );
    keys.push_back("InforceCumulativePayments"    );
    keys.push_back("Country"                      );
    keys.push_back("OverrideCoiMultiplier"        );
    keys.push_back("CountryCoiMultiplier"         );
    keys.push_back("SurviveToType"                );
    keys.push_back("SurviveToYear"                );
    keys.push_back("SurviveToAge"                 );
    keys.push_back("MaximumNaar"                  );
    keys.push_back("UseOffshoreCorridorFactor"    );
    keys.push_back("ChildRider"                   );
    keys.push_back("ChildRiderAmount"             );
    keys.push_back("SpouseRider"                  );
    keys.push_back("SpouseRiderAmount"            );
    keys.push_back("SpouseIssueAge"               );
    keys.push_back("Franchise"                    );
    keys.push_back("PolicyNumber"                 );
    keys.push_back("PolicyDate"                   );
    keys.push_back("InforceTaxBasis"              );
    keys.push_back("InforceCumulativeGlp"         );
    keys.push_back("InforceGlp"                   );
    keys.push_back("InforceGsp"                   );
    keys.push_back("InforceSevenPayPremium"       );
    keys.push_back("InforceIsMec"                 );
    keys.push_back("LastMaterialChangeDate"       );
    keys.push_back("InforceDcv"                   );
    keys.push_back("InforceDcvDeathBenefit"       );
    keys.push_back("InforceAvBeforeLastMc"        );
    keys.push_back("InforceContractYear"          );
    keys.push_back("InforceContractMonth"         );
    keys.push_back("InforceLeastDeathBenefit"     );
    keys.push_back("StateOfJurisdiction"          );
    keys.push_back("SalarySpecifiedAmountFactor"  );
    keys.push_back("SalarySpecifiedAmountCap"     );
    keys.push_back("SalarySpecifiedAmountOffset"  );
    keys.push_back("HoneymoonEndorsement"         );
    keys.push_back("PostHoneymoonSpread"          );
    keys.push_back("InforceHoneymoonValue"        );
    keys.push_back("ExtraMonthlyCustodialFee"     );
    keys.push_back("ExtraCompensationOnAssets"    );
    keys.push_back("ExtraCompensationOnPremium"   );
    keys.push_back("OffshoreCorridorFactor"       );
    keys.push_back("PartialMortalityMultiplier"   );
    keys.push_back("CurrentCoiMultiplier"         );
    keys.push_back("CurrentCoiGrading"            );
    keys.push_back("CashValueEnhancementRate"     );
    keys.push_back("CaseAssumedAssets"            );
    keys.push_back("CorporationTaxBracket"        );
    keys.push_back("TaxBracket"                   );
    keys.push_back("ProjectedSalary"              );
    keys.push_back("SpecifiedAmount"              );
    keys.push_back("DeathBenefitOption"           );
    keys.push_back("Payment"                      );
    keys.push_back("PaymentMode"                  );
    keys.push_back("CorporationPayment"           );
    keys.push_back("CorporationPaymentMode"       );
    keys.push_back("GeneralAccountRate"           );
    keys.push_back("SeparateAccountRate"          );
    keys.push_back("NewLoan"                      );
    keys.push_back("Withdrawal"                   );
    keys.push_back("FlatExtra"                    );
    keys.push_back("PolicyLevelFlatExtra"         );
    keys.push_back("HoneymoonValueSpread"         );
    keys.push_back("PremiumHistory"               );
    keys.push_back("SpecamtHistory"               );
    keys.push_back("FundAllocations"              );
    keys.push_back("DeprecatedSolveTgtAtWhich"    );
    keys.push_back("DeprecatedSolveFromWhich"     );
    keys.push_back("DeprecatedSolveToWhich"       );
    keys.push_back("DeprecatedUseDOB"             );
    keys.push_back("DeprecatedUseDOR"             );

    // Sort the std::vector result so it'll have the same order as a std::map.
    std::sort(keys.begin(), keys.end());
    return keys;
}

namespace
{
// COMPILER !! The old 1997 borland compiler (bc++5.02) doesn't follow
// the standard which was published after it. In particular, it doesn't
// understand standard template specialization. Otherwise we'd write
//
//    template<typename T>
//    std::string get_value_from_string(T& t)
//    {
//        return value_cast_ihs<std::string>(t);
//    }
//
//    template<> ...
//
// TODO ?? That really should work without specializations, if template
// function value_cast_ihs() works as intended. It suffers from the problem
// described above, but maybe we should make a stronger attempt to fix
// it, since we use value_cast_ihs extensively elsewhere.

    template<typename Essence, typename Substance>
    std::string get_value_from_string(xrange<Essence,Substance> const& t)
    {
        return t.str();
    }

    template<typename EnumType, int N>
    std::string get_value_from_string(xenum<EnumType, N> const& t)
    {
        // TRICKY !! Need to use the extractor to change ' ' to '_': this
        //   return t.str();
        // won't do.
        std::stringstream oss;
        oss << t;
        return oss.str();
    }

    std::string get_value_from_string(std::string const& t)
    {
        return t;
    }
}

//============================================================================
std::string IllusInputParms::get_value_by_name
    (std::string const& name
    ) const
{
         if(name == "IssueAge"                     ) return get_value_from_string(Status_IssueAge            );
    else if(name == "RetirementAge"                ) return get_value_from_string(Status_RetAge              );
    else if(name == "Gender"                       ) return get_value_from_string(Status_Gender              );
    else if(name == "Smoking"                      ) return get_value_from_string(Status_Smoking             );
    else if(name == "UnderwritingClass"            ) return get_value_from_string(Status_Class               );
    else if(name == "WaiverOfPremiumBenefit"       ) return get_value_from_string(Status_HasWP               );
    else if(name == "AccidentalDeathBenefit"       ) return get_value_from_string(Status_HasADD              );
    else if(name == "TermRider"                    ) return get_value_from_string(Status_HasTerm             );
    else if(name == "TermRiderAmount"              ) return get_value_from_string(Status_TermAmt             );
    else if(name == "TotalSpecifiedAmount"         ) return get_value_from_string(Status_TotalSpecAmt        );
    else if(name == "TermRiderProportion"          ) return get_value_from_string(Status_TermProportion      );
    else if(name == "TermRiderUseProportion"       ) return get_value_from_string(Status_TermUseProportion   );
    else if(name == "DateOfBirth"                  ) return get_value_from_string(Status_DOB                 );
    else if(name == "DateOfRetirement"             ) return get_value_from_string(Status_DOR                 );
    else if(name == "SubstandardTable"             ) return get_value_from_string(Status_SubstdTable         );
    else if(name == "ProductName"                  ) return get_value_from_string(ProductName                );
    else if(name == "Dumpin"                       ) return get_value_from_string(Dumpin                     );
    else if(name == "External1035ExchangeAmount"   ) return get_value_from_string(External1035ExchangeAmount );
    else if(name == "External1035ExchangeBasis"    ) return get_value_from_string(External1035ExchangeBasis  );
    else if(name == "External1035ExchangeFromMec"  ) return get_value_from_string(External1035ExchangeFromMec);
    else if(name == "Internal1035ExchangeAmount"   ) return get_value_from_string(Internal1035ExchangeAmount );
    else if(name == "Internal1035ExchangeBasis"    ) return get_value_from_string(Internal1035ExchangeBasis  );
    else if(name == "Internal1035ExchangeFromMec"  ) return get_value_from_string(Internal1035ExchangeFromMec);
    else if(name == "SolveTargetTime"              ) return get_value_from_string(SolveTgtTime               );
    else if(name == "SolveBeginTime"               ) return get_value_from_string(SolveBegTime               );
    else if(name == "SolveEndTime"                 ) return get_value_from_string(SolveEndTime               );
    else if(name == "SolveType"                    ) return get_value_from_string(SolveType                  );
    else if(name == "SolveBeginYear"               ) return get_value_from_string(SolveBegYear               );
    else if(name == "SolveEndYear"                 ) return get_value_from_string(SolveEndYear               );
    else if(name == "SolveTarget"                  ) return get_value_from_string(SolveTarget                );
    else if(name == "SolveTargetCashSurrenderValue") return get_value_from_string(SolveTgtCSV                );
    else if(name == "SolveTargetYear"              ) return get_value_from_string(SolveTgtYear               );
    else if(name == "SolveBasis"                   ) return get_value_from_string(SolveBasis                 );
    else if(name == "SolveSeparateAccountBasis"    ) return get_value_from_string(SolveSABasis               );
    else if(name == "GeneralAccountRateType"       ) return get_value_from_string(IntRateTypeGA              );
    else if(name == "SeparateAccountRateType"      ) return get_value_from_string(IntRateTypeSA              );
    else if(name == "LoanRate"                     ) return get_value_from_string(LoanIntRate                );
    else if(name == "LoanRateType"                 ) return get_value_from_string(LoanRateType               );
    else if(name == "ExperienceReserveRate"        ) return get_value_from_string(ExperienceReserveRate      );
    else if(name == "OverrideExperienceReserveRate") return get_value_from_string(OverrideExperienceReserveRate);
    else if(name == "PayLoanInterestInCash"        ) return get_value_from_string(PayLoanInt                 );
    else if(name == "WithdrawToBasisThenLoan"      ) return get_value_from_string(WDToBasisThenLoan          );
    else if(name == "UseAverageOfAllFunds"         ) return get_value_from_string(AvgFund                    );
    else if(name == "OverrideFundManagementFee"    ) return get_value_from_string(OverrideFundMgmtFee        );
    else if(name == "FundChoiceType"               ) return get_value_from_string(FundChoiceType             );
    else if(name == "InputFundManagementFee"       ) return get_value_from_string(InputFundMgmtFee           );
    else if(name == "RunOrder"                     ) return get_value_from_string(RunOrder                   );
    else if(name == "AssumedCaseNumberOfLives"     ) return get_value_from_string(AssumedCaseNumLives        );
    else if(name == "NumberOfIdenticalLives"       ) return get_value_from_string(NumIdenticalLives          );
    else if(name == "UseExperienceRating"          ) return get_value_from_string(UseExperienceRating        );
    else if(name == "UsePartialMortality"          ) return get_value_from_string(UsePartialMort             );
    else if(name == "PartialMortalityTable"        ) return get_value_from_string(PartialMortTable           );
    else if(name == "InsuredName"                  ) return get_value_from_string(InsdFirstName              );
    else if(name == "Address"                      ) return get_value_from_string(InsdAddr1                  );
    else if(name == "City"                         ) return get_value_from_string(InsdCity                   );
    else if(name == "State"                        ) return get_value_from_string(InsdState                  );
    else if(name == "ZipCode"                      ) return get_value_from_string(InsdZipCode                );
    else if(name == "SocialSecurityNumber"         ) return get_value_from_string(InsdSSN                    );
    else if(name == "EmployeeClass"                ) return get_value_from_string(InsdEeClass                );
    else if(name == "CorporationName"              ) return get_value_from_string(SponsorFirstName           );
    else if(name == "CorporationAddress"           ) return get_value_from_string(SponsorAddr1               );
    else if(name == "CorporationCity"              ) return get_value_from_string(SponsorCity                );
    else if(name == "CorporationState"             ) return get_value_from_string(SponsorState               );
    else if(name == "CorporationZipCode"           ) return get_value_from_string(SponsorZipCode             );
    else if(name == "CorporationTaxpayerId"        ) return get_value_from_string(SponsorTaxpayerID          );
    else if(name == "AgentName"                    ) return get_value_from_string(AgentFirstName             );
    else if(name == "AgentAddress"                 ) return get_value_from_string(AgentAddr1                 );
    else if(name == "AgentCity"                    ) return get_value_from_string(AgentCity                  );
    else if(name == "AgentState"                   ) return get_value_from_string(AgentState                 );
    else if(name == "AgentZipCode"                 ) return get_value_from_string(AgentZipCode               );
    else if(name == "AgentPhone"                   ) return get_value_from_string(AgentPhone                 );
    else if(name == "AgentId"                      ) return get_value_from_string(AgentID                    );
    else if(name == "InsuredPremiumTableNumber"    ) return get_value_from_string(EePremTableNum             );
    else if(name == "InsuredPremiumTableFactor"    ) return get_value_from_string(EePremTableMult            );
    else if(name == "CorporationPremiumTableNumber") return get_value_from_string(ErPremTableNum             );
    else if(name == "CorporationPremiumTableFactor") return get_value_from_string(ErPremTableMult            );
    else if(name == "EffectiveDate"                ) return get_value_from_string(EffDate                    );
    else if(name == "DefinitionOfLifeInsurance"    ) return get_value_from_string(DefnLifeIns                );
    else if(name == "DefinitionOfMaterialChange"   ) return get_value_from_string(DefnMaterialChange         );
    else if(name == "AvoidMecMethod"               ) return get_value_from_string(AvoidMec                   );
    else if(name == "RetireesCanEnroll"            ) return get_value_from_string(RetireesCanEnroll          );
    else if(name == "GroupUnderwritingType"        ) return get_value_from_string(GroupUWType                );
    else if(name == "BlendGender"                  ) return get_value_from_string(BlendMortGender            );
    else if(name == "BlendSmoking"                 ) return get_value_from_string(BlendMortSmoking           );
    else if(name == "MaleProportion"               ) return get_value_from_string(MaleProportion             );
    else if(name == "NonsmokerProportion"          ) return get_value_from_string(NonsmokerProportion        );
    else if(name == "TermProportion"               ) return get_value_from_string(InitTermProportion         );
    else if(name == "TermAdjustmentMethod"         ) return get_value_from_string(TermAdj                    );
    else if(name == "IncludeInComposite"           ) return get_value_from_string(IncludeInComposite         );
    else if(name == "Comments"                     ) return get_value_from_string(Comments                   );
    else if(name == "AmortizePremiumLoad"          ) return get_value_from_string(AmortizePremLoad           );
    else if(name == "YearsOfZeroDeaths"            ) return get_value_from_string(YrsPartMortYrsEqZero       );
    else if(name == "InforceYear"                  ) return get_value_from_string(InforceYear                );
    else if(name == "InforceMonth"                 ) return get_value_from_string(InforceMonth               );
    else if(name == "InforceGeneralAccountValue"   ) return get_value_from_string(InforceAVGenAcct           );
    else if(name == "InforceSeparateAccountValue"  ) return get_value_from_string(InforceAVSepAcct           );
    else if(name == "InforceRegularLoanValue"      ) return get_value_from_string(InforceAVRegLn             );
    else if(name == "InforcePreferredLoanValue"    ) return get_value_from_string(InforceAVPrfLn             );
    else if(name == "InforceRegularLoanBalance"    ) return get_value_from_string(InforceRegLnBal            );
    else if(name == "InforcePreferredLoanBalance"  ) return get_value_from_string(InforcePrfLnBal            );
    else if(name == "InforceCumulativeNoLapsePremium" ) return get_value_from_string(InforceCumNoLapsePrem      );
    else if(name == "InforceCumulativePayments"    ) return get_value_from_string(InforceCumPmts             );
    else if(name == "Country"                      ) return get_value_from_string(Country                    );
    else if(name == "OverrideCoiMultiplier"        ) return get_value_from_string(OverrideCOIMultiplier      );
    else if(name == "CountryCoiMultiplier"         ) return get_value_from_string(CountryCOIMultiplier       );
    else if(name == "SurviveToType"                ) return get_value_from_string(SurviveToType              );
    else if(name == "SurviveToYear"                ) return get_value_from_string(SurviveToYear              );
    else if(name == "SurviveToAge"                 ) return get_value_from_string(SurviveToAge               );
    else if(name == "MaximumNaar"                  ) return get_value_from_string(MaxNAAR                    );
    else if(name == "UseOffshoreCorridorFactor"    ) return get_value_from_string(NonUSCorridor              );
    else if(name == "ChildRider"                   ) return get_value_from_string(HasChildRider              );
    else if(name == "ChildRiderAmount"             ) return get_value_from_string(ChildRiderAmount           );
    else if(name == "SpouseRider"                  ) return get_value_from_string(HasSpouseRider             );
    else if(name == "SpouseRiderAmount"            ) return get_value_from_string(SpouseRiderAmount          );
    else if(name == "SpouseIssueAge"               ) return get_value_from_string(SpouseIssueAge             );
    else if(name == "Franchise"                    ) return get_value_from_string(Franchise                  );
    else if(name == "PolicyNumber"                 ) return get_value_from_string(PolicyNumber               );
    else if(name == "PolicyDate"                   ) return get_value_from_string(PolicyDate                 );
    else if(name == "InforceTaxBasis"              ) return get_value_from_string(InforceTaxBasis            );
    else if(name == "InforceCumulativeGlp"         ) return get_value_from_string(InforceCumGlp              );
    else if(name == "InforceGlp"                   ) return get_value_from_string(InforceGlp                 );
    else if(name == "InforceGsp"                   ) return get_value_from_string(InforceGsp                 );
    else if(name == "InforceSevenPayPremium"       ) return get_value_from_string(Inforce7pp                 );
    else if(name == "InforceIsMec"                 ) return get_value_from_string(InforceIsMec               );
    else if(name == "LastMaterialChangeDate"       ) return get_value_from_string(LastMatChgDate             );
    else if(name == "InforceDcv"                   ) return get_value_from_string(InforceDcv                 );
    else if(name == "InforceDcvDeathBenefit"       ) return get_value_from_string(InforceDcvDb               );
    else if(name == "InforceAvBeforeLastMc"        ) return get_value_from_string(InforceAvBeforeLastMc      );
    else if(name == "InforceContractYear"          ) return get_value_from_string(InforceContractYear        );
    else if(name == "InforceContractMonth"         ) return get_value_from_string(InforceContractMonth       );
    else if(name == "InforceLeastDeathBenefit"     ) return get_value_from_string(InforceLeastDeathBenefit   );
    else if(name == "StateOfJurisdiction"          ) return get_value_from_string(StateOfJurisdiction        );
    else if(name == "SalarySpecifiedAmountFactor"  ) return get_value_from_string(SalarySAPct                );
    else if(name == "SalarySpecifiedAmountCap"     ) return get_value_from_string(SalarySACap                );
    else if(name == "SalarySpecifiedAmountOffset"  ) return get_value_from_string(SalarySAOffset             );
    else if(name == "HoneymoonEndorsement"         ) return get_value_from_string(HasHoneymoon               );
    else if(name == "PostHoneymoonSpread"          ) return get_value_from_string(PostHoneymoonSpread        );
    else if(name == "InforceHoneymoonValue"        ) return get_value_from_string(InforceHoneymoonValue      );
    else if(name == "ExtraMonthlyCustodialFee"     ) return get_value_from_string(AddonMonthlyCustodialFee   );
    else if(name == "ExtraCompensationOnAssets"    ) return get_value_from_string(AddonCompOnAssets          );
    else if(name == "ExtraCompensationOnPremium"   ) return get_value_from_string(AddonCompOnPremium         );
    else if(name == "OffshoreCorridorFactor"       ) return get_value_from_string(NonUsCorridorFactor        );
    else if(name == "PartialMortalityMultiplier"   ) return get_value_from_string(PartialMortalityMultiplier );
    else if(name == "CurrentCoiMultiplier"         ) return get_value_from_string(CurrentCoiMultiplier       );
    else if(name == "CurrentCoiGrading"            ) return get_value_from_string(CurrentCoiGrading          );
    else if(name == "CashValueEnhancementRate"     ) return get_value_from_string(CashValueEnhancementRate   );
    else if(name == "CaseAssumedAssets"            ) return get_value_from_string(CaseAssumedAssets          );
    else if(name == "CorporationTaxBracket"        ) return get_value_from_string(CorpTaxBracket             );
    else if(name == "TaxBracket"                   ) return get_value_from_string(IndvTaxBracket             );
    else if(name == "ProjectedSalary"              ) return get_value_from_string(ProjectedSalary            );
    else if(name == "SpecifiedAmount"              ) return get_value_from_string(SpecifiedAmount            );
    else if(name == "DeathBenefitOption"           ) return get_value_from_string(DeathBenefitOption         );
    else if(name == "Payment"                      ) return get_value_from_string(IndvPayment                );
    else if(name == "PaymentMode"                  ) return get_value_from_string(IndvPaymentMode            );
    else if(name == "CorporationPayment"           ) return get_value_from_string(CorpPayment                );
    else if(name == "CorporationPaymentMode"       ) return get_value_from_string(CorpPaymentMode            );
    else if(name == "GeneralAccountRate"           ) return get_value_from_string(GenAcctIntRate             );
    else if(name == "SeparateAccountRate"          ) return get_value_from_string(SepAcctIntRate             );
    else if(name == "NewLoan"                      ) return get_value_from_string(NewLoan                    );
    else if(name == "Withdrawal"                   ) return get_value_from_string(Withdrawal                 );
    else if(name == "FlatExtra"                    ) return get_value_from_string(FlatExtra                  );
    else if(name == "PolicyLevelFlatExtra"         ) return get_value_from_string(PolicyLevelFlatExtra       );
    else if(name == "HoneymoonValueSpread"         ) return get_value_from_string(HoneymoonValueSpread       );
    else if(name == "PremiumHistory"               ) return get_value_from_string(PremiumHistory             );
    else if(name == "SpecamtHistory"               ) return get_value_from_string(SpecamtHistory             );
    else if(name == "FundAllocations"              ) return get_value_from_string(FundAllocations            );
    else if(name == "DeprecatedSolveTgtAtWhich"    ) return get_value_from_string(SolveTgtAtWhich            );
    else if(name == "DeprecatedSolveFromWhich"     ) return get_value_from_string(SolveFromWhich             );
    else if(name == "DeprecatedSolveToWhich"       ) return get_value_from_string(SolveToWhich               );
    else if(name == "DeprecatedUseDOB"             ) return get_value_from_string(Status_UseDOB              );
    else if(name == "DeprecatedUseDOR"             ) return get_value_from_string(Status_UseDOR              );
    else throw std::runtime_error("No member named " + name);
}

namespace
{
// COMPILER !! The old 1997 borland compiler (bc++5.02) doesn't follow
// the standard which was published after it. In particular, it doesn't
// understand standard template specialization. Otherwise we'd write
//
//    template<typename T>
//    void get_value_from_string(T& t, std::string const& s)
//    {
//        t = value_cast_ihs<T>(s);
//    }
//
//    template<> ...
//
// TODO ?? As noted above, perhaps we should find a way to make value_cast_ihs
// work well enough with this compiler that no specialization of this
// function would be needed. Until then, we leave this code the way it is:
// even though the body is uniformly 't = s;', we may need to add other
// signatures that would need a different body--for instance, 't = s;'
// wouldn't work well for arithmetic POD types.

    template<typename Essence, typename Substance>
    void set_value_from_string(xrange<Essence,Substance>& t, std::string const& s)
    {
        t = s;
    }

    template<typename EnumType, int N>
    void set_value_from_string(xenum<EnumType, N>& t, std::string const& s)
    {
        // TRICKY !! Need to use the inserter to change '_' to ' ': this
        //   t = s;
        // won't do.
        std::stringstream iss(s);
        iss >> t;
    }

    void set_value_from_string(std::string& t, std::string const& s)
    {
        t = s;
    }
}

//============================================================================

void IllusInputParms::set_value_by_name
    (std::string const& name
    ,std::string const& value
    )
{
         if(name == "IssueAge"                     ) set_value_from_string(Status_IssueAge            , value);
    else if(name == "RetirementAge"                ) set_value_from_string(Status_RetAge              , value);
    else if(name == "Gender"                       ) set_value_from_string(Status_Gender              , value);
    else if(name == "Smoking"                      ) set_value_from_string(Status_Smoking             , value);
    else if(name == "UnderwritingClass"            ) set_value_from_string(Status_Class               , value);
    else if(name == "WaiverOfPremiumBenefit"       ) set_value_from_string(Status_HasWP               , value);
    else if(name == "AccidentalDeathBenefit"       ) set_value_from_string(Status_HasADD              , value);
    else if(name == "TermRider"                    ) set_value_from_string(Status_HasTerm             , value);
    else if(name == "TermRiderAmount"              ) set_value_from_string(Status_TermAmt             , value);
    else if(name == "TotalSpecifiedAmount"         ) set_value_from_string(Status_TotalSpecAmt        , value);
    else if(name == "TermRiderProportion"          ) set_value_from_string(Status_TermProportion      , value);
    else if(name == "TermRiderUseProportion"       ) set_value_from_string(Status_TermUseProportion   , value);
    else if(name == "DateOfBirth"                  ) set_value_from_string(Status_DOB                 , value);
    else if(name == "DateOfRetirement"             ) set_value_from_string(Status_DOR                 , value);
    else if(name == "SubstandardTable"             ) set_value_from_string(Status_SubstdTable         , value);
    else if(name == "ProductName"                  ) set_value_from_string(ProductName                , value);
    else if(name == "Dumpin"                       ) set_value_from_string(Dumpin                     , value);
    else if(name == "External1035ExchangeAmount"   ) set_value_from_string(External1035ExchangeAmount , value);
    else if(name == "External1035ExchangeBasis"    ) set_value_from_string(External1035ExchangeBasis  , value);
    else if(name == "External1035ExchangeFromMec"  ) set_value_from_string(External1035ExchangeFromMec, value);
    else if(name == "Internal1035ExchangeAmount"   ) set_value_from_string(Internal1035ExchangeAmount , value);
    else if(name == "Internal1035ExchangeBasis"    ) set_value_from_string(Internal1035ExchangeBasis  , value);
    else if(name == "Internal1035ExchangeFromMec"  ) set_value_from_string(Internal1035ExchangeFromMec, value);
    else if(name == "SolveTargetTime"              ) set_value_from_string(SolveTgtTime               , value);
    else if(name == "SolveBeginTime"               ) set_value_from_string(SolveBegTime               , value);
    else if(name == "SolveEndTime"                 ) set_value_from_string(SolveEndTime               , value);
    else if(name == "SolveType"                    ) set_value_from_string(SolveType                  , value);
    else if(name == "SolveBeginYear"               ) set_value_from_string(SolveBegYear               , value);
    else if(name == "SolveEndYear"                 ) set_value_from_string(SolveEndYear               , value);
    else if(name == "SolveTarget"                  ) set_value_from_string(SolveTarget                , value);
    else if(name == "SolveTargetCashSurrenderValue") set_value_from_string(SolveTgtCSV                , value);
    else if(name == "SolveTargetYear"              ) set_value_from_string(SolveTgtYear               , value);
    else if(name == "SolveBasis"                   ) set_value_from_string(SolveBasis                 , value);
    else if(name == "SolveSeparateAccountBasis"    ) set_value_from_string(SolveSABasis               , value);
    else if(name == "GeneralAccountRateType"       ) set_value_from_string(IntRateTypeGA              , value);
    else if(name == "SeparateAccountRateType"      ) set_value_from_string(IntRateTypeSA              , value);
    else if(name == "LoanRate"                     ) set_value_from_string(LoanIntRate                , value);
    else if(name == "LoanRateType"                 ) set_value_from_string(LoanRateType               , value);
    else if(name == "ExperienceReserveRate"        ) set_value_from_string(ExperienceReserveRate      , value);
    else if(name == "OverrideExperienceReserveRate") set_value_from_string(OverrideExperienceReserveRate, value);
    else if(name == "PayLoanInterestInCash"        ) set_value_from_string(PayLoanInt                 , value);
    else if(name == "WithdrawToBasisThenLoan"      ) set_value_from_string(WDToBasisThenLoan          , value);
    else if(name == "UseAverageOfAllFunds"         ) set_value_from_string(AvgFund                    , value);
    else if(name == "OverrideFundManagementFee"    ) set_value_from_string(OverrideFundMgmtFee        , value);
    else if(name == "FundChoiceType"               ) set_value_from_string(FundChoiceType             , value);
    else if(name == "InputFundManagementFee"       ) set_value_from_string(InputFundMgmtFee           , value);
    else if(name == "RunOrder"                     ) set_value_from_string(RunOrder                   , value);
    else if(name == "AssumedCaseNumberOfLives"     ) set_value_from_string(AssumedCaseNumLives        , value);
    else if(name == "NumberOfIdenticalLives"       ) set_value_from_string(NumIdenticalLives          , value);
    else if(name == "UseExperienceRating"          ) set_value_from_string(UseExperienceRating        , value);
    else if(name == "UsePartialMortality"          ) set_value_from_string(UsePartialMort             , value);
    else if(name == "PartialMortalityTable"        ) set_value_from_string(PartialMortTable           , value);
    else if(name == "InsuredName"                  ) set_value_from_string(InsdFirstName              , value);
    else if(name == "Address"                      ) set_value_from_string(InsdAddr1                  , value);
    else if(name == "City"                         ) set_value_from_string(InsdCity                   , value);
    else if(name == "State"                        ) set_value_from_string(InsdState                  , value);
    else if(name == "ZipCode"                      ) set_value_from_string(InsdZipCode                , value);
    else if(name == "SocialSecurityNumber"         ) set_value_from_string(InsdSSN                    , value);
    else if(name == "EmployeeClass"                ) set_value_from_string(InsdEeClass                , value);
    else if(name == "CorporationName"              ) set_value_from_string(SponsorFirstName           , value);
    else if(name == "CorporationAddress"           ) set_value_from_string(SponsorAddr1               , value);
    else if(name == "CorporationCity"              ) set_value_from_string(SponsorCity                , value);
    else if(name == "CorporationState"             ) set_value_from_string(SponsorState               , value);
    else if(name == "CorporationZipCode"           ) set_value_from_string(SponsorZipCode             , value);
    else if(name == "CorporationTaxpayerId"        ) set_value_from_string(SponsorTaxpayerID          , value);
    else if(name == "AgentName"                    ) set_value_from_string(AgentFirstName             , value);
    else if(name == "AgentAddress"                 ) set_value_from_string(AgentAddr1                 , value);
    else if(name == "AgentCity"                    ) set_value_from_string(AgentCity                  , value);
    else if(name == "AgentState"                   ) set_value_from_string(AgentState                 , value);
    else if(name == "AgentZipCode"                 ) set_value_from_string(AgentZipCode               , value);
    else if(name == "AgentPhone"                   ) set_value_from_string(AgentPhone                 , value);
    else if(name == "AgentId"                      ) set_value_from_string(AgentID                    , value);
    else if(name == "InsuredPremiumTableNumber"    ) set_value_from_string(EePremTableNum             , value);
    else if(name == "InsuredPremiumTableFactor"    ) set_value_from_string(EePremTableMult            , value);
    else if(name == "CorporationPremiumTableNumber") set_value_from_string(ErPremTableNum             , value);
    else if(name == "CorporationPremiumTableFactor") set_value_from_string(ErPremTableMult            , value);
    else if(name == "EffectiveDate"                ) set_value_from_string(EffDate                    , value);
    else if(name == "DefinitionOfLifeInsurance"    ) set_value_from_string(DefnLifeIns                , value);
    else if(name == "DefinitionOfMaterialChange"   ) set_value_from_string(DefnMaterialChange         , value);
    else if(name == "AvoidMecMethod"               ) set_value_from_string(AvoidMec                   , value);
    else if(name == "RetireesCanEnroll"            ) set_value_from_string(RetireesCanEnroll          , value);
    else if(name == "GroupUnderwritingType"        ) set_value_from_string(GroupUWType                , value);
    else if(name == "BlendGender"                  ) set_value_from_string(BlendMortGender            , value);
    else if(name == "BlendSmoking"                 ) set_value_from_string(BlendMortSmoking           , value);
    else if(name == "MaleProportion"               ) set_value_from_string(MaleProportion             , value);
    else if(name == "NonsmokerProportion"          ) set_value_from_string(NonsmokerProportion        , value);
    else if(name == "TermProportion"               ) set_value_from_string(InitTermProportion         , value);
    else if(name == "TermAdjustmentMethod"         ) set_value_from_string(TermAdj                    , value);
    else if(name == "IncludeInComposite"           ) set_value_from_string(IncludeInComposite         , value);
    else if(name == "Comments"                     ) set_value_from_string(Comments                   , value);
    else if(name == "AmortizePremiumLoad"          ) set_value_from_string(AmortizePremLoad           , value);
    else if(name == "YearsOfZeroDeaths"            ) set_value_from_string(YrsPartMortYrsEqZero       , value);
    else if(name == "InforceYear"                  ) set_value_from_string(InforceYear                , value);
    else if(name == "InforceMonth"                 ) set_value_from_string(InforceMonth               , value);
    else if(name == "InforceGeneralAccountValue"   ) set_value_from_string(InforceAVGenAcct           , value);
    else if(name == "InforceSeparateAccountValue"  ) set_value_from_string(InforceAVSepAcct           , value);
    else if(name == "InforceRegularLoanValue"      ) set_value_from_string(InforceAVRegLn             , value);
    else if(name == "InforcePreferredLoanValue"    ) set_value_from_string(InforceAVPrfLn             , value);
    else if(name == "InforceRegularLoanBalance"    ) set_value_from_string(InforceRegLnBal            , value);
    else if(name == "InforcePreferredLoanBalance"  ) set_value_from_string(InforcePrfLnBal            , value);
    else if(name == "InforceCumulativeNoLapsePremium" ) set_value_from_string(InforceCumNoLapsePrem      , value);
    else if(name == "InforceCumulativePayments"    ) set_value_from_string(InforceCumPmts             , value);
    else if(name == "Country"                      ) set_value_from_string(Country                    , value);
    else if(name == "OverrideCoiMultiplier"        ) set_value_from_string(OverrideCOIMultiplier      , value);
    else if(name == "CountryCoiMultiplier"         ) set_value_from_string(CountryCOIMultiplier       , value);
    else if(name == "SurviveToType"                ) set_value_from_string(SurviveToType              , value);
    else if(name == "SurviveToYear"                ) set_value_from_string(SurviveToYear              , value);
    else if(name == "SurviveToAge"                 ) set_value_from_string(SurviveToAge               , value);
    else if(name == "MaximumNaar"                  ) set_value_from_string(MaxNAAR                    , value);
    else if(name == "UseOffshoreCorridorFactor"    ) set_value_from_string(NonUSCorridor              , value);
    else if(name == "ChildRider"                   ) set_value_from_string(HasChildRider              , value);
    else if(name == "ChildRiderAmount"             ) set_value_from_string(ChildRiderAmount           , value);
    else if(name == "SpouseRider"                  ) set_value_from_string(HasSpouseRider             , value);
    else if(name == "SpouseRiderAmount"            ) set_value_from_string(SpouseRiderAmount          , value);
    else if(name == "SpouseIssueAge"               ) set_value_from_string(SpouseIssueAge             , value);
    else if(name == "Franchise"                    ) set_value_from_string(Franchise                  , value);
    else if(name == "PolicyNumber"                 ) set_value_from_string(PolicyNumber               , value);
    else if(name == "PolicyDate"                   ) set_value_from_string(PolicyDate                 , value);
    else if(name == "InforceTaxBasis"              ) set_value_from_string(InforceTaxBasis            , value);
    else if(name == "InforceCumulativeGlp"         ) set_value_from_string(InforceCumGlp              , value);
    else if(name == "InforceGlp"                   ) set_value_from_string(InforceGlp                 , value);
    else if(name == "InforceGsp"                   ) set_value_from_string(InforceGsp                 , value);
    else if(name == "InforceSevenPayPremium"       ) set_value_from_string(Inforce7pp                 , value);
    else if(name == "InforceIsMec"                 ) set_value_from_string(InforceIsMec               , value);
    else if(name == "LastMaterialChangeDate"       ) set_value_from_string(LastMatChgDate             , value);
    else if(name == "InforceDcv"                   ) set_value_from_string(InforceDcv                 , value);
    else if(name == "InforceDcvDeathBenefit"       ) set_value_from_string(InforceDcvDb               , value);
    else if(name == "InforceAvBeforeLastMc"        ) set_value_from_string(InforceAvBeforeLastMc      , value);
    else if(name == "InforceContractYear"          ) set_value_from_string(InforceContractYear        , value);
    else if(name == "InforceContractMonth"         ) set_value_from_string(InforceContractMonth       , value);
    else if(name == "InforceLeastDeathBenefit"     ) set_value_from_string(InforceLeastDeathBenefit   , value);
    else if(name == "StateOfJurisdiction"          ) set_value_from_string(StateOfJurisdiction        , value);
    else if(name == "SalarySpecifiedAmountFactor"  ) set_value_from_string(SalarySAPct                , value);
    else if(name == "SalarySpecifiedAmountCap"     ) set_value_from_string(SalarySACap                , value);
    else if(name == "SalarySpecifiedAmountOffset"  ) set_value_from_string(SalarySAOffset             , value);
    else if(name == "HoneymoonEndorsement"         ) set_value_from_string(HasHoneymoon               , value);
    else if(name == "PostHoneymoonSpread"          ) set_value_from_string(PostHoneymoonSpread        , value);
    else if(name == "InforceHoneymoonValue"        ) set_value_from_string(InforceHoneymoonValue      , value);
    else if(name == "ExtraMonthlyCustodialFee"     ) set_value_from_string(AddonMonthlyCustodialFee   , value);
    else if(name == "ExtraCompensationOnAssets"    ) set_value_from_string(AddonCompOnAssets          , value);
    else if(name == "ExtraCompensationOnPremium"   ) set_value_from_string(AddonCompOnPremium         , value);
    else if(name == "OffshoreCorridorFactor"       ) set_value_from_string(NonUsCorridorFactor        , value);
    else if(name == "PartialMortalityMultiplier"   ) set_value_from_string(PartialMortalityMultiplier , value);
    else if(name == "CurrentCoiMultiplier"         ) set_value_from_string(CurrentCoiMultiplier       , value);
    else if(name == "CurrentCoiGrading"            ) set_value_from_string(CurrentCoiGrading          , value);
    else if(name == "CashValueEnhancementRate"     ) set_value_from_string(CashValueEnhancementRate   , value);
    else if(name == "CaseAssumedAssets"            ) set_value_from_string(CaseAssumedAssets          , value);
    else if(name == "CorporationTaxBracket"        ) set_value_from_string(CorpTaxBracket             , value);
    else if(name == "TaxBracket"                   ) set_value_from_string(IndvTaxBracket             , value);
    else if(name == "ProjectedSalary"              ) set_value_from_string(ProjectedSalary            , value);
    else if(name == "SpecifiedAmount"              ) set_value_from_string(SpecifiedAmount            , value);
    else if(name == "DeathBenefitOption"           ) set_value_from_string(DeathBenefitOption         , value);
    else if(name == "Payment"                      ) set_value_from_string(IndvPayment                , value);
    else if(name == "PaymentMode"                  ) set_value_from_string(IndvPaymentMode            , value);
    else if(name == "CorporationPayment"           ) set_value_from_string(CorpPayment                , value);
    else if(name == "CorporationPaymentMode"       ) set_value_from_string(CorpPaymentMode            , value);
    else if(name == "GeneralAccountRate"           ) set_value_from_string(GenAcctIntRate             , value);
    else if(name == "SeparateAccountRate"          ) set_value_from_string(SepAcctIntRate             , value);
    else if(name == "NewLoan"                      ) set_value_from_string(NewLoan                    , value);
    else if(name == "Withdrawal"                   ) set_value_from_string(Withdrawal                 , value);
    else if(name == "FlatExtra"                    ) set_value_from_string(FlatExtra                  , value);
    else if(name == "PolicyLevelFlatExtra"         ) set_value_from_string(PolicyLevelFlatExtra       , value);
    else if(name == "HoneymoonValueSpread"         ) set_value_from_string(HoneymoonValueSpread       , value);
    else if(name == "PremiumHistory"               ) set_value_from_string(PremiumHistory             , value);
    else if(name == "SpecamtHistory"               ) set_value_from_string(SpecamtHistory             , value);
    else if(name == "FundAllocations"              ) set_value_from_string(FundAllocations            , value);
    else if(name == "DeprecatedSolveTgtAtWhich"    ) set_value_from_string(SolveTgtAtWhich            , value);
    else if(name == "DeprecatedSolveFromWhich"     ) set_value_from_string(SolveFromWhich             , value);
    else if(name == "DeprecatedSolveToWhich"       ) set_value_from_string(SolveToWhich               , value);
    else if(name == "DeprecatedUseDOB"             ) set_value_from_string(Status_UseDOB              , value);
    else if(name == "DeprecatedUseDOR"             ) set_value_from_string(Status_UseDOR              , value);
    else throw std::runtime_error("No member named " + name);
}

#endif // old borland compiler

