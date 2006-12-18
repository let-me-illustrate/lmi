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

// $Id: passkey_test.cpp,v 1.30 2006-12-18 13:54:32 chicares Exp $

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

#include <cstdio>
#include <cstring> // std::memcmp(), std::memcpy()
#include <fstream>
#include <ios>
#include <string>
#include <vector>

// TODO ?? Add tests for diagnostics that aren't tested yet.

class PasskeyTest
{
  public:
    PasskeyTest();
    ~PasskeyTest();

    void InitializeData() const;
    void RemoveTestFiles(char const* file, int line) const;
    void Test() const;
    void Test0() const;
    void Test1() const;
    void Test2() const;
    void Test3() const;
    void Test4() const;

  private:
    calendar_date const  BeginDate_;
    calendar_date const  EndDate_;
    fs::path const       Pwd_;
    static unsigned char RimeMd5sum_[];
};

unsigned char PasskeyTest::RimeMd5sum_[md5len];

PasskeyTest::PasskeyTest()
    :BeginDate_(ymd_t(20010101))
    ,EndDate_  (ymd_t(20010103))
    ,Pwd_      (fs::current_path())
{
    // Remove test files that may be left over from a previous run
    // that failed to complete. Failing to remove them can cause
    // spurious error reports.
    RemoveTestFiles(__FILE__, __LINE__);

    InitializeData();
}

PasskeyTest::~PasskeyTest()
{
    RemoveTestFiles(__FILE__, __LINE__);
}

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

void PasskeyTest::InitializeData() const
{
    {
    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << rime;
    }

    // We need '-1 +' to avoid the trailing null character.
    md5_buffer(rime, -1 + static_cast<int>(sizeof rime), RimeMd5sum_);

    unsigned char expected[md5len];
    std::memcpy(expected, RimeMd5sum_, md5len);

    {
    std::ofstream os(md5sum_file(), ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << md5_hex_string
        (std::vector<unsigned char>(expected, expected + md5len)
        );
    os << "  coleridge\n";
    // Test whether we can initialize a string with the result of
    // md5_hex_string(), because ming29521 seems to have a problem
    // with it, which we suspect is due to nonconformance of the
    // <sstream> implementation we added to it.
    std::string s = md5_hex_string
        (std::vector<unsigned char>(expected, expected + md5len)
        );
    }

    FILE* in = std::fopen("coleridge", "rb");
    md5_stream(in, RimeMd5sum_);
    std::fclose(in);
    BOOST_TEST_EQUAL(0, std::memcmp(expected, RimeMd5sum_, sizeof expected));

    // Ascertain whether 'md5sum' program is available. Regrettably,
    // this writes to stdout, but without this test, it's hard to tell
    // whether errors in subsequent tests stem from incorrect md5sums
    // or absence of the 'md5sum' program.
    std::cout << "  Result of 'md5sum --version':" << std::endl;
    BOOST_TEST_EQUAL(0, system_command("md5sum --version"));

    // For production, we'll provide a file with md5 sums of all data
    // files. For this unit test, we'll treat file 'coleridge' as our
    // only data file; its md5 sum is already in the file created
    // above. Creating that file by running 'md5sum' is not trivial:
    // that program emits its output to stdout, and redirection can be
    // tricky.

    BOOST_TEST_EQUAL
        (0
        ,system_command("md5sum --check --status " + std::string(md5sum_file()))
        );

    FILE* md5 = std::fopen(md5sum_file(), "rb");
    md5_stream(md5, RimeMd5sum_);
    std::fclose(md5);

    // Passkey is the md5 sum of the md5 sum of the '.md5' file.
    // A more secure alternative could be wrought if wanted.

    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::memcpy(c_passkey, RimeMd5sum_, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);

    // Test with no passkey file. This is intended to fail.
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read passkey file 'passkey'. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, ".")
        );

    {
    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << md5_hex_string
        (std::vector<unsigned char>(u_passkey, u_passkey + md5len)
        );
    }
}

void PasskeyTest::Test0() const
{
    // Test with default dates, which are used if file 'expiry' fails
    // to exist, e.g. because it was deliberately deleted. This is
    // intended to fail.
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read expiry file 'expiry'. Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, ".")
        );

    // Create file 'expiry' and test with a real date.
    {
    std::ofstream os("expiry");
    BOOST_TEST(!!os);
    os << BeginDate_ << ' ' << EndDate_;
    }
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
}

void PasskeyTest::Test1() const
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

void PasskeyTest::Test2() const
{
    // The first day of the valid period should work.
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    // Repeat the test to validate caching.
    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(BeginDate_, Pwd_));
    // The last day of the valid period should work.
    calendar_date const last_date = EndDate_ - 1;
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(last_date, Pwd_));
    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(last_date, Pwd_));
    // Test one day before the period, and one day after.
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

void PasskeyTest::Test3() const
{
    // Test with an incorrect passkey. Caching can prevent this from
    // being detected: that's its purpose, because it is expensive to
    // test the data files. To demonstrate this: first validate the
    // date, in order to get it cached; then write an incorrect
    // 'passkey' file and retest.
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL("validated", SecurityValidator::Validate(BeginDate_, Pwd_));
    {
    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    std::vector<unsigned char> const wrong(md5len);
    os << md5_hex_string(wrong);
    }
    // Testing with the same date succeeds due to caching.
    BOOST_TEST_EQUAL("cached"   , SecurityValidator::Validate(BeginDate_, Pwd_));
    // Reset the cache, and the incorrect passkey is detected.
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("Passkey is incorrect for this version. Contact the home office."
        ,SecurityValidator::Validate(BeginDate_, Pwd_)
        );
}

void PasskeyTest::Test4() const
{
    // Test with altered data file. This is intended to fail.
    {
    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << "This file has the wrong md5sum.";
    }
    SecurityValidator::ResetCache();
    BOOST_TEST_EQUAL
        ("At least one required file is missing, altered, or invalid."
        " Try reinstalling."
        ,SecurityValidator::Validate(BeginDate_, Pwd_)
        );
}

void PasskeyTest::Test() const
{
    Test0();
    Test1();
    Test2();
    Test3();
    Test4();
}

int test_main(int, char*[])
{
    PasskeyTest tester;
    tester.Test();

    return EXIT_SUCCESS;
}

