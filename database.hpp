// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef database_hpp
#define database_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "dbnames.hpp"                  // e_database_key
#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <string>
#include <vector>

class database_entity;
class yare_input;

/// Database of product parameters.

class LMI_SO product_database
    :        private lmi::uncopyable <product_database>
    ,virtual private obstruct_slicing<product_database>
{
    friend class input_test; // For test_product_database().

  public:
    product_database
        (std::string const& ProductName
        ,mcenum_gender      Gender
        ,mcenum_class       UnderwritingClass
        ,mcenum_smoking     Smoking
        ,int                IssueAge
        ,mcenum_uw_basis    GroupUnderwritingType
        ,mcenum_state       StateOfJurisdiction
        );
    explicit product_database(yare_input const&);
    // Special ctor implemented only in a unit-test TU.
    explicit product_database(int length);
    ~product_database();

    int length() const;
    database_index index() const;

    double Query(e_database_key) const;
    double Query(e_database_key, database_index const&) const;
    void Query(std::vector<double>&, e_database_key) const;
    void Query(std::vector<double>&, e_database_key, database_index const&) const;

    bool are_equivalent(e_database_key, e_database_key) const;
    bool varies_by_state(e_database_key) const;

  private:
    void initialize(std::string const& product_name);

    database_entity const& entity_from_key(e_database_key) const;

    database_index  index_;
    int             length_;
    int             maturity_age_;
};

#endif // database_hpp

