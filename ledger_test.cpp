// Ledger data--unit test.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: ledger_test.cpp,v 1.1 2006-12-10 17:35:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger.hpp"

#include "account_value.hpp"
#include "inputs.hpp"
#include "test_tools.hpp"

#include <boost/shared_ptr.hpp>

struct LedgerTest
{
    LedgerTest();

    boost::shared_ptr<Ledger const> ledger_;
    static void Test();
};

LedgerTest::LedgerTest()
{
}

void LedgerTest::Test()
{
}

int test_main(int, char*[])
{
    LedgerTest::Test();
    return EXIT_SUCCESS;
}

