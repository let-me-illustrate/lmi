// Tools for working with SOA tables represented in binary format.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "alert.hpp"
#include "bourn_cast.hpp"
#include "crc32.hpp"
#include "miscellany.hpp"               // ios_in_binary(), ios_out_trunc_binary()
#include "path_utility.hpp"
#include "value_cast.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>

#include <algorithm>                    // count()
#if 202002 <= __cplusplus
#include <bit>                          // endian
#endif
#include <climits>                      // ULLONG_MAX
#include <cstdlib>                      // strtoull()
#include <cstring>                      // memcpy(), strncmp()
#include <iomanip>
#include <ios>
#include <istream>
#include <limits>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>                      // make_pair(), swap()

// Note about error handling in this code: with a few exceptions (e.g.
// strict_parse_number), most of the functions in this file throw on error.
// If the exception is thrown from a low level function, it is caught and
// re-thrown from a higher level one which called it, after building a more
// helpful error message containing both the details from the low level
// function and the information about the context from the higher level one.
//
// When throwing an exception which is meant to be caught and re-thrown, the
// associated message starts with a lower case letter and is terminated by
// std::flush without a period preceding it, so that it can be incorporated
// seamlessly into the final message. And when throwing the last exception from
// a high level function, the message is capitalized and terminated by a period
// and LMI_FLUSH.

// The SOA binary format uses IEEE 754 for the floating point values
// representation and the code in this file won't work correctly if it is
// different from their in memory representation.
static_assert(std::numeric_limits<double>::is_iec559);

// Check that WORDS_BIGENDIAN, which is supposed to be defined in makefile/at
// the compiler command line if necessary, is consistent with the actual
// endianness used.
#if 202002 <= __cplusplus
#if defined WORDS_BIGENDIAN
static_assert(std::endian::native == std::endian::big);
#else
static_assert(std::endian::native == std::endian::little);
#endif
#endif

// Helper functions used to swap bytes on big endian platforms.
namespace
{

// Defining this NOP function allows to call swap_bytes_if_big_endian() in template
// code for numbers of any size.
inline
std::uint8_t swap_bytes_if_big_endian(std::uint8_t val)
{
    return val;
}

#if defined WORDS_BIGENDIAN
inline
std::uint16_t swap_bytes_if_big_endian(std::uint16_t val)
{
    return ((val & 0x00ffU) << 8)
         | ((val & 0xff00U) >> 8)
         ;
}

inline
std::uint32_t swap_bytes_if_big_endian(std::uint32_t val)
{
    return ((val & 0x000000ffU) << 24)
         | ((val & 0x0000ff00U) <<  8)
         | ((val & 0x00ff0000U) >>  8)
         | ((val & 0xff000000U) >> 24)
         ;
}

inline
std::uint64_t swap_bytes_if_big_endian(std::uint64_t val)
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
    std::uint64_t const ui64 = swap_bytes_if_big_endian(*reinterpret_cast<std::uint64_t*>(&val));

    // And vice versa.
    return *reinterpret_cast<double*>(&ui64);
}
#else // !defined WORDS_BIGENDIAN
inline
std::uint16_t swap_bytes_if_big_endian(std::uint16_t val)
{
    return val;
}

inline
std::uint32_t swap_bytes_if_big_endian(std::uint32_t val)
{
    return val;
}

inline
double swap_bytes_if_big_endian(double val)
{
    return val;
}
#endif // !defined WORDS_BIGENDIAN

template<typename T>
inline
T from_bytes(char const* bytes)
{
    T t;
    std::memcpy(&t, bytes, sizeof(T));
    return swap_bytes_if_big_endian(t);
}

template<typename T>
inline
void to_bytes(char* bytes, T value)
{
    T const t = swap_bytes_if_big_endian(value);
    std::memcpy(bytes, &t, sizeof(T));
}

// Functions doing the same thing as istream::read() and ostream::write()
// respectively, but taking void pointers and this allowing to avoid ugly casts
// to char in the calling code. SOMEDAY !! Consider changing the type of
// 'length', which is always cast to something else before use.

inline bool stream_write(std::ostream& os, void const* data, std::size_t length)
{
    os.write(static_cast<char const*>(data), bourn_cast<std::streamsize>(length));
    return !os.fail();
}

inline bool stream_read(std::istream& is, void* data, std::size_t length)
{
    is.read(static_cast<char*>(data), bourn_cast<std::streamsize>(length));
    return is.gcount() == static_cast<std::streamsize>(length);
}

// This function removes a file ignoring any errors, it should only be used if
// there is no way to handle these errors anyhow, e.g. when we're trying to
// clean up while handling a previous exception and so can't let another one
// propagate.
//
// BOOST !! Use "ec" argument with later versions instead of throwing and
// catching the exception.
void remove_nothrow(fs::path const& path)
{
    try
        {
        fs::remove(path);
        }
    catch(fs::filesystem_error const&)
        {
        // Intentionally ignore.
        }
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
    unsigned long long int num = 0;
    char const* end = nullptr;
};

parse_result strict_parse_number(char const* start)
{
    parse_result res;

    // This check catches whitespace and the leading minus sign.
    if('0' <= *start && *start <= '9')
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
    std::uint16_t record_type; // Field record type in the binary format.
    char const*   name;        // Field name in the text format.
};

// The following two enums are deliberately distinct despite their
// close similarity, for reasons explained at length here:
//   https://lists.nongnu.org/archive/html/lmi/2018-02/msg00030.html

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

enum class table_type : std::uint8_t
    {aggregate = 'A'
    ,duration  = 'D'
    ,select    = 'S'
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

// Represents location in the input, possibly invalid if it's not available.
struct location_info
{
    explicit location_info(int line_num = 0, int position = 0)
        :line_num_ {line_num}
        ,position_ {position}
        {
        }

    int const line_num_ = 0;
    int const position_ = 0;
};

inline
std::ostream& operator<<(std::ostream& os, location_info const& loc)
{
    if(loc.line_num_)
        {
        if(loc.position_)
            {
            os << " at position " << loc.position_;
            }
        os << " at line " << loc.line_num_;
        }

    return os;
}

} // Unnamed namespace.

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
    void write(enum_soa_field field, std::optional<T> const& onum);
    void write_table_type(table_type tt);
    void write(enum_soa_field field, std::optional<std::string> const& ostr);

    void write_values
            (std::vector<double>          const& values
            ,std::optional<std::uint16_t> const& num_decimals
            ,std::optional<std::uint16_t> const& min_age
            ,std::optional<std::uint16_t> const& max_age
            ,std::optional<std::uint16_t> const& select_period
            ,std::optional<std::uint16_t> const& max_select_age
            );

    void end();

  private:
    void do_write_record_header(std::uint16_t record_type, std::uint16_t length);
    template<typename T>
    void do_write_field(enum_soa_field field, T num);

    std::ostream& os_;
};

void writer::write_values
        (std::vector<double>          const& values
        ,std::optional<std::uint16_t> const& num_decimals
        ,std::optional<std::uint16_t> const& min_age
        ,std::optional<std::uint16_t> const& max_age
        ,std::optional<std::uint16_t> const& select_period
        ,std::optional<std::uint16_t> const& max_select_age
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

#if defined WORDS_BIGENDIAN
    // Convert the values to their on disk representation.
    std::vector<double> little_endian_values;
    little_endian_values.reserve(values.size());

    for(auto const& v : values)
        {
        little_endian_values.push_back(swap_bytes_if_big_endian(v));
        }
#else // !defined WORDS_BIGENDIAN
    // No conversion necessary, don't create an extra vector needlessly, just
    // alias the existing one.
    std::vector<double> const& little_endian_values = values;
#endif // !defined WORDS_BIGENDIAN

    std::size_t const length = values.size() * sizeof(double);

    // As explained in table_impl::read_values(), length field is too small to
    // store the length of this record in general, but we still write the data
    // even in this case as this code, at least, can still read it. We set the
    // length to the maximal representable value instead of some random junk
    // that would result if we simply truncated it to 16 bits however.
    do_write_record_header
        (e_record_values
        ,std::numeric_limits<std::uint16_t>::max() < length
            ? std::numeric_limits<std::uint16_t>::max()
            : static_cast<std::uint16_t>(length)
        );

    // Normally we don't check the stream state after each write as it is
    // enough to check it once at the end, however this write, being much
    // bigger than others, has probably bigger chance of failing, so do check
    // for its success, exceptionally, in order to detect the error a.s.a.p.
    if(!stream_write(os_, &little_endian_values[0], length))
        {
        alarum() << "writing values failed" << std::flush;
        }
}

void writer::do_write_record_header(std::uint16_t record_type, std::uint16_t length)
{
    enum
        {e_header_pos_type = 0
        ,e_header_pos_len  = 2
        ,e_header_pos_max  = 4
        };

    char header[e_header_pos_max];

    to_bytes(header + e_header_pos_type, record_type);
    to_bytes(header + e_header_pos_len , length     );

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
void writer::write(enum_soa_field field, std::optional<T> const& onum)
{
    if(onum)
        {
        do_write_field(field, *onum);
        }
}

void writer::write_table_type(table_type tt)
{
    do_write_field(e_field_table_type, static_cast<std::uint8_t>(tt));
}

void writer::write(enum_soa_field field, std::optional<std::string> const& ostr)
{
    if(ostr)
        {
        if(std::numeric_limits<std::uint16_t>::max() < ostr->size())
            {
            alarum()
                << "the value of the field '"
                << soa_fields[field].name
                << "' is too long to be represented in the SOA binary format"
                << std::flush
                ;
            }

        auto const length = bourn_cast<std::uint16_t>(ostr->size());
        do_write_record_header(soa_fields[field].record_type, length);
        stream_write(os_, ostr->c_str(), length);
        }
}

void writer::end()
{
    std::uint16_t record_type = e_record_end_table;
    record_type = swap_bytes_if_big_endian(record_type);
    stream_write(os_, &record_type, sizeof(record_type));
}

} // namespace binary_format

namespace text_format
{

// Maximum number of digits used for the age column.
auto const age_width = 3;

// Number of spaces used between columns of the values table.
auto const gap_length = 2;

// Number of characters taken by a single value using the given precision.
inline int get_value_width(int num_decimals)
{
    return num_decimals + gap_length + 2; // +2 for "0."
}

// Label used for the ultimate column in the select tables.
auto const ultimate_header = "Ult.";

class writer
{
  public:
    explicit writer(std::ostream& os) : os_(os) {}

    template<typename T>
    void write(enum_soa_field field, std::optional<T> const& oval);
    void write_table_type(table_type tt);
    void write_values
            (std::vector<double>          const& values
            ,std::optional<std::uint16_t> const& num_decimals
            ,std::optional<std::uint16_t> const& min_age
            ,std::optional<std::uint16_t> const& max_age
            ,std::optional<std::uint16_t> const& select_period
            ,std::optional<std::uint16_t> const& max_select_age
            );

    void end();

  private:
    std::ostream& os_;
};

template<typename T>
void writer::write(enum_soa_field field, std::optional<T> const& oval)
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
        (std::vector<double>          const& values
        ,std::optional<std::uint16_t> const& num_decimals
        ,std::optional<std::uint16_t> const& min_age
        ,std::optional<std::uint16_t> const& max_age
        ,std::optional<std::uint16_t> const& select_period
        ,std::optional<std::uint16_t> const& max_select_age
        )
{
    write(e_field_min_age            , min_age             );
    write(e_field_max_age            , max_age             );
    write(e_field_select_period      , select_period       );
    write(e_field_max_select_age     , max_select_age      );
    write(e_field_num_decimals       , num_decimals        );

    os_ << soa_fields[e_field_values].name << ":\n";

    auto const value_width = text_format::get_value_width(*num_decimals);

    if(select_period.value_or(0))
        {
        auto const period = *select_period;

        // Make a header with the select durations.
        os_ << std::setw(text_format::age_width) << ' ';
        for(std::uint16_t d = 0; d < period; ++d)
            {
            os_ << std::setw(value_width) << (d + 1);
            }
        os_ << std::setw(text_format::gap_length) << ' '
            << text_format::ultimate_header << "\n";

        // Use indices and at() in this code as it's not performance-critical,
        // so perform the index checks in it, which wouldn't be done
        // automatically with the iterators.
        auto n = 0u;

        os_ << std::fixed << std::setprecision(*num_decimals);

        // Now print out all "full" lines, with select and ultimate values
        // which are laid out consecutively in the values array.
        for(std::uint16_t age = *min_age; age <= *max_select_age; ++age)
            {
            os_ << std::setw(text_format::age_width) << age;

            for(std::uint16_t d = 0; d <= period; ++d)
                {
                os_ << std::setw(value_width) << values.at(n++);
                }

            os_ << std::setw(text_format::gap_length) << ' '
                << std::setw(text_format::age_width) << (age + period)
                << "\n";
            }

        // And finish with the lines having just the ultimate values.
        auto ult_beg = bourn_cast<std::uint16_t>(*max_select_age + period + 1);
        for(std::uint16_t age = ult_beg; age <= *max_age; ++age)
            {
            os_ << std::setw(text_format::age_width) << age;

            for(std::uint16_t d = 0; d < period; ++d)
                {
                os_ << std::setw(value_width) << ' ';
                }

            os_ << std::setw(value_width) << values.at(n++)
                << std::setw(text_format::gap_length) << ' '
                << std::setw(text_format::age_width) << age
                << "\n";
            }

        if(n != values.size())
            {
            throw std::logic_error("bug in select table values writing code");
            }
        }
    else // Not a select table, just print out all values.
        {
        os_ << std::fixed << std::setprecision(*num_decimals);

        std::uint16_t age = *min_age;
        for(auto const& v : values)
            {
            os_ << std::setw(text_format::age_width) << age++
                << std::setw(value_width) << v << "\n";
            }
        }
}

void writer::end()
{
    // There is no table end marker in the text format, it is indicated simply
    // by the end of the file itself.
}

// Result of parse_field_and_value
struct field_and_value
{
    enum_soa_field field;
    std::string value;
};

// Parse the given line as "field: value", making an effort to avoid
// recognizing colons in the middle of the string as field separators.
// If the line isn't in this format, simply return an empty result.
// If the line is almost but not quite in this format, throw an exception
// explaining the problem.
//
// The line_num and table_number are only used for diagnostics.
std::optional<field_and_value> parse_field_and_value
    (std::string const&                  line
    ,int                                 line_num
    ,std::optional<std::uint32_t> const& table_number
    )
{
    std::optional<field_and_value> const no_field;

    auto const pos_colon = line.find(':');
    if(pos_colon == std::string::npos)
        {
        // If there are no colons at all, there are definitely no fields.
        return no_field;
        }

    std::string const name(line, 0, pos_colon);

    int n = 0;
    for(auto const& f : soa_fields)
        {
        if(name == f.name)
            {
            // Cast is safe because the valid enum values exactly correspond to
            // the entries of the fields table we iterate over.
            auto const field = static_cast<enum_soa_field>(n);

            // Special case of the table values: they start from the next line,
            // so there should be nothing else on this one.
            std::string value;
            if(field == e_field_values)
                {
                if(pos_colon + 1 != line.length())
                    {
                    alarum()
                        << "value not allowed after '" << name << ":'"
                        << location_info(line_num)
                        << std::flush
                        ;
                    }
                }
            else
                {
                if(pos_colon + 1 == line.length())
                    {
                    alarum()
                        << "value expected after '" << name << ":'"
                        << location_info(line_num, bourn_cast<int>(pos_colon + 1))
                        << std::flush
                        ;
                    }

                if(line[pos_colon + 1] != ' ')
                    {
                    alarum()
                        << "space expected after '" << name << ":'"
                        << location_info(line_num, bourn_cast<int>(pos_colon + 1))
                        << std::flush
                        ;
                    }
                value = line.substr(pos_colon + 2); // +2 to skip ": "
                }

            return field_and_value{field, value};
            }

        ++n;
        }

    // Not something we recognize, warn about a possible typo in a field name
    // after accounting for some of the special cases:

    // A valid field name can consist of a few words only, so check for this
    // to avoid giving warnings about colons appearing in the middle (or even
    // at the end of) a line.
    if(3 < std::count(line.begin(), line.begin() + bourn_cast<int>(pos_colon), ' '))
        {
        return no_field;
        }

    // There are also a few strings which are known to occur in the actual
    // tables followed by a colon in the beginning of the line, so accept them
    // silently, we know they're not errors.
    char const* const known_not_fields[] =
        {"Editor"
        ,"WARNING"
        };
    for(auto const& not_field : known_not_fields)
        {
        if(name == not_field)
            {
            return no_field;
            }
        }

    // Try to give more context if possible.
    std::string table_context;
    if(table_number)
        {
        std::ostringstream oss;
        oss << " while parsing table " << *table_number;
        table_context = oss.str();
        }

    warning()
        << "Possibly unknown field '" << name << "' ignored"
        << table_context
        << location_info(line_num)
        << "."
        << std::flush
        ;

    // Assume it's just a continuation of the previous line.
    return no_field;
}

} // namespace text_format

namespace soa_v3_format
{

class table_impl final
{
  public:
    table_impl() = default;

    // Load the table data from the stream containing the table data in either
    // binary or text representation.
    //
    // Throws std::runtime_error on error.
    static std::shared_ptr<table_impl> create_from_binary
        (std::istream& is
        ,std::uint32_t offset
        );
    static std::shared_ptr<table_impl> create_from_text(std::istream& is);

    void write_as_binary(std::ostream& os) const { do_write<binary_format::writer>(os); }
    void write_as_text(std::ostream& os) const { do_write<text_format::writer>(os); }

    bool is_equal(table_impl const& other) const;

    // Public class function implementations.
    void name(std::string const& name) { name_ = name; }
    std::uint32_t number() const { return *number_; }
    std::string const& name() const { return *name_; }
    std::uint32_t compute_hash_value() const;

  private:
    table_impl(table_impl const&) = delete;
    table_impl& operator=(table_impl const&) = delete;

    // Helper functions for IO: all of them throw std::runtime_error on failure
    // and mention the field name in the error message.
    //
    // They also check that the optional value provided as the output parameter
    // for reading data into is not initialized yet as it's an error to have
    // duplicate fields in our format.

    // read_xxx() functions for binary format.

    static void read_string
            (std::optional<std::string>& ostr
            ,enum_soa_field              field
            ,std::istream&               ifs
            ,std::uint16_t               length
            );

    template<typename T>
    static T do_read_number(char const* name, std::istream& ifs);

    void read_type(std::istream& ids, std::uint16_t length);

    template<typename T>
    static void read_number
            (std::optional<T>& onum
            ,enum_soa_field    field
            ,std::istream&     ifs
            ,std::uint16_t     length
            );

    // Similar to read_number() but also checks that values hadn't been
    // specified yet, this is useful for fields which are used to deduce the
    // number of the values to read (but not min_age_ and max_age_ as we check
    // that they had been given when reading values, so if they occurred again
    // after reading them, this would already result in a "duplicate field"
    // error).
    void read_number_before_values
            (std::optional<std::uint16_t>& onum
            ,enum_soa_field                field
            ,std::istream&                 ifs
            ,std::uint16_t                 length
            );

    // This one is different from the generic functions above as it's only used
    // for the specific values_ field and not any arbitrary vector.
    void read_values(std::istream& ifs, std::uint16_t length);

    // parse_xxx() functions for text format.

    // This function returns the pointer to ostr string value to allow further
    // modifying it later in the caller.
    static std::string* parse_string
            (std::optional<std::string>& ostr
            ,enum_soa_field              field
            ,int                         line_num
            ,std::string const&          value
            );

    // Parse number checking that it is less than the given maximal value.
    static std::uint32_t do_parse_number
            (enum_soa_field     field
            ,int                line_num
            ,std::uint32_t      max_num
            ,std::string const& value
            );

    template<typename T>
    static void parse_number
            (std::optional<T>&  onum
            ,enum_soa_field     field
            ,int                line_num
            ,std::string const& value
            );

    void parse_table_type
            (int                line_num
            ,std::string const& value
            );

    // Unlike the other functions, this one reads from the input on its own
    // (which is also why it takes line number by reference, as it modifies it).
    void parse_values(std::istream& is, int& line_num);

    // Helper of parse_values() which is only called for select tables and
    // parses (and mostly ignores) their header line.
    void parse_select_header(std::istream& is, int& line_num) const;

    // Skip the given number of spaces and throw an exception if they are not
    // present, otherwise adjust the current pointer to point past them.
    static void skip_spaces
        (int          num_spaces
        ,char const*  start
        ,char const*& current
        ,int          line_num
        );

    // Helper of parse_values() parsing an integer value of at most age_width
    // digits. Adjust the current pointer to advance past the parsed age, the
    // other parameters are only used for diagnostic purposes.
    std::uint16_t parse_age
        (char const*  start
        ,char const*& current
        ,int          line_num
        );

    // Helper of parse_values() parsing a single floating point value using the
    // exactly expected precision. Adjust the current pointer to advance past
    // the value parsed, the other parameters are only used for diagnostics.
    double parse_single_value
        (char const*  start
        ,char const*& current
        ,int          line_num
        );

    // Compute the expected number of values from minimum and maximum age
    // values and the select period and max select age if specified.
    //
    // Throws if minimum or maximum ares are not defined or are invalid.
    unsigned int get_expected_number_of_values() const;

    // Implementations of the public factory functions.
    void read_from_binary(std::istream& is, std::uint32_t offset);
    void read_from_text(std::istream& is);

    // Validate all the fields, throw an exception if any are invalid.
    //
    // After validation the following invariants hold:
    //  - number_ and type_ are valid, i.e. non-empty
    //  - min_age_ and max_age_ are valid and *(min_age_) <= *(max_age_)
    //  - values_ vector is non-empty
    //  - num_decimals_ is valid
    //  - select_period_ is valid iff type_ == select
    //  - if type_ == select, max_select_age_ is valid and otherwise
    //    max_select_age_ is either invalid of equal to max_age_
    void validate();

    // Write out the table contents in the given format.
    template<typename T>
    void do_write(std::ostream& os) const;

    // The values are not represented by std::optional<>, the emptiness of
    // the vector signals if we have any values or not.
    std::vector<double> values_;

    std::optional<std::string>
         name_
        ,contributor_
        ,data_source_
        ,data_volume_
        ,obs_period_
        ,unit_of_obs_
        ,construction_method_
        ,published_reference_
        ,comments_
        ;

    std::optional<std::uint32_t>
         number_
        ,hash_value_
        ;

    std::optional<std::uint16_t>
         num_decimals_
        ,min_age_
        ,max_age_
        ,select_period_
        ,max_select_age_
        ;

    std::optional<table_type>
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
    (bool           do_throw
    ,enum_soa_field field
    ,int            line_num = 0
    )
{
    if(do_throw)
        {
        alarum()
            << "duplicate occurrence of the field '"
            << soa_fields[field].name
            << "'"
            << location_info(line_num)
            << std::flush
            ;
        }
}

// Throw an error if the length of a field doesn't have the expected value.
void throw_if_unexpected_length
    (std::uint16_t  length
    ,std::size_t    expected_length
    ,enum_soa_field field
    )
{
    if(length != expected_length)
        {
        alarum()
            << "unexpected length " << length
            << " for the field '"
            << soa_fields[field].name
            << "', expected " << expected_length
            << std::flush
            ;
        }
}

// Throw an error indicating that some field is missing if the first argument
// is true.
template<typename T>
inline
void throw_if_missing_field(std::optional<T> const& o, enum_soa_field field)
{
    if(!o)
        {
        alarum()
            << "required field '"
            << soa_fields[field].name
            << "' was not specified"
            << std::flush
            ;
        }
}

} // Unnamed namespace.

void table_impl::read_string
        (std::optional<std::string>& ostr
        ,enum_soa_field              field
        ,std::istream&               ifs
        ,std::uint16_t               length
        )
{
    throw_if_duplicate_record(ostr.operator bool(), field);

    std::string str;
    str.resize(length);
    if(!stream_read(ifs, &str[0], length))
        {
        alarum()
            << "failed to read all " << length << " bytes of the field '"
            << soa_fields[field].name
            << "'"
            << std::flush
            ;
        }

    ostr = str;
}

template<typename T>
T table_impl::do_read_number(char const* name, std::istream& ifs)
{
    T num;
    if(!stream_read(ifs, &num, sizeof(T)))
        {
        alarum() << "failed to read field '" << name << "'" << std::flush;
        }

    return swap_bytes_if_big_endian(num);
}

void table_impl::read_type(std::istream& ifs, std::uint16_t length)
{
    throw_if_duplicate_record(type_.operator bool(), e_field_table_type);

    throw_if_unexpected_length(length, sizeof(std::uint8_t), e_field_table_type);

    auto const type
        = do_read_number<std::uint8_t>(soa_fields[e_field_table_type].name, ifs);
    switch(static_cast<table_type>(type))
        {
        case table_type::aggregate:
        case table_type::duration:
        case table_type::select:
            type_ = static_cast<table_type>(type);
            return;
        }

    alarum() << "unknown table type '" << type << "'" << std::flush;
}

template<typename T>
void table_impl::read_number
        (std::optional<T>& onum
        ,enum_soa_field    field
        ,std::istream&     ifs
        ,std::uint16_t     length
        )
{
    throw_if_duplicate_record(onum.operator bool(), field);

    throw_if_unexpected_length(length, sizeof(T), field);

    onum = do_read_number<T>(soa_fields[field].name, ifs);
}

void table_impl::read_number_before_values
        (std::optional<std::uint16_t>& onum
        ,enum_soa_field                field
        ,std::istream&                 ifs
        ,std::uint16_t                 length
        )
{
    if(!values_.empty())
        {
        alarum()
            << "field '"
            << soa_fields[field].name
            << "' must occur before the values"
            << std::flush
            ;
        }

    read_number(onum, field, ifs, length);
}

unsigned int table_impl::get_expected_number_of_values() const
{
    throw_if_missing_field(min_age_, e_field_min_age);
    throw_if_missing_field(max_age_, e_field_max_age);

    // Compute the expected number of values, checking the consistency of the
    // fields determining this as a side effect.
    if(*max_age_ < *min_age_)
        {
        alarum()
            << "minimum age " << *min_age_
            << " cannot be greater than the maximum age " << *max_age_
            << std::flush
            ;
        }

    // Start from one value per issue age, this is already the total number of
    // values for 1D tables.
    //
    // Considering that max age is a 16 bit number and int, used for
    // computations, is at least 32 bits, there is no possibility of integer
    // overflow here.
    unsigned int num_values = bourn_cast<unsigned int>(*max_age_ - *min_age_ + 1);

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
            alarum()
                << "select period " << *select_period_
                << " is too big for the age range "
                << *min_age_ << ".." << *max_age_
                << std::flush
                ;
            }

        // For 2D select-and-ultimate tables, this gives the number of values
        // in the ultimate column.
        num_values -= *select_period_;

        // In a further application of Postel's law, we consider non-specified
        // or 0 maximum select age as meaning "unlimited".
        unsigned int effective_max_select = max_select_age_.value_or(0);
        if(effective_max_select == 0)
            {
            effective_max_select = *max_age_;
            }

        unsigned int select_range = effective_max_select - *min_age_ + 1;

        // Maximum possible select_range value is 2^16 and multiplying it by
        // also 16 bit select_period_ still fits in a 32 bit unsigned int
        // value, so there is no risk of overflow here neither.
        select_range *= *select_period_;

        if(std::numeric_limits<unsigned int>::max() - num_values < select_range)
            {
            alarum()
                << "too many values in the table with maximum age " << *max_age_
                << ", select period " << *select_period_
                << " and maximum select age " << effective_max_select
                << std::flush
                ;
            }

        // No overflow due to the check above.
        num_values += select_range;
        }

    return num_values;
}

// The second argument (length) is unused, so why does it exist?
void table_impl::read_values(std::istream& ifs, std::uint16_t)
{
    throw_if_duplicate_record(!values_.empty(), e_field_values);

    // 2 byte length field can only represent values up to 2^16, i.e. only up
    // to 2^16/2^3 == 8192 double-sized elements, which is not enough for the
    // tables occurring in real-world. Because of this we don't trust the
    // length field from the file at all but deduce the number of values from
    // the previously specified age-related fields instead.
    unsigned int const num_values = get_expected_number_of_values();

    values_.resize(num_values);
    if(!stream_read(ifs, &values_[0], num_values * sizeof(double)))
        {
        alarum() << "failed to read the values" << std::flush;
        }

    for(auto& v : values_)
        {
        v = swap_bytes_if_big_endian(v);
        }
}

std::string* table_impl::parse_string
        (std::optional<std::string>& ostr
        ,enum_soa_field              field
        ,int                         line_num
        ,std::string const&          value
        )
{
    throw_if_duplicate_record(ostr.operator bool(), field, line_num);

    // With slight regret, allow the comments field to be empty because
    // some historical files have put commentary in table name instead.
    if(value.empty() && e_field_comments != field)
        {
        alarum()
            << "non-empty value must be specified for the field '"
            << soa_fields[field].name
            << "'"
            << location_info(line_num)
            << std::flush
            ;
        }

    ostr = value;

    return &ostr.operator *();
}

std::uint32_t table_impl::do_parse_number
        (enum_soa_field     field
        ,int                line_num
        ,std::uint32_t      max_num
        ,std::string const& value
        )
{
    auto const res = strict_parse_number(value.c_str());
    if(!res.end || *res.end != '\0')
        {
        alarum()
            << "value for numeric field '"
            << soa_fields[field].name
            << "' is not a number"
            << location_info(line_num)
            << std::flush
            ;
        }

    if(max_num < res.num)
        {
        alarum()
            << "value for numeric field '"
            << soa_fields[field].name
            << "' is out of range (maximum allowed is "
            << max_num
            << ")"
            << location_info(line_num)
            << std::flush
            ;
        }

    return static_cast<std::uint32_t>(res.num);
}

template<typename T>
void table_impl::parse_number
        (std::optional<T>&  onum
        ,enum_soa_field     field
        ,int                line_num
        ,std::string const& value
        )
{
    throw_if_duplicate_record(onum.operator bool(), field, line_num);

    onum = do_parse_number(field, line_num, std::numeric_limits<T>::max(), value);
}

void table_impl::parse_table_type
        (int                line_num
        ,std::string const& value
        )
{
    throw_if_duplicate_record(type_.operator bool(), e_field_table_type, line_num);

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
        alarum()
            << "invalid table type value '" << value << "'"
            << location_info(line_num)
            << " (\"" << table_type_as_string(table_type::aggregate) << "\", "
            << "\"" << table_type_as_string(table_type::duration) << "\" or "
            << "\"" << table_type_as_string(table_type::select) << "\" expected)"
            << std::flush
            ;
        }
}

void table_impl::parse_select_header(std::istream& is, int& line_num) const
{
    // There must be a header line in this case, as it's not used for anything,
    // don't perform strict checks, but still check that it has the expected
    // values.
    ++line_num;
    std::string line;
    if(!std::getline(is, line))
        {
        alarum()
            << "header expected for a select table"
            << location_info(line_num)
            << std::flush
            ;
        }

    std::istringstream iss(line);
    unsigned int actual;
    for(unsigned int expected = 1; iss >> actual; ++expected)
        {
        if(actual != expected)
            {
            alarum()
                << "expected duration " << expected
                << " and not " << actual
                << " in the select table header" << location_info(line_num)
                << std::flush
                ;
            }

        if(actual == *select_period_)
            {
            break;
            }
        }

    if(actual != *select_period_)
        {
        alarum()
            << "expected " << *select_period_
            << " duration labels and not " << actual
            << " in the select table header" << location_info(line_num)
            << std::flush
            ;
        }

    std::string header;
    iss >> header;
    if(!iss)
        {
        alarum()
            << "expected the ultimate column label \""
            << text_format::ultimate_header << "\""
            << " in the select table header" << location_info(line_num)
            << std::flush
            ;
        }

    if(header != text_format::ultimate_header)
        {
        alarum()
            << "expected the ultimate column label \""
            << text_format::ultimate_header << "\""
            << " and not \"" << header << "\""
            << " in the select table header" << location_info(line_num)
            << std::flush
            ;
        }
}

std::uint16_t table_impl::parse_age
    (char const*  start
    ,char const*& current
    ,int          line_num
    )
{
    using text_format::age_width;

    // We need to manually skip the leading whitespace as strict_parse_number()
    // doesn't accept it.
    auto start_num = current;
    while(*start_num == ' ')
        {
        if(start_num - current == age_width)
            {
            alarum()
                << "at most " << age_width - 1 << " spaces allowed"
                << location_info(line_num, current - start + 1)
                << std::flush
                ;
            }

        ++start_num;
        }

    auto const res_age = strict_parse_number(start_num);
    if(!res_age.end || (res_age.end - current != age_width))
        {
        alarum()
            << "expected a number with "
            << age_width - (start_num - current) << " digits"
            << location_info(line_num, start_num - start + 1)
            << std::flush
            ;
        }

    current = res_age.end;

    // There is no need to check for the range, we can't overflow std::uint16_t
    // with just 3 digits.
    return static_cast<std::uint16_t>(res_age.num);
}

double table_impl::parse_single_value
    (char const*  start
    ,char const*& current
    ,int          line_num
    )
{
    char const* origin = current;

    // The number of spaces before the value should be at least one,
    // and no greater than (gap_length, plus one if the number of
    // decimals is zero, because get_value_width() assumes, contrary
    // to fact, that a decimal point is written regardless).
    int const num_spaces_allowed =
          text_format::gap_length
        + (0 == *num_decimals_)
        ;
    if(*current != ' ')
        {
        alarum()
            << "expected a space"
            << location_info(line_num, current - start + 1)
            << std::flush
            ;
        }
    int num_spaces = 1;
    for(++current; *current == ' '; ++current)
        {
        ++num_spaces;
        }
    if(num_spaces_allowed < num_spaces)
        {
        alarum()
            << "too many spaces"
            << location_info(line_num, current - start + 1)
            << " (at most " << num_spaces_allowed << " allowed here)"
            << std::flush
            ;
        }

    // We can't impose the exact number of decimal digits using standard
    // functions for parsing floating point values, so do it manually.
    auto const res_int_part = strict_parse_number(current);
    if(!res_int_part.end)
        {
        alarum()
            << "expected a valid integer part"
            << location_info(line_num, current - start + 1)
            << std::flush
            ;
        }

    // Exit early if zero decimals.
    if(0 == *num_decimals_)
        {
        current = res_int_part.end;
        return bourn_cast<double>(res_int_part.num);
        }

    if(*res_int_part.end != '.')
        {
        alarum()
            << "expected decimal point"
            << location_info(line_num, res_int_part.end - start + 1)
            << std::flush
            ;
        }

    auto const res_frac_part = strict_parse_number(res_int_part.end + 1);
    if(!res_frac_part.end)
        {
        alarum()
            << "expected a valid fractional part"
            << location_info(line_num, res_frac_part.end - start + 1)
            << std::flush
            ;
        }

    if(res_frac_part.end - res_int_part.end - 1 != *num_decimals_)
        {
        alarum()
            << "expected " << *num_decimals_ << " decimal digits, not "
            << res_frac_part.end - res_int_part.end - 1
            << " in the value"
            << location_info(line_num)
            << std::flush
            ;
        }

    current = res_frac_part.end;

    return value_cast<double>(std::string(origin, current));
}

void table_impl::skip_spaces
    (int          num_spaces
    ,char const*  start
    ,char const*& current
    ,int          line_num
    )
{
    if(std::strncmp(current, std::string(bourn_cast<unsigned int>(num_spaces), ' ').c_str(), bourn_cast<unsigned int>(num_spaces)) != 0)
        {
        alarum()
            << "expected " << num_spaces << " spaces"
            << location_info(line_num, current - start + 1)
            << std::flush
            ;
        }

    current += num_spaces;
}

// This function parses a text representation of a select and ultimate table
// consisting of:
//
//  - A header with N column labels where N = *select_period_ + 1.
//  - A number of rows containing N values each.
//  - Optionally, a number of rows containing just one value in the last column.
//
// Additionally, each non header row contains the ages to which it applies on
// the left and right hand side, so the global structure of the table is:
//
//          0    1    2  Ult.
//    1   x_1  y_1  z_1  w_1      4
//    2   x_2  y_2  z_2  w_2      5
//    3   x_3  y_3  z_3  w_3      6
//    .............................
//    s   x_s  y_s  z_s  w_s    s+3
//  s+4                  w_s+1  s+4
//  s+5                  w_s+2  s+5
//    .............................
//    m   x_m  y_m  z_m  w_m      m
//
// where "s" is the max select age and "m" is the max age (min age here is 1).
void table_impl::parse_values(std::istream& is, int& line_num)
{
    unsigned int const num_values = get_expected_number_of_values();
    values_.reserve(num_values);

    if(!num_decimals_)
        {
        alarum()
            << "the '" << soa_fields[e_field_num_decimals].name << "' field "
            << "must be specified before the table values"
            << location_info(line_num)
            << std::flush
            ;
        }

    if(!type_)
        {
        alarum()
            << "table type must occur before its values"
            << location_info(line_num)
            << std::flush
            ;
        }

    // Initialize this variable using a lambda with a switch inside just to
    // make sure this code gets updated if any new table types are added.
    auto const is_select_table = [this]() {
        switch(*type_) {
            case table_type::aggregate:
            case table_type::duration:
                break;

            case table_type::select:
                return true;
        }

        return false;
    }();

    if(is_select_table)
        {
        parse_select_header(is, line_num);
        }

    for(auto age = *min_age_; age <= *max_age_; ++age)
        {
        std::string line;
        if(!std::getline(is, line))
            {
            // Complain about premature input end.
            alarum()
                << "table values for age " << age
                << " are missing" << location_info(line_num)
                << std::flush
                ;
            }
        ++line_num;

        auto const start = line.c_str();
        auto current = start;

        auto const actual_age = parse_age(start, current, line_num);
        if(actual_age != age)
            {
            // Distinguish select age at the beginning of the line from the
            // ultimate age on the right side of the table.
            alarum()
                << "incorrect "
                << (is_select_table ? "select " : "")
                << "age value " << actual_age
                << location_info(line_num)
                << " (" << age << " expected)"
                << std::flush
                ;
            }

        if(is_select_table)
            {
            if(age <= *max_select_age_)
                {
                // We are still in 2D part of the table
                for(std::uint16_t d = 0; d < *select_period_; ++d)
                    {
                    values_.push_back(parse_single_value(start, current, line_num));
                    }
                }
            else
                {
                // After the max select age only the last column remains, just
                // skip the spaces until it.
                skip_spaces
                    (*select_period_ * text_format::get_value_width(*num_decimals_)
                    ,start
                    ,current
                    ,line_num
                    );
                }
            }

        values_.push_back(parse_single_value(start, current, line_num));

        if(is_select_table)
            {
            skip_spaces(text_format::gap_length, start, current, line_num);

            auto const expected_age = age <= *max_select_age_
                ? age + *select_period_
                : age
                ;

            auto const ultimate_age = parse_age(start, current, line_num);
            if(ultimate_age != expected_age)
                {
                alarum()
                    << "incorrect ultimate age value " << ultimate_age
                    << location_info(line_num)
                    << " (" << expected_age << " expected)"
                    << std::flush
                    ;
                }
            }

        if(current - start < static_cast<int>(line.length()))
            {
            alarum()
                << "unexpected characters \"" << current << "\""
                << location_info(line_num, current - start + 1)
                << std::flush
                ;
            }

        if(is_select_table)
            {
            if(age == *max_select_age_)
                {
                // There is a jump in ages when switching from the 2D to 1D
                // part of the select and ultimate table after the select age.
                age = bourn_cast<std::uint16_t>(age + *select_period_);
                }
            }
        }
}

void table_impl::validate()
{
    // Check for the number first as we use it to construct a more detailed
    // error message below.
    throw_if_missing_field(number_, e_field_table_number);

    try
        {
        // All tables must define their type.
        throw_if_missing_field(type_, e_field_table_type);

        // Check that we have the values: this also ensures that we have the
        // correct minimum and maximum age as this is verified when filling in
        // the values.
        if(values_.empty())
            {
            alarum() << "no values defined" << std::flush;
            }

        // Validate the type and check that the select period has or hasn't
        // been given, depending on it.
        switch(*type_)
            {
            case table_type::aggregate:
            case table_type::duration:
                if(select_period_.value_or(0))
                    {
                    alarum()
                        << "select period cannot be specified for a table "
                        << "of type '" << table_type_as_string(*type_) << "'"
                        << std::flush
                        ;
                    }
                if(  max_select_age_.value_or(0)
                  && *max_select_age_ != *max_age_
                  )
                    {
                    alarum()
                        << "maximum select age " << *max_select_age_
                        << " different from the maximum age " << *max_age_
                        << " cannot be specified for a table of type '"
                        << table_type_as_string(*type_) << "'"
                        << std::flush
                        ;
                    }
                break;

            case table_type::select:
                if(!select_period_.value_or(0))
                    {
                    alarum()
                        << "select period must be specified "
                        << "for a select and ultimate table"
                        << std::flush
                        ;
                    }
                if(!max_select_age_.value_or(0))
                    {
                    alarum()
                        << "maximum select age must be specified "
                        << "for a select and ultimate table"
                        << std::flush
                        ;
                    }
                break;
            }

        if(!num_decimals_)
            {
            alarum() << "Number of decimals not specified." << LMI_FLUSH;
            }

        std::uint16_t putative_num_decimals = *num_decimals_;
        std::uint16_t required_num_decimals = bourn_cast<std::uint16_t>
            (deduce_number_of_decimals(values_)
            );
        // This condition is true only if the table is defective,
        // which should occur rarely enough that the cost of
        // recalculating the hash value both here and below
        // doesn't matter.
        if(putative_num_decimals != required_num_decimals)
            {
            warning()
                << "Table #" << *number_
                << " specifies " << putative_num_decimals
                << " decimals, but " << required_num_decimals
                << " were necessary."
                << "\nThis flaw has been corrected, and the CRC recalculated."
                << LMI_FLUSH
                ;
            *num_decimals_ = required_num_decimals;
            *hash_value_ = compute_hash_value();
            }

        // If we don't have the hash, compute it ourselves. If we do, check
        // that it corresponds to what we should have unless the hash value in
        // input is just 0 which is equivalent to being not specified (such
        // hashes are generated by the "--squeeze" option of the legacy
        // table_utilities program, so we support them for compatibility).
        auto const correct_hash_value = compute_hash_value();
        if(hash_value_ && *hash_value_)
            {
            if(*hash_value_ != correct_hash_value)
                {
                alarum()
                    << "hash value " << *hash_value_ << " doesn't match "
                    << "the computed hash value " << correct_hash_value
                    << std::flush
                    ;
                }
            }
        else
            {
            hash_value_ = correct_hash_value;
            }
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "bad data for table " << *number_ << ": "
            << e.what()
            << std::flush
            ;
        }
}

void table_impl::read_from_binary(std::istream& ifs, std::uint32_t offset)
{
    ifs.seekg(offset, std::ios::beg);
    if(!ifs)
        {
        alarum() << "seek error" << std::flush;
        }

    for(;;)
        {
        std::uint16_t const
            record_type = do_read_number<std::uint16_t>("record type", ifs);

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

        std::uint16_t const
            length = do_read_number<std::uint16_t>("record length", ifs);

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
                alarum() << "unknown field type " << record_type << std::flush;
            }
        }
}

std::shared_ptr<table_impl> table_impl::create_from_binary
    (std::istream& is
    ,std::uint32_t offset
    )
{
    std::shared_ptr<table_impl> table = std::make_shared<table_impl>();
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
        static char const* const whitespace = " \t";

        // Discard trailing whitespace, it is insignificant and would just
        // complicate the checks below.
        auto const end_line = line.find_last_not_of(whitespace);
        if(end_line == std::string::npos)
            {
            // Blank line, we only accept (and ignore) them after the end of
            // the input, so check that nothing more is left.
            int const blank_line_num = line_num;
            for(++line_num; std::getline(is, line); ++line_num)
                {
                if(line.find_first_not_of(whitespace) != std::string::npos)
                    {
                    alarum()
                        << "blank line " << blank_line_num << " "
                        << "cannot appear in the middle of the input "
                        << "and be followed by non-blank line " << line_num
                        << std::flush
                        ;
                    }
                }
            break;
            }

        auto const fv = parse_field_and_value(line, line_num, number_);
        if(fv)
            {
            // Just to avoid using "fv->" everywhere.
            auto const field = fv->field;
            auto const& value = fv->value;

            // Only one field can appear after the table values.
            if(!values_.empty() && field != e_field_hash_value)
                {
                alarum()
                    << "field '" << soa_fields[field].name << "' "
                    << "is not allowed after the table values"
                    << location_info(line_num)
                    << std::flush
                    ;
                }

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
                    parse_values(is, line_num);
                    break;
                case e_field_hash_value:
                    if(values_.empty())
                        {
                        alarum()
                            << "'" << soa_fields[field].name << "' field "
                            << "is only allowed after the table values and not "
                            << location_info(line_num)
                            << std::flush
                            ;
                        }

                    parse_number(hash_value_, field, line_num, value);
                    break;
                }
            }
        else // This line isn't of the form "field: value".
            {
            // Must be a continuation of the previous line.
            if(!last_string)
                {
                alarum()
                    << "expected a field name followed by a colon"
                    << location_info(line_num)
                    << std::flush
                    ;
                }

            *last_string += '\n';
            *last_string += line;

            // Do not change last_string, more continuation lines can follow.
            }
        }

    // Verify that all the required fields have been specified.
    validate();
}

std::shared_ptr<table_impl> table_impl::create_from_text(std::istream& is)
{
    std::shared_ptr<table_impl> table = std::make_shared<table_impl>();
    table->read_from_text(is);
    return table;
}

template<typename T>
void table_impl::do_write(std::ostream& os) const
{
    try
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
    catch(std::runtime_error const& e)
        {
        alarum()
            << "saving table " << *number_ << "failed: " << e.what()
            << std::flush
            ;
        }
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

std::uint32_t table_impl::compute_hash_value() const
{
    // This is a bug-for-bug reimplementation of the hash value computation
    // algorithm used in the original SOA format which produces compatible
    // (even if nonsensical) hash values.
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(3) << *min_age_
        << std::setw(3) << *max_age_
        << std::setw(3) << select_period_.value_or(0)
        << std::setw(3) << max_select_age_.value_or(0)
        ;

    oss << std::fixed << std::setprecision(*num_decimals_);
    unsigned int const value_width = bourn_cast<unsigned int>(*num_decimals_ + 2);

    for(auto const& v : values_)
        {
        oss << std::setw(bourn_cast<int>(value_width)) << v;
        }

    std::string s = oss.str();

    // Truncate the string for compatibility with the original code.
    s.resize(values_.size() * value_width);

    CRC crc;
    crc += s;

    // Undo the last XOR, again for compatibility.
    return crc.value() ^ 0xffffffffU;
}

table table::read_from_text(fs::path const& file)
{
    fs::ifstream ifs(file, ios_in_binary());
    if(!ifs) alarum() << "Unable to open '" << file << "'." << LMI_FLUSH;

    try
        {
        return table(table_impl::create_from_text(ifs));
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error reading table from file '" << file << "': "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

table table::read_from_text(std::string const& text)
{
    try
        {
        std::istringstream iss(text);
        return table(table_impl::create_from_text(iss));
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error reading table from string: "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void table::save_as_text(fs::path const& file) const
{
    fs::ofstream ofs(file, ios_out_trunc_binary());
    if(!ofs) alarum() << "Unable to open '" << file << "'." << LMI_FLUSH;
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
    return table::Number(bourn_cast<int>(impl_->number()));
}

std::string const& table::name() const
{
    return impl_->name();
}

std::uint32_t table::compute_hash_value() const
{
    return impl_->compute_hash_value();
}

bool table::operator==(table const& other) const
{
    return impl_->is_equal(*other.impl_);
}

class database_impl final
{
  public:
    static fs::path get_index_path(fs::path const& path)
        {
        return fs::change_extension(path, ".ndx");
        }

    static fs::path get_data_path(fs::path const& path)
        {
        return fs::change_extension(path, ".dat");
        }

    explicit database_impl(fs::path const& path);
    database_impl(std::istream& index_is, std::shared_ptr<std::istream> data_is);

    int tables_count() const;
    table get_nth_table(int idx) const;
    table find_table(table::Number number) const;
    void append_table(table const& table);
    void add_or_replace_table(table const& table);
    void delete_table(table::Number number);
    void save(fs::path const& path);
    void save(std::ostream& index_os, std::ostream& data_os);

  private:
    database_impl(database_impl const&) = delete;
    database_impl& operator=(database_impl const&) = delete;

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

    void read_index(std::istream& index_is);

    // We don't currently use the name stored in the index, so this struct
    // doesn't include it.
    struct IndexEntry
    {
        IndexEntry
            (table::Number               number
            ,std::uint32_t               offset
            ,std::shared_ptr<table_impl> table
            )
            :number_ {bourn_cast<std::uint32_t>(number.value())}
            ,offset_ {offset}
            ,table_  {table}
        {
        }

        std::uint32_t number_;

        // This field is ignored for the tables added to the database after
        // reading the original index file and is only used for loading the
        // existing tables from the original input file.
        std::uint32_t offset_;

        // table pointer may be empty for the tables present in the input
        // database file but not loaded yet.
        mutable std::shared_ptr<table_impl> table_;
    };

    // Add an entry to the index. This function should be always used instead of
    // updating index_ vector directly as it also takes care of updating
    // index_by_number_ map.
    //
    // Returns false if there was already a table with the given number, this
    // is not supposed to happen and should be treated as an error by caller.
    bool add_index_entry
        (table::Number               number
        ,std::uint32_t               offset
        ,std::shared_ptr<table_impl> table = std::shared_ptr<table_impl>()
        );

    // Remove the entry for the table with the given number from the index.
    // Also updates index_by_number_ map.
    //
    // Throws if there is no table with this number.
    void remove_index_entry(table::Number number);

    // Return the table corresponding to the given index entry, loading it from
    // the database file if this hadn't been done yet.
    std::shared_ptr<table_impl> do_get_table_impl(IndexEntry const& entry) const;
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
    typedef std::map<table::Number, unsigned int> NumberToIndexMap;
    NumberToIndexMap index_by_number_;

    // Path to the database, used only for the error messages.
    fs::path const path_;

    // The open database file: we keep it open to read table data on demand
    // from it.
    //
    // An alternative approach could be to just load everything into memory at
    // once.
    //
    // Notice that this pointer may be null if we don't have any input file or
    // if we had it but closed it because we didn't need it any more after
    // loading everything from it.
    std::shared_ptr<std::istream> data_is_;
};

database_impl::database_impl(fs::path const& path)
    :path_ {path}
{
    if(path_.empty())
        {
        // This ctor can be explicitly used with an empty path to create a
        // database not (yet) associated with any physical file.
        return;
        }

    fs::path const index_path = get_index_path(path);
    fs::ifstream ifs(index_path, ios_in_binary());
    if(!ifs) alarum() << "Unable to open '" << index_path << "'." << LMI_FLUSH;
    read_index(ifs);

    // Open the database file right now to ensure that we can do it, even if we
    // don't need it just yet. As it will be used soon anyhow, delaying opening
    // it wouldn't be a useful optimization.
    fs::path const data_path = get_data_path(path);
    auto const pifs = std::make_shared<fs::ifstream>(data_path, ios_in_binary());
    if(!*pifs) alarum() << "Unable to open '" << data_path << "'." << LMI_FLUSH;
    data_is_ = pifs;
}

database_impl::database_impl
    (std::istream&                 index_is
    ,std::shared_ptr<std::istream> data_is
    )
    :data_is_ {data_is}
{
    read_index(index_is);
}

bool database_impl::add_index_entry
    (table::Number               number
    ,std::uint32_t               offset
    ,std::shared_ptr<table_impl> table
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

void database_impl::remove_index_entry(table::Number number)
{
    auto const it = index_by_number_.find(number);

    if(it == index_by_number_.end())
        {
        std::ostringstream oss;
        oss << "Failed to delete table number " << number << ": not found.";
        throw std::invalid_argument(oss.str());
        }

    // Remove the entry corresponding to this table from both the index and the
    // lookup map.
    auto const index_deleted = it->second;
    index_.erase(index_.begin() + bourn_cast<int>(index_deleted));
    index_by_number_.erase(it);

    // But also update the remaining lookup map indices.
    for(auto& e : index_by_number_)
        {
        if(index_deleted < e.second)
            {
            --e.second;
            }
        }
}

void database_impl::read_index(std::istream& index_is)
{
    char index_record[e_index_pos_max] = {0};

    for(;;)
        {
        if(!stream_read(index_is, index_record, e_index_pos_max))
            {
            if(index_is.eof() && !index_is.gcount())
                {
                break;
                }

            alarum()
                << "error reading entry " << index_.size()
                << " from the database index"
                << std::flush
                ;
            }

        std::uint32_t const
            number = from_bytes<std::uint32_t>(&index_record[e_index_pos_number]);
        std::uint32_t const
            offset = from_bytes<std::uint32_t>(&index_record[e_index_pos_offset]);

        // Check that the cast to int below is safe.
        if(static_cast<unsigned int>(std::numeric_limits<int>::max()) <= number)
            {
            alarum()
                << "database index is corrupt: "
                << "table number " << number << " is out of range"
                << std::flush
                ;
            }

        if(!add_index_entry(table::Number(static_cast<int>(number)), offset))
            {
            alarum()
                << "database index is corrupt: "
                << "duplicate entries for the table number " << number
                << std::flush
                ;
            }
        }
}

int database_impl::tables_count() const
{
    return static_cast<int>(index_.size());
}

table database_impl::get_nth_table(int idx) const
{
    return do_get_table(index_.at(bourn_cast<unsigned int>(idx)));
}

std::shared_ptr<table_impl> database_impl::do_get_table_impl
    (IndexEntry const& entry
    ) const
{
    if(!entry.table_)
        {
        try
            {
            entry.table_ = table_impl::create_from_binary
                (*data_is_
                ,entry.offset_
                );
            }
        catch(std::runtime_error const& e)
            {
            alarum()
                << "error reading table " << entry.number_
                << " from the offset " << entry.offset_
                << " in the database '" << path_ << "': " << e.what()
                << std::flush
                ;
            }

        if(entry.table_->number() != entry.number_)
            {
            alarum()
                << "database '" << path_ << "' is corrupt: "
                << "table number " << entry.table_->number()
                << " is inconsistent with its number in the index ("
                << entry.number_ << ")"
                << std::flush
                ;
            }
        }

    return entry.table_;
}

database_impl::IndexEntry* database_impl::do_find_table_entry(table::Number number)
{
    NumberToIndexMap::const_iterator const ci = index_by_number_.find(number);

    return ci == index_by_number_.end() ? nullptr : &index_.at(ci->second);
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

void database_impl::delete_table(table::Number number)
{
    remove_index_entry(number);
}

void database_impl::save(fs::path const& path)
{
    // This class ensures that we either overwrite both the output .ndx and
    // .dat files or don't change either of them if an error happens (unless a
    // catastrophic failure prevents us from renaming the backup index file
    // back to its original name after the first renaming of the index
    // succeeded but the renaming of the data file failed -- but there is
    // nothing we can do about this without some kind of OS support).
    class safe_database_output
    {
      public:
        // Try to set up things for saving a database to the given path, throws
        // on failure.
        explicit safe_database_output(fs::path const& path)
            :path_     {path}
            ,index_    {path, "index"   , ".ndx"}
            ,database_ {path, "database", ".dat"}
            {
            }

        safe_database_output(safe_database_output const&) = delete;
        safe_database_output& operator=(safe_database_output const&) = delete;

        // Accessors for the stream to be used for saving the database.
        std::ostream& index() { return index_.ofs_; }
        std::ostream& database() { return database_.ofs_; }

        // The core of this class functionality is in this function: it tries to
        // atomically rename the files to the real output path and throws,
        // without changing the (possibly) existing file at the given path, on
        // failure.
        void close()
            {
            // It's more convenient to just append error information to this
            // stream as errors happen, so, pessimistically, start by assuming
            // that an error will happen -- if it doesn't, we'll just never use
            // this stream.
            std::ostringstream error_stream;
            error_stream << "writing database data to '" << path_ << "' failed";

            bool keep_temp_index_file = false;
            try
                {
                // First close the output files to make [as] sure [as we can]
                // that everything is written to the disk.
                index_.close();
                database_.close();

                fs::path index_backup;
                if(index_.uses_temp_file())
                    {
                    // Make a backup copy of the index to be able to restore it
                    // later if renaming the data file fails.
                    index_backup = unique_filepath(path_, ".ndx.backup");
                    fs::rename(index_.path_, index_backup);
                    }

                // And put the new version of the index in place.
                try
                    {
                    index_.rename_if_needed();
                    }
                catch(...)
                    {
                    // We don't need the backup, if rename() failed, the
                    // original file must have been left in place anyhow.
                    if(!index_backup.empty())
                        {
                        // Ensure that index_backup is empty, so that we don't
                        // tell the user to restore it manually below.
                        fs::path z;
                        std::swap(index_backup, z);

                        remove_nothrow(z);
                        }

                    throw;
                    }

                // Now put the database file in place too.
                try
                    {
                    database_.rename_if_needed();
                    }
                catch(...)
                    {
                    // Undo the index renaming if it had been done.
                    if(index_backup.empty())
                        {
                        if(index_.uses_temp_file())
                            {
                            remove_nothrow(index_.temp_path_);
                            }
                        }
                    else
                        {
                        try
                            {
                            fs::remove(index_.path_);
                            fs::rename(index_backup, index_.path_);

                            index_backup = fs::path();
                            }
                        catch(...)
                            {
                            // This is imperfect, but the best we can do and
                            // hopefully the user will be able to restore the
                            // original index file contents.
                            error_stream
                                << " but the file \"" << index_.path_ << "\""
                                << " had been modified and this modification"
                                << " could not be undone, please manually"
                                << " restore the original file from \""
                                << index_.temp_path_ << "\""
                                ;

                            keep_temp_index_file = true;
                            }
                        }

                    throw;
                    }

                if(!index_backup.empty())
                    {
                    // Even if we can't remove the index backup for some
                    // reason, don't fail, this is not really an error as the
                    // database was saved successfully.
                    remove_nothrow(index_backup);
                    }

                // Skip the error below.
                return;
                }
            catch(std::runtime_error const& e)
                {
                error_stream << " (" << e.what() << ")";
                }

            if(!keep_temp_index_file)
                {
                index_.cleanup_temp();
                }

            database_.cleanup_temp();

            alarum() << error_stream.str() << std::flush;
            }

      private:
        fs::path const& path_;

        // This struct collects the final output path for a file, a possibly
        // (but not necessarily) different temporary output path and the stream
        // opened on the latter.
        struct safe_output_file
        {
            safe_output_file
                (fs::path const& path
                ,char     const* description
                ,char     const* extension
                )
                :path_ {fs::change_extension(path, extension)}
                ,temp_path_
                    {fs::exists(path_)
                        ? unique_filepath(path_, extension + std::string(".tmp"))
                        : path_
                    }
                ,description_ {description}
            {
            ofs_.open(temp_path_, ios_out_trunc_binary());
            if(!ofs_) alarum() << "Unable to open '" << temp_path_ << "'." << LMI_FLUSH;
            }

            safe_output_file(safe_output_file const&) = delete;
            safe_output_file& operator=(safe_output_file const&) = delete;

            void close()
                {
                ofs_.close();
                if(!ofs_)
                    {
                    alarum()
                        << "failed to close the output " << description_
                        << " file \"" << temp_path_ << "\""
                        << std::flush
                        ;
                    }
                }

            bool uses_temp_file() const
                {
                return temp_path_ != path_;
                }

            void rename_if_needed()
                {
                if(uses_temp_file())
                    {
                    fs::remove(path_);
                    fs::rename(temp_path_, path_);
                    }
                }

            void cleanup_temp()
                {
                if(uses_temp_file())
                    {
                    remove_nothrow(temp_path_);
                    }
                }

            fs::path const path_;
            fs::path const temp_path_;
            char const* description_;
            fs::ofstream ofs_;
        };

        safe_output_file index_;
        safe_output_file database_;
    };

    safe_database_output output(path);

    save(output.index(), output.database());

    // Before closing the output, which will ensure that it is really written
    // to the files with the specified path, close our input stream because we
    // won't ever need it any more, as we just read all the tables in the loop
    // above, so it's useless to keep it open. But even more importantly, this
    // will allow us to write to the same database file we had been reading
    // from until now, which would fail otherwise because the file would be in
    // use.
    data_is_.reset();

    output.close();
}

void database_impl::save(std::ostream& index_os, std::ostream& data_os)
{
    char index_record[e_index_pos_max] = {0};

    for(auto const& i : index_)
        {
        std::shared_ptr<table_impl> const t = do_get_table_impl(i);

        // The offset of this table is just the current position of the output
        // stream, so get it before it changes and check that it is still
        // representable as a 4 byte offset (i.e. the file is less than 4GiB).
        std::streamoff const offset = data_os.tellp();
        std::uint32_t const offset32 = static_cast<std::uint32_t>(offset);
        if(static_cast<std::streamoff>(offset32) != offset)
            {
            alarum()
                << "database is too large to be stored in SOA v3 format."
                << std::flush
                ;
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

        stream_write(index_os, index_record, sizeof(index_record));

        t->write_as_binary(data_os);
        }
}

bool database::exists(fs::path const& path)
{
    // Normally either both files exist or none of them does, but we still
    // return true even if just one of them exists, as we don't want the
    // caller, who may decide to create a new database if none exists yet, to
    // overwrite the existing file inadvertently.
    return fs::exists(database_impl::get_index_path(path))
        || fs::exists(database_impl::get_data_path(path))
        ;
}

database::database()
    :impl_ {::new database_impl(fs::path())}
{
}

database::database(fs::path const& path)
try
    :impl_ {::new database_impl(path)}
{
}
catch(std::runtime_error const& e)
{
    alarum()
        << "Error reading database from '" << path << "': "
        << e.what()
        << "."
        << LMI_FLUSH
        ;
}

database::database
    (std::istream&                 index_is
    ,std::shared_ptr<std::istream> data_is
    )
try
    :impl_ {::new database_impl(index_is, data_is)}
{
}
catch(std::runtime_error const& e)
{
    alarum()
        << "Error reading database: "
        << e.what()
        << "."
        << LMI_FLUSH
        ;
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
    try
        {
        return impl_->get_nth_table(idx);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error getting table at index " << idx << ": "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

table database::find_table(table::Number number) const
{
    try
        {
        return impl_->find_table(number);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error getting table with number " << number << ": "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void database::append_table(table const& table)
{
    try
        {
        return impl_->append_table(table);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error appending table number " << table.number()
            << " to the database: "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        }
}

void database::add_or_replace_table(table const& table)
{
    try
        {
        return impl_->add_or_replace_table(table);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error adding table number " << table.number() << ": "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        }
}

void database::delete_table(table::Number number)
{
    try
        {
        return impl_->delete_table(number);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error deleting table number " << number << ": "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        }
}

void database::save(fs::path const& path)
{
    try
        {
        return impl_->save(path);
        }
    catch(std::runtime_error const& e)
        {
        alarum()
            << "Error saving database to '" << path << "': "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        }
}

void database::save(std::ostream& index_os, std::ostream& data_os)
{
    try
        {
        return impl_->save(index_os, data_os);
        }
    catch(std::runtime_error const& e)
        {
        // We can't really provide any extra information here, but still do it
        // just for consistency with save() above.
        alarum()
            << "Error saving database to: "
            << e.what()
            << "."
            << LMI_FLUSH
            ;
        }
}

std::ostream& operator<<(std::ostream& os, table::Number const& number)
{
    os << number.value();
    return os;
}

} // namespace soa_v3_format

/// Infer the decimal precision of a rounded decimal-formatted number.

std::size_t deduce_number_of_decimals(std::string const& arg)
{
    // Early exit: no decimal point means zero decimals.
    if(std::string::npos == arg.find('.'))
        {
        return 0;
        }

    std::string s(arg);
    std::size_t d = 0;

    // Strip leading blanks and zeros.
    std::string::size_type q = s.find_first_not_of(" 0");
    if(std::string::npos != q)
        {
        s.erase(0, q);
        }

    // Strip trailing blanks.
    std::string::size_type r = s.find_last_not_of(" ");
    if(std::string::npos != r)
        {
        s.erase(1 + r);
        }

    // Preliminary result is number of characters after '.'.
    // (Decrement for '.' unless nothing followed it.)
    d = s.size() - s.find('.');
    if(d) --d;

    // Length of stripped string is number of significant digits
    // (on both sides of the decimal point) plus one for the '.'.
    // If this total exceeds 15--i.e., if there are more than 14
    // significant digits--then there may be excess precision.
    // In that case, keep only the first 15 digits (plus the '.',
    // for a total of 16 characters), because those digits are
    // guaranteed to be significant for IEEE754 double precision;
    // drop the rest, which may include arbitrary digits. Then
    // drop any trailing string that's all zeros or nines, and
    // return the length of the remaining string. This wrongly
    // truncates a number whose representation requires 15 or 16
    // digits when the last one or more decimal digit is a nine,
    // but that doesn't matter for the present use case: rate
    // tables aren't expected to have more than about eight
    // decimal places; and this function will be called for each
    // number in a table and the maximum result used, so that
    // such incorrect truncation can only occur if every number
    // in the table is ill-conditioned in this way.
    if(15 < s.size())
        {
        s.resize(16);
        if('0' == s.back() || '9' == s.back())
            {
            d = s.find_last_not_of(s.back()) - s.find('.');
            }
        }

    return d;
}

/// Infer the decimal precision of a decimally-rounded vector<double>.
///
/// Motivation: Some historical tables were stored only in the binary
/// format. (Of course, no one wrote that by hand; text input surely
/// was written first, but was not preserved.) The number of decimals
/// implicit in the data values may defectively be inconsistent with
/// the "Number of decimal places" header, and must be deduced. It is
/// determined here as the greatest number of decimals required for
/// any value datum, so that converting to text with that precision
/// is lossless.

std::size_t deduce_number_of_decimals(std::vector<double> const& values)
{
    std::size_t z = 0;
    for(auto const& v : values)
        {
        z = std::max(z, deduce_number_of_decimals(value_cast<std::string>(v)));
        }
    return z;
}
