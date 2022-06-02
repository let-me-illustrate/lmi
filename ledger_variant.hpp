// Ledger data that vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef ledger_variant_hpp
#define ledger_variant_hpp

#include "config.hpp"

#include "ledger_base.hpp"

#include "mc_enum_type_enums.hpp"
#include "so_attributes.hpp"

#include <functional>
#include <iosfwd>
#include <map>
#include <vector>

// This class holds all the output values calculated by class
// AccountValue that vary by basis.

class BasicValues;

class LMI_SO LedgerVariant final
    :public LedgerBase
{
  public:
    // A default ctor is required because this class is used as a
    // std::map's value_type. It's okay to initialize map contents
    // with a default argument of zero because they'll be replaced
    // with new instances created with a nonzero argument.
    LedgerVariant(int len = 0);
    LedgerVariant(LedgerVariant const&);
    LedgerVariant& operator=(LedgerVariant const&);
    ~LedgerVariant() override;

    LedgerVariant& PlusEq
        (LedgerVariant const&  a_Addend
        ,std::vector<double> const& a_Inforce
        );

    void Init(BasicValues const&, mcenum_gen_basis, mcenum_sep_basis);

    bool IsFullyInitialized() const {return FullyInitialized;}

    int GetLength() const override {return Length;}

    void set_run_basis(mcenum_run_basis);

    void RecordDynamicSepAcctRate
        (double annual_rate
        ,int year
        );

    inline double GetInitAnnLoanCredRate() const
        {return InitAnnLoanCredRate;}
    inline double InitAnnGenAcctRate() const
        {return InitAnnGenAcctInt;}
    inline double InitAnnSepAcctGrossRate() const
        {return InitAnnSepAcctGrossInt;}
    inline double InitAnnSepAcctNetRate() const
        {return InitAnnSepAcctNetInt;}

    void UpdateCRC(CRC& a_crc) const override;
    void Spew(std::ostream& os) const override;

// TODO ?? Make data private. Provide const accessors. Some of these
// values could be calculated dynamically instead of stored.

    // BOY vectors.
    std::vector<double> COICharge;
    std::vector<double> RiderCharges;
    std::vector<double> ExpenseCharges;
    std::vector<double> AVRelOnDeath;
    std::vector<double> NetIntCredited;
    std::vector<double> GrossIntCredited;
    std::vector<double> LoanIntAccrued;
    std::vector<double> PolicyFee;
    std::vector<double> PremTaxLoad;
    std::vector<double> DacTaxLoad;
    std::vector<double> SpecAmtLoad;
    std::vector<double> SepAcctCharges;
    std::vector<double> ClaimsPaid;
    std::vector<double> DeathProceedsPaid;
    std::vector<double> NetClaims;
    std::vector<double> NetPmt;

    // EOY vectors
    std::vector<double> AcctVal;
    std::vector<double> AVGenAcct;
    std::vector<double> AVSepAcct;
    std::vector<double> DacTaxRsv;
    // See account value class for CSV definitions.
    std::vector<double> CSVNet;
    std::vector<double> CV7702;
    std::vector<double> EOYDeathBft;
    std::vector<double> PrefLoanBalance; // Not used yet.
    std::vector<double> TotalLoanBalance;
    std::vector<double> AvgDeathBft;     // Not used yet.
    std::vector<double> SurrChg;         // Not used yet.
    std::vector<double> TermPurchased;
    std::vector<double> BaseDeathBft;

    // Forborne vectors [none presently].

    // Nonscalable vectors.
    std::vector<double> AnnSAIntRate;
    std::vector<double> AnnGAIntRate;
    std::vector<double> AnnHoneymoonValueRate;
    std::vector<double> AnnPostHoneymoonRate;

    // Nonscalable scalars.
    double LapseMonth;
    double LapseYear;

  private:
    void Alloc(int len);
    void Copy(LedgerVariant const&);
    void Destroy();
    void Init();

    // Nonscalable scalars.
    double InitAnnLoanCredRate;
    double InitAnnGenAcctInt;
    double InitAnnSepAcctGrossInt;
    double InitAnnSepAcctNetInt;
    double InitTgtPremHiLoadRate; // Rate on first dollar, at maximum premium tax rate.
    double InitMlyPolFee;         // Nonscalable: policy parameter (unlike 'PolicyFee').

    // Special cases.
    int              Length;
    mcenum_gen_basis GenBasis_;
    mcenum_sep_basis SepBasis_;
    bool             FullyInitialized; // I.e. by Init(BasicValues const* b).
};

typedef std::map<mcenum_run_basis,LedgerVariant> ledger_map_t;

/// C++98 17.4.3.6 forbids declaring std::map<S,T> where S or T is
/// incomplete. But class ledger_map_holder can be forward declared
/// even when class LedgerVariant is forward declared.
///
/// Implicitly-declared special member functions do the right thing.

class ledger_map_holder
{
    friend class Ledger;

  public:
    ledger_map_holder() = default;
    explicit ledger_map_holder(ledger_map_t const&);
    ~ledger_map_holder() = default;

    ledger_map_t const& held() const;

  private:
    ledger_map_t held_;
};

#endif // ledger_variant_hpp
