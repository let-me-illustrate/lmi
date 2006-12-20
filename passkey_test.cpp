// Data-file and date validation--unit test.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: passkey_test.cpp,v 1.45 2006-12-20 12:51:33 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "secure_date.hpp"

#include "md5.hpp"
#include "miscellany.hpp"
#include "system_command.hpp"
#include "test_tools.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstddef> // std::size_t
#include <cstdio>
#include <cstring> // std::memcmp(), std::memcpy()
#include <fstream>
#include <string>
#include <vector>

// TODO ?? Add tests for diagnostics that aren't tested yet.

template<typename T, std::size_t n>
std::string md5_str(T(&md5sum)[n])
{
    LMI_ASSERT(n == md5len);
    typedef std::vector<unsigned char> svuc;
    return md5_hex_string(svuc(md5sum, md5sum + md5len));
}

/// Data-file and date validation--unit test.
///
/// Non-special public members are declared in invocation order.

class PasskeyTest
{
  public:
    PasskeyTest();
    ~PasskeyTest();

    void RemoveTestFiles(char const* file, int line) const;

    void InitializeDataFile() const;
    void InitializeAndTestMd5sumOfDataFile() const;
    void InitializeMd5sumFile() const;
    void InitializePasskeyFile() const;
    void InitializeExpiryFile() const;

    void TestNominal() const;
    void TestFromAfar() const;
    void TestDate() const;
    void TestPasskey() const;
    void TestDataFile() const;
    void TestExpiry() const;

  private:
    calendar_date const  BeginDate_;
    calendar_date const  EndDate_;
    fs::path const       Pwd_;

    static unsigned char DataMd5sum_[];
    static unsigned char FileMd5sum_[];
};

unsigned char PasskeyTest::DataMd5sum_[md5len];
unsigned char PasskeyTest::FileMd5sum_[md5len];

/// Before writing any test file, remove any old copy that may be left
/// over from a previous run that failed to complete, because old
/// copies can cause spurious error reports.

PasskeyTest::PasskeyTest()
    :BeginDate_(ymd_t(20010101))
    ,EndDate_  (ymd_t(20010103))
    ,Pwd_      (fs::current_path())
{
    RemoveTestFiles(__FILE__, __LINE__);

    InitializeDataFile();
    InitializeAndTestMd5sumOfDataFile();
    InitializeMd5sumFile();
    InitializePasskeyFile();
    InitializeExpiryFile();
}

PasskeyTest::~PasskeyTest()
{
    RemoveTestFiles(__FILE__, __LINE__);
}

void PasskeyTest::RemoveTestFiles(char const* file, int line) const
{
    std::vector<std::string> filenames;
    filenames.push_back("expiry");
    filenames.push_back("passkey");
    filenames.push_back("coleridge");
    filenames.push_back(md5sum_file());
    typedef std::vector<std::string>::const_iterator aut0;
    for(aut0 i = filenames.begin(); i != filenames.end(); ++i)
        {
        std::remove(i->c_str());
        INVOKE_BOOST_TEST(!fs::exists(*i), file, line);
        }
}

void PasskeyTest::InitializeDataFile() const
{
    char const rime[] =
        "It is an ancient Mariner,\n"
        "And he stoppeth one of three.\n"
        "'By thy long grey beard and glittering eye,\n"
        "Now wherefore stopp'st thou me?\n\n"

        "The Bridegroom's doors are opened wide,\n"
        "And I am next of kin;\n"
        "The guests are met, the feast is set:\n"
        "May'st hear the merry din.'\n\n"

        "He holds him with his skinny hand,\n"
        "'There was a ship,' quoth he.\n"
        "'Hold off! unhand me, grey-beard loon!'\n"
        "Eftsoons his hand dropt he.\n\n"
        ;

    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << rime;
    os.close();

    // Here, '-1 +' excludes the trailing null character.

    md5_buffer(rime, -1 + static_cast<int>(sizeof rime), DataMd5sum_);
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(DataMd5sum_));

    // Make sure the file's md5sum equals the buffer's.

    unsigned char expected[md5len];
    FILE* in = std::fopen("coleridge", "rb");
    md5_stream(in, expected);
    std::fclose(in);
    BOOST_TEST_EQUAL(0, std::memcmp(expected, DataMd5sum_, sizeof expected));
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(expected));
}

void PasskeyTest::InitializeAndTestMd5sumOfDataFile() const
{
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(DataMd5sum_));

    std::ofstream os(md5sum_file(), ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << md5_hex_string
        (std::vector<unsigned char>(DataMd5sum_, DataMd5sum_ + md5len)
        );
    os << "  coleridge\n";
    os.close();

    // Make sure an 'md5sum' program is available. Regrettably,
    // invoking that program here writes its output to stdout; but
    // without this test, it would be difficult to tell whether errors
    // in subsequent tests stem from incorrect md5sums or from that
    // program's absence.

    std::cout << "  Result of 'md5sum --version':" << std::endl;
    BOOST_TEST_EQUAL(0, system_command("md5sum --version"));

    // For production, a file with md5 sums of all data files is
    // provided. For this unit test, treat file 'coleridge' as the
    // only data file. Its md5 sum is embedded in the file created
    // above. Creating that file portably here by running 'md5sum'
    // would be no trivial task, as it would require redirection, so
    // the effect of 'md5sum' was instead emulated; testing that file
    // here with 'md5sum' validates that emulation and guards against
    // a bogus 'md5sum' program.

    BOOST_TEST_EQUAL
        (0
        ,system_command("md5sum --check --status " + std::string(md5sum_file()))
        );
}

void PasskeyTest::InitializeMd5sumFile() const
{
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(DataMd5sum_));

    FILE* md5 = std::fopen(md5sum_file(), "rb");
    md5_stream(md5, FileMd5sum_);
    std::fclose(md5);

    BOOST_TEST_EQUAL("efb7a0a972b88bb5b9ac6f60390d61bf", md5_str(FileMd5sum_));
}

void PasskeyTest::InitializePasskeyFile() const
{
    BOOST_TEST_EQUAL("efb7a0a972b88bb5b9ac6f60390d61bf", md5_str(FileMd5sum_));

    // The passkey is the md5 sum of the md5 sum of the '.md5' file.
    // A more secure alternative could be wrought if wanted, but the
    // present method is enough to stymie the unsophisticated.

    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::memcpy(c_passkey, FileMd5sum_, md5len);
    BOOST_TEST_EQUAL("efb7a0a972b88bb5b9ac6f60390d61bf", md5_str(c_passkey));
    md5_buffer(c_passkey, md5len, u_passkey);
    BOOST_TEST_EQUAL("8a4829bf31de9437c95aedaeead398d7", md5_str(u_passkey));
    std::memcpy(c_passkey, u_passkey, md5len);
    BOOST_TEST_EQUAL("8a4829bf31de9437c95aedaeead398d7", md5_str(c_passkey));
    md5_buffer(c_passkey, md5len, u_passkey);
    BOOST_TEST_EQUAL("3ff4953dbddf009634922fa52a342bfe", md5_str(u_passkey));

    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << md5_hex_string
        (std::vector<unsigned char>(u_passkey, u_passkey + md5len)
        );
}

void PasskeyTest::InitializeExpiryFile() const
{
    std::ofstream os("expiry");
    BOOST_TEST(!!os);
    os << BeginDate_ << ' ' << EndDate_;
    os.close();
}

void PasskeyTest::TestNominal() const
{
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
}

void PasskeyTest::TestFromAfar() const
{
    // Test validation from a remote directory (using a valid date).
    // This should not alter the current directory.

    fs::path const remote_dir_0(fs::complete("/tmp"));
    BOOST_TEST(fs::exists(remote_dir_0) && fs::is_directory(remote_dir_0));
    BOOST_TEST_EQUAL(0, chdir(remote_dir_0.string().c_str()));
    BOOST_TEST_EQUAL(remote_dir_0.string(), fs::current_path().string());
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL(remote_dir_0.string(), fs::current_path().string());
    BOOST_TEST_EQUAL(0, chdir(Pwd_.string().c_str()));
    BOOST_TEST_EQUAL(Pwd_.string(), fs::current_path().string());

#if defined LMI_MSW
    // Try the root directory on a different drive, on a multiple-root
    // system. This is perforce platform specific; msw is used because
    // it happens to be common. This test assumes that an 'E:' drive
    // exists and is not the "current" drive.

    fs::path const remote_dir_1(fs::complete(fs::path("E:/", fs::native)));
    BOOST_TEST(fs::exists(remote_dir_1) && fs::is_directory(remote_dir_1));
    BOOST_TEST_EQUAL(0, chdir(remote_dir_1.string().c_str()));
    BOOST_TEST_EQUAL(remote_dir_1.string(), fs::current_path().string());
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL(remote_dir_1.string(), fs::current_path().string());
    BOOST_TEST_EQUAL(0, chdir(Pwd_.string().c_str()));
    BOOST_TEST_EQUAL(Pwd_.string(), fs::current_path().string());
#endif // defined LMI_MSW
}

void PasskeyTest::TestDate() const
{
    // The first day of the valid period should work. Repeating the
    // test immediately validates caching.

    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(BeginDate_, Pwd_));

    // The last day of the valid period should work.

    calendar_date const last_date = EndDate_ - 1;
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(last_date, Pwd_));
    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(last_date, Pwd_));

    // Test one day before the valid period, one day after, and
    // another day a bit later.

    BOOST_TEST_EQUAL
        ("Current date '2000-12-31' is invalid:"
        " this system cannot be used before '2001-01-01'."
        " Contact the home office."
        ,SecurityValidator::Validate(BeginDate_ - 1, Pwd_)
        );
    BOOST_TEST_EQUAL
        ("Current date '2001-01-03' is invalid:"
        " this system expired on '2001-01-03'."
        " Contact the home office."
        ,SecurityValidator::Validate(EndDate_, Pwd_)
        );
    BOOST_TEST_EQUAL
        ("Current date '2001-01-13' is invalid:"
        " this system expired on '2001-01-03'."
        " Contact the home office."
        ,SecurityValidator::Validate(EndDate_ + 10, Pwd_)
        );

    // Make sure that the last-successfully-validated date is not
    // inadvertently accepted due only to caching. It should be
    // accepted, but only after the data files and the date have been
    // tested afresh.

    BOOST_TEST_UNEQUAL("cached", SecurityValidator::Validate(last_date, Pwd_));

    // Now that a valid date has been cached, caching should again
    // work normally.

    BOOST_TEST_EQUAL  ("cached", SecurityValidator::Validate(last_date, Pwd_));
}

void PasskeyTest::TestPasskey() const
{
    // Test with an incorrect passkey. Caching can prevent this from
    // being detected--intentionally, because it is expensive to test
    // the data files. To demonstrate this: first validate the date,
    // in order to get it cached; then write an incorrect 'passkey'
    // file and retest.

    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    std::ofstream os0("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os0);
    std::vector<unsigned char> const wrong(md5len);
    os0 << md5_hex_string(wrong);
    os0.close();

    // Testing with the same date succeeds due to caching.

    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(BeginDate_, Pwd_));

    // Reset the cache, and the incorrect passkey is detected.

    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Passkey is incorrect for this version. Contact the home office."
        ,SecurityValidator::Validate(BeginDate_, Pwd_)
        );

    // Testing with no passkey file produces a different diagnostic.

    std::remove("passkey");
    BOOST_TEST(!fs::exists("passkey"));
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read passkey file 'passkey'. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, ".")
        );

    // Testing with a passkey file of incorrect length produces yet
    // another diagnostic.

    std::ofstream os1("passkey", ios_out_trunc_binary());
    os1 << "wrong";
    BOOST_TEST(!!os1);
    os1.close();
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Length of passkey 'wrong' is 5 but should be 32. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, Pwd_)
        );

    // Fix the passkey, and everything works again.

    InitializePasskeyFile();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
}

void PasskeyTest::TestDataFile() const
{
    // Test with an altered data file. This is intended to fail.

    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << "This file has the wrong md5sum.";
    os.close();

    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("At least one required file is missing, altered, or invalid."
        " Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, Pwd_)
        );
}

void PasskeyTest::TestExpiry() const
{
    std::remove("expiry");
    BOOST_TEST(!fs::exists("expiry"));
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read expiry file 'expiry'. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, ".")
        );

    std::ofstream os("expiry");
    BOOST_TEST(!!os);
    os << "bogus dates";
    os.close();
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Error reading expiry file 'expiry'. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, ".")
        );
}

int test_main(int, char*[])
{
    PasskeyTest tester;
    tester.TestNominal();
    tester.TestFromAfar();
    tester.TestDate();
    tester.TestPasskey();
    tester.TestDataFile();
    tester.TestExpiry();

    return EXIT_SUCCESS;
}

