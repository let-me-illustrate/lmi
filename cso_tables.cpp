// 1980, 2001, and 2017 CSO tables.
//
// Copyright (C) 2019 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "cso_tables.hpp"

/// Just a placeholder for the nonce.

std::vector<double> const& cso_table
    (oenum_cso_era    // cso_era
    ,oenum_autopisty  // autopisty
    ,oenum_alb_or_anb // alb_or_anb
    ,mce_gender       // gender
    ,mce_smoking      // smoking
    )
{
    static std::vector<double> placeholder {1.0};
    return placeholder;
}
