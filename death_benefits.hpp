// Death benefits.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#ifndef death_benefits_hpp
#define death_benefits_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "obstruct_slicing.hpp"
#include "uncopyable_lmi.hpp"

#include <vector>

class yare_input;

class death_benefits
    :        private lmi::uncopyable <death_benefits>
    ,virtual private obstruct_slicing<death_benefits>
{
  public:
    death_benefits(int, yare_input const&);
    ~death_benefits();

    void set_specamt (double z, int from_year, int to_year);
    void set_supplamt(double z, int from_year, int to_year);

    std::vector<mcenum_dbopt> const& dbopt   () const;
    std::vector<double>       const& specamt () const;
    std::vector<double>       const& supplamt() const;

  private:
    int length_;
    std::vector<mcenum_dbopt> dbopt_   ;
    std::vector<double>       specamt_ ;
    std::vector<double>       supplamt_;
};

inline std::vector<mcenum_dbopt> const& death_benefits::dbopt() const
{
    return dbopt_;
}

inline std::vector<double> const& death_benefits::specamt() const
{
    return specamt_;
}

inline std::vector<double> const& death_benefits::supplamt() const
{
    return supplamt_;
}

#endif // death_benefits_hpp

