// Ledger data--unit test.
//
// Copyright (C) 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "ledger.hpp"
#include "ledger_evaluator.hpp"
#include "ledger_invariant.hpp"
#include "ledger_text_formats.hpp"      // ledger_format()
#include "ledger_variant.hpp"
#include "oecumenic_enumerations.hpp"

#include "test_tools.hpp"
#include "timer.hpp"

#include <cstdio>                       // remove()

void authenticate_system() {} // Do-nothing stub.

class ledger_test
{
  public:
    static void test()
        {
        test_default_initialization();
        test_evaluator();
        test_ledger_format();
        test_speed();
        }

  private:
    static void test_default_initialization();
    static void test_evaluator();
    static void test_ledger_format();
    static void test_speed();
};

void ledger_test::test_default_initialization()
{
    Ledger ledger(100, mce_finra, false, false, false);

    LMI_TEST_EQUAL(mce_finra, ledger.ledger_type());
    LMI_TEST_EQUAL(false    , ledger.nonillustrated());
    LMI_TEST_EQUAL(false    , ledger.no_can_issue());
    LMI_TEST_EQUAL(false    , ledger.is_composite());
    LMI_TEST_EQUAL(100      , ledger.greatest_lapse_dur());

    LedgerInvariant const& invar = ledger.GetLedgerInvariant();

    LMI_TEST_EQUAL(100      , invar.GetLength());
    LMI_TEST_EQUAL(0        , invar.Age);
    LMI_TEST_EQUAL(100      , invar.EndtAge);
}

void ledger_test::test_evaluator()
{
    Ledger ledger(100, mce_finra, false, false, false);
    ledger.ledger_invariant_->WriteTsvFile = true;
    ledger_evaluator z {ledger.make_evaluator()};
    z.write_tsv("tsv_eraseme");
    LMI_TEST(0 == std::remove("tsv_eraseme.values.tsv"));
}

void ledger_test::test_ledger_format()
{
    constexpr double pi {3.14159265358979323851};
    constexpr std::pair<int,oenum_format_style> f1(0, oe_format_normal);
    constexpr std::pair<int,oenum_format_style> f2(2, oe_format_normal);
    constexpr std::pair<int,oenum_format_style> f3(0, oe_format_percentage);
    constexpr std::pair<int,oenum_format_style> f4(2, oe_format_percentage);
    constexpr std::pair<int,oenum_format_style> g1(9, oe_format_normal);
    constexpr std::pair<int,oenum_format_style> g2(4, oe_format_percentage);
    constexpr std::pair<int,oenum_format_style> g3(4, oe_cents_as_dollars);
    LMI_TEST_EQUAL("3"          , ledger_format(pi, f1));
    LMI_TEST_EQUAL("3.14"       , ledger_format(pi, f2));
    LMI_TEST_EQUAL("314%"       , ledger_format(pi, f3));
    LMI_TEST_EQUAL("314.16%"    , ledger_format(pi, f4));
    LMI_TEST_EQUAL("3.141592654", ledger_format(pi, g1));
    LMI_TEST_EQUAL("314.1593%"  , ledger_format(pi, g2));
    LMI_TEST_EQUAL("0.0314"     , ledger_format(pi, g3));
}

void ledger_test::test_speed()
{
    Ledger ledger(100, mce_finra, false, false, false);
    ledger.ledger_invariant_->WriteTsvFile = true;
    ledger_evaluator z {ledger.make_evaluator()};

    auto f0 = [       ]() {Ledger(100, mce_finra, false, false, false);};
    auto f1 = [&ledger]() {ledger.make_evaluator();};
    auto f2 = [&z     ]() {z.write_tsv("tsv_eraseme");};
    std::cout
        << "\nLedger speed tests:"
        << "\n  construct        : " << TimeAnAliquot(f0)
        << "\n  make_evaluator() : " << TimeAnAliquot(f1)
        << "\n  write_tsv()      : " << TimeAnAliquot(f2)
        << std::endl
        ;
    LMI_TEST(0 == std::remove("tsv_eraseme.values.tsv"));
}

int test_main(int, char*[])
{
    ledger_test::test();

    return EXIT_SUCCESS;
}
