// Data-file and date validation--unit test.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "authenticity.hpp"

#include "assert_lmi.hpp"
#include "contains.hpp"
#include "md5.hpp"
#include "md5sum.hpp"
#include "miscellany.hpp"
#include "path.hpp"
#include "system_command.hpp"
#include "test_tools.hpp"

#include <cstdio>                       // remove()
#include <cstring>                      // memcpy(), strlen()
#include <fstream>
#include <string>
#include <vector>

// TODO ?? Add tests for conditions and diagnostics that aren't tested yet.

template<typename T, auto n>
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

    void EnsureMd5sumBinaryIsFound() const;
    void RemoveTestFiles(char const* file, int line) const;

    void InitializeDataFile() const;
    void InitializeMd5sumFile() const;
    void InitializePasskeyFile() const;
    void InitializeExpiryFile() const;

    void CheckNominal(char const* file, int line) const;

    void TestFromAfar() const;
    void TestDate() const;
    void TestPasskey() const;
    void TestDataFile() const;
    void TestExpiry() const;

  private:
    calendar_date const  BeginDate_;
    calendar_date const  EndDate_;
    fs::path const       Pwd_;
};

/// Before writing any test file, remove any old copy that may be left
/// over from a previous run that failed to complete, because old
/// copies can cause spurious error reports.

PasskeyTest::PasskeyTest()
    :BeginDate_ {ymd_t(20010101)}
    ,EndDate_   {ymd_t(20010103)}
    ,Pwd_       {fs::current_path()}
{
    EnsureMd5sumBinaryIsFound();
    RemoveTestFiles(__FILE__, __LINE__);

    InitializeDataFile();
    InitializeMd5sumFile();
    InitializePasskeyFile();
    InitializeExpiryFile();

    CheckNominal(__FILE__, __LINE__);
}

PasskeyTest::~PasskeyTest()
{
    CheckNominal(__FILE__, __LINE__);
    RemoveTestFiles(__FILE__, __LINE__);
}

/// Regrettably, invoking 'lmi_md5sum' through a shell just to confirm
/// its availability writes its output to stdout; however, without this
/// function, it would be difficult to tell whether downstream errors
/// stem from that program's absence.

void PasskeyTest::EnsureMd5sumBinaryIsFound() const
{
    std::cout << "  Result of 'lmi_md5sum --version':" << std::endl;
    system_command("lmi_md5sum --version");
}

void PasskeyTest::RemoveTestFiles(char const* file, int line) const
{
    std::vector<std::string> filenames;
    filenames.push_back("expiry");
    filenames.push_back("passkey");
    filenames.push_back("coleridge");
    filenames.push_back(md5sum_file());
    for(auto const& i : filenames)
        {
        std::remove(i.c_str());
        INVOKE_BOOST_TEST(!fs::exists(i), file, line);
        }
}

/// Write a data file for testing.
///
/// Calculate and verify the md5 sum of the string from which the file
/// is created, taking care to ignore that string's terminating null.

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
    BOOST_TEST(os.good());
    os << rime;
    os.close();

    unsigned char sum[md5len];
    md5_buffer(rime, std::strlen(rime), sum);
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(sum));
}

/// Write a data file to be passed to the 'lmi_md5sum' program.
///
/// For production, a file with md5 sums of crucial files is provided.
/// For this unit test, file 'coleridge' is the sole crucial file.
///
/// This file consists of the md5 sum of the data file followed by two
/// spaces and the name of the data file. Creating that file portably
/// here by running 'lmi_md5sum' would require redirection (and thus a
/// shell), so the effect of 'lmi_md5sum' is instead emulated; testing
/// that file here with 'lmi_md5sum' validates that emulation and
/// guards against a bogus 'lmi_md5sum' program.
///
/// Postcondition: the file validates with the 'lmi_md5sum' program.

void PasskeyTest::InitializeMd5sumFile() const
{
    unsigned char sum[md5len];
    std::FILE* in = std::fopen("coleridge", "rb");
    md5_stream(in, sum);
    std::fclose(in);
    BOOST_TEST_EQUAL("bf039dbb0e8061971a2c322c8336199c", md5_str(sum));

    std::ofstream os(md5sum_file(), ios_out_trunc_binary());
    BOOST_TEST(os.good());
    os << md5_hex_string(std::vector<unsigned char>(sum, sum + md5len));
    os << "  coleridge\n";
    os.close();

    std::string s = "lmi_md5sum --check --status " + std::string(md5sum_file());
    system_command(s);
}

/// The passkey is the md5 sum of the md5 sum of the '.md5' file.
/// A more secure alternative could be wrought if wanted, but the
/// present method is enough to stymie the unsophisticated.

void PasskeyTest::InitializePasskeyFile() const
{
    unsigned char sum[md5len];
    std::FILE* md5 = std::fopen(md5sum_file(), "rb");
    md5_stream(md5, sum);
    std::fclose(md5);

    BOOST_TEST_EQUAL("efb7a0a972b88bb5b9ac6f60390d61bf", md5_str(sum));

    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::memcpy(c_passkey, sum, md5len);
    BOOST_TEST_EQUAL("efb7a0a972b88bb5b9ac6f60390d61bf", md5_str(c_passkey));
    md5_buffer(c_passkey, md5len, u_passkey);
    BOOST_TEST_EQUAL("8a4829bf31de9437c95aedaeead398d7", md5_str(u_passkey));
    std::memcpy(c_passkey, u_passkey, md5len);
    BOOST_TEST_EQUAL("8a4829bf31de9437c95aedaeead398d7", md5_str(c_passkey));
    md5_buffer(c_passkey, md5len, u_passkey);
    BOOST_TEST_EQUAL("3ff4953dbddf009634922fa52a342bfe", md5_str(u_passkey));

    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(os.good());
    os << md5_hex_string
        (std::vector<unsigned char>(u_passkey, u_passkey + md5len)
        );
}

void PasskeyTest::InitializeExpiryFile() const
{
    std::ofstream os("expiry");
    BOOST_TEST(os.good());
    os << BeginDate_ << ' ' << EndDate_;
    os.close();
}

/// Verify that all conditions are "nominal" in the aeronautics sense.
///
/// This check succeeds as
///   - a postcondition of the ctor,
///   - a precondition of the dtor, and
///   - a precondition and postcondition of every 'Test*' function.
///
/// Authentication can fail due to any of several causes. Asserting
/// that it succeeds, both at entry and at exit, for each 'Test*'
/// function prevents breakage of an invariant in one place from
/// causing a symptom to appear elsewhere: i.e., it ensures that
/// tests remain orthogonal.

void PasskeyTest::CheckNominal(char const* file, int line) const
{
    Authenticity::ResetCache();
    INVOKE_BOOST_TEST_EQUAL
        ("validated"
        ,Authenticity::Assay(BeginDate_, Pwd_)
        ,file
        ,line
        );
}

/// Authenticate from afar, to ensure non-dependence on ${PWD}.
///
/// By default, lmi authenticates itself at startup. Its executable
/// and data files shouldn't need to be in any particular directory;
/// an invocation like this:
///   wine /opt/lmi/bin/lmi_wx_shared --data_path=/opt/lmi/data
/// should just work. This test checks that invariant, as follows.
///
/// First, change ${PWD}; authenticate; and restore ${PWD}, verifying
/// that the restoration succeeded.
///
/// Authenticate also from the root directory on a different drive, on
/// a multiple-root system. This is perforce platform specific; msw is
/// tested because it's common and problematic. This test assumes that
/// an 'F:' drive exists and is not the "current" drive; it is skipped
/// if no 'F:' drive exists.
///
/// BOOST !! This test traps an exception that boost-1.33.1 can throw
/// if exists("F:/") returns true but ::GetFileAttributesA() fails.
/// That's supposed to be impossible because the is_directory()
/// documentation says:
///   "Throws: if !exists(ph)"
/// but it can be reproduced by placing an unformatted disk in "F:".

void PasskeyTest::TestFromAfar() const
{
    CheckNominal(__FILE__, __LINE__);

    fs::path const tmp = "/tmp" / fs::path{__FILE__}.stem();
    fs::path const remote_dir_0(fs::absolute(tmp));
    fs::create_directory(remote_dir_0);
    BOOST_TEST(fs::exists(remote_dir_0) && fs::is_directory(remote_dir_0));
    BOOST_TEST_EQUAL(0, chdir(remote_dir_0.string().c_str()));
    BOOST_TEST_EQUAL(remote_dir_0.string(), fs::current_path().string());
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL("validated", Authenticity::Assay(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL(remote_dir_0.string(), fs::current_path().string());
    BOOST_TEST_EQUAL(0, chdir(Pwd_.string().c_str()));
    BOOST_TEST_EQUAL(Pwd_.string(), fs::current_path().string());
    fs::remove(remote_dir_0);

#if defined LMI_MSW
    CheckNominal(__FILE__, __LINE__);

    fs::path const remote_dir_1(fs::absolute("F:/"));
    if(!fs::exists(remote_dir_1))
        {
        goto done;
        }

    BOOST_TEST(fs::is_directory(remote_dir_1));

    BOOST_TEST_EQUAL(0, chdir(remote_dir_1.string().c_str()));
    BOOST_TEST_EQUAL(remote_dir_1.string(), fs::current_path().string());
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL("validated", Authenticity::Assay(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL(remote_dir_1.string(), fs::current_path().string());
    BOOST_TEST_EQUAL(0, chdir(Pwd_.string().c_str()));
    BOOST_TEST_EQUAL(Pwd_.string(), fs::current_path().string());

  done:
#endif // defined LMI_MSW

    CheckNominal(__FILE__, __LINE__);
}

/// When authentication succeeds, the date is cached. Reauthenticating
/// on the same date succeeds without testing the data files afresh.
///
/// When authentication fails, the cache is reset, and any subsequent
/// authentication tests the data files as well as the date.

void PasskeyTest::TestDate() const
{
    CheckNominal(__FILE__, __LINE__);

    Authenticity::ResetCache();
    BOOST_TEST_EQUAL("validated", Authenticity::Assay(BeginDate_, Pwd_));
    BOOST_TEST_EQUAL("cached"   , Authenticity::Assay(BeginDate_, Pwd_));

    calendar_date const last_date = EndDate_ - 1;
    BOOST_TEST_EQUAL("validated", Authenticity::Assay(last_date, Pwd_));
    BOOST_TEST_EQUAL("cached"   , Authenticity::Assay(last_date, Pwd_));

    BOOST_TEST_EQUAL
        ("Current date 2000-12-31 is invalid:"
        " this system cannot be used before 2001-01-01."
        " Contact the home office."
        ,Authenticity::Assay(BeginDate_ - 1, Pwd_)
        );
    BOOST_TEST_EQUAL
        ("Current date 2001-01-03 is invalid:"
        " this system cannot be used after 2001-01-02."
        " Contact the home office."
        ,Authenticity::Assay(EndDate_, Pwd_)
        );
    BOOST_TEST_EQUAL
        ("Current date 2001-01-13 is invalid:"
        " this system cannot be used after 2001-01-02."
        " Contact the home office."
        ,Authenticity::Assay(EndDate_ + 10, Pwd_)
        );

    BOOST_TEST_UNEQUAL("cached", Authenticity::Assay(last_date, Pwd_));
    BOOST_TEST_EQUAL  ("cached", Authenticity::Assay(last_date, Pwd_));

    CheckNominal(__FILE__, __LINE__);
}

/// Caching can prevent an incorrect or missing passkey from being
/// detected. That's deliberate: testing the data files is expensive,
/// so they're tested once, and not retested as long as the cached
/// date doesn't change.
///
/// Precondition: the system is valid for more than one day, so that
/// a valid date change may be tested.

void PasskeyTest::TestPasskey() const
{
    CheckNominal(__FILE__, __LINE__);

    calendar_date const last_date = EndDate_ - 1;
    BOOST_TEST_EQUAL("validated", Authenticity::Assay(last_date, Pwd_));

    std::ofstream os0("passkey", ios_out_trunc_binary());
    BOOST_TEST(os0.good());
    std::vector<unsigned char> const wrong(md5len);
    os0 << md5_hex_string(wrong);
    os0.close();
    BOOST_TEST_EQUAL("cached"   , Authenticity::Assay(last_date, Pwd_));
    BOOST_TEST(last_date != BeginDate_);
    BOOST_TEST_EQUAL
        ("Passkey is incorrect for this version. Contact the home office."
        ,Authenticity::Assay(BeginDate_, Pwd_)
        );

    std::remove("passkey");
    BOOST_TEST(!fs::exists("passkey"));
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read passkey file 'passkey'. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, ".")
        );

    std::ofstream os1("passkey", ios_out_trunc_binary());
    os1 << "wrong";
    BOOST_TEST(os1.good());
    os1.close();
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Length of passkey 'wrong' is 5 but should be 32. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, Pwd_)
        );

    InitializePasskeyFile();
    CheckNominal(__FILE__, __LINE__);
}

void PasskeyTest::TestDataFile() const
{
    CheckNominal(__FILE__, __LINE__);

    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(os.good());
    os << "This file has the wrong md5sum.";
    os.close();

    Authenticity::ResetCache();
    std::cout
        << "Expect"
        << "\n  Integrity check failed for 'coleridge'"
        << "\nto print:"
        << std::endl
        ;
    BOOST_TEST_EQUAL
        ("At least one required file is missing, altered, or invalid."
        " Try reinstalling."
        ,Authenticity::Assay(BeginDate_, Pwd_)
        );

    InitializeDataFile();
    CheckNominal(__FILE__, __LINE__);
}

void PasskeyTest::TestExpiry() const
{
    CheckNominal(__FILE__, __LINE__);

    std::remove("expiry");
    BOOST_TEST(!fs::exists("expiry"));
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Unable to read expiry file 'expiry'. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, ".")
        );

    {
    std::ofstream os("expiry");
    BOOST_TEST(os.good());
    os.close();
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Error reading expiry file 'expiry'. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, ".")
        );
    }

    {
    std::ofstream os("expiry");
    os << "2400000";
    BOOST_TEST(os.good());
    os.close();
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Error reading expiry file 'expiry'. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, ".")
        );
    }

    {
    std::ofstream os("expiry");
    os << "bogus dates";
    BOOST_TEST(os.good());
    os.close();
    Authenticity::ResetCache();
    BOOST_TEST_EQUAL
        ("Error reading expiry file 'expiry'. Try reinstalling."
        ,Authenticity::Assay(BeginDate_, ".")
        );
    }

    InitializeExpiryFile();
    CheckNominal(__FILE__, __LINE__);
}

int test_main(int, char*[])
{
    PasskeyTest tester;
    tester.TestFromAfar();
    tester.TestDate();
    tester.TestPasskey();
    tester.TestDataFile();
    tester.TestExpiry();

    return EXIT_SUCCESS;
}
