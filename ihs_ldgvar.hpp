// Ledger data that vary by basis.
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

// $Id: ihs_ldgvar.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef ldgvar_hpp
#define ldgvar_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "ihs_ldgbase.hpp"
#include "xenumtypes.hpp"

#include <functional>
#include <iosfwd>
#include <map>
#include <vector>

// This class holds all the output values calculated by class AccountValue
// that vary by basis.

class BasicValues;

class LMI_EXPIMP TLedgerVariant
    :public LedgerBase
{
public:
    // Need default ctor to put this thing in a map
    // TODO ?? See comments on class TLedger's default ctor.
    TLedgerVariant(int len = 100);
    TLedgerVariant(TLedgerVariant const&);
    TLedgerVariant& operator=(TLedgerVariant const&);
    virtual ~TLedgerVariant();

    TLedgerVariant& PlusEq
        (TLedgerVariant const&  a_Addend
        ,std::vector<double> const& a_Inforce
        );

    void Init
        (BasicValues*     a_BV
        ,e_basis          a_ExpAndGABasis
        ,e_sep_acct_basis a_SABasis
        );

    bool const          IsFullyInitialized() const {return FullyInitialized;}

    virtual int         GetLength() const {return Length;}

    void SetExpAndGABasis(e_basis const& a_ExpAndGABasis)
        {ExpAndGABasis = a_ExpAndGABasis;}
    void SetSABasis(e_sep_acct_basis const& a_SABasis)
        {SABasis = a_SABasis;}
    void RecordDynamicSepAcctRate
        (double annual_rate
        ,double monthly_rate
        ,int year
        );

    inline double const GetInitAnnLoanCredRate() const
        {return InitAnnLoanCredRate;}
    inline double const InitAnnGenAcctRate() const
        {return InitAnnGenAcctInt;}
    inline double const InitAnnSepAcctGrossRate() const
        {return InitAnnSepAcctGrossInt;}
    inline double const InitAnnSepAcctNetRate() const
        {return InitAnnSepAcctNetInt;}

    void UpdateCRC(CRC& a_crc) const;
    void Spew(std::ostream& os) const;

// TODO ?? PUBLIC DATA NOT GOOD

    // BOY vectors
    std::vector<double> COICharge;      // TODO ?? Never used?
    std::vector<double> ExpenseCharges;
    std::vector<double> AVRelOnDeath;
    std::vector<double> NetIntCredited;
    std::vector<double> GrossIntCredited;
    std::vector<double> ExpRsvInt;
    std::vector<double> NetCOICharge;
    std::vector<double> ExpRatRsvForborne;
    std::vector<double> ExpRatRfdForborne;
    std::vector<double> MlyPolFee;
    std::vector<double> AnnPolFee;
    std::vector<double> PolFee;
    std::vector<double> PremTaxLoad;
    std::vector<double> DacTaxLoad;
    std::vector<double> SpecAmtLoad;
    std::vector<double> AcctValLoadBOM;
    std::vector<double> AcctValLoadAMD;
    std::vector<double> ClaimsPaid;
    std::vector<double> NetClaims;
    std::vector<double> NetPmt;
// TODO ?? Why do we use "BOY" only in "BOYPrefLoan" but not in "Loan"?
    std::vector<double> BOYPrefLoan;    // TODO ?? Never used?

    // EOY vectors
    std::vector<double> AcctVal;
    std::vector<double> DacTaxRsv;
    // see account value class for CSV defns
    std::vector<double> CSVNet;
    std::vector<double> CV7702;
    std::vector<double> EOYDeathBft;
    std::vector<double> PrefLoanBalance;    // TODO ?? Never used?
    std::vector<double> TotalLoanBalance;   // TODO ?? Never used?
    std::vector<double> ExcessLoan;
    std::vector<double> NetDeathBft;    // TODO ?? Never used?
    std::vector<double> AvgDeathBft;    // TODO ?? Never used?
    std::vector<double> SurrChg;        // TODO ?? never used?
    std::vector<double> TermPurchased;
    std::vector<double> BaseDeathBft;

    // Forborne vectors
    std::vector<double> ExpRatRsvCash;
    std::vector<double> ExpRatRfdCash;

    // Nonscalable vectors
    std::vector<double> MlySAIntRate;
    std::vector<double> MlyGAIntRate;
    std::vector<double> MlyHoneymoonValueRate;
    std::vector<double> MlyPostHoneymoonRate;
    std::vector<double> AnnSAIntRate;
    std::vector<double> AnnGAIntRate;
    std::vector<double> AnnHoneymoonValueRate;
    std::vector<double> AnnPostHoneymoonRate;

    // Nonscalable scalars
    double          LapseMonth;
    double          LapseYear;

private:
    void Alloc(int len);
    void Copy(TLedgerVariant const&);
    void Destroy();
    void Init();

    // Nonscalable scalars
    double  InitAnnLoanCredRate;
    double  InitAnnGenAcctInt;
    double  InitAnnSepAcctGrossInt;
    double  InitAnnSepAcctNetInt;

    // special cases
    int              Length;
    e_basis          ExpAndGABasis;
    e_sep_acct_basis SABasis;
    bool             FullyInitialized;   // i.e. by Init(BasicValues* b)
};

typedef std::map<e_run_basis, TLedgerVariant, std::less<e_run_basis> > ledger_map;
typedef ledger_map::value_type ledger_map_val;

// The STL implementation used in designing this code doesn't allow a
// map<S,T> or even a map<S,T> unless the definitions of S and T are
// visible. That creates an undesirable coupling. And there's no portable
// way to forward declare a map. But here's a class we can forward declare:
class LMap
{
    // No virtual dtor--don't derive from this class.
    // We could enforce that constraint in code, by making the ctor private,
    // but that makes the syntax for object creation unnatural.
public:
    LMap(){}
    LMap(ledger_map a_map):LedgerMapRep(a_map){}
    ledger_map LedgerMapRep;
};
// We have a pointer to a forward-declared LMap in class TLedger. That hides
// the details of this class and its base from other classes that use TLedger.

#endif

