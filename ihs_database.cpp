// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_database.cpp,v 1.3 2005-07-18 03:33:46 chicares Exp $

// TODO ?? Should length_ be dynamically reset when IssueAge is?
// TODO ?? Should State be dynamically reset?

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "dbnames.hpp"
#include "ihs_dbdict.hpp"
#include "ihs_dbvalue.hpp"
#include "ihs_proddata.hpp"

#ifdef STANDALONE
// nothing
#else
#   include "inputs.hpp"
#   include "inputstatus.hpp"
#endif  // STANDALONE

#include "miscellany.hpp"

#include <algorithm>

// TODO ?? Things to improve:
//
// Can we dispense with Index? It's just an alias for a set of other
//   members that are used as input parameters to one ctor.
//
// Change other code to use the simpler ctor(InputParms& input)?

//============================================================================
TDatabase::TDatabase
    (std::string const& a_ProductName
    ,e_gender const&    a_Gender
    ,e_class const&     a_Class
    ,e_smoking const&   a_Smoker
    ,int                a_IssueAge
    ,e_uw_basis const&  a_UWBasis
    ,e_state const&     a_State
    )
    :Filename   (AddDataDir
                    (TProductData(a_ProductName).GetDatabaseFilename()
                    )
                )
    ,Gender     (a_Gender)
    ,Class      (a_Class)
    ,Smoker     (a_Smoker)
    ,IssueAge   (a_IssueAge)
    ,UWBasis    (a_UWBasis)
    ,State      (a_State)
{
    DBDictionary::instance().Init(Filename);
    Init();
}

//============================================================================
TDatabase::TDatabase(InputParms const& input)
    :Filename
        (AddDataDir
            (TProductData(input.ProductName).GetDatabaseFilename())
        )
{
// GET RID OF Gender, Class, Smoker, etc.
    Gender      = input.Status[0].Gender;
    Class       = input.Status[0].Class;
    Smoker      = input.Status[0].Smoking;
    IssueAge    = input.Status[0].IssueAge;
    UWBasis     = input.GroupUWType;
    State       = e_s_CT; // dummy init

    DBDictionary::instance().Init(Filename);
    Init();

    // State of jurisdiction is governed by database item DB_PremTaxState.
    // (TODO ?? misnamed--rename it when we rebuild)
    // This must be determined by a database lookup, during construction
    // of the database object.

    // State of jurisdiction must not depend on itself
    TDBValue const& StateEntry = GetEntry(DB_PremTaxState);
    if(1 != StateEntry.GetLength(5))
        {
        fatal_error()
            << "Database invalid: circular dependency."
            << " State of jurisdiction depends on itself."
            << LMI_FLUSH
            ;
        // TODO ?? We should test this in the write() method.
        }
    switch(static_cast<int>(Query(DB_PremTaxState)))
        {
        case e_ee_state:
            {
            State = input.InsdState;
            }
            break;
        case e_er_state:
            {
            State = input.SponsorState;
            }
            break;
        default:
            {
            fatal_error()
                << "Cannot determine state of jurisdiction."
                << LMI_FLUSH
                ;
            }
            break;
        }

    Index[5] = State;
    Idx.State() = State;
}

//============================================================================
TDatabase::~TDatabase()
{
}

//============================================================================
void TDatabase::Init()
{
    Index[0] = Gender;
    Index[1] = Class;
    Index[2] = Smoker;
    Index[3] = IssueAge;
    Index[4] = UWBasis;
    Index[5] = State;

    Idx.Gender      () = Gender     ;
    Idx.Class       () = Class      ;
    Idx.Smoker      () = Smoker     ;
    Idx.IssueAge    () = IssueAge   ;
    Idx.UWBasis     () = UWBasis    ;
    Idx.State       () = State      ;

// New code added to Query(int) uses length_ to test
// for validity. We can't go through that validity check when
// calculating length_ itself, which requires retrieving
// endowment age. But once we have length_, we can make sure
// endowment age doesn't vary by duration.
//  length_ = Query(DB_EndtAge) - IssueAge;
    length_ = static_cast<int>(*GetEntry(DB_EndtAge)[Idx]) - IssueAge;
    if(length_ <= 0)
        {
        hobsons_choice() << "Endowment precedes issue age." << LMI_FLUSH;
        }
    ConstrainScalar(DB_EndtAge);

    std::vector<double> misc_fund_charge;
    Query(misc_fund_charge, DB_MiscFundCharge);
    if
        (!each_equal
            (misc_fund_charge.begin()
            ,misc_fund_charge.end()
            ,0.0
            )
        )
        {
        fatal_error()
            << "Database entity 'MiscFundCharge' is no longer supported, "
            << "but its value is not zero. Change it to zero and move the "
            << "charge elsewhere in the database (e.g. 'StableValFundCharge')."
            << LMI_FLUSH
            ;
        }
}

//===========================================================================
double TDatabase::Query(int k) const
{
    ConstrainScalar(k); // TODO ?? Is the extra overhead acceptable?
    return *GetEntry(k)[Idx];
}

//===========================================================================
void TDatabase::Query(std::vector<double>& dst, int k) const
{
    TDBValue const& v = GetEntry(k);
    double const*const z = v[Idx];
    // TODO ?? Can this be right?
    if(1 == v.GetNDims())
        {
        // TODO ?? Can this be right? Seems unreachable.
        int len = length_;   // TODO ?? First arg of assign not const?
        dst.assign(len, *z);
        }
    else
        {
        LMI_ASSERT(0 <= length_);       // TODO ?? KLUDGE
        LMI_ASSERT(0 <= v.GetLength()); // TODO ?? KLUDGE
        int s = std::min(length_, v.GetLength());
//      int t = std::max(0, length_ - v.GetLength());
        int t = std::max(0, length_ - s);
        dst.resize(0);

        for(int j = 0; j < s; j++)
            {
            dst.push_back(z[j]);
            }

        if(0 == t)
            {
            return;
            }
        for(int j = 0; j < t; j++)
            {
            dst.push_back(z[s - 1]);
            }

/*
//      dst.assign(s, *z);
//      copy_n(z, v.GetLength(), dst);
//      copy(&z[0], &z[v.GetLength()], dst.begin());
//      fill(dst.begin() + v.GetLength(), dst.end(), z[v.GetLength() - 1]);
*/
        }
}

//===========================================================================
TDBValue const& TDatabase::GetEntry(int k) const
{
    dict_map const& dict = DBDictionary::instance().GetDictionary();
    dict_map::const_iterator p = dict.find(k);
    if(p == dict.end())
        {
        // TODO ?? Report value of k too...
        // and what do we fill the data with?
        hobsons_choice() << "Value not found in database." << LMI_FLUSH;
        return *new TDBValue;
        }
    else
        {
        return (*p).second;
        }
}

//===========================================================================
// Constrain the value extracted from the database to be scalar--i.e.,
// invariant by duration. The database item may nonetheless vary across
// any axis except duration.
void TDatabase::ConstrainScalar(int k) const
{
    std::vector<double> z;
    Query(z, k);
    if
        (
            (0 != z.size())
        &&  (z == std::vector<double>(z.size(), z[0]))
        )
        {
        return;
        }
    else
        {
        hobsons_choice()
            << "Database element "
            << GetDBNames()[k].ShortName
            << " varies by duration, but it must not."
            << " It will be treated as not varying."
            << " The illustration will be incorrect."
            << LMI_FLUSH
            ;
        }
}

