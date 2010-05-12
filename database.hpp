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
#include "dbnames.hpp" // e_database_key
#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

class database_entity;
class yare_input;

/// Database of product parameters.

class LMI_SO product_database
    :private boost::noncopyable
    ,virtual private obstruct_slicing<product_database>
{
    friend class input_test;

  public:
    product_database
        (std::string const& a_ProductName
        ,mcenum_gender      a_Gender
        ,mcenum_class       a_Class
        ,mcenum_smoking     a_Smoker
        ,int                a_IssueAge
        ,mcenum_uw_basis    a_UWBasis
        ,mcenum_state       a_State
        );
    explicit product_database(yare_input const&);
    // Special ctor implemented only in a unit-test TU.
    explicit product_database(int length);
    ~product_database();

    mcenum_state GetStateOfJurisdiction() const;
    int length() const;

    // Return scalar: use double because it's convertible to int, bool, etc.
    // Someday, consider doing something like:
    //   template<typename T, typename DBValue>
    //   void Query(T&, e_database_key) const;
    double Query(e_database_key) const;
    void Query(std::vector<double>&, e_database_key) const;

    bool are_equivalent(e_database_key, e_database_key) const;
    bool varies_by_state(e_database_key) const;

  private:
    void initialize();

    database_entity const& entity_from_key(e_database_key) const;

    database_index  index_;
    int             length_;

    mcenum_gender   Gender;   // gender
    mcenum_class    Class;    // underwriting class
    mcenum_smoking  Smoker;   // smoker class
    int             IssueAge; // issue age
    mcenum_uw_basis UWBasis;  // underwriting basis
    mcenum_state    State;    // state of jurisdiction
};

#endif // database_hpp

