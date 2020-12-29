// 7702 (and 7702A) interest rates.
//
// Copyright (C) 2020 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "irc7702_interest.hpp"

#include "contains.hpp"
#include "global_settings.hpp"

/// Statutory interest rate for 7702 and 7702A (except GSP).
///
/// Conditionally use the H.R. 6800 transitional rates. See:
///   https://www.congress.gov/bill/116th-congress/house-bill/6800/text

double iglp()
{
    static bool const hr6800 =
           global_settings::instance().ash_nazg()
        && contains(global_settings::instance().pyx(), "HR6800")
        ;
    return hr6800 ? 0.02 : 0.04;
}

/// Statutory interest rate for 7702 (GSP only).
///
/// Implemented in terms of iglp(), q.v.

double igsp()
{
    return 0.02 + iglp();
}
