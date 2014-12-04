// Default input test case for the GUI test suite.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "wx_test_case.hpp"

#include <wx/log.h>

#include <sstream>

/*
    Test default input.

    2. Inspect '*default.ill' for the following variable data:

     A. This date differs each month
        <EffectiveDate>2456598</EffectiveDate>

     B. Rates can change each month and differ among each distribution
        <GeneralAccountRate>0.042 [0, 1); 0.037 [1, maturity)</GeneralAccountRate>
 */
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
