// Account value.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: accountvalue.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef accountvalue_hpp
#define accountvalue_hpp

#include "config.hpp"

#include "basic_values.hpp"
#include "ledger.hpp"

#include <iosfwd>
#include <vector>

class InputParms;

class AccountValue
    :private BasicValues
    ,public TLedger
{
  public:
    enum ERunBasis
        // TODO ?? We set the enum values high so that we can get an error if
        // someone assigns a value based on EBasis from xenumtypes.hpp . This
        // seems too 'clever': the C++ type system should be strong enough to
        // prevent this without such shenanigans.
        {CurrBasisOnly = 100
        ,GuarBasisOnly
        ,MdptBasisOnly
        ,AllBases
        };
    AccountValue();
    AccountValue(InputParms const& input);
    AccountValue(AccountValue const&);
    AccountValue& operator=(AccountValue const&);
    virtual ~AccountValue();

    double      Solve();
    double      Run(ERunBasis RunBasis);
    double      PerformRun(enum_basis RunBasis);

    void        DebugPrint(std::ostream& os) const;

    TLedger* CurrValues;
    TLedger* MdptValues;
    TLedger* GuarValues;

    void SolveSetPmts
        (double a_Pmt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetSpecAmt
        (double a_Bft
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoans
        (double a_Loan
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetWDs
        (double a_WD
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );
    void SolveSetLoanThenWD
        (double a_Amt
        ,int    ThatSolveBegYear
        ,int    ThatSolveEndYear
        );

  private:
    void        Alloc(int len);
    void        Init();
    void        Copy(AccountValue const&);
    void        Destroy();

    double      RunOneBasis(enum_basis basis);
    double      RunAllBases();
    void        DoYear
                    (enum_basis a_TheBasis
                    ,int        a_Year
                    ,int        a_InforceMonth = 0
                    );
    void        DoMonth();

    void        PerformSpecAmtStrategy();
    void        PerformPmtStrategy(double* a_Pmt);

    void        TxOptChg();
    void        TxSpecAmtChg();
    void        TxSet7702();
    void        TxPmt();
    void        TxLoanRepay();

    void        TxSetBOMAV();
    void        TxSetDeathBft();
    void        TxSetCOI();
    void        TxSetRiderDed();
    void        TxDoMlyDed();

    void        TxCreditInt();
    void        TxLoanInt();
    void        TxTakeWD();
    void        TxTakeLoan();

    void        TxTestLapse();
    void        TxDebug();

    bool        IsModalPmtDate();
    int         MonthsToNextModalPmtDate();

    // mode flags
    bool        Debugging;
    bool        Solving;
    bool        ItLapsed;

    int         LapseMonth;
    int         LapseYear;

    double      Basis;
    double      CumTgtPrem;
    double      CumMinPrem;

//    std::vector<double> COI; // TODO ?? Need this?

    std::vector<double> GrossPmts;
    std::vector<double> NetPmts;
    std::vector<double> Corridor;
    std::vector<double> WPRates;
    std::vector<double> ADDRates;

    // reproposal input
    double      InforceAV;
    double      InforceCumPmts;
    double      InforceLoanBalance;

    // intermediate values
    int         Year;
    int         Month;
    double      AVUnloaned;
    double      NAAR;
    double      COI;

    // intermediate values within annual loop only
    double      pmt;
    e_mode      mode;
    int         ModeIndex;
    double      loan;

    double      AVRegLn;
    double      AVPrfLn;
    double      RegLnBal;
    double      PrfLnBal;
    double      RegLnIntCred;
    double      PrfLnIntCred;
    double      MlyRegLnCredIntRate;
    double      MlyPrfLnCredIntRate;
    double      MlyRegLnDueIntRate;
    double      MlyPrfLnDueIntRate;
    double      MaxLoan;

    double      CorridorFactor;

    double      wd;
    double      premloadtgt;
    double      polfee;
    double      mlycurrint;
    double      mlyguarv;
    double      inputspecamt;
    double      actualspecamt;
    e_dbopt     dbopt;
    double      deathbft;
    double      coirate;
    double      wprate;
    double      addrate;
    bool        haswp;
    bool        hasadd;

    double      ADDChg;
    double      WPChg;

    double      mlyded;
    double      mlydedtonextmodalpmtdate;

    // Illustrated outlay must be the same for current, guaranteed,
    // and all other bases. Outlay components are set on whichever
    // basis governs, usually current, then stored for use with all
    // other bases.
    std::vector<double> OverridingPmts;
};

#endif // accountvalue_hpp

/*
DATA
Vectors
    Pmt         io
    WD          io
    LoanTaken   i
    LoanRepaid  i
        Loan AV vs balance
    ADDPrem     o?
    ADDChg      i
    WPPrem      o?
    WPChg       i
    COIRate     i
    Corridor    o

*/

