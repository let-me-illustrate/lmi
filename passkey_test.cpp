// Passkeys--unit test.
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

// $Id: passkey_test.cpp,v 1.10 2006-12-14 23:39:29 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

// Facilities offered by both these headers are tested here.
#include "md5.hpp"
#include "secure_date.hpp"

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

char const coleridge[] =
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

void remove_test_files(char const* file, int line)
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

int test_main(int, char*[])
{
    // First, remove test files that may be left over from a previous
    // run that failed to complete. Failing to remove them can cause
    // spurious error reports.
    remove_test_files(__FILE__, __LINE__);

    {
    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << coleridge;
    }

    unsigned char result[md5len];
    // We need '-1 +' to avoid the trailing null character.
    md5_buffer(coleridge, -1 + static_cast<int>(sizeof coleridge), result);

    unsigned char expected[md5len];
    std::memcpy(expected, result, md5len);

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
    md5_stream(in, result);
    std::fclose(in);
    BOOST_TEST_EQUAL(0, std::memcmp(expected, result, sizeof expected));

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
    md5_stream(md5, result);
    std::fclose(md5);

    // We'll use the md5 sum of the md5 sum of the .md5 file as our
    // our passkey for now. It's not at all secure, but it can be
    // made secure e.g. with a strong public key system.

    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::memcpy(c_passkey, result, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);

    fs::path pwd(".");

    // Test with no passkey file. This is intended to fail.
    calendar_date candidate;
    BOOST_TEST_EQUAL
        ("Unable to read passkey file 'passkey'. Try reinstalling."
        ,secure_date::instance()->validate(candidate, pwd)
        );

    {
    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << md5_hex_string
        (std::vector<unsigned char>(u_passkey, u_passkey + md5len)
        );
    }

    // Test with default dates, which are used if file 'expiry' fails
    // to exist, e.g. because it was deliberately deleted. This is
    // intended to fail.
    BOOST_TEST_EQUAL
        ("Unable to read expiry file 'expiry'. Try reinstalling."
        ,secure_date::instance()->validate(candidate, pwd)
        );

    // Test with real dates. The current millenium began on 20010101,
    // twelve hours into JDN 2451910; that's as good a date as any.
    long int millenium = 2451910L;
    {
    std::ofstream os("expiry");
    BOOST_TEST(!!os);
    os << millenium << ' ' << (millenium + 2);
    }

    // The first day of the valid period should work.
    candidate.julian_day_number(millenium);
    BOOST_TEST_EQUAL("", secure_date::instance()->validate(candidate, pwd));
    // Repeat the test: stepping through the called function with a
    // debugger should show an early exit because the date was cached.
    BOOST_TEST_EQUAL("", secure_date::instance()->validate(candidate, pwd));
    // The last day of the valid period should work.
    candidate.julian_day_number(millenium + 1);
    BOOST_TEST_EQUAL("", secure_date::instance()->validate(candidate, pwd));
    // Test one day before the period, and one day after.
    candidate.julian_day_number(millenium - 1);
    BOOST_TEST_EQUAL
        ("Current date '2000-12-30' is invalid:"
        " this system cannot be used before '2000-12-31'."
        " Contact the home office."
        ,secure_date::instance()->validate(candidate, pwd)
        );
    candidate.julian_day_number(millenium + 2);
    BOOST_TEST_EQUAL
        ("Current date '2001-01-02' is invalid:"
        " this system expired on '2001-01-02'."
        " Contact the home office."
        ,secure_date::instance()->validate(candidate, pwd)
        );

    // Test with incorrect passkey. This is intended to fail--but see below.
    {
    std::ofstream os("passkey", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    std::vector<unsigned char> const wrong(md5len);
    os << md5_hex_string(wrong);
    }
    // But it shouldn't fail if the date was previously cached as valid.
    candidate.julian_day_number(millenium + 1);
    BOOST_TEST_EQUAL("", secure_date::instance()->validate(candidate, pwd));
    candidate.julian_day_number(millenium);
    BOOST_TEST_EQUAL
        ("Passkey is incorrect for this version. Contact the home office."
        ,secure_date::instance()->validate(candidate, pwd)
        );

    // Test with altered data file. This is intended to fail.
    {
    std::ofstream os("coleridge", ios_out_trunc_binary());
    BOOST_TEST(!!os);
    os << "This file has the wrong md5sum.";
    }
    BOOST_TEST_EQUAL
        ("At least one required file is missing, altered, or invalid."
        " Try reinstalling."
        ,secure_date::instance()->validate(candidate, pwd)
        );

    remove_test_files(__FILE__, __LINE__);

    return 0;
}

