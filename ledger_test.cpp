// Ledger data--unit test.
//
// Copyright (C) 2018 Gregory W. Chicares.
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

#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"

#include "test_tools.hpp"

void authenticate_system() {} // Do-nothing stub.

class ledger_test
{
  public:
    static void test()
        {
        test_default_initialization();
        test1();
        }

  private:
    static void test_default_initialization();
    static void test1();
};

void ledger_test::test_default_initialization()
{
    Ledger ledger(100, mce_finra, false, false, false);

    BOOST_TEST_EQUAL(mce_finra, ledger.ledger_type());
    BOOST_TEST_EQUAL(false    , ledger.nonillustrated());
    BOOST_TEST_EQUAL(false    , ledger.no_can_issue());
    BOOST_TEST_EQUAL(false    , ledger.is_composite());
    BOOST_TEST_EQUAL(100      , ledger.greatest_lapse_dur());

    LedgerInvariant const& invar = ledger.GetLedgerInvariant();

    BOOST_TEST_EQUAL(100      , invar.GetLength());
    BOOST_TEST_EQUAL(0        , invar.Age);
    BOOST_TEST_EQUAL(100      , invar.EndtAge);
}

void ledger_test::test1()
{
}

int test_main(int, char*[])
{
    ledger_test::test();
    return EXIT_SUCCESS;
}
