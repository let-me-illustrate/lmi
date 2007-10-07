// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: actuarial_table.cpp,v 1.24 2007-10-07 21:35:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "actuarial_table.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "miscellany.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>

#include <algorithm> // std::max(), std::min()
#include <cctype>    // std::toupper()
#include <climits>   // CHAR_BIT
#include <ios>
#include <istream>
#include <limits>
#include <stdexcept>

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
} // Unnamed namespace.

actuarial_table::actuarial_table(std::string const& filename, int table_number)
    :filename_       (filename)
    ,table_number_   (table_number)
    ,table_type_     ('\0')
    ,min_age_        (-1)
    ,max_age_        (-1)
    ,select_period_  (-1)
    ,max_select_age_ (-1)
    ,table_offset_   (-1)
{
    // TODO ?? Add a unit test for this.
    if(table_number_ <= 0)
        {
        fatal_error()
            << "There is no table number "
            << table_number_
            << " in file '"
            << filename_
            << "."
            << LMI_FLUSH
            ;
        }

    find_table();
    parse_table();
}

actuarial_table::~actuarial_table()
{
}

/// Read a given number of values for a given age.

std::vector<double> actuarial_table::values(int issue_age, int length) const
{
    LMI_ASSERT(min_age_ <= issue_age && issue_age <= max_age_);
    LMI_ASSERT(issue_age + length <= max_age_ + 1);

    return specific_values(issue_age, length);
}

/// Read a given number of values for a given age, using a nondefault
/// lookup method.

std::vector<double> actuarial_table::values_elaborated
    (int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      full_years_since_issue
    ,int                      full_years_since_last_rate_reset
    ) const
{
    switch(method)
        {
        case e_reenter_at_inforce_duration:
            {
            std::vector<double> v = values
                (issue_age + full_years_since_issue
                ,length    - full_years_since_issue
                );
            v.insert(v.begin(), full_years_since_issue, 0.0);
            return v;
            }
            break;
        case e_reenter_upon_rate_reset:
            {
// Minimum age is temporarily hardcoded here. It really needs to be
// ascertained from the SOA database file, but that will require an
// extensive refactoring. This value is correct for the
//   "1956 Texas Chamberlain, Male & Female, Age next"
// table used in the unit test.
            int const minimum_age = 1; // Temporary--for testing only.
            int r = std::min
                (issue_age - minimum_age
                ,full_years_since_last_rate_reset
                );
            std::vector<double> v = values
                (issue_age - r
                ,length    + r
                );
            v.erase(v.begin(), v.begin() + r);
            return v;
            }
            break;
        case e_reenter_never: // Fall through.
        default:
            {
            fatal_error()
                << "Table-lookup method "
                << method
                << " is not valid in this context."
                << LMI_FLUSH
                ;
            throw std::logic_error("Unreachable"); // Silence compiler warning.
            }
        }
}

/// SOA documentation does not specify the domain of table numbers,
/// but their tables seem to use only positive integers representable
/// as 32-bit signed int, so take that as the range.
///
/// Index records have fixed length:
///   4-byte integer:     table number
///   50-byte char array: table name
///   4-byte integer:     byte offset into '.dat' file
/// Table numbers are not necessarily consecutive or sorted.
///
/// Asserting that the table number is nonzero makes it safe to use
/// zero as a sentry.
///
/// TODO ?? Consider requiring an extra argument--a string that
/// gives context, e.g., "Current COI rates"--so that diagnostics
/// can be made more helpful.

void actuarial_table::find_table()
{
    LMI_ASSERT(0 != table_number_);

    fs::path index_path(filename_);
    index_path = fs::change_extension(index_path, ".ndx");
    fs::ifstream index_ifs(index_path, ios_in_binary());

    // TODO ?? Assert endianness too? SOA tables are not portable;
    // probably they can easily be read only on x86 hardware.

    BOOST_STATIC_ASSERT(8 == CHAR_BIT);
    BOOST_STATIC_ASSERT(4 == sizeof(int));
    BOOST_STATIC_ASSERT(2 == sizeof(short int));

    // 27.4.3.2/2 requires that this be interpreted as invalid. The
    // variable 'invalid' is not used here, because its value might
    // change someday, but only '-1' is mentioned in the standard.
    // Reinitialize it here for robustness, even though the ctor
    // already initializes it in the same way.
    table_offset_ = std::streampos(-1);

    int const index_record_length(58);
    char index_record[index_record_length] = {0};

    BOOST_STATIC_ASSERT(sizeof(boost::int32_t) <= sizeof(int));
    while(index_ifs)
        {
        int index_table_number = *reinterpret_cast<boost::int32_t*>(index_record);
        if(table_number_ == index_table_number)
            {
            char* p = 54 + index_record;
            boost::int32_t z = *reinterpret_cast<boost::int32_t*>(p);
            table_offset_ = std::streampos(static_cast<int>(z));
            break;
            }
        index_ifs.read(index_record, index_record_length);
        if(index_record_length != index_ifs.gcount())
            {
            fatal_error()
                << "Table "
                << table_number_
                << " in file '"
                << filename_
                << "': attempted to read "
                << index_record_length
                << " bytes, but got "
                << index_ifs.gcount()
                << " bytes instead."
                << LMI_FLUSH
                ;
            }
        }

    if(table_offset_ == std::streampos(-1))
        {
        fatal_error()
            << "Table "
            << table_number_
            << " in file '"
            << filename_
            << "': offset "
            << table_offset_
            << " is invalid."
            << LMI_FLUSH
            ;
        }
}

/// Data records have variable length:
///   2-byte integer: record type
///   2-byte integer: nominal length
///   [type varies]:  data
///
/// The record types of interest here are coded as:
///   9999 end of table
///   2    4-byte integer:  Table number
///   3    [unsigned] char: Table type: {A, D, S} --> {age, duration, select}
///   12   2-byte integer:  Minimum age
///   13   2-byte integer:  Maximum age
///   14   2-byte integer:  Select period
///   15   2-byte integer:  Maximum select age (if zero, then it's max age)
///   17   8-byte doubles:  Table values
///
/// Record type 17, which stores values, is of special interest.
/// The number of values equals the nominal length, in the SOA
/// implementation. That means that no table can have more than
/// 4096 values, which is a draconian restriction: 100 x 100
/// tables are common enough in real-world practice.
///
/// However, the actual number of values can always be deduced
/// correctly from context. And the context is always known when
/// the values are read, because the SOA implementation always
/// writes the values after all records that identify the context.
/// Therefore, the nominal length can be disregarded for record
/// type 17, and any desired number of values written. If the
/// actual number of values exceeds 4096, then this implementation
/// handles them correctly, but the SOA implementation does not.
///
/// GWC's email of Wednesday, December 16, 1998 5:56 PM to the
/// author of the SOA implementation proposed a patch to overcome
/// this limitation, but it was not accepted, and the limitation
/// persists in later 32-bit versions of the software distributed
/// by the SOA even as this is written on 2005-01-13.

void actuarial_table::parse_table()
{
    fs::path data_path(filename_);
    data_path = fs::change_extension(data_path, ".dat");
    fs::ifstream data_ifs(data_path, ios_in_binary());

    data_ifs.seekg(table_offset_, std::ios::beg);
    LMI_ASSERT(table_offset_ == data_ifs.tellg());

// TODO ?? It would be an error to find more than one of each record, no?
// And what if we fail to find one of each required type?

    while(data_ifs)
        {
        boost::int16_t record_type = invalid;
        read(data_ifs, record_type, sizeof(boost::int16_t));

        soa_table_length_type nominal_length = invalid;
        read(data_ifs, nominal_length, sizeof(boost::int16_t));

        switch(record_type)
            {
            case 2: // 4-byte integer: Table number.
                {
                boost::int32_t z = invalid;
                read(data_ifs, z, nominal_length);
                LMI_ASSERT(z == table_number_);
                }
                break;
            case 3: // [unsigned] char: Table type.
                {
                // Meaning: {A, D, S} --> {age, duration, select}.
                // SOA apparently permits upper or lower case.
                unsigned char z = '\0';
                read(data_ifs, z, nominal_length);
                z = std::toupper(z);
                LMI_ASSERT('A' == z || 'D' == z || 'S' == z);
                table_type_ = z;
                }
                break;
            case 12: // 2-byte integer: Minimum age.
                {
                boost::int16_t z = invalid;
                read(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                min_age_ = z;
                }
                break;
            case 13: // 2-byte integer: Maximum age.
                {
                boost::int16_t z = invalid;
                read(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                max_age_ = z;
                }
                break;
            case 14: // 2-byte integer: Select period.
                {
                boost::int16_t z = invalid;
                read(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                select_period_ = z;
                }
                break;
            case 15: // 2-byte integer: Maximum select age.
                {
                boost::int16_t z = invalid;
                read(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                max_select_age_ = z;
                }
                break;
            case 17: // 8-byte doubles: Table values.
                {
                read_values(data_ifs, nominal_length);
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
    ;
// TODO ?? Postconditions?
}

void actuarial_table::read_values(std::istream& is, int nominal_length)
{
    if('S' != table_type_)
        {
        // One might suppose that the select period for tables
        // that are not select couldn't be nonzero, but the SOA
        // publishes tables that don't honor that invariant.
        //   LMI_ASSERT(0 == select_period_); // Could fail.
        select_period_ = 0;
        }

    LMI_ASSERT(min_age_ <= max_age_);

    // If max_select_age_ is given as zero, then it's to be taken as
    // unlimited, so its value should be max_age_.
    if(0 == max_select_age_)
        {
        max_select_age_ = max_age_;
        }

    int number_of_values = 1 + max_age_ - min_age_;
    if(select_period_)
        {
        number_of_values =
              (1 + max_select_age_ - min_age_) * select_period_
          +   1 + max_age_ - min_age_ - select_period_
          ;
        }
    int deduced_length = number_of_values * sizeof(double);
    LMI_ASSERT
        (   soa_table_length_max < deduced_length
        ||  nominal_length == deduced_length
        );
    data_.resize(number_of_values);
    char z[sizeof(double)];
    for(int j = 0; j < number_of_values; ++j)
        {
        is.read(z, sizeof(double));
        data_[j] = *reinterpret_cast<double*>(z);
        }
}

std::vector<double> actuarial_table::specific_values
    (int issue_age
    ,int length
    ) const
{
    std::vector<double> v;
    switch(table_type_)
        {
        case 'A':
            {
            // Parenthesize the offsets--addition in C and C++ is
            // in effect left associative:
            //   data_.begin() + issue_age - min_age_
            // means
            //   (data_.begin() + issue_age) - min_age_
            // but the subexpression
            //   (data_.begin() + issue_age)
            // is likely to return a past-the-end iterator, which
            // libstdc++'s debug mode will dislike.
            //
            v = std::vector<double>
                (data_.begin() + (issue_age - min_age_)
                ,data_.begin() + (issue_age - min_age_ + length)
                );
            }
            break;
        case 'D':
            {
            // TODO ?? Interpret duration as index 0; what does SOA software do?
            v = std::vector<double>
                (data_.begin()
                ,data_.begin() + length
                );
            }
            break;
        case 'S':
            {
            int k =
                    std::max(0, issue_age - max_select_age_)
                +   std::min(max_select_age_, issue_age - min_age_) * (1 + select_period_)
                ;
            v.resize(length);
            for(int j = 0; j < length; ++j, ++k)
                {
                v[j] = data_[k];
                if(j + issue_age < max_select_age_ + select_period_ && select_period_ <= j)
                    {
                    k += select_period_;
                    }
                }
            }
            break;
        default:
            {
            fatal_error()
                << "Table type '"
                << table_type_
                << "' not recognized: must be one of 'A', 'D', or 'S'."
                << LMI_FLUSH
                ;
            }
        }
    LMI_ASSERT(v.size() == static_cast<unsigned int>(length));
    return v;
}

std::vector<double> actuarial_table_rates
    (std::string const& table_filename
    ,int                table_number
    ,int                issue_age
    ,int                length
    )
{
    actuarial_table z(table_filename, table_number);
    return z.values(issue_age, length);
}

std::vector<double> actuarial_table_rates_elaborated
    (std::string const&       table_filename
    ,int                      table_number
    ,int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      full_years_since_issue
    ,int                      full_years_since_last_rate_reset
    )
{
    actuarial_table z(table_filename, table_number);
    return z.values_elaborated
        (issue_age
        ,length
        ,method
        ,full_years_since_issue
        ,full_years_since_last_rate_reset
        );
}

