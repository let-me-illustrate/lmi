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

// $Id: ledgervalues.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef ledgervalues_hpp
#define ledgervalues_hpp

#include "config.hpp"

#include "ledger.hpp"

#include <iosfwd>

class InputParms;

class IllusVal
{
  public:
    IllusVal();
    IllusVal(IllusVal const&);
    IllusVal& operator=(IllusVal const&);
    virtual ~IllusVal();

    void Init
        (TLedger* a_CurrValues
        ,TLedger* a_MdptValues
        ,TLedger* a_GuarValues
        );
    double Run(InputParms const&) const;
    void Print(std::ostream&) const;
    TLedger const& GetCurrValues() const {return *CurrValues;}
    TLedger const& GetMdptValues() const {return *MdptValues;}
    TLedger const& GetGuarValues() const {return *GuarValues;}

  private:
    void Alloc();
    void Copy(IllusVal const&);
    void Destroy();

    void PrintHeader              (std::ostream& os) const;
    void PrintFooter              (std::ostream& os) const;
    void PrintNarrativeSummary    (std::ostream& os) const;
    void PrintKeyTerms            (std::ostream& os) const;
    void PrintNumericalSummary    (std::ostream& os) const;
    void PrintRequiredSignatures  (std::ostream& os) const;
    void PrintTabularDetailHeader (std::ostream& os) const;
    void PrintTabularDetail       (std::ostream& os) const;

    TLedger* CurrValues;
    TLedger* MdptValues;
    TLedger* GuarValues;
};

#endif // ledgervalues_hpp

