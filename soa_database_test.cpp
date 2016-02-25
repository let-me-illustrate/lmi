// SOA tables represented in binary SOA format--unit test.
//
// Copyright (C) 2015, 2016 Gregory W. Chicares.
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

#include <iomanip>

using namespace soa_v3_format;

// Unit test helpers for working with files.
namespace
{

// Class temporarily redirecting std::cout to a string: this is useful to check
// that the expected output appears on cout or just to suppress some output
// we're not interested in.
class std_out_redirector
{
  public:
    std_out_redirector()
        :orig_streambuf_(std::cout.rdbuf(stream_out_.rdbuf()))
        {
        }

    ~std_out_redirector()
        {
        std::cout.rdbuf(orig_streambuf_);
        }

    std_out_redirector(std_out_redirector const&) = delete;
    std_out_redirector& operator=(std_out_redirector const&) = delete;

    // For convenience, this method returns everything output so far and clears
    // the output, i.e. the next call to it will only return output appearing
    // after this call. The name is supposed to emphasize this.
    std::string take_output()
        {
        std::string const output = stream_out_.str();
        stream_out_.str(std::string());
        return output;
        }

  private:
    // The order of declarations here is important: stream_out_ must be
    // initialzied before orig_streambuf_ whose initialization uses it.

    // The stream where cout is redirected during this object life-time.
    std::ostringstream stream_out_;

    // The original buffer used by cout before we redirected it.
    std::streambuf* const orig_streambuf_;
};

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

// NB: "1+" is used here just to allow formatting multiline strings in a
// natural way and strips the leading new line.

/// Prefix used for the test tables.
std::string const simple_table_header(1 + R"table(
Table number: 1
Table type: Aggregate
Minimum age: 0
Maximum age: 1
Number of decimal places: 5
Table values:
)table");

std::string const simple_table_values(1 + R"table(
  0  0.12345
  1  0.23456
)table");

/// Minimal valid SOA table in text format.
std::string const simple_table_text(simple_table_header + simple_table_values);

} // Unnamed namespace.

/// Test opening database files.
///
/// Both '.ndx' and '.dat' files must exist.

void test_database_open()
{
    BOOST_TEST_THROW
        (database("nonexistent")
        ,std::runtime_error
        ,match_substr("'nonexistent.ndx' could not be opened")
        );

    test_file_eraser erase("eraseme.ndx");
    std::ifstream ifs((qx_cso_path + ".ndx").c_str(), ios_in_binary());
    std::ofstream ofs("eraseme.ndx", ios_out_trunc_binary());
    ofs << ifs.rdbuf();
    ofs.close();
    BOOST_TEST_THROW
        (database("eraseme")
        ,std::runtime_error
        ,match_substr("'eraseme.dat' could not be opened")
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

void test_from_text()
{
    // Using unknown header in a place where it can't be parsed as a
    // continuation of the previous line should fail.
    {
    std_out_redirector std_out_redir;
    BOOST_TEST_THROW
        (table::read_from_text("Bloordyblop: yes\n" + simple_table_text)
        ,std::runtime_error
        ,match_substr("expected a field name")
        );
    }

    // However using it as part of a multiline field should succeed, albeit
    // with a warning.
    {
    std_out_redirector std_out_redir;
    table::read_from_text("Comments: no\nBloordyblop: yes\n" + simple_table_text);
    BOOST_TEST(std_out_redir.take_output().find("Bloordyblop") != std::string::npos);
    }

    // Using too many values should fail.
    BOOST_TEST_THROW
        (table::read_from_text(simple_table_text + "  2  0.34567\n")
        ,std::runtime_error
        ,match_substr("expected a field")
        );

    // And so should using too few of them: chop of the last line to test.
    BOOST_TEST_THROW
        (table::read_from_text(simple_table_header + "  0  0.12345")
        ,std::runtime_error
        ,match_substr("missing")
        );

    // Using bad hash value should fail.
    BOOST_TEST_THROW
        (table::read_from_text(simple_table_text + "Hash value: 1234567890\n")
        ,std::runtime_error
        ,match_substr("hash value 1234567890")
        );

    // Using values greater than 1 should be possible.
    table::read_from_text
        (simple_table_header
        +"  0  0.12345\n"
         "  1 10.98765\n"
        );
}

void test_save()
{
    database qx_ins(qx_ins_path);

    test_file_eraser erase_ndx("eraseme.ndx");
    test_file_eraser erase_dat("eraseme.dat");
    qx_ins.save("eraseme");

    TEST_FILES_EQUAL("eraseme.ndx", qx_ins_path + ".ndx");
    TEST_FILES_EQUAL("eraseme.dat", qx_ins_path + ".dat");

    database db_tmp("eraseme");
    BOOST_TEST_EQUAL(qx_ins.tables_count(), db_tmp.tables_count());

    db_tmp.save("eraseme");
    TEST_FILES_EQUAL("eraseme.ndx", qx_ins_path + ".ndx");
    TEST_FILES_EQUAL("eraseme.dat", qx_ins_path + ".dat");
}

void test_add_table()
{
    table const t = table::read_from_text(simple_table_text);

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

void test_delete()
{
    database qx_ins(qx_ins_path);
    int const initial_count = qx_ins.tables_count();

    BOOST_TEST_THROW
        (qx_ins.delete_table(table::Number(1))
        ,std::invalid_argument
        ,match_substr("not found")
        );

    qx_ins.delete_table(table::Number(250));
    BOOST_TEST_EQUAL(qx_ins.tables_count(), initial_count - 1);

    qx_ins.delete_table(table::Number(202));
    BOOST_TEST_EQUAL(qx_ins.tables_count(), initial_count - 2);

    test_file_eraser erase_ndx("eraseme.ndx");
    test_file_eraser erase_dat("eraseme.dat");
    qx_ins.save("eraseme");

    database db_tmp("eraseme");
    BOOST_TEST_EQUAL(db_tmp.tables_count(), initial_count - 2);
}

void do_test_copy(std::string const& path)
{
    database db_orig(path);
    auto const tables_count = db_orig.tables_count();

    std::stringstream index_ss;
    shared_ptr<std::stringstream> data_ss = std::make_shared<std::stringstream>();

    // Make a copy of the database under new name.
    {
    database db_new;
    for(int i = 0; i != tables_count; ++i)
        {
        // Check that each table can be serialized to and deserialized from the
        // text.
        auto const& orig_table = db_orig.get_nth_table(i);
        auto const orig_text = orig_table.save_as_text();
        table const& new_table = table::read_from_text(orig_text);
        auto const new_text = new_table.save_as_text();
        BOOST_TEST_EQUAL(new_text, orig_text);

        db_new.append_table(new_table);
        }

    db_new.save(index_ss, *data_ss);
    }

    // And read it back.
    database db_new(index_ss, data_ss);
    BOOST_TEST_EQUAL(db_new.tables_count(), tables_count);

    // In general, we can't just use TEST_FILES_EQUAL() to compare the files
    // here because the order of tables in the original .dat file is lost and
    // it does not need to be the same as the order in the index file, so we
    // just compare the logical contents.
    for(int i = 0; i != tables_count; ++i)
        {
        BOOST_TEST_EQUAL
            (db_new.get_nth_table(i).save_as_text()
            ,db_orig.get_nth_table(i).save_as_text()
            );
        }
}

void test_copy()
{
    do_test_copy(qx_cso_path);
    do_test_copy(qx_ins_path);
}

int test_main(int, char*[])
{
    test_database_open();
    test_table_access_by_index();
    test_table_access_by_number();
    test_save();
    test_to_from_text();
    test_from_text();
    test_add_table();
    test_delete();
    test_copy();

    return EXIT_SUCCESS;
}

