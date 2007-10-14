// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: actuarial_table.hpp,v 1.7 2007-10-14 09:48:22 chicares Exp $

#ifndef actuarial_table_hpp
#define actuarial_table_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <iosfwd>
#include <string>
#include <vector>

/// Table-reentry methods. Let
///   j = projected duration from date of [re]illustration
///   r = number of full years since last rate reset
///   s = number of full years since issue
/// for 0 <= j, r, s.
///
/// e_reenter_never
///   map [x]+s+j to [x]+s+j [the identity mapping]
/// This is the default behavior.
///
/// e_reenter_at_inforce_duration
///   map [x]+s+j to [x+s]+j
/// Use this when rates are deemed to reset each year, but
/// illustrations are to reflect reentry only retrospectively, e.g.,
/// because no future reset is guaranteed.
///
/// e_reenter_upon_rate_reset
///   map [x]+s+j to [x-r]+s+r+j
/// Use this when rates were reset on a specified date, but
/// illustrations are to reflect reentry only retrospectively, e.g.,
/// because no future reset is guaranteed. This reset date can precede
/// the issue date, in order to accommodate certificates issued to a
/// group with a common reset date.
///
/// Reentry occurs only on anniversary.

enum e_actuarial_table_method
    {e_reenter_never               = 0
    ,e_reenter_at_inforce_duration = 1
    ,e_reenter_upon_rate_reset     = 2
    };

// Read a table from a database in the binary format designed by the
// Society of Actuaries (SOA) and used for the tables SOA publishes.

// Do not check CRCs of these tables as SOA's software does. Tests
// show that CRC checking makes the illustration system considerably
// slower. Data should generally be validated against published
// checksums when acquired (e.g., downloaded), not before each use.
// Local hardware that stores SOA tables probably performs a CRC
// already, as do networks and communications links. Repeating such
// tests in software is costly and redundant except for authentication
// of downloads, particularly against tampering; but for that purpose,
// a more secure algorithm should be used. Besides, SOA's software
// calculates CRCs incorrectly.

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

    // Table parameters, in order read from header.
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

