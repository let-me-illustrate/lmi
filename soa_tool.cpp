// Life insurance illustrations: SOA mortality table utility.
//
// Copyright (C) 2003, 2004, 2015, 2016 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "getopt.hpp"
#include "license.hpp"
#include "main_common.hpp"
#include "path_utility.hpp"
#include "soa_database.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

using namespace soa_v3_format;

std::map<table::Number, std::string> name_map;

void calculate_and_display_crcs(fs::path const& database_filename)
{
    database const table_file(database_filename);
    for(int i = 0; i != table_file.tables_count(); ++i)
        {
        table const& t = table_file.get_nth_table(i);
        unsigned long int crc = t.compute_hash_value();
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

// Return a sorted vector of all table numbers.
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
    for(auto num: numbers)
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

void squeeze
    (fs::path const& database_filename
    ,fs::path const& new_database_filename
    )
{
    database const table_file(database_filename);
    database new_file;

    auto const numbers = get_all_tables_numbers(table_file);
    for(auto num: numbers)
        {
        table t = table_file.find_table(num);

        // Also adjust the table names if requested.
        std::map<table::Number, std::string>::const_iterator n = name_map.find(num);
        if(n != name_map.end())
            {
            t.name(n->second);
            }
        new_file.append_table(t);
        }

    new_file.save(new_database_filename);

    std::cout
        << "Squeezed database into new file '"
        << new_database_filename
        << "'\n"
        ;
}

void merge
    (fs::path const& database_filename
    ,fs::path const& filename_to_merge
    )
{
    database table_file(database_filename);
    table const& t = table::read_from_text(filename_to_merge);
    table_file.add_or_replace_table(t);
    table_file.save(database_filename);
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

// Save the given table in a text file with its number as name, return the name
// of this file.
std::string do_save_as_text_file(table const& t)
{
    std::ostringstream oss;
    oss << t.number() << ".txt";
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

    auto const tables_count = table_file.tables_count();
    for(int i = 0; i != tables_count; ++i)
        {
        do_save_as_text_file(table_file.get_nth_table(i));
        }

    std::cout << "Extracted " << tables_count << " tables.\n";
}

void rename_tables
    (fs::path const& filename_of_table_names
    )
{
    std::ifstream ifs(filename_of_table_names.string().c_str());
    std::string line;
    while(std::getline(ifs, line, '\n'))
        {
        std::string number = line.substr
            (line.find_first_not_of('0')
            ,line.find_first_of(' ')
            );
        int const index = std::atoi(number.c_str());
        if(0 == index)
            {
            std::cerr << "Invalid line:\n" << line << '\n';
            return;
            }
        name_map[table::Number(index)] = line;
        }
// Use this only with 'squeeze'.
}

int try_main(int argc, char* argv[])
{
    int c;
    int option_index = 0;
    static struct Option long_options[] =
      {
        {"help"           , NO_ARG,   0, 'h', 0    , "display this help and exit"},
        {"delete"         , REQD_ARG, 0, 'd', 0    , "delete table #n from database"},
        {"license"        , NO_ARG,   0, 'l', 0    , "display license and exit"},
        {"accept"         , NO_ARG,   0, 'a', 0    , "accept license (-l to display)"},
        {"file=FILE"      , REQD_ARG, 0, 'f', 0    , "use database FILE"},
        {"crc"            , NO_ARG,   0, 'c', 0    , "show CRCs of all tables"},
        {"list"           , NO_ARG,   0, 't', 0    , "list all tables"},
        {"squeeze=NEWFILE", REQD_ARG, 0, 's', 0    , "compress database into NEWFILE"},
        {"merge=TEXTFILE" , REQD_ARG, 0, 'm', 0    , "merge TEXTFILE into database"},
        {"extract=n"      , REQD_ARG, 0, 'e', 0    , "extract table #n into n.txt"},
        {"extract-all"    , NO_ARG,   0, 'x', 0    , "extract all tables to text files"},
        {"rename=NAMEFILE", REQD_ARG, 0, 'r', 0    , "rename tables from NAMEFILE"},
        {0                , NO_ARG,   0,   0, 0    , ""}
      };
    bool license_accepted = false;
    bool show_license     = false;
    bool show_help        = false;
    bool run_crc          = false;
    bool run_list         = false;
    bool run_squeeze      = false;
    bool run_merge        = false;
    bool run_delete       = false;
    bool run_extract      = false;
    bool run_extract_all  = false;
    bool run_rename       = false;

    int  num_to_do        = 0;      // Number of actions to perform.
    bool needs_database   = true;

    fs::path database_filename;
    fs::path new_database_filename;
    fs::path filename_to_merge;
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
        switch (c)
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

          case 's':
            {
            run_squeeze = true;
            ++num_to_do;
            new_database_filename = getopt_long.optarg;
            }
            break;

          case 'm':
            {
            run_merge = true;
            ++num_to_do;
            filename_to_merge = getopt_long.optarg;
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
            filename_of_table_names = getopt_long.optarg;
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
                std::cerr << "Please use exactly one of --crc, --list, --squeeze, --merge or --extract.\n";
                command_line_syntax_error = true;
                }
            break;

        case 1:
            if(run_delete && !run_extract)
                {
                std::cerr << "--delete can only be combined with --extract.\n";
                command_line_syntax_error = true;
                }
            if(run_rename && !run_squeeze)
                {
                std::cerr << "--rename can only be used together with --squeeze.\n";
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
        rename_tables(filename_of_table_names);
        }

    if(run_squeeze)
        {
        squeeze(database_filename, new_database_filename);
        return EXIT_SUCCESS;
        }

    if(run_merge)
        {
        merge(database_filename, filename_to_merge);
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

    std::cerr << "Unexpected unknown run mode, nothing done.\n";
    return EXIT_FAILURE;
}
