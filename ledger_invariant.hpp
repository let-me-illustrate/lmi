// Ledger data that does not vary by basis.
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

// $Id: ledger_invariant.hpp,v 1.5 2005-10-09 23:25:28 chicares Exp $

#ifndef ledger_invariant_hpp
#define ledger_invariant_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "ledger_base.hpp"
#include "xenumtypes.hpp"

#include <map>
#include <ostream>
#include <string>
#include <vector>

// This class holds all the output values calculated by class AccountValue
// that don't vary by basis.

class BasicValues;
class Ledger;

class LMI_EXPIMP LedgerInvariant
    :public LedgerBase
{
  public:
    // TODO ?? '100' here is poor. See inline comments on class
    // Ledger's default ctor.
    explicit LedgerInvariant(int len = 100);
    LedgerInvariant(LedgerInvariant const&);
    LedgerInvariant& operator=(LedgerInvariant const&);
    virtual ~LedgerInvariant();

    void Init(BasicValues*);

    LedgerInvariant& PlusEq(LedgerInvariant const& a_Addend);

    bool                         IsFullyInitialized()    const;
    virtual int                  GetLength()             const;
    std::vector<double> const&   GetInforceLives()       const;
    double                       GetInitAnnLoanDueRate() const;
    std::string const&           GetStatePostalAbbrev()  const;
    double const&                GetStatePremTaxLoad()   const;
    double const&                GetStatePremTaxRate()   const;
    double const&                GetDacTaxPremLoadRate() const;

    // TODO ?? Does this really belong here?
    // Yes: it keeps the ledger object small; otherwise, numerous IRR
    //   columns would need to be stored.
    // No: its purpose is to push IRR calculations into formatting
    //   routines, which ought not to do any serious calculations.
    int irr_precision;
    void CalculateIrrs(Ledger const&);

    void UpdateCRC(CRC& a_crc) const;
    void Spew(std::ostream& os) const;

// TODO ?? Make data private. Provide const accessors. Some values
// (e.g., outlay) could be calculated dynamically instead of stored.

    // BOY vectors.
    std::vector<double> EePmt;
    std::vector<double> ErPmt;
    std::vector<double> TgtPrem;    // Not currently used?
    std::vector<double> GrossPmt;   // TODO ?? Why "gross"?
    std::vector<double> EeGrossPmt;
    std::vector<double> ErGrossPmt;
    std::vector<double> NetWD;      // TODO ?? Unclear whether this is truly net.
    std::vector<double> NewCashLoan;
    std::vector<double> Outlay;
    std::vector<double> GptForceout;
    std::vector<double> NaarForceout;

    // EOY vectors.
    std::vector<double> TermSpecAmt;
    std::vector<double> SpecAmt;
    // Compensation is treated as an EOY vector because it must
    // reflect between-anniversary processing: premium may be
    // limited, and assets are certain to vary during the year.
    std::vector<double> ProducerCompensation;

    // Other vectors.
    std::vector<double> IndvTaxBracket;
    std::vector<double> CorpTaxBracket;
    std::vector<double> Salary;
    std::vector<double> MonthlyFlatExtra;
    std::vector<double> HoneymoonValueSpread;
    std::vector<double> PartMortTableMult;
    std::vector<double> AddonMonthlyFee;
    std::vector<double> AddonCompOnAssets;
    std::vector<double> AddonCompOnPremium;
    std::vector<double> CorridorFactor;
    std::vector<double> CurrMandE;
    // TODO ?? This is misbegotten. It's supposed to mean IMF including
    // any tiered component, but in that case it would vary by basis.
    std::vector<double> TotalIMF;
    std::vector<double> RefundableSalesLoad;

    // Special-case vectors (not <double>, or different length than others).
    std::vector<e_mode> EeMode;
    std::vector<e_mode> ErMode;
    std::vector<e_dbopt>DBOpt;

    std::vector<double>      FundNumbers;
    std::vector<std::string> FundNames;
    std::vector<int>         FundAllocs; // Obsolete--spreadsheet only.
    std::vector<double>      FundAllocations;

    std::vector<double> InforceLives;

    // IRRs that we need to think about some more before deciding
    // where to put them.

    std::vector<double> IrrCsvGuar0    ;
    std::vector<double> IrrDbGuar0     ;
    std::vector<double> IrrCsvCurr0    ;
    std::vector<double> IrrDbCurr0     ;
    std::vector<double> IrrCsvGuarInput;
    std::vector<double> IrrDbGuarInput ;
    std::vector<double> IrrCsvCurrInput;
    std::vector<double> IrrDbCurrInput ;

    // Scalable scalars.
    double          InitBaseSpecAmt;
    double          InitTermSpecAmt;
    double          ChildRiderAmount;
    double          SpouseRiderAmount;
    double          InitPrem;
    double          GuarPrem;
    double          InitSevenPayPrem;
    double          InitGSP;
    double          InitGLP;
    double          InitTgtPrem;
    double          PostHoneymoonSpread;
    double          Dumpin;
    double          External1035Amount;
    double          Internal1035Amount;

    // Nonscalable scalars.
    double          MaleProportion;
    double          NonsmokerProportion;
    double          GuarMaxMandE;
    double          GenderDistinct;
    double          GenderBlended;
    double          SmokerDistinct;
    double          SmokerBlended;
    double          SubstdTable; // Prefer string 'SubstandardTable'.
    double          Age;
    double          RetAge;
    double          EndtAge;
    double          UseExperienceRating;
    double          UsePartialMort;
    double          AvgFund;
    double          CustomFund;
    double          IsMec;
    double          InforceIsMec;
    double          MecYear;
    double          MecMonth;
    double          HasWP;
    double          HasADD;
    double          HasTerm;
    double          HasChildRider;
    double          HasSpouseRider;
    double          SpouseIssueAge;
    double          HasHoneymoon;
    double          AllowDbo3;
    double          IsInforce;
    double          CountryCOIMultiplier;
    double          PremiumTaxLoadIsTiered;
    double          NoLapseAlwaysActive;
    double          NoLapseMinDur;
    double          NoLapseMinAge;
    double          NominallyPar;
    double          Has1035ExchCharge;
    double          EffDateJdn;
    double          GenAcctAllocation;
    double          SupplementalReport;

    std::string     PolicyForm;
    std::string     PolicyMktgName;
    std::string     PolicyLegalName;
    std::string     InsCoShortName;
    std::string     InsCoName;
    std::string     InsCoAddr;
    std::string     InsCoStreet;
    std::string     InsCoPhone;
    std::string     MainUnderwriter;
    std::string     MainUnderwriterAddress;
    std::string     CoUnderwriter;
    std::string     CoUnderwriterAddress;
    std::string     AvName;
    std::string     CsvName;
    std::string     CsvHeaderName;
    std::string     NoLapseProvisionName;
    std::string     InterestDisclaimer;

    std::string     ProducerName;
    std::string     ProducerStreet;
    std::string     ProducerCity;
    std::string     CorpName;
    std::string     CertificateNumber;
    std::string     PolicyNumber;
    std::string     Insured1;
    std::string     Gender;
    std::string     UWType;
    std::string     Smoker;
    std::string     UWClass;
    std::string     SubstandardTable;
    std::string     DefnLifeIns;
    std::string     DefnMaterialChange;
    std::string     AvoidMec;
    std::string     PartMortTableName;

    std::string     CountryIso3166Abbrev;
    std::string     Comments;

    std::string SupplementalReportColumn00;
    std::string SupplementalReportColumn01;
    std::string SupplementalReportColumn02;
    std::string SupplementalReportColumn03;
    std::string SupplementalReportColumn04;
    std::string SupplementalReportColumn05;
    std::string SupplementalReportColumn06;
    std::string SupplementalReportColumn07;
    std::string SupplementalReportColumn08;
    std::string SupplementalReportColumn09;
    std::string SupplementalReportColumn10;
    std::string SupplementalReportColumn11;

    // Special-case strings.

    // 'Effdate' is special because date formatting might be
    // customized or treated differently by different platforms.
    // Therefore, for CRC checking, data member 'EffDateJdn' holds
    // the corresponding unformatted julian day number.
    std::string     EffDate;

  private:
    void Alloc(int len);
    void Copy(LedgerInvariant const&);
    void Destroy();
    void Init();

    std::string     StatePostalAbbrev;

    // Nonscalable scalars.
    double          StatePremTaxLoad;
    double          StatePremTaxRate;
    double          DacTaxPremLoadRate;
    double          InitAnnLoanDueRate;

    // Special cases.
    int             Length;
    bool            FullyInitialized;   // I.e. by Init(BasicValues* b).
};

inline bool LedgerInvariant::IsFullyInitialized() const
{
    return FullyInitialized;
}

inline int LedgerInvariant::GetLength() const
{
    return Length;
}

inline std::vector<double> const& LedgerInvariant::GetInforceLives() const
{
    return InforceLives;
}

inline double LedgerInvariant::GetInitAnnLoanDueRate() const
{
    return InitAnnLoanDueRate;
}

inline std::string const& LedgerInvariant::GetStatePostalAbbrev() const
{
    return StatePostalAbbrev;
}

inline double const& LedgerInvariant::GetStatePremTaxLoad() const
{
    return StatePremTaxLoad;
}

inline double const& LedgerInvariant::GetStatePremTaxRate() const
{
    return StatePremTaxRate;
}

inline double const& LedgerInvariant::GetDacTaxPremLoadRate() const
{
    return DacTaxPremLoadRate;
}

#endif // ledger_invariant_hpp

