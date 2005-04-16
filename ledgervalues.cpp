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

// $Id: ledgervalues.cpp,v 1.9 2005-04-16 02:05:41 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledgervalues.hpp"

#include "account_value.hpp"
#include "group_values.hpp"
#include "inputs.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp" // iso_8601_datestamp_terse()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>      // std::max(), std::min()
#include <iomanip>
#include <ios>
#include <ostream>
#include <string>

//============================================================================
IllusVal::IllusVal(std::string const& filename)
    :filename_ (filename)
    ,ledger_   (0)
{
}

//============================================================================
IllusVal::IllusVal(Ledger* ledger, std::string const& filename)
    :filename_ (filename)
    ,ledger_   (new Ledger(*ledger))
{
}

//============================================================================
IllusVal::~IllusVal()
{
}

//============================================================================
IllusVal& IllusVal::operator+=(Ledger const& addend)
{
    ledger_->PlusEq(addend);
    return *this;
}

//============================================================================
double IllusVal::Run(InputParms const& IP)
{
    AccountValue av(IP);

    fs::path debug_filename = fs::change_extension
        (fs::path(filename_)
        ,".debug"
        );
    av.SetDebugFilename(debug_filename.string());

    double z = av.RunAV();
// TODO ?? Consider using boost::shared_ptr to avoid copying.
    ledger_.reset(new Ledger(av.LedgerValues()));

// TODO ?? Temporary code for trying to track down a problem.
#if 0
//LMI_ASSERT(av.LedgerValues().GetLedgerMap() == ledger_->GetLedgerMap());
//LMI_ASSERT(av.LedgerValues().GetLedgerInvariant() == ledger_->GetLedgerInvariant());
LMI_ASSERT(av.LedgerValues().GetLedgerInvariant().GetInforceLives() == ledger_->GetLedgerInvariant().GetInforceLives());

LMI_ASSERT(av.LedgerValues().GetLedgerType() == ledger_->GetLedgerType());
LMI_ASSERT(av.LedgerValues().GetRunBases() == ledger_->GetRunBases());
LMI_ASSERT(av.LedgerValues().GetIsComposite() == ledger_->GetIsComposite());
#endif // 0

    return z;
}

// One column of seven characters ('Age 100')
// plus ten columns of twelve characters each (' 999,999,999')
// equals 127 columns; a nine-point font can do that on a page
// eight inches wide and still leave about a half-inch margin
// on both sides.
//
// TODO ?? Avoid overflow by scaling everything if anything's $1B or over.
// TODO ?? Add thousands separators.
namespace
{
    int g_width = 128;
    std::string center(std::string const& s)
        {
        int z = s.length();
        // TODO ?? Strings in the input class might be too wide;
        // absent more graceful handling, at least no attempt is made
        // to cure that problem with a negative number of spaces.
        std::string spaces(std::max(0, (g_width - z) / 2), char(' '));
        return spaces + s;
        }

    std::ostream& endrow(std::ostream& os)
        {
        os << '\n';
        return os;
        }
}

//============================================================================
void IllusVal::Print(std::ostream& os) const
{
    // TODO ?? Check os state at entry and exit.
    // TODO ?? Split into numbered pages.

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
}

//============================================================================
void IllusVal::PrintHeader(std::ostream& os) const
{
    os << center("Life Insurance Basic Illustration") << endrow;
    os << endrow;
    os << center("Prepared on " + iso_8601_datestamp_terse() + " by") << endrow;
    os << center(invar().ProducerName) << endrow;
    os << center(invar().ProducerStreet) << endrow;
    os << center(invar().ProducerCity) << endrow;
    os << "Insured: " << invar().Insured1 << endrow;
    os << "Issue age: " << invar().Age << endrow;
    os << invar().Gender << endrow;
// TODO ?? Add gender, smoker, and underwriting class.
}

//============================================================================
void IllusVal::PrintFooter(std::ostream& os) const
{
// TODO ?? Add page numbers.
    os << "\f";
}

//============================================================================
void IllusVal::PrintNarrativeSummary(std::ostream& os) const
{
//        "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 "
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

void IllusVal::PrintNumericalSummary(std::ostream& os) const
{
    os << center("Numerical summary") << endrow;
    os << endrow;
    os << "                    ------------Guaranteed------------- -------------Midpoint-------------- ----------Non-guaranteed-----------" << endrow;
    os << "            Premium     Account   Surrender       Death     Account   Surrender       Death     Account   Surrender       Death" << endrow;
    os << "   Year      Outlay       Value       Value     Benefit       Value       Value     Benefit       Value       Value     Benefit" << endrow;
    os << endrow;

    int summary_rows[] = {4, 9, 19, std::min(99, 69 - static_cast<int>(invar().Age))};

    for(int j = 0; j < static_cast<int>(sizeof summary_rows / sizeof(int)); ++j)
        {
        int row = summary_rows[j];
        // Skip row if it doesn't exist. For instance, if issue age is
        // 85 and maturity age is 100, then there is no twentieth duration.
        if(ledger_->GetMaxLength() < 1 + row)
            {
            continue;
            }

        os.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os.precision(0);
        os.width(7);

        os << std::setw( 7) << (1 + row)                ;

        os.precision(2);

        os << std::setw(12) << invar().GrossPmt[row]    ;

        os << std::setw(12) << guar_().AcctVal[row]     ;
        os << std::setw(12) << guar_().CSVNet[row]      ;
        os << std::setw(12) << guar_().EOYDeathBft[row] ;

        os << std::setw(12) << mdpt_().AcctVal[row]     ;
        os << std::setw(12) << mdpt_().CSVNet[row]      ;
        os << std::setw(12) << mdpt_().EOYDeathBft[row] ;

        os << std::setw(12) << curr_().AcctVal[row]     ;
        os << std::setw(12) << curr_().CSVNet[row]      ;
        os << std::setw(12) << curr_().EOYDeathBft[row] ;

        os << endrow;
        }

// TODO ?? Print "Age  70" instead of duration for last row. Does the
// illustration reg require any other ages?
    os << endrow;
}

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
    for(int j = 0; j < ledger_->GetMaxLength(); ++j)
        {
        os.setf(std::ios_base::fixed, std::ios_base::floatfield);
        os.precision(0);
        os.width(7);

        os << std::setw( 7) << (1 + j)                ;
        os << std::setw(12) << (1 + j + invar().Age)  ;

        os.precision(2);

        os << std::setw(12) << invar().GrossPmt[j]    ;

        os << std::setw(12) << guar_().AcctVal[j]     ;
        os << std::setw(12) << guar_().CSVNet[j]      ;
        os << std::setw(12) << guar_().EOYDeathBft[j] ;

        os << std::setw(12) << curr_().AcctVal[j]     ;
        os << std::setw(12) << curr_().CSVNet[j]      ;
        os << std::setw(12) << curr_().EOYDeathBft[j] ;

        os << endrow;
        }
}

inline LedgerInvariant const& IllusVal::invar() const
{return ledger_->GetLedgerInvariant();}

inline LedgerVariant   const& IllusVal::curr_() const
{return ledger_->GetCurrFull();}

inline LedgerVariant   const& IllusVal::guar_() const
{return ledger_->GetGuarFull();}

inline LedgerVariant   const& IllusVal::mdpt_() const
{return ledger_->GetMdptFull();}

