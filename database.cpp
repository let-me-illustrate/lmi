// Product database.
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

// $Id: database.cpp,v 1.2 2005-02-14 04:35:18 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database.hpp"

#include "alert.hpp"
#include "dbdict.hpp"
#include "dbnames.hpp"

#include <algorithm>    // std::min()
#include <iterator>
#include <ostream>

//============================================================================
TDatabase::TDatabase
    (std::string const& a_ProductName
    ,e_gender    const& a_Gender
    ,e_class     const& a_Class
    ,e_smoking   const& a_Smoker
    ,int                a_IssueAge
    ,e_uw_basis  const& a_UWBasis
    ,e_state     const& a_State
    )
    :Filename   (a_ProductName)
    ,length_    (0)
    ,Gender     (a_Gender)
    ,Class      (a_Class)
    ,Smoker     (a_Smoker)
    ,IssueAge   (a_IssueAge)
    ,UWBasis    (a_UWBasis)
    ,State      (a_State)
{
    DBDictionary::instance().Init(Filename);
    Init();
    length_ = 100 - IssueAge;
// TODO ?? This is better...once we implement DB_EndtAge.
//    length_ = static_cast<int>(Query(DB_EndtAge)) - IssueAge;
}

//============================================================================
TDatabase::~TDatabase()
{
}

//============================================================================
void TDatabase::Init()
{
    Index[0] = Gender;
    Index[1] = static_cast<int>(Class   );
    Index[2] = static_cast<unsigned int>(Smoker  );
    Index[3] = static_cast<unsigned int>(IssueAge);
    Index[4] = static_cast<unsigned int>(UWBasis );
    Index[5] = static_cast<unsigned int>(State   );

    Idx.Gender      () = Gender     ;
    Idx.Class       () = Class      ;
    Idx.Smoker      () = Smoker     ;
    Idx.IssueAge    () = IssueAge   ;
    Idx.UWBasis     () = UWBasis    ;
    Idx.State       () = State      ;
}

//===========================================================================
double TDatabase::Query(int k) const
{
    ConstrainScalar(k); // TODO ?? Is the extra overhead acceptable?
    return *GetEntry(k)[Index];
}

//===========================================================================
void TDatabase::Query(std::vector<double>& dst, int k) const
{
    TDBValue const& v = GetEntry(k);
    double const*const z = v.operator[](Index);
    dst.resize(length_);
    if(1 == v.GetNDims())
        {
        dst.assign(length_, *z);
        }
    else
        {
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
TDBValue const& TDatabase::GetEntry(int k) const
{
    TDBDictionary const& d = DBDictionary::instance().GetDictionary();
    TDBDictionary::const_iterator i = d.find(k);

    if(i == d.end())
        {
        fatal_error()
            << "Key "
            << GetDBNames()[k].ShortName
            << " not found. These keys were found:"
            ;
        for(i = d.begin(); i != d.end(); ++i)
            {
            fatal_error() << " " << GetDBNames()[(*i).first].ShortName;
            }
        if(d.empty())
            {
            fatal_error() << " [none]. Dictionary is empty." << (*i).first;
            }
        fatal_error() << LMI_FLUSH;
        }
    return (*i).second;
}

//===========================================================================
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
            << " varies by duration, but it must not. "
            << "It will be treated as not varying. "
            << "The illustration will be incorrect. "
            << "Values by duration: "
            ;
        std::copy
            (z.begin()
            ,z.end()
            ,std::ostream_iterator<double>(hobsons_choice(), " ")
            );
        hobsons_choice() << LMI_FLUSH;
//        throw std::runtime_error(error.str());
        }
}

