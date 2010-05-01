// Product-database entity.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef dbvalue_hpp
#define dbvalue_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <string>
#include <vector>

/// Product-database entity.
///
/// Each entity varies across zero or more of the following axes:
///   - gender
///   - underwriting class
///   - smoker
///   - issue age
///   - underwriting basis
///   - state
///   - duration [i.e., number of years since issue]
/// in that order.
///
/// The last index is duration; i.e., duration varies most rapidly of
/// all axes. In a typical query, all other axes are single-valued,
/// but all durations are wanted; this axis ordering puts consecutive
/// durational values in contiguous storage for efficient retrieval.

class LMI_SO TDBValue
    :virtual private obstruct_slicing<TDBValue>
{
  public:
    TDBValue();
    TDBValue
        (int                key
        ,int                ndims
        ,int const*         dims
        ,double const*      data
        ,std::string const& gloss = std::string()
        );
    TDBValue(TDBValue const&);
    TDBValue& operator=(TDBValue const&);
    ~TDBValue();

    double const* operator[](int const* idx) const;
    int GetKey()    const {return key_;}
    int GetNDims()  const {return ndims_;}
    int GetLength() const {return dims_[TDBIndex::MaxIndex];}

  private:
    int  getndata()      const;

    int                 key_;
    int                 ndims_; // Antediluvian: number of dimensions
    int*                dims_;  // Antediluvian: dimensions
    int                 ndata_; // Antediluvian: number of data
    double*             data_;  // Antediluvian: data
    std::string         gloss_;
};

/*
Database items should be allowed to vary across numerous axes, such as
    gender
    underwriting class (e.g. preferred, standard, and various substd tables)
    smoker
    issue age (TODO ?? or attained age as optional alternative?)
    medical/paramedical/nonmedical
    rate bands (see below)
and maybe
    months (e.g. lapse skewness)
    mode (e.g. for lapse rate or mode weighting)
and last of all
    duration

Does it make sense to use one axis each for
    issue age--every year
    issue age--quinquennial
    issue age--decennial?
Is it more natural to allow just the first, and offer a variety of methods
for interpolation? Or does it make sense to offer just one issue-age axis,
but provide a means of choosing whether it means annual, quinquennial, or
whatever? I'm inclined to use just one axis.

Rate bands are a horse of a different color. All axes are discrete, but for the
others, the quantum values are dictated by nature. Even if a fractional gender
status is contemplated as for a unisex product, database items are likely to be
either a combination of discrete quantum states or a precalculated average that
does not vary across the gender axis. But band breaks may vary across products.

We could address this by adding a list of values, rather than hardcoding it.
If we do that for band, then why not for gender as well? Why not for all axes?

We choose not to make current/guaranteed a database axis. Of course it's a
conceptual axis, across which many database entries do vary. But in practice
the guaranteed and current versions of such an entry will often have different
shapes. For instance, current COI rates may be select and ultimate while
guaranteed COI rates are attained age--and if we represent this variation as
an axis here, guaranteed COI rates must be coerced into a select and ultimate
form. We think this problem is unlikely to arise with the axes we've chosen.

The intention is to use this database for offline storage of almost all data.
We want to provide an interface to the SOA's mortality table manager as an
option. This is advantageous because it's a standard published program with
carefully checked tables that will probably be expanded in the future. It's an
option because not everyone will have it installed; for a build of this system
that is limited to illustration applications, it may be desired not to use the
SOA program for reasons of space.
*/

#endif // dbvalue_hpp

