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

#ifndef database_hpp
#define database_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

// Database of product parameters

class TDBValue;
class yare_input;

class LMI_SO TDatabase
    :private boost::noncopyable
    ,virtual private obstruct_slicing<TDatabase>
{
  public:
    TDatabase
        (std::string const& a_ProductName
        ,mcenum_gender      a_Gender
        ,mcenum_class       a_Class
        ,mcenum_smoking     a_Smoker
        ,int                a_IssueAge
        ,mcenum_uw_basis    a_UWBasis
        ,mcenum_state       a_State
        );
    explicit TDatabase(yare_input const&);
    // Ctor for unit-testing support.
    explicit TDatabase(int length);
    ~TDatabase();

    mcenum_state GetStateOfJurisdiction() const;
    int length() const;

    // Return scalar: use double because it's convertible to int, bool, etc.
    // We'd rather do something like
    //   template<typename T, typename DBValue>
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

    int           length_;

    mcenum_gender   Gender;     // gender
    mcenum_class    Class;      // underwriting class
    mcenum_smoking  Smoker;     // smoker class
    int             IssueAge;   // issue age
    mcenum_uw_basis UWBasis;    // underwriting basis
    mcenum_state    State;      // state of jurisdiction
};

#endif // database_hpp

