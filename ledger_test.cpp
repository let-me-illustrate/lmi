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

// $Id: ledger_test.cpp,v 1.2 2006-12-11 14:31:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger.hpp"
#include "ledger_formatter.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "global_settings.hpp"
#include "inputillus.hpp"
#include "single_cell_document.hpp"
#include "test_tools.hpp"

#include <boost/shared_ptr.hpp>

#include <ios>

class LedgerTest
{
  public:
    LedgerTest();

    void Test() const;

  private:
    boost::shared_ptr<Ledger const> ledger_;
};

LedgerTest::LedgerTest()
{
    global_settings::instance().set_data_directory("/opt/lmi/data");

    single_cell_document document("sample.ill");
    AccountValue av(document.input_data());
    av.RunAV();
    // TODO ?? Should this be an invariant postcondition?
    LMI_ASSERT(av.ledger_from_av().get());
    ledger_ = av.ledger_from_av();
}

void LedgerTest::Test() const
{
    LMI_ASSERT(ledger_.get());
    Ledger const& ledger = *ledger_.get();

    std::ios_base::openmode const open_flags =
            std::ios_base::out
        |   std::ios_base::trunc
        |   std::ios_base::binary
        ;

    std::ofstream ofs0("sample.xml", open_flags);
    ledger.write(ofs0);
    BOOST_TEST(ofs0.good());

    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter ledger_formatter = factory.CreateFormatter(ledger);

    std::ofstream ofs1("calculation_summary.html", open_flags);
    ledger_formatter.FormatAsHtml(ofs1);
    BOOST_TEST(ofs1.good());

    std::ofstream ofs2("calculation_summary.tsv", open_flags);
    ledger_formatter.FormatAsLightTSV(ofs2);
    BOOST_TEST(ofs2.good());

    std::ofstream ofs3("microcosm.tsv", open_flags);
    ledger_formatter.FormatAsTabDelimited(ofs3);
    BOOST_TEST(ofs3.good());
}

int test_main(int, char*[])
{
    LedgerTest tester;
    tester.Test();

    return EXIT_SUCCESS;
}

