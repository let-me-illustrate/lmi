// Ledger data--unit test.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: ledger_test.cpp,v 1.6 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// Facilities offered by both these headers are tested here.
#include "ledger.hpp"
#include "ledger_formatter.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "global_settings.hpp"
#include "inputillus.hpp"
#include "miscellany.hpp" // files_are_identical()
#include "single_cell_document.hpp"
#include "test_tools.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <ios>

class LedgerTest
{
  public:
    LedgerTest();

    void Test() const;
    void ValidateFile(fs::path const&, char const* file, int line) const;

  private:
    boost::shared_ptr<Ledger const> ledger_;
};

LedgerTest::LedgerTest()
{
    global_settings::instance().set_data_directory("/opt/lmi/data");
    global_settings::instance().set_regression_testing(true);

    single_cell_document document("sample.ill");
    AccountValue av(document.input_data());
    av.RunAV();
    // TODO ?? Should this be an invariant postcondition?
    LMI_ASSERT(av.ledger_from_av().get());
    ledger_ = av.ledger_from_av();
}

/// This test compares the files it creates to touchstones that, for
/// now at least, are saved in a particular directory, but in the
/// future might be put in cvs.

void LedgerTest::Test() const
{
    LMI_ASSERT(ledger_.get());
    Ledger const& ledger = *ledger_.get();

    fs::path filepath0("sample.xml");
    fs::ofstream ofs0(filepath0, ios_out_trunc_binary());
    ledger.write(ofs0);
    BOOST_TEST(ofs0.good());
    ofs0.close();
    ValidateFile(filepath0, __FILE__, __LINE__);

    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter ledger_formatter = factory.CreateFormatter(ledger);

    fs::path filepath1("calculation_summary.html");
    fs::ofstream ofs1(filepath1, ios_out_trunc_binary());
    ledger_formatter.FormatAsHtml(ofs1);
    BOOST_TEST(ofs1.good());
    ofs1.close();
    ValidateFile(filepath1, __FILE__, __LINE__);

    fs::path filepath2("calculation_summary.tsv");
    fs::ofstream ofs2(filepath2, ios_out_trunc_binary());
    ledger_formatter.FormatAsLightTSV(ofs2);
    BOOST_TEST(ofs2.good());
    ofs2.close();
    ValidateFile(filepath2, __FILE__, __LINE__);

    fs::path filepath3("microcosm.tsv");
    fs::ofstream ofs3(filepath3, ios_out_trunc_binary());
    ledger_formatter.FormatAsTabDelimited(ofs3);
    BOOST_TEST(ofs3.good());
    ofs3.close();
    ValidateFile(filepath3, __FILE__, __LINE__);
}

void LedgerTest::ValidateFile
    (fs::path const& p
    ,char const* file
    ,int line
    ) const
{
    fs::path testpath("/opt/lmi/stuff");
    fs::path testfile = testpath / p;
    bool okay = files_are_identical(p.string(), testfile.string());

    INVOKE_BOOST_TEST(okay, file, line);
    // Leave the file for analysis if it didn't match.
    if(okay)
        {
        INVOKE_BOOST_TEST
            (0 == std::remove(p.string().c_str())
            ,file
            ,line
            );
        }
}

int test_main(int, char*[])
{
    LedgerTest tester;
    tester.Test();

    return EXIT_SUCCESS;
}

