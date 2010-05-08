// Product database.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "dbdict.hpp"
#include "dbnames.hpp"
#include "yare_input.hpp"

#include <algorithm> // std::copy(), std::min()
#include <iterator>  // ostream_iterator

//============================================================================
product_database::product_database
    (std::string const& // a_ProductName
    ,mcenum_gender      a_Gender
    ,mcenum_class       a_Class
    ,mcenum_smoking     a_Smoker
    ,int                a_IssueAge
    ,mcenum_uw_basis    a_UWBasis
    ,mcenum_state       a_State
    )
    :length_  (0)
    ,Gender   (a_Gender)
    ,Class    (a_Class)
    ,Smoker   (a_Smoker)
    ,IssueAge (a_IssueAge)
    ,UWBasis  (a_UWBasis)
    ,State    (a_State)
{
//    DBDictionary::instance().Init(a_ProductName);
    DBDictionary::instance().InitAntediluvian();
    initialize();
}

//============================================================================
product_database::product_database(yare_input const& input)
{
    Gender      = input.Gender;
    Class       = input.UnderwritingClass;
    Smoker      = input.Smoking;
    IssueAge    = input.IssueAge;
    UWBasis     = input.GroupUnderwritingType;
    State       = input.State;

//    DBDictionary::instance().Init("Irrelevant in antediluvian branch.");
    DBDictionary::instance().InitAntediluvian();
    initialize();
}

//============================================================================
product_database::~product_database()
{
}

//============================================================================
mcenum_state product_database::GetStateOfJurisdiction() const
{
    return State;
}

//============================================================================
int product_database::length() const
{
    return length_;
}

//============================================================================
void product_database::initialize()
{
    index_ = database_index(Gender, Class, Smoker, IssueAge, UWBasis, State);
    length_ = static_cast<int>(*GetEntry(DB_EndtAge)[index_]) - IssueAge;
}

//===========================================================================
double product_database::Query(int k) const
{
    constrain_scalar(k); // TODO ?? Is the extra overhead acceptable?
    return *GetEntry(k)[index_];
}

//===========================================================================
void product_database::Query(std::vector<double>& dst, int k) const
{
    database_entity const& v = GetEntry(k);
    double const*const z = v[index_];
    if(1 == v.GetLength())
        {
        dst.assign(length_, *z);
        }
    else
        {
        dst.resize(length_);
        int s = std::min(length_, v.GetLength());
        for(int j = 0; j < s; ++j)
            {
            dst[j] = z[j];
            }
        for(int j = s; j < length_; ++j)
            {
            dst[j] = z[s - 1];
            }
        }
}

//===========================================================================
database_entity const& product_database::GetEntry(int k) const
{
    dict_map const& d = DBDictionary::instance().GetDictionary();
    LMI_ASSERT(d.size() == GetDBNames().size());
    LMI_ASSERT(d.size() == DB_LAST);
    LMI_ASSERT(0 == DB_FIRST);
    LMI_ASSERT(DB_FIRST <= k && k < DB_LAST);
    dict_map::const_iterator i = d.find(k);
    LMI_ASSERT(i != d.end());
    return i->second;
}

/// Constrain the value extracted from the database to be scalar--i.e.,
/// invariant by duration. The database item may nonetheless vary
/// across any axis except duration.

void product_database::constrain_scalar(int k) const
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

