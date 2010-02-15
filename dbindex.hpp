// Product database lookup index.
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

#include <vector>

// Database lookup index

class TDBIndex
{
  public:
    enum{MaxIndex = 6};

    TDBIndex()
        :idx(6)
        {
        }

/*
    TDBIndex
        (mcenum_gender       a_Gender    // gender
        ,mcenum_class        a_Class     // underwriting class
        ,mcenum_smoking      a_Smoker    // smoker class
        ,int                 a_IssueAge  // issue age
        ,mcenum_uw_basis     a_UWBasis   // underwriting basis
        ,mcenum_state        a_State     // state of jurisdiction
        );
*/

// TODO ?? These data aren't very well encapsulated.
    double& Gender  () {return idx[0];}
    double& Class   () {return idx[1];}
    double& Smoker  () {return idx[2];}
    double& IssueAge() {return idx[3];}
    double& UWBasis () {return idx[4];}
    double& State   () {return idx[5];}

    std::vector<double> const& GetIdx() const {return idx;}

  private:
// TODO ?? Erase these?
    TDBIndex(TDBIndex const&);
    TDBIndex& operator=(TDBIndex const&);

    std::vector<double> idx;
};

#endif // dbindex_hpp

