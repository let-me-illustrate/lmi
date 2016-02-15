// SOA tables represented in binary SOA format--unit test.
//
// Copyright (C) 2015 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "soa_database.hpp"

#include "assert_lmi.hpp"
#include "miscellany.hpp"
#include "test_tools.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

using namespace soa_v3_format;

// Unit test helpers for working with files.
namespace
{

// Class ensuring that the file with the given name is removed when the test
// ends, whether it succeeds or fails.
class test_file_eraser
    :private lmi::uncopyable<test_file_eraser>
{
  public:
    explicit test_file_eraser(fs::path const& path)
        :path_(path)
        {
        }

    ~test_file_eraser()
        {
        try
            {
            fs::remove(path_);
            }
        catch(...)
            {
            // Failing to remove a temporary test file is not fatal and should
            // not result in abnormal program termination as would be the case
            // if we allowed the exception to escape from this dtor which
            // could, itself, be executing during the stack unwinding due to a
            // previous test failure. Do nothing here.
            }
        }

  private:
    fs::path path_;
};

// Check that the two binary files contents is identical, failing the current
// test if it isn't.
//
// BOOST !! We could use BOOST_CHECK_EQUAL_COLLECTIONS if we could use the
// full Boost.Test framework.
void check_files_equal
    (fs::path const& path1
    ,fs::path const& path2
    ,char const* file
    ,int line
    )
{
    fs::ifstream ifs1(path1, std::ios_base::in | std::ios_base::binary);
    INVOKE_BOOST_TEST(!ifs1.bad(), file, line);

    fs::ifstream ifs2(path2, std::ios_base::in | std::ios_base::binary);
    INVOKE_BOOST_TEST(!ifs2.bad(), file, line);

    // Compare the file sizes.
    ifs1.seekg(0, std::ios_base::end);
    ifs2.seekg(0, std::ios_base::end);
    INVOKE_BOOST_TEST_EQUAL(ifs1.tellg(), ifs2.tellg(), file, line);
    if(ifs1.tellg() != ifs2.tellg())
        {
        lmi_test::record_error();
        lmi_test::error_stream()
            << "Files '" << path1 << "' and '" << path2 << "' "
            << "have different sizes: " << ifs1.tellg() << " and "
            << ifs2.tellg() << " respectively."
            << BOOST_TEST_FLUSH
            ;
        return;
        }

    // Rewind back to the beginning.
    ifs1.seekg(0, std::ios_base::beg);
    ifs2.seekg(0, std::ios_base::beg);

    // Look for differences: using istream_iterator<char> here would be simpler
    // but also much less efficient, so read the file by larger blocks instead.
    const int buffer_size = 4096;
    char buf1[buffer_size];
    char buf2[buffer_size];
    for(std::streamsize offset = 0;;)
        {
        ifs1.read(buf1, buffer_size);
        INVOKE_BOOST_TEST(!ifs1.bad(), file, line);

        ifs2.read(buf2, buffer_size);
        INVOKE_BOOST_TEST(!ifs2.bad(), file, line);

        std::streamsize const count = ifs1.gcount();
        INVOKE_BOOST_TEST_EQUAL(count, ifs2.gcount(), file, line);

        if(!count)
            {
            return;
            }

        for(std::streamsize pos = 0; pos < count; ++pos)
            {
            if(buf1[pos] != buf2[pos])
                {
                lmi_test::record_error();
                lmi_test::error_stream()
                    << "Files '" << path1 << "' and '" << path2 << "' "
                    << "differ at offset " << offset + pos << ": "
                    << std::hex << std::setfill('0')
                    << std::setw(2)
                    << static_cast<int>(static_cast<unsigned char>(buf1[pos]))
                    << " != "
                    << std::setw(2)
                    << static_cast<int>(static_cast<unsigned char>(buf2[pos]))
                    << std::dec
                    << BOOST_TEST_FLUSH
                    ;
                return;
                }
            }

        offset += count;
        }
}

// Macro allowing to easily pass the correct file name and line number to
// check_files_equal().
#define TEST_FILES_EQUAL(path1, path2) \
    check_files_equal(path1, path2, __FILE__, __LINE__)

} // Unnamed namespace.

namespace
{
/// SOA regulatory table database.

std::string const qx_cso_path("/opt/lmi/data/qx_cso");

int const qx_cso_num_tables = 142;

/// SOA insurance table database.

std::string const qx_ins_path("/opt/lmi/data/qx_ins");

/// 'qx_ins' table 750: "1924 US Linton A Lapse"
///
/// Table type: Duration.
///
/// Parameters:
///   min "age" 1; max "age" 15

std::vector<double> table_750()
{
    static int const n = 15;
    static double const q[n] =
        //   0      1      2      3      4      5      6      7      8      9
        {0.100, 0.060, 0.050, 0.044, 0.040, 0.036, 0.032, 0.029, 0.027, 0.025 // 00
        ,0.024, 0.023, 0.022, 0.021, 0.020                                    // 10
        };
    return std::vector<double>(q, q + n);
}

/// 'qx_cso' table 42: "1980 US CSO Male Age nearest"
///
/// Table type: Aggregate.
///
/// Parameters:
///   min age 0; max age 99

std::vector<double> table_42(int age)
{
    static int const n = 100;
    static double const q[n] =
        //     0        1        2        3        4        5        6        7        8        9
        {0.00418, 0.00107, 0.00099, 0.00098, 0.00095, 0.00090, 0.00086, 0.00080, 0.00076, 0.00074 // 00
        ,0.00073, 0.00077, 0.00085, 0.00099, 0.00115, 0.00133, 0.00151, 0.00167, 0.00178, 0.00186 // 10
        ,0.00190, 0.00191, 0.00189, 0.00186, 0.00182, 0.00177, 0.00173, 0.00171, 0.00170, 0.00171 // 20
        ,0.00173, 0.00178, 0.00183, 0.00191, 0.00200, 0.00211, 0.00224, 0.00240, 0.00258, 0.00279 // 30
        ,0.00302, 0.00329, 0.00356, 0.00387, 0.00419, 0.00455, 0.00492, 0.00532, 0.00574, 0.00621 // 40
        ,0.00671, 0.00730, 0.00796, 0.00871, 0.00956, 0.01047, 0.01146, 0.01249, 0.01359, 0.01477 // 50
        ,0.01608, 0.01754, 0.01919, 0.02106, 0.02314, 0.02542, 0.02785, 0.03044, 0.03319, 0.03617 // 60
        ,0.03951, 0.04330, 0.04765, 0.05264, 0.05819, 0.06419, 0.07053, 0.07712, 0.08390, 0.09105 // 70
        ,0.09884, 0.10748, 0.11725, 0.12826, 0.14025, 0.15295, 0.16609, 0.17955, 0.19327, 0.20729 // 80
        ,0.22177, 0.23698, 0.25345, 0.27211, 0.29590, 0.32996, 0.38455, 0.48020, 0.65798, 1.00000 // 90
        };
    return std::vector<double>(q + age, q + n);
}

/// 'qx_ins' table 256: "1934 UK A1924-29, Male+Female, Age nearest"
///
/// Table type: Select.
///
/// Parameters:
///   min age 10; max age 121; select period 3; max select age 80
/// This is a good table for testing because none of its parameters is
/// degenerate: minimum age is not zero, and maximum age differs from
/// maximum select age.

std::vector<double> table_256(int age, int duration)
{
    LMI_ASSERT(0 <= duration && duration <= 3);
    // Select: issue age by duration.
    static int const nsel = 71 * 3;
    static double const qsel[nsel] =
        //     1        2        3
        {0.00106 ,0.00140 ,0.00165 // 10
        ,0.00113 ,0.00148 ,0.00175 // 11
        ,0.00120 ,0.00157 ,0.00184 // 12
        ,0.00127 ,0.00165 ,0.00193 // 13
        ,0.00134 ,0.00174 ,0.00202 // 14
        ,0.00141 ,0.00182 ,0.00210 // 15
        ,0.00147 ,0.00189 ,0.00216 // 16
        ,0.00153 ,0.00195 ,0.00219 // 17
        ,0.00158 ,0.00197 ,0.00220 // 18
        ,0.00160 ,0.00198 ,0.00220 // 19
        ,0.00160 ,0.00198 ,0.00220 // 20
        ,0.00160 ,0.00198 ,0.00220 // 21
        ,0.00160 ,0.00198 ,0.00220 // 22
        ,0.00160 ,0.00198 ,0.00220 // 23
        ,0.00160 ,0.00198 ,0.00220 // 24
        ,0.00160 ,0.00198 ,0.00220 // 25
        ,0.00160 ,0.00198 ,0.00221 // 26
        ,0.00160 ,0.00199 ,0.00223 // 27
        ,0.00160 ,0.00200 ,0.00226 // 28
        ,0.00161 ,0.00202 ,0.00230 // 29
        ,0.00162 ,0.00205 ,0.00236 // 30
        ,0.00164 ,0.00210 ,0.00244 // 31
        ,0.00167 ,0.00216 ,0.00254 // 32
        ,0.00171 ,0.00224 ,0.00266 // 33
        ,0.00177 ,0.00235 ,0.00280 // 34
        ,0.00185 ,0.00248 ,0.00297 // 35
        ,0.00194 ,0.00263 ,0.00316 // 36
        ,0.00205 ,0.00279 ,0.00337 // 37
        ,0.00217 ,0.00297 ,0.00359 // 38
        ,0.00230 ,0.00316 ,0.00382 // 39
        ,0.00244 ,0.00336 ,0.00406 // 40
        ,0.00258 ,0.00356 ,0.00431 // 41
        ,0.00273 ,0.00377 ,0.00458 // 42
        ,0.00288 ,0.00400 ,0.00487 // 43
        ,0.00304 ,0.00425 ,0.00519 // 44
        ,0.00322 ,0.00453 ,0.00556 // 45
        ,0.00342 ,0.00485 ,0.00599 // 46
        ,0.00365 ,0.00522 ,0.00648 // 47
        ,0.00391 ,0.00564 ,0.00703 // 48
        ,0.00421 ,0.00611 ,0.00764 // 49
        ,0.00455 ,0.00663 ,0.00832 // 50
        ,0.00493 ,0.00721 ,0.00908 // 51
        ,0.00535 ,0.00786 ,0.00993 // 52
        ,0.00581 ,0.00859 ,0.01089 // 53
        ,0.00632 ,0.00941 ,0.01199 // 54
        ,0.00690 ,0.01035 ,0.01326 // 55
        ,0.00757 ,0.01143 ,0.01470 // 56
        ,0.00834 ,0.01265 ,0.01629 // 57
        ,0.00920 ,0.01399 ,0.01801 // 58
        ,0.01014 ,0.01544 ,0.01986 // 59
        ,0.01115 ,0.01700 ,0.02184 // 60
        ,0.01223 ,0.01867 ,0.02398 // 61
        ,0.01339 ,0.02048 ,0.02635 // 62
        ,0.01464 ,0.02247 ,0.02902 // 63
        ,0.01601 ,0.02471 ,0.03206 // 64
        ,0.01754 ,0.02726 ,0.03551 // 65
        ,0.01927 ,0.03016 ,0.03938 // 66
        ,0.02123 ,0.03341 ,0.04365 // 67
        ,0.02343 ,0.03699 ,0.04830 // 68
        ,0.02585 ,0.04087 ,0.05330 // 69
        ,0.02847 ,0.04503 ,0.05863 // 70
        ,0.03127 ,0.04947 ,0.06431 // 71
        ,0.03424 ,0.05420 ,0.07036 // 72
        ,0.03738 ,0.05923 ,0.07682 // 73
        ,0.04070 ,0.06459 ,0.08373 // 74
        ,0.04421 ,0.07031 ,0.09112 // 75
        ,0.04793 ,0.07641 ,0.09901 // 76
        ,0.05188 ,0.08291 ,0.10741 // 77
        ,0.05607 ,0.08982 ,0.11632 // 78
        ,0.06051 ,0.09714 ,0.12572 // 79
        ,0.06520 ,0.10486 ,0.13557 // 80
        };
    // Ultimate: attained age.
    static int const nult = 121 - 13 + 1;
    static double const qult[nult] =
        //     0        1        2        3        4        5        6        7        8        9
        {                           0.00186 ,0.00196 ,0.00206 ,0.00216 ,0.00225 ,0.00231 ,0.00234 // 10
        ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00235 ,0.00236 ,0.00238 // 20
        ,0.00241 ,0.00246 ,0.00253 ,0.00262 ,0.00273 ,0.00286 ,0.00302 ,0.00320 ,0.00341 ,0.00364 // 30
        ,0.00388 ,0.00413 ,0.00439 ,0.00466 ,0.00495 ,0.00527 ,0.00563 ,0.00604 ,0.00651 ,0.00704 // 40
        ,0.00764 ,0.00831 ,0.00906 ,0.00990 ,0.01084 ,0.01190 ,0.01311 ,0.01450 ,0.01608 ,0.01783 // 50
        ,0.01973 ,0.02176 ,0.02394 ,0.02631 ,0.02893 ,0.03188 ,0.03524 ,0.03908 ,0.04338 ,0.04812 // 60
        ,0.05327 ,0.05881 ,0.06473 ,0.07104 ,0.07777 ,0.08497 ,0.09268 ,0.10093 ,0.10974 ,0.11913 // 70
        ,0.12910 ,0.13962 ,0.15066 ,0.16221 ,0.17425 ,0.18676 ,0.19973 ,0.21315 ,0.22702 ,0.24134 // 80
        ,0.25611 ,0.27133 ,0.28700 ,0.30312 ,0.31970 ,0.33675 ,0.35428 ,0.37231 ,0.39086 ,0.40995 // 90
        ,0.42960 ,0.44983 ,0.47066 ,0.49211 ,0.51420 ,0.53695 ,0.56038 ,0.58451 ,0.60936 ,0.63495 // 100
        ,0.66130 ,0.68843 ,0.71636 ,0.74511 ,0.77470 ,0.80515 ,0.83648 ,0.86871 ,0.90186 ,0.93595 // 110
        ,0.97100 ,1.00000                                                                         // 120
        };
    int isel = 3 * (age - 10);
    std::vector<double> v(qsel + isel + duration, qsel + isel + 3);
    v.insert(v.end(), qult + age - 10, qult + nult);
    return v;
}

} // Unnamed namespace.

/// Test opening database files.
///
/// Both '.ndx' and '.dat' files must exist.

void test_database_open()
{
    BOOST_TEST_THROW
        (database("nonexistent")
        ,std::runtime_error
        ,"File 'nonexistent.ndx' could not be opened for reading."
        );

    test_file_eraser erase("eraseme.ndx");
    std::ifstream ifs((qx_cso_path + ".ndx").c_str(), ios_in_binary());
    std::ofstream ofs("eraseme.ndx", ios_out_trunc_binary());
    ofs << ifs.rdbuf();
    ofs.close();
    BOOST_TEST_THROW
        (database("eraseme")
        ,std::runtime_error
        ,"File 'eraseme.dat' could not be opened for reading."
        );
}

void test_table_access_by_index()
{
    database qx_cso(qx_cso_path);
    BOOST_TEST(qx_cso_num_tables == qx_cso.tables_count());

    // Just check that using valid indices doesn't throw.
    qx_cso.get_nth_table(0);
    qx_cso.get_nth_table(1);
    qx_cso.get_nth_table(qx_cso_num_tables - 1);

    BOOST_TEST_THROW
        (qx_cso.get_nth_table(-1)
        ,std::out_of_range
        ,""
        );

    BOOST_TEST_THROW
        (qx_cso.get_nth_table(qx_cso_num_tables)
        ,std::out_of_range
        ,""
        );

    BOOST_TEST_THROW
        (qx_cso.get_nth_table(qx_cso_num_tables + 1)
        ,std::out_of_range
        ,""
        );
}

void test_table_access_by_number()
{
    database qx_cso(qx_cso_path);

    table::Number const number(qx_cso.get_nth_table(0).number());
    BOOST_TEST_EQUAL(qx_cso.find_table(number).number(), number);

    BOOST_TEST_THROW
        (qx_cso.find_table(table::Number(0))
        ,std::invalid_argument
        ,"table number 0 not found."
        );

    BOOST_TEST_THROW
        (qx_cso.find_table(table::Number(0xbadf00d))
        ,std::invalid_argument
        ,"table number 195948557 not found."
        );
}

void do_test_table_to_from_text(table const& table_orig)
{
    std::string const text_orig = table_orig.save_as_text();
    table const table_copy = table::read_from_text(text_orig);
    std::string const text_copy = table_copy.save_as_text();

    BOOST_TEST(text_orig == text_copy);
    BOOST_TEST(table_orig == table_copy);
}

void test_to_from_text()
{
    database qx_ins(qx_ins_path);

    // Test with aggregate, select and duration tables.
    do_test_table_to_from_text(qx_ins.find_table(table::Number(250)));
    do_test_table_to_from_text(qx_ins.find_table(table::Number(256)));
    do_test_table_to_from_text(qx_ins.find_table(table::Number(750)));
}

void test_save()
{
    database qx_ins(qx_ins_path);

    test_file_eraser erase_ndx("eraseme.ndx");
    test_file_eraser erase_dat("eraseme.dat");
    qx_ins.save("eraseme");

    TEST_FILES_EQUAL("eraseme.ndx", qx_ins_path + ".ndx");
    TEST_FILES_EQUAL("eraseme.dat", qx_ins_path + ".dat");
}

void test_add_table()
{
#if 0
    database qx_cso(qx_cso_path);

    BOOST_TEST_THROW
        (qx_cso.append_table(table::read_from_text(std::string("")))
        ,std::invalid_argument
        ,""
        );
#endif
}

int test_main(int, char*[])
{
    test_database_open();
    test_table_access_by_index();
    test_table_access_by_number();
    test_save();
    test_to_from_text();
    test_add_table();

    return EXIT_SUCCESS;
}

