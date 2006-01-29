// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: database.hpp,v 1.6 2006-01-29 13:52:00 chicares Exp $

#ifndef database_hpp
#define database_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "xenumtypes.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

// Database of product parameters

class TDBValue;
class InputParms;

class LMI_SO TDatabase
    :private boost::noncopyable
    ,virtual private obstruct_slicing<TDatabase>
{
  public:
    TDatabase
        (std::string const& a_ProductName
        ,e_gender    const& a_Gender
        ,e_class     const& a_Class
        ,e_smoking   const& a_Smoker
        ,int                a_IssueAge
        ,e_uw_basis  const& a_UWBasis
        ,e_state     const& a_State
        );
    explicit TDatabase(InputParms const&);
    // Ctor for unit-testing support.
    explicit TDatabase(int length);
    ~TDatabase();

    e_state const& GetStateOfJurisdiction() const;
    int length() const;

    // Return scalar: use double because it's convertible to int, bool, etc.
    // We'd rather do something like
    //   template<class T, class DBValue>
    //   void Query(T& dst, int k) const;
    // but bc++5.02 can't handle member template functions.
    double Query(int k) const;

    void Query(std::vector<double>& dst, int k) const;

    void ConstrainScalar(int k) const;

    TDBValue const& GetEntry(int k) const;

  private:
    TDatabase();

    void Init();
    void Init(std::string const& NewFilename);

    int           Index[TDBIndex::MaxIndex];
    TDBIndex      Idx;
    std::string   Filename;

    int        length_;

    e_gender   Gender;     // gender
    e_class    Class;      // underwriting class
    e_smoking  Smoker;     // smoker class
    int        IssueAge;   // issue age
    e_uw_basis UWBasis;    // underwriting basis
    e_state    State;      // state of jurisdiction
};

#endif // database_hpp

