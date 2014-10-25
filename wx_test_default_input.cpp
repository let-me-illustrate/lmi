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

LMI_WX_TEST_CASE(default_input)
{
    calendar_date const today;
    calendar_date const first_of_month(today.year(), today.month(), 1);

    Input const& cell = default_cell();
    calendar_date const effective_date = exact_cast<tnr_date>(cell["EffectiveDate"])->value();
    LMI_ASSERT_EQUAL(effective_date, first_of_month);

    std::string const general_account_date = exact_cast<numeric_sequence>(cell["GeneralAccountRate"])->value();
    LMI_ASSERT(!general_account_date.empty());
    wxLogMessage("GeneralAccountRate is \"%s\"", general_account_date.c_str());
}
