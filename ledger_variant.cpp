// Ledger data that vary by basis.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ledger_variant.hpp"

#include "assert_lmi.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()

#include <algorithm>                    // max()
#include <ostream>

//============================================================================
LedgerVariant::LedgerVariant(int len)
    :LedgerBase       (len)
    ,GenBasis_        {mce_gen_curr}
    ,SepBasis_        {mce_sep_full}
    ,FullyInitialized {false}
{
    Alloc(len);
}

//============================================================================
LedgerVariant::LedgerVariant(LedgerVariant const& obj)
    :LedgerBase       {obj}
    ,GenBasis_        {mce_gen_curr}
    ,SepBasis_        {mce_sep_full}
    ,FullyInitialized {false}
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

    BegYearVectors  ["COICharge"              ] = &COICharge              ; // f5
    BegYearVectors  ["RiderCharges"           ] = &RiderCharges           ; // f5
    BegYearVectors  ["ExpenseCharges"         ] = &ExpenseCharges         ; // f5

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
    BegYearVectors  ["AVRelOnDeath"           ] = &AVRelOnDeath           ; // f5
    BegYearVectors  ["NetIntCredited"         ] = &NetIntCredited         ; // f5
    BegYearVectors  ["GrossIntCredited"       ] = &GrossIntCredited       ; // f5
    BegYearVectors  ["LoanIntAccrued"         ] = &LoanIntAccrued         ; // f5

    BegYearVectors  ["PolicyFee"              ] = &PolicyFee              ; // f5
    BegYearVectors  ["PremTaxLoad"            ] = &PremTaxLoad            ; // f5
    BegYearVectors  ["DacTaxLoad"             ] = &DacTaxLoad             ; // f5
    BegYearVectors  ["SpecAmtLoad"            ] = &SpecAmtLoad            ; // f5
    BegYearVectors  ["SepAcctCharges"         ] = &SepAcctCharges         ; // f5

    // Deaths are assumed to come at the end of the year only; but
    // they're discounted by the proportion in force at the beginning.
    BegYearVectors  ["ClaimsPaid"             ] = &ClaimsPaid             ; // f5
    BegYearVectors  ["DeathProceedsPaid"      ] = &DeathProceedsPaid      ; // f5
    BegYearVectors  ["NetClaims"              ] = &NetClaims              ; // f5
    BegYearVectors  ["NetPmt"                 ] = &NetPmt                 ; // f5

    EndYearVectors  ["AcctVal"                ] = &AcctVal                ; // f5
    EndYearVectors  ["AVGenAcct"              ] = &AVGenAcct              ; // f5
    EndYearVectors  ["AVSepAcct"              ] = &AVSepAcct              ; // f5
    EndYearVectors  ["DacTaxRsv"              ] = &DacTaxRsv              ; // f5
    EndYearVectors  ["CSVNet"                 ] = &CSVNet                 ; // f5
    EndYearVectors  ["CV7702"                 ] = &CV7702                 ; // f5
    EndYearVectors  ["EOYDeathBft"            ] = &EOYDeathBft            ; // f5
    EndYearVectors  ["PrefLoanBalance"        ] = &PrefLoanBalance        ; // f5
    EndYearVectors  ["TotalLoanBalance"       ] = &TotalLoanBalance       ; // f5
    EndYearVectors  ["AvgDeathBft"            ] = &AvgDeathBft            ; // f5
    EndYearVectors  ["SurrChg"                ] = &SurrChg                ; // f5
    EndYearVectors  ["TermPurchased"          ] = &TermPurchased          ; // f5
    EndYearVectors  ["BaseDeathBft"           ] = &BaseDeathBft           ; // f5

    OtherVectors    ["AnnSAIntRate"           ] = &AnnSAIntRate           ;
    OtherVectors    ["AnnGAIntRate"           ] = &AnnGAIntRate           ;
    OtherVectors    ["AnnHoneymoonValueRate"  ] = &AnnHoneymoonValueRate  ;
    OtherVectors    ["AnnPostHoneymoonRate"   ] = &AnnPostHoneymoonRate   ;

    OtherScalars    ["LapseMonth"             ] = &LapseMonth             ;
    OtherScalars    ["LapseYear"              ] = &LapseYear              ;
    OtherScalars    ["InitAnnLoanCredRate"    ] = &InitAnnLoanCredRate    ;
    OtherScalars    ["InitAnnGenAcctInt"      ] = &InitAnnGenAcctInt      ;
    OtherScalars    ["InitAnnSepAcctGrossInt" ] = &InitAnnSepAcctGrossInt ;
    OtherScalars    ["InitAnnSepAcctNetInt"   ] = &InitAnnSepAcctNetInt   ;
    OtherScalars    ["InitTgtPremHiLoadRate"  ] = &InitTgtPremHiLoadRate  ;
    OtherScalars    ["InitMlyPolFee"          ] = &InitMlyPolFee          ;

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
    InitTgtPremHiLoadRate       = std::max(InitTgtPremHiLoadRate, a_Addend.InitTgtPremHiLoadRate);
    InitMlyPolFee               = std::max(InitMlyPolFee        , a_Addend.InitMlyPolFee        );
    InitAnnLoanCredRate         = a_Addend.InitAnnLoanCredRate;

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
    ,int year
    )
{
    AnnSAIntRate[year] = annual_rate;
    if(0 == year)
        {
        InitAnnSepAcctNetInt = annual_rate;
        }
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

ledger_map_holder::ledger_map_holder(ledger_map_t const& z)
    :held_ {z}
{
}

ledger_map_t const& ledger_map_holder::held() const
{
    return held_;
}
