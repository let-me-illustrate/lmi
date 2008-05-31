// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: actuarial_table.hpp,v 1.14 2008-05-31 02:55:24 chicares Exp $

#ifndef actuarial_table_hpp
#define actuarial_table_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <iosfwd>
#include <string>
#include <vector>

/// Reentry methods for select tables.
///
/// Reentry occurs only on anniversary.
///
/// At least in the present implementation, illustrations reflect
/// reentry only retrospectively, because future reentry is subject to
/// qualification. Furthermore, it is assumed in general that only the
/// latest reentry is known and prior history is unavailable.
/// Therefore, reentry occurs at most once, and it is handled by
/// transforming the arguments of the raw table-lookup functions.
///
/// Important note: These transformations take the general form
///   map [A]+B to [C]+D
/// it being understood that a table's age limits must be respected:
/// if C would violate that constraint, then
///   [C+Z]+D-Z
/// is used instead, where Z (unconstrained as to sign) is chosen so
/// that [C+Z] is the closest permissible select age.
///
/// Let
///   CD = contract date
///   SD = [re]illustration date, CD <= SD
///   RD = last reset date,       RD <= SD
/// where RD may in general precede, follow, or coincide with CD. Let
///   s = [years by which SD follows  CD],  0 <= s
///   r = [years by which RD precedes CD], -r <= s
/// where '[]' is the greatest-integer function. Both durations are
/// measured from CD because the absolute value of the number of full
/// years between two dates depends on which is taken as the base for
/// calculation. They're measured in opposite directions to conform to
/// customary usage: speaking loosely, rates reset "r years ago", but
/// a reillustration begins "s years hence". Let
///   x = normal issue age
///   j = projection duration, measured from from (CD+s), 0 <= j
/// so the first rate actually used is for projection duration j=0.
///
/// e_reenter_never
///   map [x]+s+j to [x]+s+j [the identity mapping]
/// This is the default behavior.
///
/// e_reenter_at_inforce_duration
///   map [x]+s+j to [x+s]+j
/// Use this when rates are deemed to reset each year, but
/// illustrations are to reflect reentry only retrospectively, e.g.,
/// because no future reset is guaranteed. Rates for attained ages in
/// the half-open interval [x, x+s) would not be used because they
/// correspond to policy years preceding the illustration date, and
/// are indeterminate anyway because reentry history is unknown;
/// therefore, they may be set to zero, though that behavior is not
/// guaranteed.
///
/// e_reenter_upon_rate_reset
///   map [x]+s+j to [x-r]+r+s+j
/// Use this when rates were reset on a specified date, but
/// illustrations are to reflect reentry only retrospectively, e.g.,
/// because no future reset is guaranteed. Certificates issued on
/// divers dates may share a common group reset date that need not
/// coincide with a certificate anniversary; in that case, rates reset
/// on the last certificate anniversary preceding or coincident with
/// the group reset date. As the same case makes clear, there can be
/// no lower limit on the reset date: it can even precede a group
/// insured's date of birth.
///
/// All three methods are affected by the "Important note" above: even
/// e_reenter_never, if [x] exceeds max_select_age_ (see documentation
/// for specific_values()).
///
/// These methods are designed for select-and-ultimate tables. For
/// attained-age tables, they are all equivalent wrt all values that
/// are not documented above as indeterminate.

enum e_actuarial_table_method
    {e_reenter_never               = 0
    ,e_reenter_at_inforce_duration = 1
    ,e_reenter_upon_rate_reset     = 2
    };

/// Read a table from a database in the binary format designed by the
/// Society of Actuaries (SOA) and used for the tables SOA publishes.
///
/// Do not check CRCs of these tables as the SOA software does. Tests
/// show that CRC checking makes the illustration system considerably
/// slower. Data should generally be validated against published
/// checksums when acquired (e.g., downloaded), not before each use.
/// CRC checking was useful for guarding against transmission errors
/// when the first SOA tables were published, prior to the internet
/// revolution, but today it's a relic of a bygone era, and lmi uses a
/// stronger algorithm for data authentication already. Besides, the
/// CRCs in the SOA's tables have always been incorrect, and the SOA
/// has apparently chosen to leave them that way for backward
/// compatibility.

class actuarial_table
    :private boost::noncopyable
    ,virtual private obstruct_slicing<actuarial_table>
{
  public:
    actuarial_table(std::string const& filename, int table_number);
    ~actuarial_table();

    std::vector<double> values(int issue_age, int length) const;
    std::vector<double> values_elaborated
        (int                      issue_age
        ,int                      length
        ,e_actuarial_table_method method
        ,int                      full_years_since_issue
        ,int                      full_years_since_last_rate_reset
        ) const;

    std::string const& filename       () const {return filename_       ;}
    int                table_number   () const {return table_number_   ;}
    char               table_type     () const {return table_type_     ;}
    int                min_age        () const {return min_age_        ;}
    int                max_age        () const {return max_age_        ;}
    int                select_period  () const {return select_period_  ;}
    int                max_select_age () const {return max_select_age_ ;}

  private:
    void find_table();
    void parse_table();
    void read_values(std::istream& is, int nominal_length);
    std::vector<double> specific_values(int issue_age, int length) const;

    // Ctor arguments.
    std::string filename_     ;
    int         table_number_ ;

    // Table parameters, in order read from table header.
    char table_type_     ;
    int  min_age_        ;
    int  max_age_        ;
    int  select_period_  ;
    int  max_select_age_ ;

    std::vector<double> data_;

    std::streampos table_offset_;
};

/// Convenience function: read particular values from a table stored
/// in the SOA table-manager format.

std::vector<double> actuarial_table_rates
    (std::string const& table_filename
    ,int                table_number
    ,int                issue_age
    ,int                length
    );

/// Convenience function: read particular values from a table stored
/// in the SOA table-manager format, using a nondefault lookup method.

std::vector<double> actuarial_table_rates_elaborated
    (std::string const&       table_filename
    ,int                      table_number
    ,int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      full_years_since_issue
    ,int                      full_years_since_last_rate_reset
    );

#endif // actuarial_table_hpp

