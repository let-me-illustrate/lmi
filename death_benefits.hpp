// Death benefits.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: death_benefits.hpp,v 1.2 2005-02-13 23:17:18 chicares Exp $

#ifndef death_benefits_hpp
#define death_benefits_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "xenumtypes.hpp"

#include <boost/utility.hpp>

#include <vector>

class BasicValues;

class death_benefits
    :private boost::noncopyable
    ,virtual private obstruct_slicing<death_benefits>
{
  public:
    explicit death_benefits(BasicValues const&);
    ~death_benefits();

    void set_specamt(double amount, int begin_year, int end_year);

    std::vector<e_dbopt> const& dbopt()   const;
    std::vector<double>  const& specamt() const;

  private:
    int length_;
    std::vector<e_dbopt> dbopt_;
    std::vector<double>  specamt_;
};

inline std::vector<e_dbopt> const& death_benefits::dbopt() const
{
    return dbopt_;
}

inline std::vector<double> const& death_benefits::specamt() const
{
    return specamt_;
}

#endif // death_benefits_hpp

