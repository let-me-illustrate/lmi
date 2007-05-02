// Ledger data.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ledgervalues.hpp,v 1.15 2007-05-02 13:49:14 chicares Exp $

#ifndef ledgervalues_hpp
#define ledgervalues_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <iosfwd>
#include <string>

class InputParms;
class Ledger;
class LedgerInvariant;
class LedgerVariant;

// Implementation note.
//
// Member function Run() ought to take a const& argument. Passing the
// argument by pointer avoids a problem encountered on the msw
// platform with the gnu linker: the (implicit) copy ctor for class
// Inputs "can't be auto-imported". It would be better to fix class
// Inputs, but that class and class IllusVal itself are both targeted
// for expunction, so a workaround here seems acceptable for now.

class LMI_SO IllusVal
    :private boost::noncopyable
    ,virtual private obstruct_slicing<IllusVal>
{
  public:
    explicit IllusVal(std::string const& filename = "anonymous");
    explicit IllusVal(Ledger*, std::string const& filename = "anonymous");
    ~IllusVal();

    double Run(InputParms const* ip);
    void Print(std::ostream&) const;

    Ledger const& ledger() const {return *ledger_;}

  private:
    std::string filename_;
    boost::shared_ptr<Ledger const> ledger_;
};

void LMI_SO PrintLedgerFlatText(Ledger const&, std::ostream&);

class LMI_SO FlatTextLedgerPrinter
    :private boost::noncopyable
    ,virtual private obstruct_slicing<FlatTextLedgerPrinter>
{
  public:
    FlatTextLedgerPrinter(Ledger const&, std::ostream&);
    ~FlatTextLedgerPrinter();

    void Print() const;

  private:
    void PrintHeader             () const;
    void PrintFooter             () const;
    void PrintNarrativeSummary   () const;
    void PrintKeyTerms           () const;
    void PrintNumericalSummary   () const;
    void PrintRequiredSignatures () const;
    void PrintTabularDetailHeader() const;
    void PrintTabularDetail      () const;

    LedgerInvariant const& invar() const;
    LedgerVariant   const& curr_() const;
    LedgerVariant   const& guar_() const;
    LedgerVariant   const& mdpt_() const;

    Ledger const& ledger_;
    std::ostream& os_;
};

#endif // ledgervalues_hpp

