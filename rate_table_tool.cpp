// Life insurance illustrations: SOA mortality table utility.
//
// Copyright (C) 2003, 2004, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "alert.hpp"
#include "getopt.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "path.hpp"
#include "path_utility.hpp"
#include "rate_table.hpp"

#include <algorithm>                    // sort()
#include <cstdio>                       // fflush()
#include <cstdlib>                      // atoi()
#include <exception>
#include <fstream>
#include <iomanip>                      // setw(), setfill()
#include <iostream>                     // cout, cerr
#include <map>
#include <memory>                       // unique_ptr
#include <ostream>                      // endl
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace soa_v3_format;

void calculate_and_display_crcs(fs::path const& database_filename)
{
    database const table_file(database_filename);
    for(int i = 0; i != table_file.tables_count(); ++i)
        {
        table const& t = table_file.get_nth_table(i);
        std::uint32_t crc = t.compute_hash_value();
        std::cout
            << std::dec << std::setw( 5) << std::setfill('0')
            << t.number().value()
            << ' '
            << std::dec << std::setw(10) << std::setfill('0')
            << crc
            << ' '
            << std::hex << std::setw( 8) << std::setfill('0')
            << crc
            << ' '
            << t.name()
            << '\n'
            ;
        }
}

/// Return a sorted vector of all table numbers.

std::vector<table::Number> get_all_tables_numbers(database const& table_file)
{
    int const tables_count = table_file.tables_count();
    std::vector<table::Number> numbers;
    numbers.reserve(tables_count);
    for(int i = 0; i != tables_count; ++i)
        {
        numbers.push_back(table_file.get_nth_table(i).number());
        }

    std::sort(numbers.begin(), numbers.end());

    return numbers;
}

void list_tables(fs::path const& database_filename)
{
    database const table_file(database_filename);

    auto const numbers = get_all_tables_numbers(table_file);
    for(auto const& num : numbers)
        {
        table const& t = table_file.find_table(num);
        std::cout
            << std::setw(5) << std::setfill('0')
            << t.number().value()
            << ' '
            << t.name()
            << '\n'
            ;
        }
}

/// Merge 'path_to_merge' into 'database_filename'.
///
/// If no 'database_filename' exists, create it, as an incidental side
/// effect of merging. No command other than '--merge' has any need to
/// create an empty database, so no separate '--create' command would
/// be useful.
///
/// If 'path_to_merge' names a file, then merge that file. If it names
/// a directory, then merge all '*.rates' files in that directory.
/// Rationale:
///   https://lists.nongnu.org/archive/html/lmi/2016-11/msg00025.html

void merge
    (fs::path const& database_filename
    ,fs::path const& path_to_merge
    )
{
    std::unique_ptr<database> table_file;
    if(database::exists(database_filename))
        {
        table_file.reset(::new database(database_filename));
        }
    else
        {
        table_file.reset(::new database);
        }

    int count = 0;

    if(fs::is_directory(path_to_merge))
        {
        // Merge tables in sorted order, so that adding identical sets
        // of tables to identical databases yields identical results.
        // An entire database may thus be sorted by extracting all its
        // tables and merging them into a new database in a single
        // operation. The only real benefit is that identity is easier
        // to verify than equivalence: databases created this way from
        // the same data on different machines have identical md5sums.
        std::vector<fs::path> table_names;
        for(auto const& i : fs::directory_iterator(path_to_merge))
            {
            if(".rates" == i.path().extension())
                {
                table_names.push_back(i.path());
                }
            }
        std::sort(table_names.begin(), table_names.end());
        for(auto const& j : table_names)
            {
            table const& t = table::read_from_text(j);
            table_file->add_or_replace_table(t);
            ++count;
            }
        }
    else
        {
        table const& t = table::read_from_text(path_to_merge);
        table_file->add_or_replace_table(t);
        ++count;
        }

    table_file->save(database_filename);

    std::cout << "Number of tables merged: " << count << "\n";
}

void delete_table
    (fs::path database_filename
    ,int      table_number_to_delete
    )
{
    database table_file(database_filename);
    table_file.delete_table(table::Number(table_number_to_delete));
    table_file.save(database_filename);
}

/// Save the given table in a text file with its number as name;
/// return the name of this file.

std::string do_save_as_text_file(table const& t)
{
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(5) << t.number() << ".rates";
    std::string const filename = oss.str();
    t.save_as_text(filename);
    return filename;
}

void extract
    (fs::path database_filename
    ,int      table_number_to_extract
    )
{
    database const table_file(database_filename);

    table const&
        t = table_file.find_table(table::Number(table_number_to_extract));

    std::cout << "Extracted: " << do_save_as_text_file(t) << '\n';
}

void extract_all(fs::path database_filename)
{
    database const table_file(database_filename);

    auto const count = table_file.tables_count();
    for(int i = 0; i != count; ++i)
        {
        do_save_as_text_file(table_file.get_nth_table(i));
        }

    std::cout << "Number of tables extracted: " << count << "\n";
}

void rename_tables
    (fs::path const& database_filename
    ,fs::path const& filename_of_table_names
    )
{
    database table_file(database_filename);
    auto const numbers = get_all_tables_numbers(table_file);

    // This map has all valid table numbers as keys and the value is non-empty
    // iff the table with the corresponding key needs to be renamed to it.
    std::map<table::Number, std::string> name_map;
    for(auto const& num : numbers)
        {
        name_map.emplace(num, std::string());
        }

    // Read new names from the provided file in the "number name" format.
    std::ifstream ifs(filename_of_table_names.string().c_str());
    if(!ifs)
        {
        alarum()
            << "File with the new table names \"" << filename_of_table_names
            << "\" couldn't be opened."
            << std::flush
            ;
        }

    int line_num = 1;
    for(std::string line; std::getline(ifs, line); ++line_num)
        {
        // Parse the number at the beginning of the line taking care to handle
        // exceptions from stoi() because we want to throw our own exception,
        // with more information about the failure location.
        table::Number num(0);
        std::string error;
        try
            {
            int const n = std::stoi(line);
            if(n <= 0)
                {
                error = "table number must be strictly positive";
                }
            else
                {
                num = table::Number(n);

                // Also check that the table number is valid and hasn't
                // occurred before.
                auto const it = name_map.find(num);
                if(it == name_map.end())
                    {
                    std::ostringstream oss;
                    oss << "invalid table number " << num;
                    error = oss.str();
                    }
                else if(!it->second.empty())
                    {
                    std::ostringstream oss;
                    oss << "duplicate table number " << num;
                    error = oss.str();
                    }
                }
            }
        catch(std::invalid_argument const&)
            {
            error = "number expected at the beginning of the line";
            }
        catch(std::out_of_range const&)
            {
            error = "table number is too big";
            }

        if(!error.empty())
            {
            alarum()
                << "Error in new table names file \"" << filename_of_table_names
                << "\": " << error << " at line " << line_num << "."
                << std::flush
                ;
            }

        name_map[num] = line;
        }

    for(int i = 0; i != table_file.tables_count(); ++i)
        {
        table t = table_file.get_nth_table(i);

        auto const it = name_map.find(t.number());
        if(it != name_map.end())
            {
            t.name(it->second);
            table_file.add_or_replace_table(t);
            }
        }

    table_file.save(database_filename);
}

/// Return the number of tables that failed verification.

int verify(fs::path const& database_filename)
{
    database const orig_db(database_filename);

    int errors = 0;

    // Check that each table can be loaded and converted to/from text
    // losslessly.
    //
    // Make the output ordered by table numbers.
    auto const numbers = get_all_tables_numbers(orig_db);
    for(auto const& num : numbers)
        {
        try
            {
            table const& orig_table = orig_db.find_table(num);
            auto const orig_text = orig_table.save_as_text();
            table const& new_table = table::read_from_text(orig_text);
            auto const new_text = new_table.save_as_text();
            if(new_text != orig_text)
                {
                alarum()
                    << "After loading and saving the original table '\n"
                    << orig_text
                    << "' became '\n"
                    << new_text
                    << "'\n"
                    << LMI_FLUSH
                    ;
                }
            if(new_table != orig_table)
                {
                alarum()
                    << "After loading and saving the original table \n"
                    << "binary contents differed.\n"
                    << LMI_FLUSH
                    ;
                }
            }
        catch(std::exception const& e)
            {
            std::cout
                << "Verification failed for table #" << num << ": "
                << e.what()
                << std::endl
                ;

            ++errors;
            }
        }

    // Also make a copy of the database using our code.
    std::stringstream index_ss;
    std::shared_ptr<std::stringstream> data_ss = std::make_shared<std::stringstream>();

    auto const tables_count = orig_db.tables_count();
    {
    database new_db;
    for(int i = 0; i != orig_db.tables_count(); ++i)
        {
        new_db.append_table(orig_db.get_nth_table(i));
        }
    new_db.save(index_ss, *data_ss);
    }

    // Now reload database from it.
    database new_db(index_ss, data_ss);

    // And check that it's logically the same.
    //
    // Notice that index is also physically, i.e. byte-by-byte, identical to
    // the original index file, but the data file isn't necessarily identical
    // because the tables are always in the index order in the files we create
    // but this could have been not the case for the original file, so we can't
    // just use memcmp() for comparison here.
    if(new_db.tables_count() != tables_count)
        {
        std::cout
            << "Wrong number of tables " << new_db.tables_count()
            << " instead of expected " << tables_count
            << " after making a copy."
            << std::endl
            ;

        ++errors;
        }
    else
        {
        for(int i = 0; i != orig_db.tables_count(); ++i)
            {
            table const& orig_table = orig_db.get_nth_table(i);
            table const& new_table = new_db.get_nth_table(i);
            if(new_table != orig_table)
                {
                std::cout
                    << "Copy of the table #" << orig_table.number() << "'\n"
                    << new_table.save_as_text()
                    << "' differs from the original table '\n"
                    << orig_table.save_as_text()
                    << "'"
                    << std::endl
                    ;

                ++errors;
                }
            }
        }

    if(!errors)
        {
        std::cout << "All " << numbers.size() << " tables passed.\n";
        }

    return errors;
}

int try_main(int argc, char* argv[])
{
    int c;
    int option_index = 0;
    static Option long_options[] =
      {
        {"help"        ,NO_ARG   ,nullptr ,'h' ,nullptr ,"display this help and exit"},
        {"delete"      ,REQD_ARG ,nullptr ,'d' ,nullptr ,"delete table #n from database"},
        {"license"     ,NO_ARG   ,nullptr ,'l' ,nullptr ,"display license and exit"},
        {"accept"      ,NO_ARG   ,nullptr ,'a' ,nullptr ,"accept license (-l to display)"},
        {"file=FILE"   ,REQD_ARG ,nullptr ,'f' ,nullptr ,"use database FILE"},
        {"crc"         ,NO_ARG   ,nullptr ,'c' ,nullptr ,"show CRCs of all tables"},
        {"list"        ,NO_ARG   ,nullptr ,'t' ,nullptr ,"list all tables"},
        {"merge=PATH"  ,REQD_ARG ,nullptr ,'m' ,nullptr ,"merge PATH (file or dir) into database"},
        {"extract=n"   ,REQD_ARG ,nullptr ,'e' ,nullptr ,"extract table #n into '0000n.rates'"},
        {"extract-all" ,NO_ARG   ,nullptr ,'x' ,nullptr ,"extract all tables to '.rates' files"},
        {"rename=FILE" ,REQD_ARG ,nullptr ,'r' ,nullptr ,"rename tables from FILE"},
        {"verify"      ,NO_ARG   ,nullptr ,'v' ,nullptr ,"verify integrity of all tables"},
        {nullptr       ,NO_ARG   ,nullptr ,000 ,nullptr ,""}
      };
    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;
    bool run_crc          = false;
    bool run_list         = false;
    bool run_merge        = false;
    bool run_delete       = false;
    bool run_extract      = false;
    bool run_extract_all  = false;
    bool run_rename       = false;
    bool run_verify       = false;

    int  num_to_do        = 0;      // Number of actions to perform.
    bool needs_database   = true;

    fs::path database_filename;
    fs::path new_database_filename;
    fs::path path_to_merge;
    int table_number_to_extract = 0;
    int table_number_to_delete = 0;
    fs::path filename_of_table_names;

    GetOpt getopt_long
        (argc
        ,argv
        ,"acf:hls:e:m:"
        ,long_options
        ,&option_index
        ,1
        );

    bool command_line_syntax_error = false;

    while(EOF != (c = getopt_long ()))
      {
        switch(c)
          {
          case 'a':
            {
            license_accepted = true;
            }
            break;

          case 'f':
            {
            database_filename = getopt_long.optarg;
            }
            break;

          case 'h':
            {
            show_help = true;
            ++num_to_do;
            needs_database = false;
            }
            break;

          case 'l':
            {
            show_license = true;
            ++num_to_do;
            needs_database = false;
            }
            break;

          case 'c':
            {
            run_crc = true;
            ++num_to_do;
            }
            break;

          case 't':
            {
            run_list = true;
            ++num_to_do;
            }
            break;

          case 'm':
            {
            run_merge = true;
            ++num_to_do;
            path_to_merge = getopt_long.optarg;
            }
            break;

          case 'd':
            {
            run_delete = true;
            table_number_to_delete = std::atoi(getopt_long.optarg);
            }
            break;

          case 'e':
            {
            run_extract = true;
            ++num_to_do;
            table_number_to_extract = std::atoi(getopt_long.optarg);
            }
            break;

          case 'x':
            {
            run_extract_all = true;
            ++num_to_do;
            }
            break;

          case 'r':
            {
            run_rename = true;
            ++num_to_do;
            filename_of_table_names = getopt_long.optarg;
            }
            break;

          case 'v':
            {
            run_verify = true;
            ++num_to_do;
            }
            break;

          default:
            // Error message was already given from getopt() code, so no need
            // to output anything else here, but do flush its output so that it
            // appears before the usage message.
            std::fflush(stderr);

            command_line_syntax_error = true;

            // No need to continue with any other arguments neither, force
            // exiting from the loop.
            c = EOF;
          }
      }

    if((c = getopt_long.optind) < argc)
        {
        if(database_filename.string().empty())
            {
            database_filename = argv[c];
            }
        else
            {
            std::cerr << "Either positional argument or --file option can be used, but not both.\n";
            command_line_syntax_error = true;
            }

        if(c + 1 != argc)
            {
            std::cerr << "Only a single database file argument allowed.\n";
            command_line_syntax_error = true;
            }
        }

    switch(num_to_do)
        {
        case 0:
            if(!run_delete)
                {
                std::cerr
                    << "Please use exactly one of the following options:\n"
                    << "--crc, --list, --rename, --merge, --extract or --verify.\n";
                command_line_syntax_error = true;
                }
            break;

        case 1:
            if(run_delete && !run_extract)
                {
                std::cerr << "--delete can only be combined with --extract.\n";
                command_line_syntax_error = true;
                }
            break;

        default:
            std::cerr << "At most one operation can be selected.\n";
            command_line_syntax_error = true;
        }

    if(!command_line_syntax_error
      && needs_database
      && database_filename.string().empty()
      )
        {
        std::cerr << "Database file must be specified.\n";
        command_line_syntax_error = true;
        }

    // usage(), possibly called below, doesn't allow us to specify the
    // arguments directly, so force it to show it in this ugly way:
    std::string name_with_arg(argv[0]);
    name_with_arg += " <database-file>";
    argv[0] = const_cast<char*>(name_with_arg.c_str());

    if(command_line_syntax_error)
        {
        getopt_long.usage();
        return EXIT_FAILURE;
        }

    if(!license_accepted)
        {
        std::cerr << license_notices_as_text() << "\n\n";
        }

    if(show_license)
        {
        std::cerr << license_as_text() << "\n\n";
        return EXIT_SUCCESS;
        }

    if(show_help)
        {
        getopt_long.usage();
        return EXIT_SUCCESS;
        }

    if(run_crc)
        {
        calculate_and_display_crcs(database_filename);
        return EXIT_SUCCESS;
        }

    if(run_list)
        {
        list_tables(database_filename);
        return EXIT_SUCCESS;
        }

    if(run_rename)
        {
        rename_tables(database_filename, filename_of_table_names);
        return EXIT_SUCCESS;
        }

    if(run_merge)
        {
        merge(database_filename, path_to_merge);
        return EXIT_SUCCESS;
        }

    if(run_extract)
        {
        extract(database_filename, table_number_to_extract);
        return EXIT_SUCCESS;
        }

    if(run_extract_all)
        {
        extract_all(database_filename);
        return EXIT_SUCCESS;
        }

    // Order matters here: if both --delete and --extract are used, we need to
    // extract the table before removing it.
    if(run_delete)
        {
        delete_table(database_filename, table_number_to_delete);
        return EXIT_SUCCESS;
        }

    if(run_verify)
        {
        return verify(database_filename) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
        }

    std::cerr << "Unexpected unknown run mode, nothing done.\n";
    return EXIT_FAILURE;
}
