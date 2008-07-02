// Life insurance illustration inputs.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: inputstatus.cpp,v 1.8 2008-07-02 12:44:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "inputstatus.hpp"

#include "alert.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace
{
    int KludgeLength = 100; // TODO ?? KLUDGE
}

//============================================================================
InputStatus::InputStatus()
    :IssueAge          (45)
    ,RetAge            (65)
    ,Gender            (e_male)
    ,Smoking           (e_nonsmoker)
    ,Class             (e_standard)
    ,HasWP             ("No")
    ,HasADD            ("No")
    ,HasTerm           ("No")
    ,TermAmt           (0.0)
    ,TotalSpecAmt      (1000000.0)
    ,TermProportion    (0.0)
    ,TermUseProportion ("No")
    ,UseDOB            ("No")
    ,UseDOR            ("No")
    ,SubstdTable       (e_table_none)
{
    VectorMonthlyFlatExtra.assign(KludgeLength, 0.0);
}

//============================================================================
InputStatus::~InputStatus()
{
}

//============================================================================
bool InputStatus::operator==(InputStatus const& z) const
{
    if(IssueAge               != z.IssueAge              ) return false;
    if(RetAge                 != z.RetAge                ) return false;
    if(Gender                 != z.Gender                ) return false;
    if(Smoking                != z.Smoking               ) return false;
    if(Class                  != z.Class                 ) return false;
    if(HasWP                  != z.HasWP                 ) return false;
    if(HasADD                 != z.HasADD                ) return false;
    if(HasTerm                != z.HasTerm               ) return false;
    if(TermAmt                != z.TermAmt               ) return false;
    if(TotalSpecAmt           != z.TotalSpecAmt          ) return false;
    if(TermProportion         != z.TermProportion        ) return false;
    if(TermUseProportion      != z.TermUseProportion     ) return false;
    if(DOB                    != z.DOB                   ) return false;
    if(UseDOB                 != z.UseDOB                ) return false;
    if(DOR                    != z.DOR                   ) return false;
    if(UseDOR                 != z.UseDOR                ) return false;
    if(SubstdTable            != z.SubstdTable           ) return false;
    if(VectorMonthlyFlatExtra != z.VectorMonthlyFlatExtra) return false;

    return true;
}

//============================================================================
bool InputStatus::operator!=(InputStatus const& z) const
{
    return !operator==(z);
}

//============================================================================
int InputStatus::YearsToRetirement() const
{
    return RetAge.value() - IssueAge.value();
}

//============================================================================

/// TODO ?? Presumably the last argument to add_years() should be
/// 'false' in both instances below.
///
/// A person born on a leap-year day attains legal majority on the
/// first of March, not the twenty-eighth of February.

bool InputStatus::MakeAgesAndDatesConsistent
    (calendar_date const& EffDate
    ,bool                 UseANB
    ) const
{
    bool problem = false;
    if(UseDOB)
        {
        int const z = attained_age(DOB, EffDate, UseANB);
        if(z != IssueAge)
            {
            problem = true;
            warning()
                << "IssueAge is " << IssueAge
                << " but " << z
                << " was expected.\n"
                << IssueAge << " = IssueAge \n"
                << DOB.operator calendar_date const&().str() << " = DOB \n"
                << EffDate.str() << " = EffDate \n"
                << UseANB << " = UseANB \n"
                << LMI_FLUSH
                ;
            r_iss_age& mutable_issue_age = const_cast<r_iss_age&>(IssueAge);
            mutable_issue_age = z;
            }
        }
    else
        {
        calendar_date const z = add_years
            (DOB.operator calendar_date const&()
            ,attained_age(DOB, EffDate, UseANB) - IssueAge
            ,true
            );
        if(z != DOB)
            {
            problem = true;
            warning()
                << "DOB is " << DOB.operator calendar_date const&().str()
                << " but " << z.str()
                << " was expected.\n"
                << IssueAge << " = IssueAge \n"
                << DOB.operator calendar_date const&().str() << " = DOB \n"
                << EffDate.str() << " = EffDate \n"
                << UseANB << " = UseANB \n"
                << LMI_FLUSH
                ;
            calendar_date& mutable_dob = const_cast<calendar_date&>
                (DOB.operator calendar_date const&()
                );
            // If no DOB is supplied, a birthday is assumed to occur on the
            // issue date--as good an assumption as any, and the simplest.
            mutable_dob = z;
            }
        }

#if 0
// TODO ?? Either make this work correctly, or expunge it and remove
// retirement date completely. Making it work requires fixing the
// problem noted below, and modifying
//   Input::DoTransmogrify()
//   Input::DoHarmonize()
// accordingly.
    if(UseDOR)
        {
        r_ret_age& mutable_ret_age = const_cast<r_ret_age&>(RetAge);
        // TODO ?? Is this right? It seems to say that if a retirement
        // date is explicitly given, then attained age at retirement
        // can be calculated from retirement and effective dates alone;
        // but it would seem that birthdate should enter into that.
        // Is it the case that we constrain retirement dates to birthdays?
        mutable_ret_age = IssueAge + attained_age(EffDate, DOR, UseANB);
        }
    else
        {
        calendar_date& mutable_dor = const_cast<calendar_date&>
            (DOR.operator calendar_date const&()
            );
        mutable_dor = add_years
            (mutable_dor
            ,RetAge - attained_age(DOB, DOR, UseANB)
            ,true
            );
        }
#endif // 0
    return problem;
}

//============================================================================
bool InputStatus::AreFlatExtrasPresent() const
{
/* TODO ?? We'd like to do something like this, but need to
   make bc++5.02 support std::count() first:
VectorMonthlyFlatExtra.size()
!=
    std::count
        (VectorMonthlyFlatExtra.begin()
        ,VectorMonthlyFlatExtra.end()
        ,0.0
        );
*/
    bool flats_present = false;
    std::vector<double>::const_iterator i_flats;
    for
        (i_flats = VectorMonthlyFlatExtra.begin()
        ;i_flats < VectorMonthlyFlatExtra.end()
        ;++i_flats
        )
        {
        if(0.0 != *i_flats)
            {
            flats_present = true;
            break;
            }
        }
    return flats_present;
}

//============================================================================
bool InputStatus::IsPolicyRated() const
{
    return AreFlatExtrasPresent() || (e_table_none != SubstdTable);
}

