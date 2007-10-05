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

// $Id: actuarial_table.hpp,v 1.4 2007-10-05 00:32:28 chicares Exp $

#ifndef actuarial_table_hpp
#define actuarial_table_hpp

#include "config.hpp"

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

/// Read values for a given age from an SOA table-manager database.

std::vector<double> actuarial_table
    (std::string const& table_filename
    ,int                table_number
    ,int                issue_age
    ,int                length
    );

/// Read values for a given age from an SOA table-manager database,
/// using a nondefault lookup method.

std::vector<double> actuarial_table_elaborated
    (std::string const&       table_filename
    ,int                      table_number
    ,int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      full_years_since_issue
    ,int                      full_years_since_last_rate_reset
    );

#endif // actuarial_table_hpp

