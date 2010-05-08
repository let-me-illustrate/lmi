#include "ihs_database.cpp"
#if 0
// Product database.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
// [merged file will inherit 'ihs_database.cpp' copyright:]
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "dbdict.hpp"
#include "dbnames.hpp"
#include "dbvalue.hpp"
#include "oecumenic_enumerations.hpp" // methuselah
#include "yare_input.hpp"

#include <algorithm> // std::min()

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
    :length_  (0) // Undesirable: set to a nonzero value by initialize().
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
    length_ = static_cast<int>(Query(DB_EndtAge)) - IssueAge;
    LMI_ASSERT(0 < length_ && length_ <= methuselah);
}

//===========================================================================
double product_database::Query(int k) const
{
    database_entity const& v = GetEntry(k);
    LMI_ASSERT(1 == v.GetLength());
    return *v[index_];
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
        dst.reserve(length_);
        dst.assign(z, z + std::min(length_, v.GetLength()));
        dst.resize(length_, dst.back());
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
#endif // 0

