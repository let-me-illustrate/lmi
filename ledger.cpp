// Ledger data.
//
// Copyright (C) 1998, 2001, 2002, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "deathbenefits.hpp"
#include "inputs.hpp"
#include "outlay.hpp"

#include <algorithm>    // std::min(), std::max()

//============================================================================
TLedger::TLedger(int len)
{
    Alloc(len);
}

//============================================================================
TLedger::TLedger(TLedger const& obj)
{
    Alloc(obj.GetLength());
    Copy(obj);
}

//============================================================================
TLedger& TLedger::operator=(TLedger const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Alloc(obj.Length);
        Copy(obj);
        }
    return *this;
}

//============================================================================
TLedger::~TLedger()
{
    Destroy();
}

//============================================================================
void TLedger::Alloc(int len)
{
    Length = len;
    Init();
}

//============================================================================
void TLedger::Copy(TLedger const& obj)
{
    Duration          = obj.Duration         ;
    Pmt               = obj.Pmt              ;
    Mode              = obj.Mode             ;
    TgtPrem           = obj.TgtPrem          ;
    GrossPmt          = obj.GrossPmt         ;
    NegativePmt       = obj.NegativePmt      ;
    ForcedPmt         = obj.ForcedPmt        ;
    EOYDeathBft       = obj.EOYDeathBft      ;
    AcctVal           = obj.AcctVal          ;
    CSV               = obj.CSV              ;
    COI               = obj.COI              ;
    Charges           = obj.Charges          ;
    IntCredited       = obj.IntCredited      ;
    WD                = obj.WD               ;
    Loan              = obj.Loan             ;
    BOYPrefLoan       = obj.BOYPrefLoan      ;
    PrefLoanBalance   = obj.PrefLoanBalance  ;
    TotalLoanBalance  = obj.TotalLoanBalance ;
    ExcessLoan        = obj.ExcessLoan       ;
    SpecAmt           = obj.SpecAmt          ;
    NetDeathBft       = obj.NetDeathBft      ;
    AvgDeathBft       = obj.AvgDeathBft      ;
    SurrChg           = obj.SurrChg          ;
    DBOpt             = obj.DBOpt            ;

    CompanyName       = obj.CompanyName      ;
    ProductName       = obj.ProductName      ;
    PolicyForm        = obj.PolicyForm       ;
    ProducerName      = obj.ProducerName     ;
    ProducerStreet    = obj.ProducerStreet   ;
    ProducerCity      = obj.ProducerCity     ;
    ClientName        = obj.ClientName       ;
    Pages             = obj.Pages            ;
    CertificateNumber = obj.CertificateNumber;
    Insured1          = obj.Insured1         ;
    Gender            = obj.Gender           ;

    GenderDistinct    = obj.GenderDistinct   ;
    Smoker            = obj.Smoker           ;
    SmokerDistinct    = obj.SmokerDistinct   ;
    Preferred         = obj.Preferred        ;
    EffDate           = obj.EffDate          ;
    CurrentPolicyYear = obj.CurrentPolicyYear;
    Age               = obj.Age              ;
    RetAge            = obj.RetAge           ;
    EndtAge           = obj.EndtAge          ;
    BaseFace          = obj.BaseFace         ;
    DBOptInit         = obj.DBOptInit        ;
    DBOptPostRet      = obj.DBOptPostRet     ;
    InitPrem          = obj.InitPrem         ;
    GuarPremium       = obj.GuarPremium      ;
    CredRate          = obj.CredRate         ; // TODO ?? Apparently unused.
    GuarRate          = obj.GuarRate         ;
}

//============================================================================
void TLedger::Destroy()
{
}

//============================================================================
void TLedger::Init()
{
    Duration.resize(Length);
    for(int j = 0; j < Length; ++j)
        {
        Duration[j] = j;
        }

    Pmt              .assign(Length, 0.0);
    Mode             .assign(Length, e_mode(e_annual));
    TgtPrem          .assign(Length, 0.0);
    GrossPmt         .assign(Length, 0.0);
    NegativePmt      .assign(Length, 0.0);
    ForcedPmt        .assign(Length, 0.0);
    EOYDeathBft      .assign(Length, 0.0);
    AcctVal          .assign(Length, 0.0);
    CSV              .assign(Length, 0.0);
    COI              .assign(Length, 0.0);
    Charges          .assign(Length, 0.0);
    IntCredited      .assign(Length, 0.0);
    WD               .assign(Length, 0.0);
    Loan             .assign(Length, 0.0);
    BOYPrefLoan      .assign(Length, 0.0);
    PrefLoanBalance  .assign(Length, 0.0);
    TotalLoanBalance .assign(Length, 0.0);
    ExcessLoan       .assign(Length, 0.0);
    SpecAmt          .assign(Length, 0.0);
    NetDeathBft      .assign(Length, 0.0);
    AvgDeathBft      .assign(Length, 0.0);
    SurrChg          .assign(Length, 0.0);
    DBOpt            .assign(Length, e_dbopt(e_option1));

    Pages                   = 0;

    GenderDistinct          = 0;
    Smoker                  = 0;
    SmokerDistinct          = 0;
    Preferred               = 0;
    EffDate                 = 0L;
    CurrentPolicyYear       = 0;
    Age                     = 0;
    RetAge                  = 0;
    EndtAge                 = 0;
    BaseFace                = 0.0;
    DBOptInit               = e_option1;
    DBOptPostRet            = e_option1;
    InitPrem                = 0.0;
    GuarPremium             = 0.0;
    CredRate                = 0.0;
    GuarRate                = 0.0;
}

//============================================================================
void TLedger::Init(BasicValues* b)
{
// TODO ?? Reconsider the many commented-out lines here.
    Init();    // zero out (almost) everything to start

//    Duration         .assign(Length, p);
    Pmt              = b->Outlay_->ee_modal_premiums();
    Mode             = b->Outlay_->ee_premium_modes();
//    TgtPrem          .assign(Length, p);
//    GrossPmt         .assign(Length, p);
//    NegativePmt      .assign(Length, p);
//    ForcedPmt        .assign(Length, p);
//    EOYDeathBft      .assign(Length, p);
//    AcctVal          .assign(Length, p);
//    CSV              .assign(Length, p);
//    COI              .assign(Length, p);
//    Charges          .assign(Length, p);
//    IntCredited      .assign(Length, p);
    Loan             = b->Outlay_->new_cash_loans();
    WD               = b->Outlay_->withdrawals();
//    BOYPrefLoan      .assign(Length, p);
//    PrefLoanBalance  .assign(Length, p);
//    TotalLoanBalance .assign(Length, p);
//    ExcessLoan       .assign(Length, p);
    SpecAmt          = b->DeathBfts->GetSpecAmt();
//    NetDeathBft      .assign(Length, p);
//    AvgDeathBft      .assign(Length, p);
//    SurrChg          .assign(Length, p);
    DBOpt            = b->DeathBfts->GetDBOpt();

    CompanyName             = b->Input->SponsorFirstName;
//           ProductName      = obj.ProductName;
//           PolicyForm       = obj.PolicyForm;

    ProducerName            = b->Input->AgentFullName();
    ProducerStreet =
          b->Input->AgentAddr1
        + ", "
        + b->Input->AgentAddr2
        ;
    ProducerCity =
          b->Input->AgentCity
        + ", "
        + b->Input->AgentState.str()
        + " "
        + b->Input->AgentZipCode
        ;

    ClientName            = b->Input->SponsorFirstName;

//    Pages                 = ?;
//    CertificateNumber     = obj.CertificateNumber);

    Insured1                = b->Input->InsdFullName();

    InputStatus const& S = b->Input->Status[0]; // TODO ?? DB based on first life only.

    Gender = S.Gender.str();

//    GenderDistinct          = 0;
// TODO ?? Why are we using the int value rather than the string?
    Smoker                  = S.Smoking.value();
//    SmokerDistinct          = 0;
// TODO ?? Why are we using the int value rather than the string?
    Preferred               = S.Class.value();
//    EffDate                 = 0;
//    CurrentPolicyYear       = 0;
    Age                     = S.IssueAge.value();
    RetAge                  = S.RetAge.value();
    EndtAge                 = S.IssueAge.value() + b->GetLength();
//    BaseFace                = 0;
//    DBOptInit               = 0;
//    DBOptPostRet            = 0;
//    InitPrem                = 0;
//    GuarPremium             = 0;
//    CredRate                = 0;
//    GuarRate                = 0;
}

//============================================================================
TLedger& TLedger::operator+=(TLedger const& obj)
{
// TODO ?? Reconsider the many commented-out lines here.
    int max_idx = std::min(Length, obj.Length);
    LMI_ASSERT(obj.Length <= Length);
    for(int j = 0; j < max_idx; j++)
        {
//        Duration            [j] += obj.Duration         [j];
        Pmt                 [j] += obj.Pmt              [j];
//        Mode                [j] += obj.Mode             [j];
        TgtPrem             [j] += obj.TgtPrem          [j];
        GrossPmt            [j] += obj.GrossPmt         [j];
        NegativePmt         [j] += obj.NegativePmt      [j];
        ForcedPmt           [j] += obj.ForcedPmt        [j];
        EOYDeathBft         [j] += obj.EOYDeathBft      [j];
        AcctVal             [j] += obj.AcctVal          [j];
        CSV                 [j] += obj.CSV              [j];
        COI                 [j] += obj.COI              [j];
        Charges             [j] += obj.Charges          [j];
        IntCredited         [j] += obj.IntCredited      [j];
        WD                  [j] += obj.WD               [j];
        Loan                [j] += obj.Loan             [j];
        BOYPrefLoan         [j] += obj.BOYPrefLoan      [j];
        PrefLoanBalance     [j] += obj.PrefLoanBalance  [j];
        TotalLoanBalance    [j] += obj.TotalLoanBalance [j];
        ExcessLoan          [j] += obj.ExcessLoan       [j];
        SpecAmt             [j] += obj.SpecAmt          [j];
        NetDeathBft         [j] += obj.NetDeathBft      [j];
        AvgDeathBft         [j] += obj.AvgDeathBft      [j];
        SurrChg             [j] += obj.SurrChg          [j];
//        DBOpt               [j] += obj.DBOpt            [j];
        }

//    GenderDistinct          = 0;
//    Smoker                  = obj.Smoking;
//    SmokerDistinct          = 0;
//    Preferred               = obj.Class;
//    EffDate                 = 0;
//    CurrentPolicyYear       = 0;
    Age                     = std::min(Age, obj.Age);
    RetAge                  = std::min(RetAge, obj.RetAge); // TODO ?? does this make sense?
    EndtAge                 = std::max(EndtAge, obj.EndtAge);
//    BaseFace                = 0;
//    DBOptInit               = 0;
//    DBOptPostRet            = 0;
//    InitPrem                = 0;
//    GuarPremium             = 0;
//    CredRate                = 0;
//    GuarRate                = 0;

    return *this;
}

