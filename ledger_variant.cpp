// Ledger data that vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_variant.hpp"

#include "assert_lmi.hpp"
#include "basic_values.hpp"
#include "database.hpp" // Used only for initial loan rate.
#include "dbnames.hpp"  // Used only for initial loan rate.
#include "interest_rates.hpp"
#include "mc_enum_types_aux.hpp" // mc_str()
#include "outlay.hpp"

#include <algorithm>
#include <ostream>

//============================================================================
LedgerVariant::LedgerVariant(int len)
    :LedgerBase      (len)
    ,GenBasis_       (mce_gen_curr)
    ,SepBasis_       (mce_sep_full)
    ,FullyInitialized(false)
{
    Alloc(len);
}

//============================================================================
LedgerVariant::LedgerVariant(LedgerVariant const& obj)
    :LedgerBase(obj)
    ,GenBasis_       (mce_gen_curr)
    ,SepBasis_       (mce_sep_full)
    ,FullyInitialized(false)
{
    Alloc(obj.GetLength());
    Copy(obj);
}

//============================================================================
LedgerVariant& LedgerVariant::operator=(LedgerVariant const& obj)
{
    if(this != &obj)
        {
        LedgerBase::operator=(obj);
        Destroy();
        Alloc(obj.Length);
        Copy(obj);
        }
    return *this;
}

//============================================================================
LedgerVariant::~LedgerVariant()
{
    Destroy();
}

//============================================================================
void LedgerVariant::Alloc(int len)
{
    Length  = len;

    BegYearVectors  ["COICharge"            ] = &COICharge              ;
    BegYearVectors  ["ExpenseCharges"       ] = &ExpenseCharges         ;

    // Account value released on death might at first seem to be
    // an end-of-year item, since we use curtate mortality and the
    // release must occur at the end of the year. It is actually a
    // beginning-of-year item because it is q times account value,
    // and q is as of the beginning of the year. Perhaps more
    // importantly, it is intended for use on a financial-statement
    // basis, which states values for an *interval* of a year
    // rather than at either of the year's endpoints. The same is
    // true of various other items such as policy fee and interest
    // credited as used here.
    BegYearVectors  ["AVRelOnDeath"         ] = &AVRelOnDeath           ;
    BegYearVectors  ["NetIntCredited"       ] = &NetIntCredited         ;
    BegYearVectors  ["GrossIntCredited"     ] = &GrossIntCredited       ;
    BegYearVectors  ["LoanIntAccrued"       ] = &LoanIntAccrued         ;

    BegYearVectors  ["NetCOICharge"         ] = &NetCOICharge           ;

    ForborneVectors ["ExperienceReserve"    ] = &ExperienceReserve      ;

    BegYearVectors  ["PolicyFee"            ] = &PolicyFee              ;
    BegYearVectors  ["PremTaxLoad"          ] = &PremTaxLoad            ;
    BegYearVectors  ["DacTaxLoad"           ] = &DacTaxLoad             ;
    BegYearVectors  ["SpecAmtLoad"          ] = &SpecAmtLoad            ;
    BegYearVectors  ["SepAcctCharges"       ] = &SepAcctCharges         ;

    // Deaths are assumed to come at the end of the year only; but
    // they're discounted by the proportion in force at the beginning.
    BegYearVectors  ["ClaimsPaid"           ] = &ClaimsPaid             ;
    BegYearVectors  ["DeathProceedsPaid"    ] = &DeathProceedsPaid      ;
    BegYearVectors  ["NetClaims"            ] = &NetClaims              ;
    BegYearVectors  ["NetPmt"               ] = &NetPmt                 ;

    EndYearVectors  ["AcctVal"              ] = &AcctVal                ;
    EndYearVectors  ["AVGenAcct"            ] = &AVGenAcct              ;
    EndYearVectors  ["AVSepAcct"            ] = &AVSepAcct              ;
    EndYearVectors  ["DacTaxRsv"            ] = &DacTaxRsv              ;
    EndYearVectors  ["CSVNet"               ] = &CSVNet                 ;
    EndYearVectors  ["CV7702"               ] = &CV7702                 ;
    EndYearVectors  ["EOYDeathBft"          ] = &EOYDeathBft            ;
    EndYearVectors  ["PrefLoanBalance"      ] = &PrefLoanBalance        ;
    EndYearVectors  ["TotalLoanBalance"     ] = &TotalLoanBalance       ;
    EndYearVectors  ["AvgDeathBft"          ] = &AvgDeathBft            ;
    EndYearVectors  ["SurrChg"              ] = &SurrChg                ;
    EndYearVectors  ["TermPurchased"        ] = &TermPurchased          ;
    EndYearVectors  ["BaseDeathBft"         ] = &BaseDeathBft           ;
    EndYearVectors  ["ProjectedCoiCharge"   ] = &ProjectedCoiCharge     ;

    OtherVectors    ["MlySAIntRate"         ] = &MlySAIntRate           ;
    OtherVectors    ["MlyGAIntRate"         ] = &MlyGAIntRate           ;
    OtherVectors    ["MlyHoneymoonValueRate"] = &MlyHoneymoonValueRate  ;
    OtherVectors    ["MlyPostHoneymoonRate" ] = &MlyPostHoneymoonRate   ;
    OtherVectors    ["AnnSAIntRate"         ] = &AnnSAIntRate           ;
    OtherVectors    ["AnnGAIntRate"         ] = &AnnGAIntRate           ;
    OtherVectors    ["AnnHoneymoonValueRate"] = &AnnHoneymoonValueRate  ;
    OtherVectors    ["AnnPostHoneymoonRate" ] = &AnnPostHoneymoonRate   ;
    OtherVectors    ["KFactor"              ] = &KFactor                ;

    OtherScalars    ["LapseMonth"           ] = &LapseMonth             ;
    OtherScalars    ["LapseYear"            ] = &LapseYear              ;
    OtherScalars    ["InitAnnLoanCredRate"  ] = &InitAnnLoanCredRate    ;
    OtherScalars    ["InitAnnGenAcctInt"    ] = &InitAnnGenAcctInt      ;
    OtherScalars    ["InitAnnSepAcctGrossInt"] = &InitAnnSepAcctGrossInt    ;
    OtherScalars    ["InitAnnSepAcctNetInt" ] = &InitAnnSepAcctNetInt   ;

    LedgerBase::Alloc();

    Init();
}

//============================================================================
void LedgerVariant::Copy(LedgerVariant const& obj)
{
    LedgerBase::Copy(obj);

    GenBasis_        = obj.GenBasis_       ;
    SepBasis_        = obj.SepBasis_       ;
    FullyInitialized = obj.FullyInitialized;
}

//============================================================================
void LedgerVariant::Destroy()
{
    FullyInitialized = false;
}

//============================================================================
void LedgerVariant::Init()
{
    // Initializes (almost) everything with zeros.
    LedgerBase::Initialize(GetLength());

    GenBasis_ = mce_gen_curr;
    SepBasis_ = mce_sep_full;

    LapseYear               = Length;
    LapseMonth              = 11;

    FullyInitialized        = false;
}

//============================================================================
void LedgerVariant::Init
    (BasicValues const& bv
    ,mcenum_gen_basis   gen_basis
    ,mcenum_sep_basis   sep_basis
    )
{
    Init(); // Zero out (almost) everything to start.

    GenBasis_ = gen_basis;
    SepBasis_ = sep_basis;

//  EOYDeathBft     =
//  AcctVal         =
//  CSVNet          =
//  CV7702          =
//  COICharge       =
//  ExpenseCharges  =
    MlySAIntRate               = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_monthly_rate
        );
    MlyGAIntRate               = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_monthly_rate
        );
    MlyHoneymoonValueRate      = bv.InterestRates_->HoneymoonValueRate
        (GenBasis_
        ,mce_monthly_rate
        );
    MlyPostHoneymoonRate       = bv.InterestRates_->PostHoneymoonGenAcctRate
        (GenBasis_
        ,mce_monthly_rate
        );
    AnnSAIntRate               = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_annual_rate
        );
    AnnGAIntRate               = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_annual_rate
        );
    AnnHoneymoonValueRate      = bv.InterestRates_->HoneymoonValueRate
        (GenBasis_
        ,mce_annual_rate
        );
    AnnPostHoneymoonRate       = bv.InterestRates_->PostHoneymoonGenAcctRate
        (GenBasis_
        ,mce_annual_rate
        );

//  PrefLoanBalance =
//  TotalLoanBalance=
//  AvgDeathBft     =
//  SurrChg         =
//  TermPurchased   =
//  BaseDeathBft    =
//  ProjectedCoiCharge =
//  KFactor         =

    InitAnnLoanCredRate = bv.InterestRates_->RegLnCredRate
        (GenBasis_
        ,mce_annual_rate
        )[0];

    InitAnnGenAcctInt = bv.InterestRates_->GenAcctNetRate
        (GenBasis_
        ,mce_annual_rate
        )
        [0]
        ;

    InitAnnSepAcctGrossInt = bv.InterestRates_->SepAcctGrossRate(SepBasis_)[0];

    InitAnnSepAcctNetInt = bv.InterestRates_->SepAcctNetRate
        (SepBasis_
        ,GenBasis_
        ,mce_annual_rate
        )
        [0]
        ;

    FullyInitialized = true;
}

//============================================================================
void LedgerVariant::set_run_basis(mcenum_run_basis b)
{
    set_cloven_bases_from_run_basis(b, GenBasis_, SepBasis_);
}

//============================================================================
LedgerVariant& LedgerVariant::PlusEq
    (LedgerVariant const&      a_Addend
    ,std::vector<double> const& a_Inforce
    )
{
    LedgerBase::PlusEq(a_Addend, a_Inforce);

    // Make sure total (this) has enough years to add all years of a_Addend to.
    LMI_ASSERT(a_Addend.Length <= Length);

    // TODO ?? If interest rates vary across a census, this will be wrong.
    InitAnnGenAcctInt           = a_Addend.InitAnnGenAcctInt;
    InitAnnSepAcctGrossInt      = a_Addend.InitAnnSepAcctGrossInt;
    InitAnnSepAcctNetInt        = a_Addend.InitAnnSepAcctNetInt;
    InitAnnLoanCredRate         = a_Addend.InitAnnLoanCredRate;
    // ET !! This is of the form 'x = (lengthof x) take y'.
    for(int j = 0; j < a_Addend.Length; j++)
        {
        MlySAIntRate              [j]  = a_Addend.MlySAIntRate              [j];
        MlyGAIntRate              [j]  = a_Addend.MlyGAIntRate              [j];
        MlyHoneymoonValueRate     [j]  = a_Addend.MlyHoneymoonValueRate     [j];
        MlyPostHoneymoonRate      [j]  = a_Addend.MlyPostHoneymoonRate      [j];
        AnnSAIntRate              [j]  = a_Addend.AnnSAIntRate              [j];
        AnnGAIntRate              [j]  = a_Addend.AnnGAIntRate              [j];
        AnnHoneymoonValueRate     [j]  = a_Addend.AnnHoneymoonValueRate     [j];
        AnnPostHoneymoonRate      [j]  = a_Addend.AnnPostHoneymoonRate      [j];
        KFactor                   [j]  = a_Addend.KFactor                   [j];
        }

    LMI_ASSERT(a_Addend.GenBasis_ == GenBasis_);
    LMI_ASSERT(a_Addend.SepBasis_ == SepBasis_);

    if(LapseYear == a_Addend.LapseYear)
        {
        LapseMonth              = std::max(LapseMonth, a_Addend.LapseMonth);
        }
    else if(LapseYear < a_Addend.LapseYear)
        {
        LapseMonth              = a_Addend.LapseMonth;
        }
    LapseYear                   = std::max(LapseYear, a_Addend.LapseYear);

    return *this;
}

//============================================================================
void LedgerVariant::RecordDynamicSepAcctRate
    (double annual_rate
    ,double monthly_rate
    ,int year
    )
{
    AnnSAIntRate[year] = annual_rate;
    if(0 == year)
        {
        InitAnnSepAcctNetInt = annual_rate;
        }
    MlySAIntRate[year] = monthly_rate;
}

//============================================================================
void LedgerVariant::UpdateCRC(CRC& a_crc) const
{
    LedgerBase::UpdateCRC(a_crc);
}

//============================================================================
void LedgerVariant::Spew(std::ostream& os) const
{
    mcenum_run_basis b(mce_run_gen_curr_sep_full);
    set_run_basis_from_cloven_bases(b, GenBasis_, SepBasis_);
    os << "Basis" << "==" << mc_str(b) << '\n';
    LedgerBase::Spew(os);
}

ledger_map_holder::ledger_map_holder()
{
}

ledger_map_holder::ledger_map_holder(ledger_map_t const& z)
    :held_(z)
{
}

ledger_map_holder::~ledger_map_holder()
{
}

ledger_map_t const& ledger_map_holder::held() const
{
    return held_;
}

