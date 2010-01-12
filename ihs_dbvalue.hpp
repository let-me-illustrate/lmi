// Database entities.
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

#ifndef ihs_dbvalue_hpp
#define ihs_dbvalue_hpp

#ifdef dbvalue_hpp
#   error Probable lmi/ihs header conflict.
#endif // dbvalue_hpp

#include "config.hpp"

#include "dbindex.hpp"
#include "so_attributes.hpp"

#include "ihs_fpios.hpp"
namespace JRPS = JOSHUA_ROWE_PERSISTENT_STREAMS;

#include <iosfwd>
#include <string>
#include <vector>

// Value of an entry in the database dictionary.

enum e_IdxType
    {e_Offset
    ,e_Discrete
    ,e_LowBound
    ,e_Incremental
    };

class LMI_SO TDBValue
    :private JRPS::JrPs_pstreamable
{
    friend std::istream& operator>>(std::istream&, TDBValue&);
    friend std::ostream& operator<<(std::ostream&, TDBValue const&);

  public:
    // Separate enumerators here facilitate compile-time assertions
    // in the database GUI, q.v.--an array could not be indexed to
    // produce an arithmetic constant expression [5.19/3].
    enum {e_number_of_axes    = 1 + TDBIndex::MaxIndex};
    enum {e_max_dim_gender    =   3};
    enum {e_max_dim_class     =   4};
    enum {e_max_dim_smoking   =   3};
    enum {e_max_dim_issue_age = 100};
    enum {e_max_dim_uw_basis  =   5};
    enum {e_max_dim_state     =  53};
    enum {e_max_dim_duration  = 100};

    TDBValue();
// TODO ?? Deprecate this interface in favor of the one that takes vectors.
    TDBValue
        (int           a_key
        ,int           a_ndims
        ,int const*    a_dims
        ,double const* a_data
        );
    TDBValue
        (int                        a_key
        ,std::vector<int> const&    a_dims
        ,std::vector<double> const& a_data
        );
    TDBValue
        (int    a_key
        ,double a_datum
        );
    TDBValue
        (int                      a_key
        ,int                      a_ndims
        ,int const*               a_dims
        ,double const*            a_data
        ,std::vector<std::string> a_extra_axes_names
        ,std::vector<double>      a_extra_axes_values
        );
    TDBValue(TDBValue const&);
    TDBValue& operator=(TDBValue const&);
    virtual ~TDBValue();

    int GetKey()              const;
    // TODO ?? Isn't the following function useless?
    int GetNDims()            const;
    int GetLength()           const;
    int GetLength(int a_axis) const;
    std::vector<int> const& GetAxisLengths() const;

    double const* operator[](TDBIndex const& a_idx) const;
    double&       operator[](std::vector<int> const& a_idx);

    void Reshape(std::vector<int> const& a_dims);

    std::ostream& write(std::ostream&) const;

    static std::vector<int> const& maximum_dimensions();
    static bool Equivalent(TDBValue const&, TDBValue const&);
    // TODO ?? Experimental. If this turns out to be a good idea,
    // then parameterize it.
    static bool VariesByState(TDBValue const&);

  private:
    int  getndata()                           const;
    void ParanoidCheck()                      const;
    bool AreAllAxesOK()                       const;
    void FixupIndex(std::vector<double>& idx) const;

    int  key;        // Database dictionary key

    // Each database item has a number of axes.
    // Every item has
    //   gender
    //   underwriting class
    //   smoker
    //   issue age
    //   underwriting basis
    //   state
    // in that order as its first six axes.
    // Any item can have any number of additional custom axes after those six.
    // All items have duration as their last axis. Duration comes last
    // so that a pointer calculated from all preceding axes points to
    // consecutive durational elements in contiguous storage.
    std::vector<int>    axis_lengths;
    std::vector<double> data_values;

    // For the first six axes, the allowable range of values is hardcoded;
    // we know their names because they are fixed.
    // We know the names of the first six axes because they are fixed,
    // and we know their ranges of permissible values because they are
    // hardcoded. For each custom axis, we need to specify its name and
    // enumerate its of permissible values; its number of permissible
    // values is in axis_lengths.
    std::vector<std::string> extra_axes_names;
    std::vector<double>      extra_axes_values;
    std::vector<e_IdxType>   extra_axes_types;

// The following sections don't follow the normal order for access
// specifiers. Grouping them together here facilitates their
// expunction as soon as we get rid of 'ihs_[f]pios.?pp'.

  public:
    static JRPS::JrPs_pstreamable* jrps_build();
    friend JRPS::JrPs_ipstream& operator>> (JRPS::JrPs_ipstream&, TDBValue*&);
    friend JRPS::JrPs_opstream& operator<< (JRPS::JrPs_opstream&, TDBValue const*);

  protected:
    virtual void*   read(JRPS::JrPs_ipstream&);
    virtual void    write(JRPS::JrPs_opstream&) const;

  private:
    TDBValue(JRPS::JrPs_pstreamableInit);
    virtual char const* streamableName() const;
    enum {StreamingVersion = 1};
};

/*
Database items should be allowed to vary across numerous axes, such as
    gender
    underwriting class (e.g. preferred, standard, and various substd tables)
    smoker
    issue age (or attained age as optional alternative?)
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

#endif // ihs_dbvalue_hpp

