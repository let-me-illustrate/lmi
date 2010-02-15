// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// TODO ?? Should length_ be dynamically reset when IssueAge is?
// TODO ?? Should State be dynamically reset?

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "dbnames.hpp"
#include "ihs_dbdict.hpp"
#include "ihs_dbvalue.hpp"
#include "ihs_proddata.hpp"
#include "oecumenic_enumerations.hpp"
#include "yare_input.hpp"

#include <algorithm>

// TODO ?? Things to improve:
//
// Can we dispense with Index? It's just an alias for a set of other
//   members that are used as input parameters to one ctor.

//============================================================================
TDatabase::TDatabase
    (std::string const& a_ProductName
    ,mcenum_gender      a_Gender
    ,mcenum_class       a_Class
    ,mcenum_smoking     a_Smoker
    ,int                a_IssueAge
    ,mcenum_uw_basis    a_UWBasis
    ,mcenum_state       a_State
    )
    :Filename   (AddDataDir
                    (TProductData(a_ProductName).GetDatabaseFilename()
                    )
                )
    ,Gender   (a_Gender)
    ,Class    (a_Class)
    ,Smoker   (a_Smoker)
    ,IssueAge (a_IssueAge)
    ,UWBasis  (a_UWBasis)
    ,State    (a_State)
{
    DBDictionary::instance().Init(Filename);
    Init();
}

//============================================================================
TDatabase::TDatabase(yare_input const& input)
    :Filename
        (AddDataDir
            (TProductData(input.ProductName).GetDatabaseFilename())
        )
{
// GET RID OF Gender, Class, Smoker, etc.
    Gender      = input.Gender;
    Class       = input.UnderwritingClass;
    Smoker      = input.Smoking;
    IssueAge    = input.IssueAge;
    UWBasis     = input.GroupUnderwritingType;
    State       = mce_s_CT; // Dummy initialization.

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
        case oe_ee_state:
            {
            State = input.State;
            }
            break;
        case oe_er_state:
            {
            State = input.CorporationState;
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
mcenum_state TDatabase::GetStateOfJurisdiction() const
{
    return State;
}

//============================================================================
int TDatabase::length() const
{
    return length_;
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
        fatal_error() << "Endowment age precedes issue age." << LMI_FLUSH;
        }
    ConstrainScalar(DB_EndtAge);
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
        std::string s("no name");
        if(0 <= k && static_cast<unsigned int>(k) < GetDBNames().size())
            {
            s = GetDBNames()[k].ShortName;
            }
        fatal_error()
            << "Database element "
            << k
            << " ('" << s << "')"
            << " not found."
            << LMI_FLUSH
            ;
        return *new TDBValue;
        }
    else
        {
        return (*p).second;
        }
}

//===========================================================================
/// Constrain the value extracted from the database to be scalar--i.e.,
/// invariant by duration. The database item may nonetheless vary
/// across any axis except duration.

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
        fatal_error()
            << "Database element "
            << GetDBNames()[k].ShortName
            << " varies by duration, but it must not. "
            << "Values by duration: "
            ;
        std::copy
            (z.begin()
            ,z.end()
            ,std::ostream_iterator<double>(fatal_error(), " ")
            );
        fatal_error() << LMI_FLUSH;
        }
}

