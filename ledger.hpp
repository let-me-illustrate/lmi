// Ledger data.
//
// Copyright (C) 1998, 2001, 2005 Gregory W. Chicares.
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

// $Id: ledger.hpp,v 1.2 2005-01-31 13:12:48 chicares Exp $

#ifndef ledger_hpp
#define ledger_hpp

#include "config.hpp"

#include "xenumtypes.hpp"

#include <string>
#include <vector>

// Ledger statement values

class BasicValues;

class TLedger
{
  public:
    TLedger(int len = 0);
    TLedger(TLedger const&);
    TLedger& operator=(TLedger const&);
    TLedger& operator+=(TLedger const&);
    virtual ~TLedger();

    int GetLength() const {return Length;}

    std::vector<int>     Duration;
    std::vector<double>  Pmt;
    std::vector<e_mode>   Mode;
    std::vector<double>  TgtPrem;
    std::vector<double>  GrossPmt;
    std::vector<double>  NegativePmt;
    std::vector<double>  ForcedPmt;
    std::vector<double>  EOYDeathBft;
    std::vector<double>  AcctVal;
    std::vector<double>  CSV;
    std::vector<double>  COI;
    std::vector<double>  Charges;
    std::vector<double>  IntCredited;
    std::vector<double>  WD;
    std::vector<double>  Loan;
    std::vector<double>  BOYPrefLoan;
    std::vector<double>  PrefLoanBalance;
    std::vector<double>  TotalLoanBalance;
    std::vector<double>  ExcessLoan;
    std::vector<double>  SpecAmt;
    std::vector<double>  NetDeathBft;
    std::vector<double>  AvgDeathBft;
    std::vector<double>  SurrChg;
    std::vector<e_dbopt> DBOpt;

//    int        LapseYear; // TODO ?? Why not needed?
    std::string CompanyName;
    std::string ProductName;
    std::string PolicyForm;
    std::string ProducerName;
    std::string ProducerStreet;
    std::string ProducerCity;
    std::string ClientName;
    int     Pages;
    std::string CertificateNumber;
    std::string Insured1;
    std::string Gender;
    int     GenderDistinct;
    int     Smoker;
    int     SmokerDistinct;
    int     Preferred;
    // TODO ?? flats, ratings omitted
    long int EffDate;
    int     CurrentPolicyYear;
    int     Age;
    int     RetAge;
    int     EndtAge;
    double  BaseFace;
    e_dbopt  DBOptInit;
    e_dbopt  DBOptPostRet;
    double  InitPrem;
    double  GuarPremium;
    double  CredRate;
    double  GuarRate;

  protected:
    void Init(BasicValues*);

  private:
    void Alloc(int len);
    void Copy(TLedger const&);
    void Destroy();
    void Init();

    int Length;
};

#endif // ledger_hpp

