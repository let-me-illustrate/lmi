// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: actuarial_table.cpp,v 1.13 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "actuarial_table.hpp"

#include "alert.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <cctype>    // std::toupper()
#include <climits>   // CHAR_BIT
#include <ios>
#include <limits>

// Read a table from a database in the binary format designed by the
// Society of Actuaries (SOA) and used for the tables SOA publishes.

// Do not check CRCs of these tables as SOA's software does. Tests
// show that CRC checking makes the illustration system considerably
// slower. Data should generally be validated against published
// checksums when acquired (e.g., downloaded), not before each use.
// Local hardware that stores SOA tables probably performs a CRC
// already, as do networks and communications links. Repeating such
// tests in software is costly and redundant except for authentication
// of downloads, particularly against tampering; but for that purpose,
// a more secure algorithm should be used. Besides, SOA's software
// calculates CRCs incorrectly.

namespace
{
    // TODO ?? Add unit tests for tables with length
    //   - more than 2^15 but less than 2^16; and
    //   - exactly 2^16
    //   - more than 2^16

    // The binary format that the SOA uses for its published tables
    // has only a sixteen-bit integer to represent a table's length
    // in bytes. This length must be read as unsigned because it may
    // be 2^15 or greater, but less than 2^16.
    typedef boost::uint16_t soa_table_length_type;
    int soa_table_length_max =
        std::numeric_limits<soa_table_length_type>::max()
        ;

    // The value -1 is invalid for all data fields except table name
    // and values.
    int invalid = -1;

    int methuselah = 969; // Genesis 5:27.

    template<typename T>
    T read(std::istream& is, T& t, boost::int16_t nominal_length)
    {
        LMI_ASSERT(sizeof(T) == nominal_length);
        char z[sizeof(T)];
        is.read(z, sizeof(T));
        t = *reinterpret_cast<T*>(z);
        LMI_ASSERT(invalid != t);
        return t;
    }

    void read_values
        (std::vector<double>& values
        ,std::istream&        is
        ,int                  nominal_length
        ,int                  min_age
        ,int                  max_age
        ,unsigned char        table_type
        ,int                  max_select_age
        ,int                  select_period
        )
    {
        if('S' != table_type)
            {
            // One might suppose that the select period for tables
            // that are not select couldn't be nonzero, but the SOA
            // publishes tables that don't honor that invariant.
//            LMI_ASSERT(0 == select_period); // Nope!
            select_period = 0;
            }

        LMI_ASSERT(min_age <= max_age);

        // If 'max_select_age' is given as zero, then it's to be taken as
        // unlimited, so its value should be 'max_age'.
        if(0 == max_select_age)
            {
            max_select_age = max_age;
            }

        int number_of_values = 1 + max_age - min_age;
        if(select_period)
            {
            number_of_values =
                  (1 + max_select_age - min_age) * select_period
              +   1 + max_age - min_age - select_period
              ;
            }
        int deduced_length = number_of_values * sizeof(double);
        LMI_ASSERT
            (   soa_table_length_max < deduced_length
            ||  nominal_length == deduced_length
            );
        values.resize(number_of_values);
        char z[sizeof(double)];
        for(int j = 0; j < number_of_values; ++j)
            {
            is.read(z, sizeof(double));
            values[j] = *reinterpret_cast<double*>(z);
            }
    }

    // TODO ?? This function has too many arguments. Consider making
    // it a member of a new class.
    //
    std::vector<double> particular_values
        (std::vector<double> const& values
        ,std::istream&        // is
        ,int                  // nominal_length
        ,int                  min_age
        ,int                  // max_age
        ,unsigned char        table_type
        ,int                  max_select_age
        ,int                  select_period
        ,int                  age
        ,int                  length
        )
    {
        std::vector<double> v;
        switch(table_type)
          {
          case 'A':
            {
            // Parenthesize the offsets--addition in C and C++ is
            // in effect left associative:
            //   values.begin() + age - min_age
            // means
            //   (values.begin() + age) - min_age
            // but the subexpression
            //   (values.begin() + age)
            // is likely to return a past-the-end iterator, which
            // libstdc++'s debug mode will dislike.
            //
            v = std::vector<double>
                (values.begin() + (age - min_age)
                ,values.begin() + (age - min_age + length)
                );
            }
            break;
          case 'D':
            {
            // TODO ?? Interpret duration as index 0; what does SOA software do?
            v = std::vector<double>
                (values.begin()
                ,values.begin() + length
                );
            }
            break;
          case 'S':
            {
            int k =
                    std::max(0, age - max_select_age)
                +   std::min(max_select_age, age - min_age) * (1 + select_period)
                ;
            v.resize(length);
            for(int j = 0; j < length; ++j, ++k)
                {
                v[j] = values[k];
                if(j + age < max_select_age + select_period && select_period <= j)
                    {
                    k += select_period;
                    }
                }
            }
            break;
          default:
            {
            fatal_error()
                << "Table type '"
                << table_type
                << "' not recognized: must be one of 'A', 'D', or 'S'."
                << LMI_FLUSH
                ;
            }
          }
        LMI_ASSERT(v.size() == static_cast<unsigned int>(length));
        return v;
    }
} // Unnamed namespace.

std::vector<double> actuarial_table
    (std::string const& a_table_filename
    ,int                a_table_number
    ,int                a_age
    ,int                a_len
    )
{
    // SOA documentation does not specify the domain of table numbers,
    // but their tables seem to use only positive integers
    // representable as 32-bit signed int, so take that as the range.
    // Asserting that the table number is nonzero makes it safe to use
    // zero as a sentry.

    // TODO ?? Consider requiring an extra argument--a string that
    // gives context, e.g., "Current COI rates"--so that diagnostics
    // can be made more helpful.

    // TODO ?? Add a unit test for this.
    if(a_table_number <= 0)
        {
        fatal_error()
            << "There is no table number "
            << a_table_number
            << " in file '"
            << a_table_filename
            << "."
            << LMI_FLUSH
            ;
        }

    fs::path index_path(a_table_filename);
    index_path = fs::change_extension(index_path, ".ndx");
    fs::ifstream index_ifs(index_path, std::ios_base::in | std::ios_base::binary);

    // Index records have fixed length:
    //   4-byte integer:     table number
    //   50-byte char array: table name
    //   4-byte integer:     byte offset into '.dat' file
    // Table numbers are not necessarily consecutive or sorted.

    // TODO ?? Assert endianness too? SOA tables are not portable;
    // probably they can easily be read only on x86 hardware.

    LMI_ASSERT(8 == CHAR_BIT);
    LMI_ASSERT(4 == sizeof(int));
    LMI_ASSERT(2 == sizeof(short int));

    // 27.4.3.2/2 requires that this be interpreted as invalid. The
    // variable 'invalid' is not used here, because its value might
    // change someday, but only '-1' is mentioned in the standard.
    std::streampos table_offset(-1);

    int const index_record_length(58);
    char index_record[index_record_length] = {0};

    LMI_ASSERT(sizeof (boost::int32_t) <= sizeof(int));
    while(index_ifs)
        {
        int index_table_number = *reinterpret_cast<boost::int32_t*>(index_record);
        if(a_table_number == index_table_number)
            {
            char* p = 54 + index_record;
            boost::int32_t z = *reinterpret_cast<boost::int32_t*>(p);
            table_offset = std::streampos(static_cast<int>(z));
            break;
            }
        index_ifs.read(index_record, index_record_length);
        if(index_record_length != index_ifs.gcount())
            {
            fatal_error()
                << "Table "
                << a_table_number
                << " in file '"
                << a_table_filename
                << "': attempted to read "
                << index_record_length
                << " bytes, but got "
                << index_ifs.gcount()
                << " bytes instead."
                << LMI_FLUSH
                ;
            }
        }

    if(table_offset == std::streampos(-1))
        {
        fatal_error()
            << "Table "
            << a_table_number
            << " in file '"
            << a_table_filename
            << "': offset "
            << table_offset
            << " is invalid."
            << LMI_FLUSH
            ;
        }

    // Data records have variable length:
    //   2-byte integer: record type
    //   2-byte integer: nominal length
    //   [type varies]:  data
    //
    // The record types of interest here are coded as:
    //   9999 end of table
    //   2    4-byte integer:  table_number
    //   3    [unsigned] char: table_type: {A, D, S} --> {age, duration, select}
    //   12   2-byte integer:  min_age
    //   13   2-byte integer:  max_age
    //   14   2-byte integer:  select_period
    //   15   2-byte integer:  max_select_age (if zero, then it's max_age)
    //   17   8-byte doubles:  values
    //
    // Record type 17, which stores values, is of special interest.
    // The number of values equals the nominal length, in the SOA
    // implementation. That means that no table can have more than
    // 4096 values, which is a draconian restriction: 100 x 100
    // tables are common enough in real-world practice.
    //
    // However, the actual number of values can always be deduced
    // correctly from context. And the context is always known when
    // the values are read, because the SOA implementation always
    // writes the values after all records that identify the context.
    // Therefore, the nominal length can be disregarded for record
    // type 17, and any desired number of values written. If the
    // actual number of values exceeds 4096, then this implementation
    // handles them correctly, but the SOA implementation does not.
    //
    // GWC's email of Wednesday, December 16, 1998 5:56 PM to the
    // author of the SOA implementation proposed a patch to overcome
    // this limitation, but it was not accepted, and the limitation
    // persists in later 32-bit versions of the software distributed
    // by the SOA even as this is written on 2005-01-13.

    fs::path data_path(a_table_filename);
    data_path = fs::change_extension(data_path, ".dat");
    fs::ifstream data_ifs(data_path, std::ios_base::in | std::ios_base::binary);

    data_ifs.seekg(table_offset, std::ios::beg);
    LMI_ASSERT(table_offset == data_ifs.tellg());

    boost::int32_t table_number   = invalid;
    unsigned char  table_type     = invalid;
    boost::int16_t min_age        = invalid;
    boost::int16_t max_age        = invalid;
    boost::int16_t select_period  = invalid;
    boost::int16_t max_select_age = invalid;

    std::vector<double> values;

// TODO ?? It would be an error to find more than one of each record, no?

    while(data_ifs)
        {
        boost::int16_t record_type = invalid;
        read(data_ifs, record_type, sizeof(boost::int16_t));

        soa_table_length_type nominal_length = invalid;
        read(data_ifs, nominal_length, sizeof(boost::int16_t));

        switch(record_type)
          {
            case 2: // 4-byte integer: table_number.
                {
                read(data_ifs, table_number, nominal_length);
                LMI_ASSERT(table_number == a_table_number);
                }
                break;
            case 3: // unsigned char: table_type.
                {
                // Meaning: {A, D, S} --> {age, duration, select}.
                // SOA apparently permits upper or lower case.
                read(data_ifs, table_type, nominal_length);
                table_type = std::toupper(table_type);
                LMI_ASSERT('A' == table_type || 'D' == table_type || 'S' == table_type);
                }
                break;
            case 12: // 2-byte integer: min_age.
                {
                read(data_ifs, min_age, nominal_length);
                LMI_ASSERT(0 <= min_age && min_age <= methuselah);
                }
                break;
            case 13: // 2-byte integer: max_age.
                {
                read(data_ifs, max_age, nominal_length);
                LMI_ASSERT(0 <= max_age && max_age <= methuselah);
                }
                break;
            case 14: // 2-byte integer: select_period.
                {
                read(data_ifs, select_period, nominal_length);
                LMI_ASSERT(0 <= select_period && select_period <= methuselah);
                }
                break;
            case 15: // 2-byte integer: max_select_age.
                {
                read(data_ifs, max_select_age, nominal_length);
                LMI_ASSERT(0 <= max_select_age && max_select_age <= methuselah);
                }
                break;
            case 17: // 8-byte doubles: values.
                {
                read_values
                    (values
                    ,data_ifs
                    ,nominal_length
                    ,min_age
                    ,max_age
                    ,table_type
                    ,max_select_age
                    ,select_period
                    );
                }
                break;
            case 9999: // End of table.
                {
                goto done;
                }
            default:
                {
                char skipped[65536];
                data_ifs.read(skipped, nominal_length);
                }
          }
        }

    done:

    LMI_ASSERT(min_age <= a_age && a_age <= max_age);
    LMI_ASSERT(a_age + a_len <= max_age + 1);
// TODO ?? Assert that there are enough values.

    return particular_values
                    (values
                    ,data_ifs
                    ,0
                    ,min_age
                    ,max_age
                    ,table_type
                    ,max_select_age
                    ,select_period
                    ,a_age
                    ,a_len
                    );
}

