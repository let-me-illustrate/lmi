// SOA tables represented in binary SOA format--unit test.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "rate_table.hpp"

#include "assert_lmi.hpp"
#include "miscellany.hpp"
#include "path.hpp"
#include "path_utility.hpp"
#include "test_tools.hpp"

#include <fstream>
#include <iomanip>                      // setw(), setfill()
#include <ios>
#include <sstream>
#include <stdexcept>
#include <streambuf>

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
        :orig_streambuf_ {std::cout.rdbuf(stream_out_.rdbuf())}
        {
        }

    ~std_out_redirector()
        {
        std::cout.rdbuf(orig_streambuf_);
        }

    std_out_redirector(std_out_redirector const&) = delete;
    std_out_redirector& operator=(std_out_redirector const&) = delete;

    // For convenience, this function returns everything output so far and clears
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
    // initialized before orig_streambuf_ whose initialization uses it.

    // The stream where cout is redirected during this object life-time.
    std::ostringstream stream_out_;

    // The original buffer used by cout before we redirected it.
    std::streambuf* const orig_streambuf_;
};

// Class ensuring that the file with the given name is removed when the test
// ends, whether it succeeds or fails.
class test_file_eraser
{
  public:
    explicit test_file_eraser(fs::path const& path)
        :path_ {path}
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
    test_file_eraser(test_file_eraser const&) = delete;
    test_file_eraser& operator=(test_file_eraser const&) = delete;

    fs::path path_;
};
} // Unnamed namespace.

namespace
{
/// SOA regulatory table database.

std::string const qx_cso_path("/opt/lmi/data/qx_cso");

int const qx_cso_num_tables = 142;

/// SOA insurance table database.

std::string const qx_ins_path("/opt/lmi/data/qx_ins");

/// Prefix used for the test tables.
std::string const simple_table_header =
1 + R"--cut-here--(
Table number: 1
Table type: Aggregate
Minimum age: 0
Maximum age: 1
Number of decimal places: 5
Table values:
)--cut-here--";

std::string const simple_table_values =
1 + R"--cut-here--(
  0  0.12345
  1  0.23456
)--cut-here--";

/// Minimal valid SOA table in text format.
std::string const simple_table_text(simple_table_header + simple_table_values);

/// A table with zero decimals. The original SOA code, and the code in
/// 'rate_table.cpp', both write these table values in a field of width
/// four: two spaces between columns, plus one for the data, plus one
/// for a nonexistent decimal point.
std::string const integral_table =
1 + R"--cut-here--(
Table number: 1
Table type: Aggregate
Minimum age: 0
Maximum age: 1
Number of decimal places: 0
Table values:
  0   0
  1   1
)--cut-here--";
} // Unnamed namespace.

/// Test opening database files.
///
/// Both '.ndx' and '.dat' files must exist.

void test_database_open()
{
    LMI_TEST_THROW
        (database("nonexistent")
        ,std::runtime_error
        ,lmi_test::what_regex("Unable to open 'nonexistent\\.ndx'")
        );

    test_file_eraser erase("eraseme.ndx");
    std::ifstream ifs((qx_cso_path + ".ndx").c_str(), ios_in_binary());
    std::ofstream ofs("eraseme.ndx", ios_out_trunc_binary());
    ofs << ifs.rdbuf();
    ofs.close();
    LMI_TEST_THROW
        (database("eraseme")
        ,std::runtime_error
        ,lmi_test::what_regex("Unable to open 'eraseme\\.dat'")
        );
}

void test_table_access_by_index()
{
    database qx_cso(qx_cso_path);
    LMI_TEST(qx_cso_num_tables == qx_cso.tables_count());

    // Just check that using valid indices doesn't throw.
    qx_cso.get_nth_table(0);
    qx_cso.get_nth_table(1);
    qx_cso.get_nth_table(qx_cso_num_tables - 1);

    LMI_TEST_THROW
        (qx_cso.get_nth_table(-1)
        ,std::runtime_error
        ,""
        );

    LMI_TEST_THROW
        (qx_cso.get_nth_table(qx_cso_num_tables)
        ,std::out_of_range
        ,""
        );

    LMI_TEST_THROW
        (qx_cso.get_nth_table(qx_cso_num_tables + 1)
        ,std::out_of_range
        ,""
        );
}

void test_table_access_by_number()
{
    database qx_cso(qx_cso_path);

    table::Number const number(qx_cso.get_nth_table(0).number());
    LMI_TEST_EQUAL(qx_cso.find_table(number).number(), number);

    LMI_TEST_THROW
        (qx_cso.find_table(table::Number(0))
        ,std::invalid_argument
        ,"table number 0 not found."
        );

    LMI_TEST_THROW
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

    LMI_TEST(text_orig == text_copy);
    LMI_TEST(table_orig == table_copy);
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
    std::cout << "Expect 'Possibly unknown field...':" << std::endl;
    LMI_TEST_THROW
        (table::read_from_text("Bloordyblop: yes\n" + simple_table_text)
        ,std::runtime_error
        ,lmi_test::what_regex("expected a field name")
        );
    }

    // However using it as part of a multiline field should succeed, albeit
    // with a warning.
    {
    std_out_redirector std_out_redir;
    table::read_from_text("Comments: no\nBloordyblop: yes\n" + simple_table_text);
    LMI_TEST(std_out_redir.take_output().find("Bloordyblop") != std::string::npos);
    }

    // Using too many values should fail.
    LMI_TEST_THROW
        (table::read_from_text(simple_table_text + "  2  0.34567\n")
        ,std::runtime_error
        ,lmi_test::what_regex("expected a field")
        );

    // And so should using too few of them: chop off the last line to test.
    LMI_TEST_THROW
        (table::read_from_text(simple_table_header + "  0  0.12345")
        ,std::runtime_error
        ,lmi_test::what_regex("missing")
        );

    // Using bad hash value should fail.
    LMI_TEST_THROW
        (table::read_from_text(simple_table_text + "Hash value: 1234567890\n")
        ,std::runtime_error
        ,lmi_test::what_regex("hash value 1234567890")
        );

    // Using values greater than 1 should be possible.
    table::read_from_text
        (simple_table_header
        +"  0  0.12345\n"
         "  1 10.98765\n"
        );

    // The number of decimals may be zero.
    table::read_from_text(integral_table);
}

void test_save()
{
    database qx_ins(qx_ins_path);

    qx_ins.save("eraseme0");

    bool okay_ndx0 = files_are_identical("eraseme0.ndx", qx_ins_path + ".ndx");
    bool okay_dat0 = files_are_identical("eraseme0.dat", qx_ins_path + ".dat");
    LMI_TEST(okay_ndx0);
    LMI_TEST(okay_dat0);

    database db_tmp("eraseme0");
    LMI_TEST_EQUAL(qx_ins.tables_count(), db_tmp.tables_count());

    // File 'eraseme0.dat' is still open and cannot be removed yet.
    // Saving 'db_tmp' closes the file so that it can be removed.
    db_tmp.save("eraseme1");

    // Leave the files for analysis if they didn't match.
    if(okay_ndx0 && okay_dat0)
        {
        LMI_TEST(0 == std::remove("eraseme0.ndx"));
        LMI_TEST(0 == std::remove("eraseme0.dat"));
        }

    bool okay_ndx1 = files_are_identical("eraseme1.ndx", qx_ins_path + ".ndx");
    bool okay_dat1 = files_are_identical("eraseme1.dat", qx_ins_path + ".dat");
    LMI_TEST(okay_ndx1);
    LMI_TEST(okay_dat1);
    // Leave the files for analysis if they didn't match.
    if(okay_ndx1 && okay_dat1)
        {
        LMI_TEST(0 == std::remove("eraseme1.ndx"));
        LMI_TEST(0 == std::remove("eraseme1.dat"));
        }
}

void test_add_table()
{
    table const t = table::read_from_text(simple_table_text);

    database qx_ins(qx_ins_path);
    int const count = qx_ins.tables_count();

    qx_ins.append_table(t);
    LMI_TEST_EQUAL(qx_ins.tables_count(), count + 1);

    LMI_TEST_THROW
        (qx_ins.append_table(t)
        ,std::invalid_argument
        ,"table number 1 already exists."
        );

    qx_ins.add_or_replace_table(t);
    LMI_TEST_EQUAL(qx_ins.tables_count(), count + 1);
}

void test_delete()
{
    database qx_ins(qx_ins_path);
    int const initial_count = qx_ins.tables_count();

    LMI_TEST_THROW
        (qx_ins.delete_table(table::Number(1))
        ,std::invalid_argument
        ,lmi_test::what_regex("not found")
        );

    qx_ins.delete_table(table::Number(250));
    LMI_TEST_EQUAL(qx_ins.tables_count(), initial_count - 1);

    qx_ins.delete_table(table::Number(202));
    LMI_TEST_EQUAL(qx_ins.tables_count(), initial_count - 2);

    test_file_eraser erase_ndx("eraseme.ndx");
    test_file_eraser erase_dat("eraseme.dat");
    qx_ins.save("eraseme");

    database db_tmp("eraseme");
    LMI_TEST_EQUAL(db_tmp.tables_count(), initial_count - 2);
}

void do_test_copy(std::string const& path)
{
    database db_orig(path);
    auto const tables_count = db_orig.tables_count();

    std::stringstream index_ss;
    std::shared_ptr<std::stringstream> data_ss = std::make_shared<std::stringstream>();

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
        LMI_TEST_EQUAL(new_text, orig_text);

        db_new.append_table(new_table);
        }

    db_new.save(index_ss, *data_ss);
    }

    // And read it back.
    database db_new(index_ss, data_ss);
    LMI_TEST_EQUAL(db_new.tables_count(), tables_count);

    // Compare binary rate-table files logically rather than literally.
    // These files are unlikely to be identical because the order of
    // the tables in the original .dat file is lost and need not be the
    // same as the order in the index file.
    for(int i = 0; i != tables_count; ++i)
        {
        LMI_TEST_EQUAL
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

/// Test deduce_number_of_decimals(std::string const&).
///
/// The tested function's argument is a string for generality, though
/// in its intended use that string is always a value returned by
/// value_cast<std::string>(double). The extra generality makes it
/// easier to write tests here. Some of the failing tests in comments
/// indicate improvements needed if a more general version of the
/// tested function is ever desired for other purposes.

void test_decimal_deduction()
{
    //                                               1 234567890123456
    LMI_TEST_EQUAL( 9, deduce_number_of_decimals("0002.123456789000001"));
    LMI_TEST_EQUAL( 8, deduce_number_of_decimals("0002.123456789999991"));

    LMI_TEST_EQUAL( 8, deduce_number_of_decimals("0002.12345678999999 "));
    LMI_TEST_EQUAL(13, deduce_number_of_decimals("0002.1234567899999  "));

    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   0.000000000000000"));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   0.000000000000000000000000"));
    // Fails, but value_cast can't return this.
//  LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   0.0              "));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   0.               "));
    // Fails, but value_cast can't return this.
//  LMI_TEST_EQUAL( 0, deduce_number_of_decimals("    .0              "));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("    .               "));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   0                "));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   1                "));
    LMI_TEST_EQUAL( 0, deduce_number_of_decimals("   9                "));

    //                                              123456789012345678
    LMI_TEST_EQUAL( 5, deduce_number_of_decimals("0.012830000000000001"));
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
    test_decimal_deduction();

    return EXIT_SUCCESS;
}
