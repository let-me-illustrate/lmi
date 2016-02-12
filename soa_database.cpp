// Tools for working with SOA tables represented in binary format.
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

#include "crc32.hpp"
#include "path_utility.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>

#include <cmath>
#include <cstdint>
#include <cstdlib>      // for strtoull()
#include <fstream>
#include <limits>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>      // make_pair()
#include <vector>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

// The SOA binary format uses IEEE 754 for the floating point values
// representation and the code in this file won't work correctly if it is
// different from their in memory representation.
BOOST_STATIC_ASSERT(std::numeric_limits<double>::is_iec559);

// Helper functions used to swap bytes on big endian platforms.
//
// BOOST !! Replace these functions with Boost.Endian library once a version
// of Boost new enough to have it is used by lmi.
namespace
{

// Defining this NOP function allows to call swap_bytes_if_big_endian() in template
// code for numbers of any size.
inline
uint8_t swap_bytes_if_big_endian(uint8_t val)
{
    return val;
}

// We rely on makefile defining WORDS_BIGENDIAN on big endian architectures,
// conversions from little endian format are only needed there and are trivial
// on little endian machines.
#ifdef WORDS_BIGENDIAN
inline
uint16_t swap_bytes_if_big_endian(uint16_t val)
{
    return ((val & 0x00ffU) << 8)
         | ((val & 0xff00U) >> 8)
         ;
}

inline
uint32_t swap_bytes_if_big_endian(uint32_t val)
{
    return ((val & 0x000000ffU) << 24)
         | ((val & 0x0000ff00U) <<  8)
         | ((val & 0x00ff0000U) >>  8)
         | ((val & 0xff000000U) >> 24)
         ;
}

inline
uint64_t swap_bytes_if_big_endian(uint64_t val)
{
    return ((val & 0x00000000000000ffULL) << 56)
         | ((val & 0x000000000000ff00ULL) << 40)
         | ((val & 0x0000000000ff0000ULL) << 24)
         | ((val & 0x00000000ff000000ULL) <<  8)
         | ((val & 0x000000ff00000000ULL) >>  8)
         | ((val & 0x0000ff0000000000ULL) >> 24)
         | ((val & 0x00ff000000000000ULL) >> 40)
         | ((val & 0xff00000000000000ULL) >> 56)
         ;
}

inline
double swap_bytes_if_big_endian(double val)
{
    // When using IEEE 754 (as checked in the beginning of this file), we can
    // treat a double value as a 64 bit integer.
    uint64_t const ui64 = swap_bytes_if_big_endian(*reinterpret_cast<uint64_t*>(&val));

    // And vice versa.
    return *reinterpret_cast<double*>(&ui64);
}
#else // !WORDS_BIGENDIAN
inline
uint16_t swap_bytes_if_big_endian(uint16_t val)
{
    return val;
}

inline
uint32_t swap_bytes_if_big_endian(uint32_t val)
{
    return val;
}

inline
double swap_bytes_if_big_endian(double val)
{
    return val;
}
#endif // WORDS_BIGENDIAN/!WORDS_BIGENDIAN

template<typename T>
inline
T from_bytes(char const* bytes)
{
    return swap_bytes_if_big_endian(*reinterpret_cast<T const*>(bytes));
}

template<typename T>
inline
void to_bytes(char* bytes, T value)
{
    *reinterpret_cast<T*>(bytes) = swap_bytes_if_big_endian(value);
}

// BOOST !! Replace the use of this function with member value_or() present in
// the later Boost.Optional versions.
template<typename T, typename U>
inline
T get_value_or(boost::optional<T> const& o, U v)
{
    return o ? *o : v;
}

template<typename T>
struct open_file_traits;

template<>
struct open_file_traits<fs::ifstream>
{
    static std::ios_base::openmode get_mode() { return std::ios_base::in; }
    static char const* describe_access() { return "reading"; }
};

template<>
struct open_file_traits<fs::ofstream>
{
    static std::ios_base::openmode get_mode() { return std::ios_base::out; }
    static char const* describe_access() { return "writing"; }
};

// Helper function opening the stream for reading or writing the given file and
// throwing an exception on error. It shouldn't be used directly, prefer to use
// the more readable open_{text,binary}_file() helpers below.
template<typename T>
void open_file(T& ifs, fs::path const& path, std::ios_base::openmode mode)
{
    ifs.open(path, open_file_traits<T>::get_mode() | mode);
    if(!ifs)
        {
        std::ostringstream oss;
        oss << "File '" << path << "' could not be opened for "
            << open_file_traits<T>::describe_access() << ".";
        throw std::runtime_error(oss.str());
        }
}

template<typename T>
inline void open_text_file(T& fs, fs::path const& path)
{
    open_file(fs, path, static_cast<std::ios_base::openmode>(0));
}

template<typename T>
inline void open_binary_file(T& fs, fs::path const& path)
{
    open_file(fs, path, std::ios_base::binary);
}

// Functions doing the same thing as istream::read() and ostream::write()
// respectively, but taking void pointers and this allowing to avoid ugly casts
// to char in the calling code.
inline bool stream_write(std::ostream& os, void const* data, std::size_t length)
{
    os.write(static_cast<char const*>(data), length);
    return !!os;
}

inline bool stream_read(std::istream& is, void* data, std::size_t length)
{
    is.read(static_cast<char*>(data), length);
    return is.gcount() == static_cast<std::streamsize>(length);
}

// Helper function wrapping std::strtoull() and hiding its peculiarities:
//
//  - It uses base 10 and doesn't handle leading "0x" as hexadecimal nor,
//    especially perniciously, leading "0"s as octal.
//  - It checks for all possible errors: failure to parse anything at all or
//    overflow.
//  - It doesn't skip leading whitespace.
//  - It does not accept negative numbers.
//  - And it doesn't accept plus sign neither, just for consistency.
//
// Returns the parse_result struct containing the pointer to the place where
// parsing of the number stopped or nullptr on failure. On success, the num
// field of the struct is filled with the parsed value.
struct parse_result
{
    unsigned long long num = 0;
    const char* end = nullptr;
};

parse_result strict_parse_number(char const* start)
{
    parse_result res;

    // This check catches whitespace and the leading minus sign.
    if(*start >= '0' && *start <= '9')
        {
        char* end = nullptr;
        res.num = std::strtoull(start, &end, 10);

        if(end != start && res.num != ULLONG_MAX)
            {
            res.end = end;
            }
        }

    return res;
}

// Description of all the SOA fields for both formats.
struct soa_field
{
    boost::uint16_t record_type;    // Field record type in the binary format.
    char const* name;               // Field name in the text format.
};

// This enum defines the indices of all the known fields in soa_fields array,
// its elements are consecutive.
enum enum_soa_field
    {e_field_table_name
    ,e_field_table_number
    ,e_field_table_type
    ,e_field_contributor
    ,e_field_data_source
    ,e_field_data_volume
    ,e_field_obs_period
    ,e_field_unit_of_obs
    ,e_field_construction_method
    ,e_field_published_reference
    ,e_field_comments
    ,e_field_min_age
    ,e_field_max_age
    ,e_field_select_period
    ,e_field_max_select_age
    ,e_field_num_decimals
    ,e_field_values
    ,e_field_hash_value
    };

// This enum defines the field record types used in the binary SOA format and
// exists mostly to allow writing a switch on the record type in a readable way.
enum
    {e_record_table_name          =  1
    ,e_record_table_number        =  2
    ,e_record_table_type          =  3
    ,e_record_contributor         =  4
    ,e_record_data_source         =  5
    ,e_record_data_volume         =  6
    ,e_record_obs_period          =  7
    ,e_record_unit_of_obs         =  8
    ,e_record_construction_method =  9
    ,e_record_published_reference = 10
    ,e_record_comments            = 11
    ,e_record_min_age             = 12
    ,e_record_max_age             = 13
    ,e_record_select_period       = 14
    ,e_record_max_select_age      = 15
    ,e_record_num_decimals        = 16
    ,e_record_values              = 17
    ,e_record_hash_value          = 18
    ,e_record_end_table           = 9999
    };

// This array is indexed by enum_soa_field.
static soa_field const soa_fields[] =
{
    { e_record_table_name         , "Table name"               }
   ,{ e_record_table_number       , "Table number"             }
   ,{ e_record_table_type         , "Table type"               }
   ,{ e_record_contributor        , "Contributor"              }
   ,{ e_record_data_source        , "Source of data"           }
   ,{ e_record_data_volume        , "Volume of data"           }
   ,{ e_record_obs_period         , "Observation period"       }
   ,{ e_record_unit_of_obs        , "Unit of observation"      }
   ,{ e_record_construction_method, "Construction method"      }
   ,{ e_record_published_reference, "Published reference"      }
   ,{ e_record_comments           , "Comments"                 }
   ,{ e_record_min_age            , "Minimum age"              }
   ,{ e_record_max_age            , "Maximum age"              }
   ,{ e_record_select_period      , "Select period"            }
   ,{ e_record_max_select_age     , "Maximum select age"       }
   ,{ e_record_num_decimals       , "Number of decimal places" }
   ,{ e_record_values             , "Table values"             }
   ,{ e_record_hash_value         , "Hash value"               }
};

enum class table_type : uint8_t
{
    aggregate = 'A',
    duration  = 'D',
    select    = 'S',
};

char const* table_type_as_string(table_type tt)
{
    switch(tt)
        {
        case table_type::aggregate: return "Aggregate";
        case table_type::duration:  return "Duration" ;
        case table_type::select:    return "Select"   ;
        }

    return nullptr;
}

} // anonymous namespace

// Classes abstracting the difference between text and binary formats: both
// namespaces define the same classes inside them, but the first one works with
// the data in binary format while the second one uses the text format.
namespace binary_format
{

class writer
{
  public:
    explicit writer(std::ostream& os) : os_(os) {}

    template<typename T>
    void write(enum_soa_field field, boost::optional<T> const& onum);
    void write_table_type(table_type tt);
    void write(enum_soa_field field, boost::optional<std::string> const& ostr);

    void write_values
            (std::vector<double> const& values
            ,boost::optional<uint16_t> const& num_decimals
            ,boost::optional<uint16_t> const& min_age
            ,boost::optional<uint16_t> const& max_age
            ,boost::optional<uint16_t> const& select_period
            ,boost::optional<uint16_t> const& max_select_age
            );

    void end();

  private:
    void do_write_record_header(uint16_t record_type, uint16_t length);
    template<typename T>
    void do_write_field(enum_soa_field field, T num);

    std::ostream& os_;
};

void writer::write_values
        (std::vector<double> const& values
        ,boost::optional<uint16_t> const& num_decimals
        ,boost::optional<uint16_t> const& min_age
        ,boost::optional<uint16_t> const& max_age
        ,boost::optional<uint16_t> const& select_period
        ,boost::optional<uint16_t> const& max_select_age
        )
{
    // Notice that to keep things more interesting, number of decimals comes
    // before the other parameters in binary format, but after them in the text
    // one.
    write(e_field_num_decimals       , num_decimals        );
    write(e_field_min_age            , min_age             );
    write(e_field_max_age            , max_age             );
    write(e_field_select_period      , select_period       );
    write(e_field_max_select_age     , max_select_age      );

#ifdef WORDS_BIGENDIAN
    // Convert the values to their on disk representation.
    std::vector<double> little_endian_values;
    little_endian_values.reserve(values.size());

    for(auto v: values)
        {
        little_endian_values.push_back(swap_bytes_if_big_endian(v));
        }
#else // !WORDS_BIGENDIAN
    // No conversion necessary, don't create an extra vector needlessly, just
    // alias the existing one.
    std::vector<double> const& little_endian_values = values;
#endif // WORDS_BIGENDIAN/!WORDS_BIGENDIAN

    std::size_t const length = values.size()*sizeof(double);

    // As explained in table_impl::read_values(), length field is too small to
    // store the length of this record in general, but we still write the data
    // even in this case as this code, at least, can still read it. We set the
    // length to the maximal representable value instead of some random junk
    // that would result if we simply truncated it to 16 bits however.
    do_write_record_header
        (e_record_values
        ,length > std::numeric_limits<uint16_t>::max()
            ? std::numeric_limits<uint16_t>::max()
            : static_cast<uint16_t>(length)
        );

    // Normally we don't check the stream state after each write as it is
    // enough to check it once at the end, however this write, being much
    // bigger than others, has probably bigger chance of failing, so do check
    // for its success, exceptionally, in order to detect the error a.s.a.p.
    if(!stream_write(os_, &little_endian_values[0], length))
        {
        throw std::runtime_error("writing values failed");
        }
}

void writer::do_write_record_header(uint16_t record_type, uint16_t length)
{
    enum
        {e_header_pos_type = 0
        ,e_header_pos_len  = 2
        ,e_header_pos_max  = 4
        };

    char header[e_header_pos_max];

    *reinterpret_cast<uint16_t*>(header + e_header_pos_type)
        = swap_bytes_if_big_endian(record_type);
    *reinterpret_cast<uint16_t*>(header + e_header_pos_len)
        = swap_bytes_if_big_endian(length);

    stream_write(os_, header, sizeof(header));
}

template<typename T>
void writer::do_write_field(enum_soa_field field, T num)
{
    num = swap_bytes_if_big_endian(num);
    do_write_record_header(soa_fields[field].record_type, sizeof(num));
    stream_write(os_, &num, sizeof(num));
}

template<typename T>
void writer::write(enum_soa_field field, boost::optional<T> const& onum)
{
    if(onum)
        {
        do_write_field(field, *onum);
        }
}

void writer::write_table_type(table_type tt)
{
    do_write_field(e_field_table_type, static_cast<uint8_t>(tt));
}

void writer::write(enum_soa_field field, boost::optional<std::string> const& ostr)
{
    if(ostr)
        {
        std::string::size_type const length = ostr->size();
        if(length > std::numeric_limits<uint16_t>::max())
            {
            std::ostringstream oss;
            oss << "the value of the field '"
                << soa_fields[field].name
                << "' is too long to be represented in the SOA binary format";
            throw std::runtime_error(oss.str());
            }

        do_write_record_header(soa_fields[field].record_type, length);
        stream_write(os_, ostr->c_str(), length);
        }
}

void writer::end()
{
    uint16_t record_type = e_record_end_table;
    record_type = swap_bytes_if_big_endian(record_type);
    stream_write(os_, &record_type, sizeof(record_type));
}

} // namespace binary_format

namespace text_format
{

class writer
{
  public:
    explicit writer(std::ostream& os) : os_(os) {}

    template<typename T>
    void write(enum_soa_field field, boost::optional<T> const& oval);
    void write_table_type(table_type tt);
    void write_values
            (std::vector<double> const& values
            ,boost::optional<uint16_t> const& num_decimals
            ,boost::optional<uint16_t> const& min_age
            ,boost::optional<uint16_t> const& max_age
            ,boost::optional<uint16_t> const& select_period
            ,boost::optional<uint16_t> const& max_select_age
            );

    void end();

  private:
    std::ostream& os_;
};

template<typename T>
void writer::write(enum_soa_field field, boost::optional<T> const& oval)
{
    if(oval)
        {
        os_ << soa_fields[field].name << ": " << *oval << "\n";
        }
}

void writer::write_table_type(table_type tt)
{
    os_ << soa_fields[e_field_table_type].name << ": "
        << table_type_as_string(tt) << "\n"
        ;
}

void writer::write_values
        (std::vector<double> const& values
        ,boost::optional<uint16_t> const& num_decimals
        ,boost::optional<uint16_t> const& min_age
        ,boost::optional<uint16_t> const& max_age
        ,boost::optional<uint16_t> const& select_period
        ,boost::optional<uint16_t> const& max_select_age
        )
{
    write(e_field_min_age            , min_age             );
    write(e_field_max_age            , max_age             );
    write(e_field_select_period      , select_period       );
    write(e_field_max_select_age     , max_select_age      );
    write(e_field_num_decimals       , num_decimals        );

    os_ << soa_fields[e_field_values].name << ":\n";
    if(get_value_or(select_period, 0))
        {
        throw std::runtime_error("NIY");
        }
    else
        {
        os_ << std::fixed << std::setprecision(*num_decimals);

        unsigned const value_width = *num_decimals + 2;

        uint16_t age = *min_age;
        for(auto v: values)
            {
            os_ << std::setw(3) << age++ << "  "
                << std::setw(value_width) << v << "\n";
            }
        }
}

void writer::end()
{
    // There is no table end marker in the text format, it is indicated simply
    // by the end of the file itself.
}

// Return the field corresponding to the given name or throw an exception if
// none was found (the line number appears in the error message).
enum_soa_field parse_field_name(std::string const& name, int line_num)
{
    int n = 0;
    for(soa_field const& f: soa_fields)
        {
        if(name == f.name)
            {
            // Cast is safe because the valid enum values exactly correspond to
            // the entries of the fields table we iterate over.
            return static_cast<enum_soa_field>(n);
            }

        ++n;
        }

    std::ostringstream oss;
    oss << "Unrecognized field '" << name << "' at line number " << line_num;
    throw std::runtime_error(oss.str());
}

} // namespace text_format

namespace soa_v3_format
{

class table_impl
    :        private lmi::uncopyable <table_impl>
    ,virtual private obstruct_slicing<table_impl>
{
  public:
    // Load the table data from the stream containing the table data in either
    // binary or text representation.
    //
    // Throws std::runtime_error on error.
    static shared_ptr<table_impl> create_from_binary
        (std::istream& is
        ,uint32_t offset
        );
    static shared_ptr<table_impl> create_from_text(std::istream& is);

    void write_as_binary(std::ostream& os) const { do_write<binary_format::writer>(os); }
    void write_as_text(std::ostream& os) const { do_write<text_format::writer>(os); }

    bool is_equal(table_impl const& other) const;

    // Public class method implementations.
    void name(std::string const& name) { name_ = name; }
    uint32_t number() const { return *number_; }
    std::string const& name() const { return *name_; }
    char const* type_as_string() const { return table_type_as_string(*type_); }
    unsigned long compute_hash_value() const;

  private:
    // Helper methods for IO: all of them throw std::runtime_error on failure
    // and mention the field name in the error message.
    //
    // They also check that the optional value provided as the output parameter
    // for reading data into is not initialized yet as it's an error to have
    // duplicate fields in our format.

    // read_xxx() methods for binary format.

    static
    void read_string
            (boost::optional<std::string>& ostr
            ,enum_soa_field field
            ,std::istream& ifs
            ,uint16_t length
            );

    template<typename T>
    static
    T do_read_number(char const* name, std::istream& ifs);

    void read_type(std::istream& ids, uint16_t length);

    template<typename T>
    static
    void read_number
            (boost::optional<T>& onum
            ,enum_soa_field field
            ,std::istream& ifs
            ,uint16_t length
            );

    // Similar to read_number() but also checks that values hadn't been
    // specified yet, this is useful for fields which are used to deduce the
    // number of the values to read (but not min_age_ and max_age_ as we check
    // that they had been given when reading values, so if they occurred again
    // after reading them, this would already result in a "duplicate field"
    // error).
    void read_number_before_values
            (boost::optional<uint16_t>& onum
            ,enum_soa_field field
            ,std::istream& ifs
            ,uint16_t length
            );

    // This one is different from the generic methods above as it's only used
    // for the specific values_ field and not any arbitrary vector.
    void read_values(std::istream& ifs, uint16_t length);

    // parse_xxx() methods for text format.

    // This method returns the pointer to ostr string value to allow further
    // modifying it later in the caller.
    static
    std::string* parse_string
            (boost::optional<std::string>& ostr
            ,enum_soa_field field
            ,int line_num
            ,std::string const& value
            );

    // Parse number checking that it is less than the given maximal value.
    static
    unsigned long do_parse_number
            (enum_soa_field field
            ,int line_num
            ,unsigned long max_num
            ,std::string const& value
            );

    template<typename T>
    static
    void parse_number
            (boost::optional<T>& onum
            ,enum_soa_field field
            ,int line_num
            ,std::string const& value
            );

    void parse_table_type
            (int line_num
            ,std::string const& value
            );

    // Unlike the other functions, this one reads from the input on its own
    // (which is also why it takes line number by reference, as it modifies it).
    void parse_values(std::istream& is, int& line_num);

    // Compute the expected number of values from minimum and maximum age
    // values and the select period and max select age if specified.
    //
    // Throws if minimum or maximum ares are not defined or are invalid.
    unsigned get_expected_number_of_values() const;

    // Implementations of the public factory functions.
    void read_from_binary(std::istream& is, uint32_t offset);
    void read_from_text(std::istream& is);

    // Validate all the fields, throw an exception if any are invalid.
    void validate();

    // Write out the table contents in the given format.
    template<typename T>
    void do_write(std::ostream& os) const;

    // The values are not represented by boost::optional<>, the emptiness of
    // the vector signals if we have any values or not.
    std::vector<double> values_;

    boost::optional<std::string>
        name_,
        contributor_,
        data_source_,
        data_volume_,
        obs_period_,
        unit_of_obs_,
        construction_method_,
        published_reference_,
        comments_;

    boost::optional<uint32_t>
        number_,
        hash_value_;

    boost::optional<uint16_t>
        num_decimals_,
        min_age_,
        max_age_,
        select_period_,
        max_select_age_;

    boost::optional<table_type>
        type_;
};

namespace
{

// Throw an error indicating duplicate occurrence of some field if the first
// argument is true.
//
// If the line number if specified, it is appended to the error message (it
// should be specified when reading text files, but not when reading from
// binary files).
inline
void throw_if_duplicate_record
    (bool do_throw
    ,enum_soa_field field
    ,int line_num = 0
    )
{
    if(do_throw)
        {
        std::ostringstream oss;
        oss << "duplicate occurrence of the field '"
            << soa_fields[field].name
            << "'"
            ;
        if(line_num)
            {
            oss << " at line " << line_num;
            }
        throw std::runtime_error(oss.str());
        }
}

// Throw an error if the length of a field doesn't have the expected value.
void throw_if_unexpected_length
    (uint16_t length
    ,std::size_t expected_length
    ,enum_soa_field field
    )
{
    if(length != expected_length)
        {
        std::ostringstream oss;
        oss << "unexpected length " << length
            << " for the field '"
            << soa_fields[field].name
            << "', expected " << expected_length
            ;
        throw std::runtime_error(oss.str());
        }
}

// Throw an error indicating that some field is missing if the first argument
// is true.
template<typename T>
inline
void throw_if_missing_field(boost::optional<T> const& o, enum_soa_field field)
{
    if(!o)
        {
        std::ostringstream oss;
        oss << "required field '"
            << soa_fields[field].name
            << "' was not specified";
        throw std::runtime_error(oss.str());
        }
}

} // anonymous namespace

void table_impl::read_string
        (boost::optional<std::string>& ostr
        ,enum_soa_field field
        ,std::istream& ifs
        ,uint16_t length
        )
{
    throw_if_duplicate_record(ostr.is_initialized(), field);

    std::string str;
    str.resize(length);
    if(!stream_read(ifs, &str[0], length))
        {
        std::ostringstream oss;
        oss << "failed to read all " << length << " bytes of the field '"
            << soa_fields[field].name
            << "'"
            ;
        throw std::runtime_error(oss.str());
        }

    ostr = str;
}

template<typename T>
T table_impl::do_read_number(char const* name, std::istream& ifs)
{
    T num;
    if(!stream_read(ifs, &num, sizeof(T)))
        {
        std::ostringstream oss;
        oss << "failed to read field '" << name << "'";
        throw std::runtime_error(oss.str());
        }

    return swap_bytes_if_big_endian(num);
}

void table_impl::read_type(std::istream& ifs, uint16_t length)
{
    throw_if_duplicate_record(type_.is_initialized(), e_field_table_type);

    throw_if_unexpected_length(length, sizeof(uint8_t), e_field_table_type);

    auto const type
        = do_read_number<uint8_t>(soa_fields[e_field_table_type].name, ifs);
    switch(type)
        {
        case table_type::aggregate:
        case table_type::duration:
        case table_type::select:
            type_ = static_cast<table_type>(type);
            break;

        default:
            std::ostringstream oss;
            oss << "unknown table type '" << type << "'";
            throw std::runtime_error(oss.str());
        }
}

template<typename T>
void table_impl::read_number
        (boost::optional<T>& onum
        ,enum_soa_field field
        ,std::istream& ifs
        ,uint16_t length
        )
{
    throw_if_duplicate_record(onum.is_initialized(), field);

    throw_if_unexpected_length(length, sizeof(T), field);

    onum = do_read_number<T>(soa_fields[field].name, ifs);
}

void table_impl::read_number_before_values
        (boost::optional<uint16_t>& onum
        ,enum_soa_field field
        ,std::istream& ifs
        ,uint16_t length
        )
{
    if(!values_.empty())
        {
        std::ostringstream oss;
        oss << "field '"
            << soa_fields[field].name
            << "' must occur before the values"
            ;
        throw std::runtime_error(oss.str());
        }

    read_number(onum, field, ifs, length);
}

unsigned table_impl::get_expected_number_of_values() const
{
    throw_if_missing_field(min_age_, e_field_min_age);
    throw_if_missing_field(max_age_, e_field_max_age);

    // Compute the expected number of values, checking the consistency of the
    // fields determining this as a side effect.
    if(*min_age_ > *max_age_)
        {
        std::ostringstream oss;
        oss << "minimum age " << *min_age_ << " cannot be greater than the "
               "maximum age " << *max_age_;
        throw std::runtime_error(oss.str());
        }

    // Start from one value per issue age, this is already the total number of
    // values for 1D tables.
    //
    // Considering that max age is a 16 bit number and int, used for
    // computations, is at least 32 bits, there is no possibility of integer
    // overflow here.
    unsigned num_values = *max_age_ - *min_age_ + 1;

    // We are liberal in what we accept and use the default values for the
    // selection period and max select age because we don't need them, strictly
    // speaking, even if normally they ought to be specified (possibly as
    // zeroes) in the input, so don't complain if select period is not given.
    if(select_period_ && *select_period_)
        {
        // Select period can't be greater than the age range and would result
        // in integer overflow below if it were allowed.
        if(num_values < *select_period_)
            {
            std::ostringstream oss;
            oss << "select period " << *select_period_ << " is too big "
                   "for the age range " << *min_age_ << ".." << *max_age_;
            throw std::runtime_error(oss.str());
            }

        // For 2D select-and-ultimate tables, this gives the number of values
        // in the ultimate column.
        num_values -= *select_period_;

        // In a further application of Postel's law, we consider non-specified
        // or 0 maximum select age as meaning "unlimited".
        unsigned effective_max_select = get_value_or(max_select_age_, 0);
        if(effective_max_select == 0)
            {
            effective_max_select = *max_age_;
            }

        unsigned select_range = effective_max_select - *min_age_ + 1;

        // Maximum possible select_range value is 2^16 and multiplying it by
        // also 16 bit select_period_ still fits in a 32 bit unsigned value, so
        // there is no risk of overflow here neither.
        select_range *= *select_period_;

        if(select_range > std::numeric_limits<unsigned>::max() - num_values)
            {
            std::ostringstream oss;
            oss << "too many values in the table with maximum age " << *max_age_
                << ", select period " << *select_period_
                << " and maximum select age " << effective_max_select;
            throw std::runtime_error(oss.str());
            }

        // No overflow due to the check above.
        num_values += select_range;
        }

    return num_values;
}

void table_impl::read_values(std::istream& ifs, uint16_t /* length */)
{
    throw_if_duplicate_record(!values_.empty(), e_field_values);

    // 2 byte length field can only represent values up to 2^16, i.e. only up
    // to 2^16/2^3 == 8192 double-sized elements, which is not enough for the
    // tables occurring in real-world. Because of this we don't trust the
    // length field from the file at all but deduce the number of values from
    // the previously specified age-related fields instead.
    unsigned const num_values = get_expected_number_of_values();

    values_.resize(num_values);
    if(!stream_read(ifs, &values_[0], num_values*sizeof(double)))
        {
        throw std::runtime_error("failed to read the values");
        }

    for(auto& v: values_)
        {
        v = swap_bytes_if_big_endian(v);
        }
}

std::string* table_impl::parse_string
        (boost::optional<std::string>& ostr
        ,enum_soa_field field
        ,int line_num
        ,std::string const& value
        )
{
    throw_if_duplicate_record(ostr.is_initialized(), field, line_num);

    if(value.empty())
        {
        std::ostringstream oss;
        oss << "non-empty value must be specified for the field '"
            << soa_fields[field].name
            << "' at line"
            << line_num
            ;
        throw std::runtime_error(oss.str());
        }

    ostr = value;

    return &ostr.get();
}

unsigned long table_impl::do_parse_number
        (enum_soa_field field
        ,int line_num
        ,unsigned long max_num
        ,std::string const& value
        )
{
    auto const res = strict_parse_number(value.c_str());
    if(!res.end || *res.end != '\0')
        {
        std::ostringstream oss;
        oss << "value for numeric field '"
            << soa_fields[field].name
            << "' is not a number at line "
            << line_num
            ;
        throw std::runtime_error(oss.str());
        }

    if(res.num > max_num)
        {
        std::ostringstream oss;
        oss << "value for numeric field '"
            << soa_fields[field].name
            << "' is out of range (maximum allowed is "
            << max_num
            << ") at line "
            << line_num
            ;
        throw std::runtime_error(oss.str());
        }

    return static_cast<unsigned long>(res.num);
}

template<typename T>
void table_impl::parse_number
        (boost::optional<T>& onum
        ,enum_soa_field field
        ,int line_num
        ,std::string const& value
        )
{
    throw_if_duplicate_record(onum.is_initialized(), field, line_num);

    onum = do_parse_number(field, line_num, std::numeric_limits<T>::max(), value);
}

void table_impl::parse_table_type
        (int line_num
        ,std::string const& value
        )
{
    throw_if_duplicate_record(type_.is_initialized(), e_field_table_type, line_num);

    if(value == table_type_as_string(table_type::aggregate))
        {
        type_ = table_type::aggregate;
        }
    else if(value == table_type_as_string(table_type::duration))
        {
        type_ = table_type::duration;
        }
    else if(value == table_type_as_string(table_type::select))
        {
        type_ = table_type::select;
        }
    else
        {
        std::ostringstream oss;
        oss << "invalid table type value '" << value << "' at line " << line_num
            << " (\"" << table_type_as_string(table_type::aggregate) << "\", "
            << "\"" << table_type_as_string(table_type::duration) << "\" or "
            << "\"" << table_type_as_string(table_type::select) << "\" expected)"
            ;
        throw std::runtime_error(oss.str());
        }
}

void table_impl::parse_values(std::istream& is, int& line_num)
{
    unsigned const num_values = get_expected_number_of_values();
    values_.resize(num_values);

    if(!num_decimals_)
        {
        std::ostringstream oss;
        oss << "the '" << soa_fields[e_field_num_decimals].name << "' field "
            << "must be specified before the table values at line " << line_num
            ;
        throw std::runtime_error(oss.str());
        }

    auto const exponent = std::pow(10, *num_decimals_);

    auto const min_age = *min_age_;
    boost::optional<uint16_t> last_age;
    for(std::string line; ++line_num, std::getline(is, line);)
        {
        // Perform strict format checks: the value must use exactly 3 (space
        // padded) characters for the age and exactly the given precision with
        // two spaces between them.
        static auto const age_width = 3;
        static auto const gap_length = 2;

        auto const start = line.c_str();

        // We need to manually skip the leading whitespace as
        // strict_parse_number() doesn't accept it.
        auto start_num = start;
        while(*start_num == ' ' || *start_num == '\t')
            {
            if(start_num - start == age_width)
                {
                std::ostringstream oss;
                oss << "at most " << age_width - 1 << " spaces allowed "
                    << "at the beginning of line " << line_num
                    ;
                throw std::runtime_error(oss.str());
                }

            ++start_num;
            }

        auto const res_age = strict_parse_number(start_num);
        if(!res_age.end || (res_age.end - start != age_width))
            {
            std::ostringstream oss;
            oss << "expected a number with at most " << age_width << " digits "
                << "at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        // There is no need to check for the range, we can't overflow uint16_t
        // with just 3 digits.
        auto const age = static_cast<uint16_t>(res_age.num);

        if(line.compare(age_width, gap_length, std::string(gap_length, ' ')) != 0)
            {
            std::ostringstream oss;
            oss << "expected a " << gap_length << " spaces after the age "
                << "at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        // We can't impose the exact number of decimal digits using standard
        // functions for parsing floating point values, so do it manually.
        auto const res_int_part = strict_parse_number(res_age.end + gap_length);
        if(!res_int_part.end)
            {
            std::ostringstream oss;
            oss << "expected a valid integer part at position "
                << age_width + gap_length + 1
                << " at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        if(*res_int_part.end != '.')
            {
            std::ostringstream oss;
            oss << "expected decimal point at position "
                << res_int_part.end - start + 1
                << " at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        auto const res_frac_part = strict_parse_number(res_int_part.end + 1);
        if(!res_frac_part.end)
            {
            std::ostringstream oss;
            oss << "expected a valid fractional part at position "
                << res_frac_part.end - start + 1
                << " at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        if(res_frac_part.end - res_int_part.end - 1 != *num_decimals_)
            {
            std::ostringstream oss;
            oss << "expected " << *num_decimals_ << " decimal digits, not "
                << res_frac_part.end - res_int_part.end - 1
                << " in the value at line " << line_num
                ;
            throw std::runtime_error(oss.str());
            }

        double value = res_frac_part.num;
        value /= exponent;
        value += res_int_part.num;

        // Check that we have the correct age: we must start with the minimum
        // age and each next value must be one greater than the previous one.
        auto const age_expected = last_age ? *last_age + 1 : min_age;
        if(age != age_expected)
            {
            std::ostringstream oss;
            oss << "incorrect age value " << age
                << " at line " << line_num
                << " (" << age_expected << " expected)"
                ;
            throw std::runtime_error(oss.str());
            }

        last_age = age;

        // Because of the check above we can be certain that age > min_age.
        unsigned const n = age - min_age;
        values_[n] = value;

        if (n == num_values - 1)
            {
            return;
            }
        }

    // Check for premature input end.
    std::ostringstream oss;
    if(last_age)
        {
        oss << "only " << (*last_age - min_age + 1) << " values specified, "
            << "but " << num_values << " expected"
            ;
        }
    else
        {
        oss << "table values are missing after line " << line_num;
        }

    throw std::runtime_error(oss.str());
}

void table_impl::validate()
{
    // Check that the fields we absolutely need were specified.
    throw_if_missing_field(number_, e_field_table_number);
    throw_if_missing_field(type_, e_field_table_type);

    // Check that we have the values: this also ensures that we have the
    // correct minimum and maximum age as this is verified when filling in the
    // values.
    if(values_.empty())
        {
        throw std::runtime_error("no values defined");
        }

    // Validate the type and check that the select period has or hasn't been
    // given, depending on it.
    switch(*type_)
        {
        case table_type::aggregate:
        case table_type::duration:
            if(get_value_or(select_period_, 0))
                {
                std::ostringstream oss;
                oss << "select period cannot be specified for a table of type "
                    << "'" << table_type_as_string(*type_) << "'";
                throw std::runtime_error(oss.str());
                }
            if(get_value_or(max_select_age_, 0) && *max_select_age_ != *max_age_)
                {
                std::ostringstream oss;
                oss << "maximum select age " << *max_select_age_
                    << " different from the maximum age " << *max_age_
                    << " cannot be specified for a table of type"
                       " '" << table_type_as_string(*type_) << "'";
                throw std::runtime_error(oss.str());
                }
            break;

        case table_type::select:
            if(!get_value_or(select_period_, 0))
                {
                throw std::runtime_error
                    ("select period must be specified for a select and ultimate table"
                    );
                }
            break;
        }

    // We have a reasonable default for this value, so don't complain if it's
    // absent.
    if(!num_decimals_)
        {
        num_decimals_ = 6;
        }
}

void table_impl::read_from_binary(std::istream& ifs, uint32_t offset)
{
    ifs.seekg(offset, std::ios::beg);
    if(!ifs)
        {
        throw std::runtime_error("seek error");
        }

    for(;;)
        {
        uint16_t const
            record_type = do_read_number<uint16_t>("record type", ifs);

        // Check for the special case of the end table record type as it's the
        // only one without any contents following it (this also explains why
        // we can't read both the record type and length in one go as we'd
        // prefer to do for efficiency: this would fail for the last table of
        // the file).
        if(record_type == e_record_end_table)
            {
            validate();
            return;
            }

        uint16_t const
            length = do_read_number<uint16_t>("record length", ifs);

        switch(record_type)
            {
            case e_record_table_name:
                read_string(name_, e_field_table_name, ifs, length);
                break;
            case e_record_table_number:
                read_number(number_, e_field_table_number, ifs, length);
                break;
            case e_record_table_type:
                read_type(ifs, length);
                break;
            case e_record_contributor:
                read_string(contributor_, e_field_contributor, ifs, length);
                break;
            case e_record_data_source:
                read_string(data_source_, e_field_data_source, ifs, length);
                break;
            case e_record_data_volume:
                read_string(data_volume_, e_field_data_volume, ifs, length);
                break;
            case e_record_obs_period:
                read_string(obs_period_, e_field_obs_period, ifs, length);
                break;
            case e_record_unit_of_obs:
                read_string(unit_of_obs_, e_field_unit_of_obs, ifs, length);
                break;
            case e_record_construction_method:
                read_string(construction_method_, e_field_construction_method, ifs, length);
                break;
            case e_record_published_reference:
                read_string(published_reference_, e_field_published_reference, ifs, length);
                break;
            case e_record_comments:
                read_string(comments_, e_field_comments, ifs, length);
                break;
            case e_record_min_age:
                read_number(min_age_, e_field_min_age, ifs, length);
                break;
            case e_record_max_age:
                read_number(max_age_, e_field_max_age, ifs, length);
                break;
            case e_record_select_period:
                read_number_before_values(select_period_, e_field_select_period, ifs, length);
                break;
            case e_record_max_select_age:
                read_number_before_values(max_select_age_, e_field_max_select_age, ifs, length);
                break;
            case e_record_num_decimals:
                read_number(num_decimals_, e_field_num_decimals, ifs, length);
                break;
            case e_record_values:
                read_values(ifs, length);
                break;
            case e_record_hash_value:
                read_number(hash_value_, e_field_hash_value, ifs, length);
                break;
            default:
                std::ostringstream oss;
                oss << "unknown field type " << record_type;
                throw std::runtime_error(oss.str());
            }
        }
}

shared_ptr<table_impl> table_impl::create_from_binary
    (std::istream& is
    ,uint32_t offset
    )
{
    shared_ptr<table_impl> table = std::make_shared<table_impl>();
    table->read_from_binary(is, offset);
    return table;
}

void table_impl::read_from_text(std::istream& is)
{
    using namespace text_format;

    // The text format is line-oriented with a typical line containing a
    // colon-separated "key: value" pair, however if a line doesn't contain a
    // column it's supposed to be a continuation of the value of the previous
    // line, which allows for multiline values (but without any commas except
    // on the first line!).

    // Current line number, only used for the error messages.
    int line_num = 1;

    // Now-owning pointer to the last string field value or null if none (e.g.
    // no fields parsed at all yet or the last one wasn't a string). This is
    // used for continuation lines handling.
    std::string* last_string = nullptr;

    for(std::string line; std::getline(is, line); ++line_num)
        {
        auto const pos_colon = line.find(':');
        if(pos_colon != std::string::npos)
            {
            static char const* const whitespace = " \t";

            // Discard trailing whitespace, it is insignificant and would just
            // complicate the checks below.
            auto const end_line = line.find_last_not_of(whitespace);
            if(end_line == std::string::npos)
                {
                // Blank line, we only accept (and ignore) them after the end
                // of the input, so check that nothing more is left.
                int const blank_line_num = line_num;
                for(++line_num; std::getline(is, line); ++line_num)
                    {
                    if(line.find_first_not_of(whitespace) != std::string::npos)
                        {
                        std::ostringstream oss;
                        oss << "Blank line " << blank_line_num << " "
                            << "cannot appear in the middle of the input "
                            << "and be followed by non-blank line " << line_num
                            ;
                        throw std::runtime_error(oss.str());
                        }
                    }
                break;
                }

            std::string const key(line, 0, pos_colon);

            auto const field = parse_field_name(key, line_num);

            // Special case of the table values field which doesn't have any
            // value on this line itself.
            if(field == e_field_values)
                {
                if(pos_colon + 1 != line.length())
                    {
                    std::ostringstream oss;
                    oss << "Value not allowed after '" << key << ":' "
                        << "at line " << line_num
                        ;
                    throw std::runtime_error(oss.str());
                    }

                parse_values(is, line_num);
                continue;
                }

            // Almost all the other fields may only come before the table
            // values.
            if(!values_.empty() && field != e_field_hash_value)
                {
                std::ostringstream oss;
                oss << "Field '" << key << "' is not allowed after the table "
                    << "values at line " << line_num
                    ;
                throw std::runtime_error(oss.str());
                }

            if(pos_colon + 1 == line.length())
                {
                std::ostringstream oss;
                oss << "Value expected after '" << key << ":' "
                    << "at line " << line_num
                    ;
                throw std::runtime_error(oss.str());
                }

            if(line[pos_colon + 1] != ' ')
                {
                std::ostringstream oss;
                oss << "Space expected after '" << key << ":' "
                    << "at line " << line_num
                    ;
                throw std::runtime_error(oss.str());
                }

            std::string const value(line, pos_colon + 2); // +2 to skip ": "

            last_string = nullptr; // reset it for non-string fields

            switch(field)
                {
                case e_field_table_name:
                    last_string = parse_string(name_, field, line_num, value);
                    break;
                case e_field_table_number:
                    parse_number(number_, field, line_num, value);
                    break;
                case e_field_table_type:
                    // This is a string field which is represented as an
                    // integer internally, so it needs special handling.
                    parse_table_type(line_num, value);
                    break;
                case e_field_contributor:
                    last_string = parse_string(contributor_, field, line_num, value);
                    break;
                case e_field_data_source:
                    last_string = parse_string(data_source_, field, line_num, value);
                    break;
                case e_field_data_volume:
                    last_string = parse_string(data_volume_, field, line_num, value);
                    break;
                case e_field_obs_period:
                    last_string = parse_string(obs_period_, field, line_num, value);
                    break;
                case e_field_unit_of_obs:
                    last_string = parse_string(unit_of_obs_, field, line_num, value);
                    break;
                case e_field_construction_method:
                    last_string = parse_string(construction_method_, field, line_num, value);
                    break;
                case e_field_published_reference:
                    last_string = parse_string(published_reference_, field, line_num, value);
                    break;
                case e_field_comments:
                    last_string = parse_string(comments_, field, line_num, value);
                    break;
                case e_field_min_age:
                    parse_number(min_age_, field, line_num, value);
                    break;
                case e_field_max_age:
                    parse_number(max_age_, field, line_num, value);
                    break;
                case e_field_select_period:
                    parse_number(select_period_, field, line_num, value);
                    break;
                case e_field_max_select_age:
                    parse_number(max_select_age_, field, line_num, value);
                    break;
                case e_field_num_decimals:
                    parse_number(num_decimals_, field, line_num, value);
                    break;
                case e_field_values:
                    // This field has been handled specially above, but still
                    // have a case for it here to be warned by the compiler
                    // about any missing enum values.
                    throw std::logic_error
                        ("Internal error: table values field impossible here."
                        );
                case e_field_hash_value:
                    if(values_.empty())
                        {
                        std::ostringstream oss;
                        oss << "'" << key << "' field is only allowed after "
                            << "the table values, not at line " << line_num
                            ;
                        throw std::runtime_error(oss.str());
                        }

                    parse_number(hash_value_, field, line_num, value);
                    break;
                }
            }
        else // no colon in this line
            {
            // Must be a continuation of the previous line.
            if(!last_string)
                {
                std::ostringstream oss;
                oss << "Expected a colon on line " << line_num;
                throw std::runtime_error(oss.str());
                }

            *last_string += '\n';
            *last_string += line;

            // Do not change last_string, more continuation lines can follow.
            }
        }

    // Verify that all the required fields have been specified.
    validate();
}

shared_ptr<table_impl> table_impl::create_from_text(std::istream& is)
{
    shared_ptr<table_impl> table = std::make_shared<table_impl>();
    table->read_from_text(is);
    return table;
}

template<typename T>
void table_impl::do_write(std::ostream& os) const
{
    T w(os);

    w.write(e_field_table_name         , name_                );
    w.write(e_field_table_number       , number_              );
    w.write_table_type(*type_);
    w.write(e_field_contributor        , contributor_         );
    w.write(e_field_data_source        , data_source_         );
    w.write(e_field_data_volume        , data_volume_         );
    w.write(e_field_obs_period         , obs_period_          );
    w.write(e_field_unit_of_obs        , unit_of_obs_         );
    w.write(e_field_construction_method, construction_method_ );
    w.write(e_field_published_reference, published_reference_ );
    w.write(e_field_comments           , comments_            );
    w.write_values
        (values_
        ,num_decimals_
        ,min_age_
        ,max_age_
        ,select_period_
        ,max_select_age_
        );
    w.write(e_field_hash_value         , hash_value_          );
    w.end();
}

bool table_impl::is_equal(table_impl const& other) const
{
    return values_ == other.values_
        && name_ == other.name_
        && contributor_ == other.contributor_
        && data_source_ == other.data_source_
        && data_volume_ == other.data_volume_
        && obs_period_ == other.obs_period_
        && unit_of_obs_ == other.unit_of_obs_
        && construction_method_ == other.construction_method_
        && published_reference_ == other.published_reference_
        && comments_ == other.comments_
        && number_ == other.number_
        && hash_value_ == other.hash_value_
        && num_decimals_ == other.num_decimals_
        && min_age_ == other.min_age_
        && max_age_ == other.max_age_
        && select_period_ == other.select_period_
        && max_select_age_ == other.max_select_age_
        && type_ == other.type_
        ;
}

unsigned long table_impl::compute_hash_value() const
{
    // This is a bug-for-bug reimplementation of the hash value computation
    // algorithm used in the original SOA format which produces compatible
    // (even if nonsensical) hash values.
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(3) << *min_age_
        << std::setw(3) << *max_age_
        << std::setw(3) << get_value_or(select_period_, 0)
        << std::setw(3) << get_value_or(max_select_age_, 0)
        ;

    oss << std::fixed << std::setprecision(*num_decimals_);
    unsigned const value_width = *num_decimals_ + 2;

    for(auto v: values_)
        {
        oss << std::setw(value_width) << v;
        }

    std::string s = oss.str();

    // Truncate the string for compatibility with the original code.
    s.resize(values_.size()*value_width);

    CRC crc;
    crc += s;

    // Undo the last XOR, again for compatibility.
    return crc.value() ^ 0xffffffffU;
}

table table::read_from_text(fs::path const& file)
{
    fs::ifstream ifs;
    open_text_file(ifs, file);

    return table(table_impl::create_from_text(ifs));
}

table table::read_from_text(std::string const& text)
{
    std::istringstream iss(text);

    return table(table_impl::create_from_text(iss));
}

void table::save_as_text(fs::path const& file) const
{
    fs::ofstream ofs;
    open_text_file(ofs, file);

    impl_->write_as_text(ofs);
}

std::string table::save_as_text() const
{
    std::ostringstream oss;

    impl_->write_as_text(oss);

    return oss.str();
}

void table::name(std::string const& n)
{
    return impl_->name(n);
}

table::Number table::number() const
{
    return table::Number(impl_->number());
}

std::string const& table::name() const
{
    return impl_->name();
}

char const* table::type_as_string() const
{
    return impl_->type_as_string();
}

unsigned long table::compute_hash_value() const
{
    return impl_->compute_hash_value();
}

bool table::operator==(table const& other) const
{
    return impl_->is_equal(*other.impl_);
}

class database_impl
    :        private lmi::uncopyable <database_impl>
    ,virtual private obstruct_slicing<database_impl>
{
  public:
    database_impl();
    explicit database_impl(fs::path const& path);

    int tables_count() const;
    table get_nth_table(int idx) const;
    table find_table(table::Number number) const;
    void append_table(table const& table);
    void add_or_replace_table(table const& table);
    void save(fs::path const& path);

  private:
    // An index record is composed of:
    //
    //  -  4 byte table number
    //  - 50 byte table name (ignored here)
    //  -  4 byte offset of the table in the database file
    //
    // The numbers are in little endian format.
    enum
        {e_index_pos_number = 0
        ,e_index_pos_name   = 4
        ,e_index_pos_offset = 54
        ,e_index_pos_max    = 58
        };

    void read_index(fs::path const& path);

    // We don't currently use the name stored in the index, so this struct
    // doesn't include it.
    struct IndexEntry
    {
        IndexEntry
            (table::Number number
            ,uint32_t offset
            ,shared_ptr<table_impl> table
            )
            :number_(number.value())
            ,offset_(offset)
            ,table_(table)
        {
        }

        uint32_t number_;

        // This field is ignored for the tables added to the database after
        // reading the original index file and is only used for loading the
        // existing tables from the original input file.
        uint32_t offset_;

        // table pointer may be empty for the tables present in the input
        // database file but not loaded yet.
        mutable shared_ptr<table_impl> table_;
    };

    // Add an entry to the index. This method should be always used instead of
    // updating index_ vector directly as it also takes care of updating
    // index_by_number_ map.
    //
    // Returns false if there was already a table with the given number, this
    // is not supposed to happen and should be treated as an error by caller.
    bool add_index_entry
        (table::Number number
        ,uint32_t offset
        ,shared_ptr<table_impl> table = shared_ptr<table_impl>()
        );

    // Return the table corresponding to the given index entry, loading it from
    // the database file if this hadn't been done yet.
    shared_ptr<table_impl> do_get_table_impl(IndexEntry const& entry) const;
    table do_get_table(IndexEntry const& entry) const
        {
        return table(do_get_table_impl(entry));
        }

    // Return the pointer to the index entry with the given table number or
    // null if no such entry could be found.
    IndexEntry* do_find_table_entry(table::Number number);

    // Add a new table with a number not present in the index yet.
    void do_append_table(table const& table);


    // All entries read from the index file.
    std::vector<IndexEntry> index_;

    // Map allowing efficient table lookup by its number. Its values are
    // indices into index_ vector.
    typedef std::map<table::Number, unsigned> NumberToIndexMap;
    NumberToIndexMap index_by_number_;

    // Path to the database, used only for the error messages.
    fs::path const path_;

    // The open database file: we keep it open to read table data on demand
    // from it.
    //
    // An alternative approach could be to just load everything into memory at
    // once.
    mutable fs::ifstream database_ifs_;
};

database_impl::database_impl()
{
}

database_impl::database_impl(fs::path const& path)
    :path_(path)
{
    read_index(path);

    // Open the database file right now to ensure that we can do it, even if we
    // don't need it just yet. As it will be used soon anyhow, delaying opening
    // it wouldn't be a useful optimization.
    open_binary_file(database_ifs_, fs::change_extension(path, ".dat"));
}

bool database_impl::add_index_entry
    (table::Number number
    ,uint32_t offset
    ,shared_ptr<table_impl> table
    )
{
    index_.push_back(IndexEntry(number, offset, table));

    // The index of this entry is the last index of the index_, by construction.
    if(!index_by_number_.insert(std::make_pair(number, index_.size() - 1)).second)
        {
        // We expect an insertion to be made as the map shouldn't contain this
        // number yet, but can't generate the appropriate error message here if
        // it does, so let the caller do it.
        return false;
        }

    return true;
}

void database_impl::read_index(fs::path const& path)
{
    fs::path const index_path = fs::change_extension(path, ".ndx");

    fs::ifstream index_ifs;
    open_binary_file(index_ifs, index_path);

    char index_record[e_index_pos_max] = {0};

    for(;;)
        {
        if(!stream_read(index_ifs, index_record, e_index_pos_max))
            {
            if(index_ifs.eof() && !index_ifs.gcount())
                {
                break;
                }

            std::ostringstream oss;
            oss << "Error reading entry " << index_.size()
                << " from the database index '" << index_path << "'.";
            throw std::runtime_error(oss.str());
            }

        uint32_t const
            number = from_bytes<uint32_t>(&index_record[e_index_pos_number]);
        uint32_t const
            offset = from_bytes<uint32_t>(&index_record[e_index_pos_offset]);

        // Check that the cast to int below is safe.
        if(number >= static_cast<unsigned>(std::numeric_limits<int>::max()))
            {
            std::ostringstream oss;
            oss << "database index '" << index_path << "' is corrupt: "
                   "table number " << number << " is out of range.";
            throw std::runtime_error(oss.str());
            }

        if(!add_index_entry(table::Number(static_cast<int>(number)), offset))
            {
            std::ostringstream oss;
            oss << "database index '" << index_path << "' is corrupt: "
                   "duplicate entries for the table number " << number;
            throw std::runtime_error(oss.str());
            }
        }
}

int database_impl::tables_count() const
{
    return static_cast<int>(index_.size());
}

table database_impl::get_nth_table(int idx) const
{
    return do_get_table(index_.at(idx));
}

shared_ptr<table_impl> database_impl::do_get_table_impl
    (IndexEntry const& entry
    ) const
{
    if(!entry.table_)
        {
        try
            {
            entry.table_ = table_impl::create_from_binary
                (database_ifs_
                ,entry.offset_
                );
            }
        catch(std::runtime_error const& e)
            {
            std::ostringstream oss;
            oss << "Error reading table " << entry.number_
                << " from the offset " << entry.offset_
                << " in the database '" << path_ << "': " << e.what();
            throw std::runtime_error(oss.str());
            }

        if(entry.table_->number() != entry.number_)
            {
            std::ostringstream oss;
            oss << "database '" << path_ << "' is corrupt: "
                   "table number " << entry.table_->number()
                << " is inconsistent with its number in the index ("
                << entry.number_ << ")";
            throw std::runtime_error(oss.str());
            }
        }

    return entry.table_;
}

database_impl::IndexEntry* database_impl::do_find_table_entry(table::Number number)
{
    NumberToIndexMap::const_iterator const ci = index_by_number_.find(number);

    return ci == index_by_number_.end() ? NULL : &index_.at(ci->second);
}

table database_impl::find_table(table::Number number) const
{
    IndexEntry* const
        entry = const_cast<database_impl*>(this)->do_find_table_entry(number);

    if(!entry)
        {
        std::ostringstream oss;
        oss << "table number " << number << " not found.";
        throw std::invalid_argument(oss.str());
        }

    return do_get_table(*entry);
}

void database_impl::do_append_table(table const& table)
{
    if(!add_index_entry(table.number(), 0, table.impl_))
        {
        std::ostringstream oss;
        oss << "Internal program error: unexpectedly duplicate table "
               "number " << table.number();
        throw std::logic_error(oss.str());
        }
}

void database_impl::append_table(table const& table)
{
    table::Number const num = table.number();
    if(do_find_table_entry(num))
        {
        std::ostringstream oss;
        oss << "table number " << num << " already exists.";
        throw std::invalid_argument(oss.str());
        }

    do_append_table(table);
}

void database_impl::add_or_replace_table(table const& table)
{
    IndexEntry* const entry = do_find_table_entry(table.number());
    if(entry)
        {
        entry->table_ = table.impl_;
        }
    else
        {
        do_append_table(table);
        }
}

void database_impl::save(fs::path const& path)
{
    fs::ofstream index_ofs;
    open_binary_file(index_ofs, fs::change_extension(path, ".ndx"));

    fs::ofstream database_ofs;
    open_binary_file(database_ofs, fs::change_extension(path, ".dat"));

    char index_record[e_index_pos_max] = {0};

    for(auto const& i: index_)
        {
        shared_ptr<table_impl> const t = do_get_table_impl(i);

        // The offset of this table is just the current position of the output
        // stream, so get it before it changes and check that it is still
        // representable as a 4 byte offset (i.e. the file is less than 4GiB).
        std::streamoff const offset = database_ofs.tellp();
        uint32_t const offset32 = static_cast<uint32_t>(offset);
        if(static_cast<std::streamoff>(offset32) != offset)
            {
            throw std::runtime_error
                    ("Database is too large to be stored in SOA v3 format."
                    );
            }

        to_bytes(&index_record[e_index_pos_number], t->number());

        // We need to pad the name with NUL bytes if it's shorter than maximum
        // length, so use strncpy() to do it.
        strncpy
            (&index_record[e_index_pos_name]
            ,t->name().c_str()
            ,e_index_pos_offset - e_index_pos_name - 1
            );

        // However (mainly for compatibility with the existing files as this
        // code doesn't rely on it) the name still has to be NUL-terminated, in
        // spite of being fixed size, so ensure this is the case.
        index_record[e_index_pos_offset - 1] = '\0';

        to_bytes(&index_record[e_index_pos_offset], offset32);

        stream_write(index_ofs, index_record, sizeof(index_record));

        t->write_as_binary(database_ofs);
        }

    index_ofs.close();
    database_ofs.close();

    if(!database_ofs || !index_ofs)
        {
        std::ostringstream oss;
        oss << "Writing database data to '" << path << "' failed.";
        throw std::runtime_error(oss.str());
        }
}

database::database()
    :impl_(new database_impl())
{
}

database::database(fs::path const& path)
    :impl_(new database_impl(path))
{
}

database::~database()
{
    delete impl_;
}

int database::tables_count() const
{
    return impl_->tables_count();
}

table database::get_nth_table(int idx) const
{
    return impl_->get_nth_table(idx);
}

table database::find_table(table::Number number) const
{
    return impl_->find_table(number);
}

void database::append_table(table const& table)
{
    return impl_->append_table(table);
}

void database::add_or_replace_table(table const& table)
{
    return impl_->add_or_replace_table(table);
}

void database::save(fs::path const& path)
{
    return impl_->save(path);
}

} // namespace soa_v3_format
