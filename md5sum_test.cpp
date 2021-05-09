// MD5 sum--unit test.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#include "md5sum.hpp"

#include "path.hpp"
#include "test_tools.hpp"

#include <cstdio>                       // remove()
#include <fstream>
#include <ios>                          // ios_base
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
char const* test_filename = "md5_file";

// Use the windows line ending format to test the text mode too.
std::string const test_text =
    "The couple bought a lot of vegetables and paid with a cheque. They\r\n"
    "did some sightseeing, but it was too hot outside. In a park they met\r\n"
    "a colleague and had a dinner romantically. After the meal they\r\n"
    "decided to visit the local zoo. A lot of people saw a leopard at the\r\n"
    "zoo. Near the cage they met the colonel, a well known man doing\r\n"
    "business. He looked a little bit strange, maybe his ancestors were\r\n"
    "apache or shoshone indians. They may have travelled from coast to\r\n"
    "coast and saw interesting places. In the evening every building\r\n"
    "was lit."
    ;

char const* test_bin_checksum =  "40e541710871106ebc596595da341dcb";

#if defined LMI_MSW
char const* test_text_checksum = "e9f0fbd7a758d253ecd48ccded29234c";
#else
// Under Linux text and binary modes are the same.
char const* test_text_checksum = test_bin_checksum;
#endif // defined LMI_MSW

char const* md5sums_filename = "md5_sums";

// Doubled '""': avoid "should fuse '*' with type" warning.
std::string const md5sums_text =
    "00112233445566778899aabbccddeeff  test.txt\n"
    "ffeeddccbbaa99887766554433221100 *""test.bin\n"
    ;

}

std::ostream& operator<<(std::ostream& os, std::vector<md5sum_for_file> const& v)
{
    for(auto const& s : v)
        {
        char delimiter;
        switch(s.file_mode)
            {
            case md5_file_mode::binary:
                delimiter = '*';
                break;
            case md5_file_mode::text:
                delimiter = ' ';
                break;
            default:
                throw std::runtime_error("Unrecognized file mode value");
            }
        os << s.md5sum << ' ' << delimiter << "'" << s.filename.string() << "'\n";
        }
    return os;
}

bool operator==(md5sum_for_file const& l, md5sum_for_file const& r)
{
    return
           l.filename  == r.filename
        && l.md5sum    == r.md5sum
        && l.file_mode == r.file_mode
        ;
}

/// MD5 sum--unit test.
///
/// Public members are declared in invocation order.

class MD5SumTest
{
  public:
    MD5SumTest();
    ~MD5SumTest();

    void TestMD5Calculation() const;
    void TestMD5Reading() const;
    void TestMD5ToHexString() const;

  private:
    void RemoveTestFilesIfNecessary(char const* file, int line) const;
    void WriteAndCheckFile
        (char const*        filename
        ,std::string const& text
        ,char const*        file
        ,int                line
        ) const;

    void InitializeTestFile() const;
    void InitializeMD5SumsFile() const;
};

/// Before writing any test file, remove any old copy that may be left
/// over from a previous run that failed to complete, because old
/// copies can cause spurious error reports.

MD5SumTest::MD5SumTest()
{
    RemoveTestFilesIfNecessary(__FILE__, __LINE__);

    InitializeTestFile();
    InitializeMD5SumsFile();
}

MD5SumTest::~MD5SumTest()
{
    RemoveTestFilesIfNecessary(__FILE__, __LINE__);
}

/// Test md5_read_checksum_stream and md5_read_checksum_file
/// functions.

void MD5SumTest::TestMD5Calculation() const
{
    // Test md5_calculate_stream_checksum function.
    std::ifstream is_text{test_filename, std::ios_base::in};
    LMI_TEST_EQUAL
        (test_text_checksum
        ,md5_calculate_stream_checksum(is_text, test_filename)
        );

    std::ifstream is_bin{test_filename, ios_in_binary()};
    LMI_TEST_EQUAL
        (test_bin_checksum
        ,md5_calculate_stream_checksum(is_bin, test_filename)
        );

    std::istringstream is_fail{test_text, std::ios_base::in};
    is_fail.setstate(std::ios_base::failbit);
    LMI_TEST_THROW
        (md5_calculate_stream_checksum(is_fail, test_filename)
        ,std::runtime_error
        ,"'md5_file': failed to read data while computing md5sum"
        );

    // Test md5_calculate_file_checksum function.
    LMI_TEST_EQUAL
        (test_text_checksum
        ,md5_calculate_file_checksum(test_filename, md5_file_mode::text)
        );

    LMI_TEST_EQUAL
        (test_bin_checksum
        ,md5_calculate_file_checksum(test_filename, md5_file_mode::binary)
        );

    LMI_TEST_EQUAL
        (md5_calculate_file_checksum(md5sums_filename, md5_file_mode::text)
        ,md5_calculate_file_checksum(md5sums_filename, md5_file_mode::binary)
        );

    LMI_TEST_THROW
        (md5_calculate_file_checksum("_ghost_")
        ,std::runtime_error
        ,"'_ghost_': no such file or directory"
        );
}

/// Test md5_calculate_stream_checksum and md5_calculate_file_checksum
/// functions.

void MD5SumTest::TestMD5Reading() const
{
    std::vector<md5sum_for_file> md5sums
        {{"test.txt", "00112233445566778899aabbccddeeff", md5_file_mode::text}
        ,{"test.bin", "ffeeddccbbaa99887766554433221100", md5_file_mode::binary}
        };

    // Test md5_read_checksum_stream function.
    std::istringstream is_sums{md5sums_text};
    LMI_TEST_EQUAL
        (md5sums
        ,md5_read_checksum_stream(is_sums, md5sums_filename)
        );

    std::istringstream is_throw1{"00112233445566778899aabbccddeeff  \n"};
    LMI_TEST_THROW
        (md5_read_checksum_stream(is_throw1, "test1")
        ,std::runtime_error
        ,"'test1': line too short at line 1"
        );

    std::istringstream is_throw2{"00112233445566778899aabbccddeeff_test\n"};
    LMI_TEST_THROW
        (md5_read_checksum_stream(is_throw2, "test2")
        ,std::runtime_error
        ,"'test2': incorrect checksum line format at line 1"
        );

    std::istringstream is_throw3{"00112233445566778899aabbccddeeff test\n"};
    LMI_TEST_THROW
        (md5_read_checksum_stream(is_throw3, "test3")
        ,std::runtime_error
        ,"'test3': incorrect checksum line format at line 1"
        );

    std::istringstream is_throw4{"00112233445566778899aabbccddee  test\n"};
    LMI_TEST_THROW
        (md5_read_checksum_stream(is_throw4, "test4")
        ,std::runtime_error
        ,"'test4': incorrect MD5 sum format at line 1"
        );

    // Test md5_read_checksum_file function.
    LMI_TEST_EQUAL(md5sums, md5_read_checksum_file(md5sums_filename));

    LMI_TEST_THROW
        (md5_read_checksum_file("_ghost_")
        ,std::runtime_error
        ,"'_ghost_': no such file or directory"
        );
}

/// Test md5_hex_string function.

void MD5SumTest::TestMD5ToHexString() const
{
    std::vector<unsigned char> v
        {0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78
        ,0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0
        };
    LMI_TEST_EQUAL("0f1e2d3c4b5a69788796a5b4c3d2e1f0", md5_hex_string(v));
}

void MD5SumTest::RemoveTestFilesIfNecessary(char const* file, int line) const
{
    auto const RemoveIfNecessary = [file, line](char const* filename)
        {
        if(fs::exists(filename))
            {
            INVOKE_LMI_TEST(std::remove(filename) == 0, file, line);
            }
        };

    RemoveIfNecessary(test_filename);
    RemoveIfNecessary(md5sums_filename);
}

/// Write the text to the file.
///
/// Read the file back after the writing and check the content.

void MD5SumTest::WriteAndCheckFile
    (char const*        filename
    ,std::string const& text
    ,char const*        file
    ,int                line
    ) const
{
    std::ofstream os{filename, ios_out_trunc_binary()};
    INVOKE_LMI_TEST(os.good(), file, line);
    os << text;
    os.close();

    // Read the file back.
    std::ifstream is{filename, ios_in_binary()};
    INVOKE_LMI_TEST(is.good(), file, line);
    std::ostringstream oss;
    oss << is.rdbuf();
    is.close();

    INVOKE_LMI_TEST_EQUAL(text, oss.str(), file, line);
}

/// Write a text file for testing.

void MD5SumTest::InitializeTestFile() const
{
    WriteAndCheckFile(test_filename, test_text, __FILE__, __LINE__);
}

/// Write the file with the file name and the md5 sum of the test file.

void MD5SumTest::InitializeMD5SumsFile() const
{
    WriteAndCheckFile(md5sums_filename, md5sums_text, __FILE__, __LINE__);
}

int test_main(int, char*[])
{
    MD5SumTest tester;
    tester.TestMD5Calculation();
    tester.TestMD5Reading();
    tester.TestMD5ToHexString();

    return EXIT_SUCCESS;
}
