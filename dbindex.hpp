// Product-database lookup index.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef dbindex_hpp
#define dbindex_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"

#include <vector>

/// Number of database axes, excluding the special duration axis.

enum
    {number_of_indices = 6
    };

/// Ordered database axes.

enum
    {e_axis_gender    = 0
    ,e_axis_class     = 1
    ,e_axis_smoking   = 2
    ,e_axis_issue_age = 3
    ,e_axis_uw_basis  = 4
    ,e_axis_state     = 5
    ,e_axis_duration  = 6
    };

/// These enumerators facilitate compile-time assertions in the
/// product-database GUI, q.v.: an array cannot be indexed to
/// produce an arithmetic constant expression [5.19/3].

enum enum_database_dimensions
    {e_number_of_axes    = 1 + number_of_indices
    ,e_max_dim_gender    =   3
    ,e_max_dim_class     =   4
    ,e_max_dim_smoking   =   3
    ,e_max_dim_issue_age = 100
    ,e_max_dim_uw_basis  =   5
    ,e_max_dim_state     =  53
    ,e_max_dim_duration  = 100
    };

/// Product-database lookup index.
///
/// Implicitly-declared special member functions do the right thing.

class database_index
{
  public:
    database_index()
        :idx_(number_of_indices)
    {}

    database_index
        (mcenum_gender   gender
        ,mcenum_class    uw_class
        ,mcenum_smoking  smoker
        ,int             issue_age
        ,mcenum_uw_basis uw_basis
        ,mcenum_state    state
        )
        :idx_(number_of_indices)
    {
        idx_[0] = gender   ;
        idx_[1] = uw_class ;
        idx_[2] = smoker   ;
        idx_[3] = issue_age;
        idx_[4] = uw_basis ;
        idx_[5] = state    ;
    }

    std::vector<int> const& index_vector() const {return idx_;}

  private:
    std::vector<int> idx_;
};

#endif // dbindex_hpp

