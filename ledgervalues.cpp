// Ledger data.
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

// $Id: ledgervalues.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledgervalues.hpp"

#include "accountvalue.hpp"
#include "alert.hpp"
#include "inputs.hpp"
#include "miscellany.hpp" // iso_8601_datestamp_terse

#include <algorithm>      // std::max(), std::min()
#include <iomanip>
#include <ios>
#include <ostream>
#include <string>

//============================================================================
IllusVal::IllusVal()
{
    Alloc();
}

//============================================================================
IllusVal::IllusVal(IllusVal const& obj)
{
    Alloc();
    Copy(obj);
}

//============================================================================
IllusVal& IllusVal::operator=(IllusVal const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Alloc();
        Copy(obj);
        }
    return *this;
}

//============================================================================
IllusVal::~IllusVal()
{
    Destroy();
}

//============================================================================
void IllusVal::Alloc()
{
    CurrValues = new TLedger(0);
    MdptValues = new TLedger(0);
    GuarValues = new TLedger(0);
}

//============================================================================
void IllusVal::Copy(IllusVal const& obj)
{
    CurrValues = obj.CurrValues;
    MdptValues = obj.MdptValues;
    GuarValues = obj.GuarValues;
}

//============================================================================
void IllusVal::Destroy()
{
    delete CurrValues;
    delete MdptValues;
    delete GuarValues;
}

//============================================================================
void IllusVal::Init
    (TLedger* a_CurrValues
    ,TLedger* a_MdptValues
    ,TLedger* a_GuarValues
    )
{
    *CurrValues = *a_CurrValues;
    *MdptValues = *a_MdptValues;
    *GuarValues = *a_GuarValues;
}

//============================================================================
double IllusVal::Run(InputParms const& IP) const
{
    AccountValue* AV = new AccountValue(IP);
    double z = AV->Run(AccountValue::AllBases);
    *CurrValues = *AV->CurrValues;
    *GuarValues = *AV->GuarValues;
    *MdptValues = *AV->MdptValues;
    delete AV;

    for(int j = 0; j < CurrValues->GetLength(); j++)
        {
        // I realize that the assertion as coded here must always fire.
        // I did it this way temporarily so that I could put a stop on
        // the assertion. The extra work doesn't belong in production code.
        // On the other hand, it affects speed very little and accuracy not at all.
        if
            (
                GuarValues->GrossPmt[j] != 0.0
            &&  GuarValues->GrossPmt[j] != CurrValues->GrossPmt[j]
            )
            LMI_ASSERT(GuarValues->GrossPmt[j] == CurrValues->GrossPmt[j]);
        if
            (
                MdptValues->GrossPmt[j] != 0.0
            &&  MdptValues->GrossPmt[j] != CurrValues->GrossPmt[j]
            )
            LMI_ASSERT(MdptValues->GrossPmt[j] == CurrValues->GrossPmt[j]);
        }

    return z;
}

// One column of seven characters ('Age 100')
// plus ten columns of twelve characters each (' 999,999,999')
// equals 127 columns; a nine-point font can do that on a page
// eight inches wide and still leave about a half-inch margin
// on both sides.
//
// TODO ?? Need to scale everything if anything's $1B or over.
// TODO ?? Want thousands separators.
namespace
{
    int g_width = 128;
    std::string center(std::string const& s)
        {
        int z = s.length();
        // std::max() used here because strings in inputs class might be too wide.
        std::string spaces(std::max(0, (g_width - z) / 2), char(' '));
        return spaces + s;
        }

    // Could put some html stuff here, e.g.
    std::ostream& endrow(std::ostream& os)
        {
        os << '\n';
        return os;
        }
}

//============================================================================
void IllusVal::Print(std::ostream& os) const
{
    // TODO ?? check os state

    // TODO ?? page number in footers
    PrintHeader              (os);
    PrintNarrativeSummary    (os);
    PrintKeyTerms            (os);
    PrintFooter              (os);
    PrintNumericalSummary    (os);
    PrintRequiredSignatures  (os);
    PrintFooter              (os);
    PrintTabularDetailHeader (os);
    PrintTabularDetail       (os);
    PrintFooter              (os);
    // TODO ?? Split into pages and paginate

    // TODO ?? check os state
}

//============================================================================
void IllusVal::PrintHeader(std::ostream& os) const
{
    os << center("Life Insurance Basic Illustration") << endrow;
    os << endrow;
    os << center("Prepared on " + iso_8601_datestamp_terse() + " by") << endrow;
    os << center(CurrValues->ProducerName) << endrow;
    os << center(CurrValues->ProducerStreet) << endrow;
    os << center(CurrValues->ProducerCity) << endrow;
    os << "Insured: " << CurrValues->Insured1 << endrow;
    os << "Issue age: " << CurrValues->Age << endrow;
    os << CurrValues->Gender << endrow;
// TODO ?? Add these things:
// =Gender&" "
// &IF(Preferred=0,"Preferred","Standard")&" "
// &IF(Smoker=0,"Smoker","")
// &IF(Smoker=1,"Nonsmoker","")
// &IF(Smoker=2,"Unismoke","")
//
//    CurrValues->Smoker
//    CurrValues->Preferred
//    CurrValues->RetAge
//    CurrValues->EndtAge
}

//============================================================================
void IllusVal::PrintFooter(std::ostream& os) const
{
// TODO ?? Page numbers
    os << "\f";
// TODO ?? Page breaks for HTML (CSS2 browsers only)
/*
 <div style="page-break-before:always">
 stuff on a new page
 </div>
*/
}

//============================================================================
void IllusVal::PrintNarrativeSummary(std::ostream& os) const
{
//  os << "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "
    os << center("Narrative summary") << endrow;
    os << endrow;
    os << "This is an illustration of a life insurance policy. It is not an offer of insurance. Availability is subject to underwriting." << endrow;
    os << endrow;
    os << "The premium outlay that must be paid to guarantee coverage for the term of the contract, subject to maximum premiums allowable" << endrow;
    os << "to qualify as a life insurance policy under the applicable provisions of the Internal Revenue Code, is" << endrow;
    os << endrow;
    os << "Policy features, riders or options, guaranteed or non-guaranteed, shown in the basic illustration include:" << endrow;
    os << endrow;
    os << "Their charges reduce the benefits and values of the policy." << endrow;
    os << endrow;
    os << "This illustration assumes that the currently illustrated nonguaranteed elements will continue unchanged for all years shown." << endrow;
    os << "This is not likely to occur, and actual results may be more or less favorable than those shown." << endrow;
    os << endrow;
    os << "Overhead is fully allocated to expense charges." << endrow;
    os << endrow;
    os << "Except as otherwise specified, values and benefits are not guaranteed. The assumptions on which they are based are subject to" << endrow;
    os << "change by the insurer. Actual results may be more or less favorable. If policy charges are paid using non-guaranteed values, the" << endrow;
    os << "charges continue to be required. Depending on actual results, the premium payer may need to continue or resume premium outlays." << endrow;
    os << endrow;
    os << "This illustration assumes that premiums are paid at the beginning of each period. Values and benefits are shown as of the end of" << endrow;
    os << "the year." << endrow;
    os << endrow;
}

//============================================================================
void IllusVal::PrintKeyTerms(std::ostream& os) const
{
    os << center("Column headings and key terms") << endrow;
    os << endrow;
    os << "Premium Outlay is the amount assumed to be paid." << endrow;
    os << endrow;
    os << "Deductions are expense charges and mortality charges." << endrow;
    os << endrow;
    os << "Account Value is the accumulated value of Premium Outlay, minus Deductions, plus interest." << endrow;
    os << endrow;
    os << "Surrender Value is what you get if you surrender the policy: Account Value minus any surrender charges or outstanding loans." << endrow;
    os << endrow;
    os << "Death Benefit is the amount payable by reason of death." << endrow;
    os << endrow;
    os << "Guaranteed values reflect guaranteed interest and Deductions." << endrow;
    os << endrow;
    os << "Non-guaranteed values reflect non-guaranteed interest and Deductions, which are subject to change." << endrow;
    os << endrow;
    os << "Midpoint values reflect the midpoint of guaranteed and non-guaranteed interest and Deductions." << endrow;
    os << endrow;
}

//--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+---
void IllusVal::PrintNumericalSummary(std::ostream& os) const
{
    os << center("Numerical summary") << endrow;
    os << endrow;
    os << "                    ------------Guaranteed------------- -------------Midpoint-------------- ----------Non-guaranteed-----------" << endrow;
    os << "            Premium     Account   Surrender       Death     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os << "   Year      Outlay       Value       Value     Benefit       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os << endrow;

    int summary_rows[] = {4, 9, 19, std::min(99, 69 - CurrValues->Age)};

    for(int j = 0; j < static_cast<int>(sizeof summary_rows / sizeof(int)); j++)
        {
        int row = summary_rows[j];
        // Skip row if it doesn't exist. For instance, if issue age is
        // 85 and maturity age is 100, then there is no twentieth duration.
        if(CurrValues->GetLength() < 1 + row)
            {
            continue;
            }

        os.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os.precision(0);
        os.width(7);

        os << std::setw( 7) << (1 + row)                    ;

        os.precision(2);

        os << std::setw(12) << CurrValues->GrossPmt[row]    ;

        os << std::setw(12) << GuarValues->AcctVal[row]     ;
        os << std::setw(12) << GuarValues->CSV[row]         ;
        os << std::setw(12) << GuarValues->EOYDeathBft[row] ;

        os << std::setw(12) << MdptValues->AcctVal[row]     ;
        os << std::setw(12) << MdptValues->CSV[row]         ;
        os << std::setw(12) << MdptValues->EOYDeathBft[row] ;

        os << std::setw(12) << CurrValues->AcctVal[row]     ;
        os << std::setw(12) << CurrValues->CSV[row]         ;
        os << std::setw(12) << CurrValues->EOYDeathBft[row] ;

        os << endrow;
        }

// TODO ?? Print "Age  70" instead of duration for last row.
// TODO ?? Does the reg require any other ages?
    os << endrow;
}

// TODO ?? Consider putting the signature blocks side by side.
//============================================================================
void IllusVal::PrintRequiredSignatures(std::ostream& os) const
{
    os << center("Required signatures") << endrow;
    os << endrow;
    os << "I have received a copy of this illustration and understand that" << endrow;
    os << "any non-guaranteed elements illustrated are subject to change" << endrow;
    os << "and could be either higher or lower." << endrow;
    os << "The agent has told me they are not guaranteed." << endrow;
    os << endrow;
    os << endrow;
    os << "_________________________  ______" << endrow;
    os << "Applicant or policy owner  Date" << endrow;
    os << endrow;
    os << "I certify that this illustration has been presented to the applicant" << endrow;
    os << "and that I have explained that any non-guaranteed elements" << endrow;
    os << "illustrated are subject to change. I have made no statements that" << endrow;
    os << "are inconsistent with the illustration." << endrow;
    os << endrow;
    os << endrow;
    os << "___________________________________________  ______" << endrow;
    os << "Producer or other authorized representative  Date" << endrow;
}

//============================================================================
void IllusVal::PrintTabularDetailHeader(std::ostream& os) const
{
    os << "Tabular detail" << endrow;
    os << endrow;
    os << "                                ------------Guaranteed------------- ----------Non-guaranteed-----------" << endrow;
    os << "             Age at     Premium     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os << "   Year    Year End      Outlay       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os << endrow;
}

//============================================================================
void IllusVal::PrintTabularDetail(std::ostream& os) const
{
    for(int j = 0; j < CurrValues->GetLength(); j++)
        {
        os.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os.precision(0);
        os.width(7);

        os << std::setw( 7) << (1 + j)                    ;
        os << std::setw(12) << (1 + j + CurrValues->Age)  ;

        os.precision(2);

        os << std::setw(12) << CurrValues->GrossPmt[j]    ;
//        os << std::setw(12) << CurrValues->WD[j]          ;
//        os << std::setw(12) << CurrValues->Loan[j]        ;
//        double net_pmt =
//                CurrValues->GrossPmt[j]
//            -   CurrValues->WD[j]
//            -   CurrValues->Loan[j]
//            ;
//        os << std::setw(12) << net_pmt                    ;

        os << std::setw(12) << GuarValues->AcctVal[j]     ;
        os << std::setw(12) << GuarValues->CSV[j]         ;
        os << std::setw(12) << GuarValues->EOYDeathBft[j] ;

        os << std::setw(12) << CurrValues->AcctVal[j]     ;
        os << std::setw(12) << CurrValues->CSV[j]         ;
        os << std::setw(12) << CurrValues->EOYDeathBft[j] ;

        os << endrow;
        }
}

