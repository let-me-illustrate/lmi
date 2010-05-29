// Product-database entity.
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

#ifndef dbvalue_hpp
#define dbvalue_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <iosfwd>
#include <string>
#include <vector>

namespace xml_serialize {template<typename T> struct xml_io;}

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
    {e_number_of_axes    = 1 + database_index::number_of_indices
    ,e_max_dim_gender    =   3
    ,e_max_dim_class     =   4
    ,e_max_dim_smoking   =   3
    ,e_max_dim_issue_age = 100
    ,e_max_dim_uw_basis  =   5
    ,e_max_dim_state     =  53
    ,e_max_dim_duration  = 100
    };

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
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO database_entity
    :virtual private obstruct_slicing<database_entity>
{
    friend struct xml_serialize::xml_io<database_entity>;

  public:
    database_entity();
    database_entity
        (int                key
        ,int                ndims
        ,int const*         dims
        ,double const*      data
        ,std::string const& gloss = std::string()
        );
    database_entity
        (int                        key
        ,std::vector<int> const&    dims
        ,std::vector<double> const& data
        ,std::string const&         gloss = std::string()
        );
    database_entity
        (int                key
        ,double             datum
        ,std::string const& gloss = std::string()
        );
    ~database_entity();

    void reshape(std::vector<int> const& dims);

    double const* operator[](database_index const& idx) const;
    double&       operator[](std::vector<int> const& idx);

    int key() const;
    int extent() const;
    std::vector<int>    const& axis_lengths() const;
    std::vector<double> const& data_values () const;

    std::ostream& write(std::ostream&) const;

  private:
    void assert_invariants() const;
    int getndata() const;
    static int getndata(std::vector<int> const&);

    void read (xml::element const&);
    void write(xml::element&) const;

    int                 key_;
    std::vector<int>    axis_lengths_;
    std::vector<double> data_values_;
    std::string         gloss_;
};

std::vector<int> const& maximum_database_dimensions();

/*
Some data are scalar by nature--for example, maturity age (DB_EndtAge).
Someday we might add a flag here to express that constraint; until then,
it's enforced passim by using a scalar-only Query() function.

Database items might include:
    attained age as an optional alternative to issue age
    rate bands (see below)
or even:
    months (e.g. lapse skewness)
    mode (e.g. for lapse rate or mode weighting)

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

Note however that the SOA program does not handle very large tables correctly
without modification. And even with modification it handles such tables slowly.
The CRC check is costly.

Probably the best approach is to use the SOA program for the things it does
well, and the database otherwise. What does the SOA program do well?
  usable GUI; new spreadsheet interface
    apparently an add-in written only for one non-free spreadsheet
  many tables, independently checked, often updated
It seems better to provide a utility to "compile" an SOA table to this database
format, and then always use the database. One advantage is that it'll run a lot
faster. Another is that the tables are less easily viewed or modified by people
who shouldn't; protecting integrity of data is a public policy concern, and
preventing fraud not inconsistent with open source software. Even though the
database code is open source, the data files it reads are not. It would be
simple enough to add a proprietary encryption layer as a plugin between the
present software and any sensitive file, with a default implementation that
performs no encryption.
*/

#endif // dbvalue_hpp

