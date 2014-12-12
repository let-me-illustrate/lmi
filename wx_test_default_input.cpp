// Test selected parameters in the user-customizable default cell.
//
// Copyright (C) 2014 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "wx_test_case.hpp"

#include <wx/log.h>

#include <sstream>

// ERASE THIS BLOCK COMMENT WHEN IMPLEMENTATION COMPLETE. The block
// comment below changes the original specification, and does not
// yet describe the present code. Desired changes:
//  - Run this test only when the '--distribution' option is given.
//  - Write selected parameters to stdout as prescribed.
//  - "EffectiveDate" is now compared to the first day of the current
//    month, but should instead equal the first day of the next month.

/// Test selected parameters in the user-customizable default cell.
///
/// Run this test only when the '--distribution' option is given.
///
/// Write "ProductName" and "GeneralAccountRate" to stdout in that
/// order on a single line. We maintain several different binary
/// distributions, each with a specific default product, and that
/// product's general-account rate is a crucial parameter that often
/// varies from one month to the next, so a spot check seems wise.
///
/// Write "EffectiveDate" to stdout, as JDN and as YYYYMMDD,
/// all on a single line, e.g.:
///   EffectiveDate: 2457024 2015-01-01
///
/// Validate "EffectiveDate": when a binary distribution is prepared,
/// this is normally the first day of the next month. Perform this
/// test after writing parameters to stdout, so that they're still
/// written even if this test fails.

LMI_WX_TEST_CASE(default_input)
{
    calendar_date const today;
    calendar_date const first_of_month(today.year(), today.month(), 1);

    Input const& cell = default_cell();
    calendar_date effective_date;
    std::istringstream is(cell["EffectiveDate"].str());
    LMI_ASSERT(is >> effective_date);
    LMI_ASSERT_EQUAL(effective_date, first_of_month);

    std::string const general_account_rate = cell["GeneralAccountRate"].str();
    LMI_ASSERT(!general_account_rate.empty());
    wxLogMessage("GeneralAccountRate is \"%s\"", general_account_rate.c_str());
}

