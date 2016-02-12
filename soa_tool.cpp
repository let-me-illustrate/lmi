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
#include "path_utility.hpp"
#include "soa_database.hpp"

#include <algorithm>
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

void list_tables(fs::path const& database_filename)
{
    database const table_file(database_filename);
    for(int i = 0; i != table_file.tables_count(); ++i)
        {
        table const& t = table_file.get_nth_table(i);
        std::cout
            << std::setw( 5) << std::setfill('0')
            << t.number().value()
            << ' '
            << t.name()
            << " [" << t.type_as_string() << "]\n"
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

    // Build the list of all existing tables numbers.
    int const tables_count = table_file.tables_count();
    std::vector<table::Number> numbers;
    numbers.reserve(tables_count);
    for(int i = 0; i != tables_count; ++i)
        {
        numbers.push_back(table_file.get_nth_table(i).number());
        }

    // And now add them to the new database in order of their numbers.
    std::sort(numbers.begin(), numbers.end());

    for(int i = 0; i != tables_count; ++i)
        {
        table::Number const num = numbers[i];

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

void extract
    (fs::path database_filename
    ,int      table_number_to_extract
    )
{
    database const table_file(database_filename);

    table const&
        t = table_file.find_table(table::Number(table_number_to_extract));

    std::ostringstream oss;
    oss << table_number_to_extract << ".txt";
    t.save_as_text(oss.str());
    std::cout << "Extracted: " << oss.str().c_str() << '\n';
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

int main(int argc, char* argv[])
{
    int c;
    int option_index = 0;
    static struct Option long_options[] =
      {
        {"help"           , NO_ARG,   0, 'h', 0    , "display this help and exit"},
//        {"delete"         , REQD_ARG, 0,   0, 0    , "delete something"},
        {"license"        , NO_ARG,   0, 'l', 0    , "display license and exit"},
        {"accept"         , NO_ARG,   0, 'a', 0    , "accept license (-l to display)"},
        {"file=FILE"      , REQD_ARG, 0, 'f', 0    , "use database FILE"},
        {"crc"            , NO_ARG,   0, 'c', 0    , "show CRCs of all tables"},
        {"list"           , NO_ARG,   0, 't', 0    , "list all tables"},
        {"squeeze=NEWFILE", REQD_ARG, 0, 's', 0    , "compress database into NEWFILE"},
        {"merge=TEXTFILE" , REQD_ARG, 0, 'm', 0    , "merge TEXTFILE into database"},
        {"extract=n"      , REQD_ARG, 0, 'e', 0    , "extract table #n into n.txt"},
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
    bool run_extract      = false;
    bool run_rename       = false;
    int  num_to_run       = 0;

    fs::path database_filename;
    fs::path new_database_filename;
    fs::path filename_to_merge;
    int table_number_to_extract = 0;
    fs::path filename_of_table_names;

    try
        {
        GetOpt getopt_long
            (argc
            ,argv
            ,"acf:hls:e:m:"
            ,long_options
            ,&option_index
            ,1
            );

        while(EOF != (c = getopt_long ()))
          {
            switch (c)
              {
              case 0:
                {
                char const* current_option = long_options[option_index].name;
                std::printf ("option %s", current_option);
                if(getopt_long.optarg)
                  std::printf (" with arg %s", getopt_long.optarg);
                std::printf ("\n");
                }
                break;

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
                }
                break;

              case 'l':
                {
                show_license = true;
                }
                break;

              case 'c':
                {
                run_crc = true;
                ++num_to_run;
                }
                break;

              case 't':
                {
                run_list = true;
                ++num_to_run;
                }
                break;

              case 's':
                {
                run_squeeze = true;
                ++num_to_run;
                new_database_filename = getopt_long.optarg;
                }
                break;

              case 'm':
                {
                run_merge = true;
                ++num_to_run;
                filename_to_merge = getopt_long.optarg;
                }
                break;

              case 'e':
                {
                run_extract = true;
                ++num_to_run;
                table_number_to_extract = std::atoi(getopt_long.optarg);
                }
                break;

              case 'r':
                {
                run_rename = true;
                filename_of_table_names = getopt_long.optarg;
                }
                break;

              case 'o':
                {
                std::printf ("option o");
                if(getopt_long.optarg)
                  std::printf (" with value '%s'", getopt_long.optarg);
                std::printf ("\n");
                }
                break;

              default:
                std::printf ("? getopt returned character code 0%o ?\n", c);
              }
          }

        if((c = getopt_long.optind) < argc)
          {
            std::printf ("non-option ARGV-elements: ");
            while (c < argc)
              std::printf ("%s ", argv[c++]);
            std::printf ("\n");
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

        switch(num_to_run)
            {
            case 0:
                std::cerr << "Please use exactly one of --crc, --list, --squeeze, --merge or --extract.\n";
                getopt_long.usage();
                return EXIT_FAILURE;

            case 1:
                // Continue and process the single selected operation below.
                break;

            default:
                std::cerr << "At most one operation can be selected.\n";
                return EXIT_FAILURE;
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

        std::cerr << "Unexpected unknown run mode, nothing done.\n";
        }
    catch(std::exception& e)
        {
        std::cerr << "Fatal exception: " << e.what() << '\n';
        }
    catch(...)
        {
        std::cerr << "Nonspecific fatal exception\n";
        }
    return EXIT_FAILURE;
}
