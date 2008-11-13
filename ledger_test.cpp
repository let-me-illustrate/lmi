// Ledger data--unit test.
//
// Copyright (C) 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger_test.cpp,v 1.16 2008-11-13 21:37:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// Facilities offered by all of these headers are tested here.
#include "ledger.hpp"
#include "ledger_formatter.hpp"
#include "ledger_text_formats.hpp"

#include "account_value.hpp"
#include "assert_lmi.hpp"
#include "fenv_guard.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp" // files_are_identical()
#include "single_cell_document.hpp"
#include "test_tools.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <cstdio> // std::remove()
#include <ios>

class LedgerTest
{
  public:
    LedgerTest();

    void Test() const;
    void ValidateFile(fs::path const&, char const* file, int line) const;

  private:
    fs::path dir_cxx_;
    fs::path dir_xsl_;
    boost::shared_ptr<Ledger const> ledger_;
};

LedgerTest::LedgerTest()
    :dir_cxx_("eraseme_cxx")
    ,dir_xsl_("eraseme_xsl")
{
    fs::create_directory(dir_cxx_);
    fs::create_directory(dir_xsl_);

    global_settings::instance().set_data_directory("/opt/lmi/data");
    global_settings::instance().set_regression_testing(true);

    single_cell_document document("sample.ill");

    { // Begin fenv_guard scope.
    fenv_guard fg;
    AccountValue av(document.input_data());
    av.RunAV();
    ledger_ = av.ledger_from_av();
    } // End fenv_guard scope.
}

/// Compare different methods of writing ledger output.

void LedgerTest::Test() const
{
    LMI_ASSERT(ledger_.get());
    Ledger const& ledger = *ledger_.get();

    fs::path filepath0("sample.xml");
    // 'ledger_xml_io.cpp'
    fs::ofstream ofs0cxx(dir_cxx_ / filepath0, ios_out_trunc_binary());
    ledger.write(ofs0cxx);
    BOOST_TEST(ofs0cxx.good());
    ofs0cxx.close();
    // 'ledger_xml_io2.cpp'
    fs::ofstream ofs0xsl(dir_xsl_ / filepath0, ios_out_trunc_binary());
    ledger.writeXXX(ofs0xsl);
    BOOST_TEST(ofs0xsl.good());
    ofs0xsl.close();
    ValidateFile(filepath0, __FILE__, __LINE__);

    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter ledger_formatter = factory.CreateFormatter(ledger);

    fs::path filepath1("calculation_summary.html");
    // C++
    fs::ofstream ofs1cxx(dir_cxx_ / filepath1, ios_out_trunc_binary());
    ofs1cxx << FormatSelectedValuesAsHtml(ledger);
    BOOST_TEST(ofs1cxx.good());
    ofs1cxx.close();
    // xslt
    fs::ofstream ofs1xsl(dir_xsl_ / filepath1, ios_out_trunc_binary());
    ledger_formatter.FormatAsHtml(ofs1xsl);
    BOOST_TEST(ofs1xsl.good());
    ofs1xsl.close();
    ValidateFile(filepath1, __FILE__, __LINE__);

    fs::path filepath2("calculation_summary.tsv");
    // C++
    fs::ofstream ofs2cxx(dir_cxx_ / filepath2, ios_out_trunc_binary());
    ofs2cxx << FormatSelectedValuesAsTsv(ledger);
    BOOST_TEST(ofs2cxx.good());
    ofs2cxx.close();
    // xslt
    fs::ofstream ofs2xsl(dir_xsl_ / filepath2, ios_out_trunc_binary());
    ledger_formatter.FormatAsLightTSV(ofs2xsl);
    BOOST_TEST(ofs2xsl.good());
    ofs2xsl.close();
    ValidateFile(filepath2, __FILE__, __LINE__);

    fs::path filepath3("microcosm.tsv");
    // C++
    std::remove((dir_cxx_ / filepath3).string().c_str());
    PrintFormTabDelimited(ledger, (dir_cxx_ / filepath3).string());
    // xslt
    fs::ofstream ofs3xsl(dir_xsl_ / filepath3, ios_out_trunc_binary());
    ledger_formatter.FormatAsTabDelimited(ofs3xsl);
    BOOST_TEST(ofs3xsl.good());
    ofs3xsl.close();
    ValidateFile(filepath3, __FILE__, __LINE__);

    fs::path filepath4("sample.fo.xml");
    // 'ledger_xml_io.cpp'
    fs::ofstream ofs4cxx(dir_cxx_ / filepath4, ios_out_trunc_binary());
    ledger.write_xsl_fo(ofs4cxx);
    BOOST_TEST(ofs4cxx.good());
    ofs4cxx.close();
    // 'ledger_xml_io2.cpp' and 'ledger_formatter.cpp'
    fs::ofstream ofs4xsl(dir_xsl_ / filepath4, ios_out_trunc_binary());
    ledger_formatter.FormatAsXslFo(ofs4xsl);
    BOOST_TEST(ofs4xsl.good());
    ofs4xsl.close();
    ValidateFile(filepath4, __FILE__, __LINE__);
}

void LedgerTest::ValidateFile
    (fs::path const& p
    ,char const* file
    ,int line
    ) const
{
    std::string const file_cxx((dir_cxx_ / p).string());
    std::string const file_xsl((dir_xsl_ / p).string());
    bool okay = files_are_identical(file_cxx, file_xsl);

    INVOKE_BOOST_TEST(okay, file, line);
    // Leave the file for analysis if it didn't match.
    if(okay)
        {
        INVOKE_BOOST_TEST(0 == std::remove(file_cxx.c_str()), file, line);
        INVOKE_BOOST_TEST(0 == std::remove(file_xsl.c_str()), file, line);
        }
}

int test_main(int, char*[])
{
    LedgerTest tester;
    tester.Test();

    return EXIT_SUCCESS;
}

