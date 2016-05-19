// Auxiliary routines for tables in deprecated SOA format.
//
// Copyright (C) 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef soa_helpers_hpp
#define soa_helpers_hpp

#include "config.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "deserialize_cast.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"             // fs::path inserter

#include <boost/cstdint.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>

#include <climits>                      // CHAR_BIT
#include <exception>
#include <string>
#include <vector>

// SOA actuarial table format helpers.

struct soa_record_info
{
    int         index;
    std::string name;
};

std::vector<soa_record_info> list_soa_file_tables(char const* filename)
{
    std::vector<soa_record_info> v;

    fs::path index_path(filename);
    index_path = fs::change_extension(index_path, ".ndx");
    fs::ifstream index_ifs(index_path, ios_in_binary());
    if(!index_ifs)
        {
        fatal_error()
            << "File '"
            << index_path
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    // Index records have fixed length:
    //   4-byte integer:     table number
    //   50-byte char array: table name
    //   4-byte integer:     byte offset into '.dat' file
    // Table numbers are not necessarily consecutive or sorted.

    // SOA !! Assert endianness too? SOA tables are not portable;
    // probably they can easily be read only on x86 hardware.

    BOOST_STATIC_ASSERT(8 == CHAR_BIT);
    BOOST_STATIC_ASSERT(4 == sizeof(int));
    BOOST_STATIC_ASSERT(2 == sizeof(short int));

    int const index_record_length(58);
    char index_record[index_record_length] = {0};

    BOOST_STATIC_ASSERT(sizeof(boost::int32_t) <= sizeof(int));
    while(index_ifs)
        {
        index_ifs.read(index_record, index_record_length);
        if(index_record_length != index_ifs.gcount())
            {
            if(!index_ifs)
                {
                break;
                }
            fatal_error()
                << "Index file '"
                << index_path.string()
                << "': attempted to read "
                << index_record_length
                << " bytes, but got "
                << index_ifs.gcount()
                << " bytes instead."
                << LMI_FLUSH
                ;
            }

        soa_record_info rec;
        rec.index = deserialize_cast<boost::int32_t>(index_record);
        rec.name.assign(index_record + 4);
        v.push_back(rec);
        }

    return v;
}

#endif // soa_helpers_hpp

