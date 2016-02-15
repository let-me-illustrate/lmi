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
    // Notice "1+" to skip the leading new line.
    table const t = table::read_from_text(std::string(1 + R"table(
Table number: 1
Table type: Aggregate
Minimum age: 0
Maximum age: 1
Number of decimal places: 5
Table values:
  0  0.12345
  1  0.23456
)table"));

    database qx_ins(qx_ins_path);
    int const count = qx_ins.tables_count();

    qx_ins.append_table(t);
    BOOST_TEST_EQUAL(qx_ins.tables_count(), count + 1);

    BOOST_TEST_THROW
        (qx_ins.append_table(t)
        ,std::invalid_argument
        ,"table number 1 already exists."
        );

    qx_ins.add_or_replace_table(t);
    BOOST_TEST_EQUAL(qx_ins.tables_count(), count + 1);
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

