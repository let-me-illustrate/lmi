// Tools for working with SOA tables represented in binary format.
//
// Copyright (C) 2015 Gregory W. Chicares.
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

#include <boost/cstdint.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <limits>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <utility>      // make_pair()
#include <vector>

using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

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

// Defining this NOP function allows to call from_little_endian() in template
// code for numbers of any size.
inline
uint8_t from_little_endian(uint8_t val)
{
    return val;
}

// We rely on makefile defining WORDS_BIGENDIAN on big endian architectures,
// conversions from little endian format are only needed there and are trivial
// on little endian machines.
#ifdef WORDS_BIGENDIAN
inline
uint16_t from_little_endian(uint16_t val)
{
    return ((val & 0x00ffU) << 8)
         | ((val & 0xff00U) >> 8)
         ;
}

inline
uint32_t from_little_endian(uint32_t val)
{
    return ((val & 0x000000ffU) << 24)
         | ((val & 0x0000ff00U) <<  8)
         | ((val & 0x00ff0000U) >>  8)
         | ((val & 0xff000000U) >> 24)
         ;
}

inline
uint64_t from_little_endian(uint64_t val)
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
double from_little_endian(double val)
{
    // When using IEEE 754 (as checked in the beginning of this file), we can
    // treat a double value as a 64 bit integer.
    uint64_t const ui64 = from_little_endian(*reinterpret_cast<uint64_t*>(&val));

    // And vice versa.
    return *reinterpret_cast<double*>(&ui64);
}
#else // !WORDS_BIGENDIAN
inline
uint16_t from_little_endian(uint16_t val)
{
    return val;
}

inline
uint32_t from_little_endian(uint32_t val)
{
    return val;
}

inline
double from_little_endian(double val)
{
    return val;
}
#endif // WORDS_BIGENDIAN/!WORDS_BIGENDIAN

template<typename T>
inline
T from_bytes(char const* bytes)
{
    return from_little_endian(*reinterpret_cast<T const*>(bytes));
}

// BOOST !! Replace the use of this function with member value_or() present in
// the later Boost.Optional versions.
template<typename T, typename U>
inline
T get_value_or(boost::optional<T> const& o, U v)
{
    return o ? *o : v;
}

} // anonymous namespace

namespace soa_binary_format
{

class table_impl
    :        private lmi::uncopyable <table_impl>
    ,virtual private obstruct_slicing<table_impl>
{
  public:
    // Load the table data from the specified offset of the given stream.
    //
    // Throws std::runtime_error on error.
    table_impl(std::ifstream& ifs, uint32_t offset);

    void save_as_text(fs::path const& file) const;
    void name(std::string const& name) { name_ = name; }
    table::Number number() const { return table::Number(*number_); }
    std::string const& name() const { return *name_; }
    unsigned long compute_hash_value() const;

  private:
    // All field types in the binary SOA format.
    enum
        {e_field_table_name          =  1
        ,e_field_table_number        =  2
        ,e_field_table_type          =  3
        ,e_field_contributor         =  4
        ,e_field_data_source         =  5
        ,e_field_data_volume         =  6
        ,e_field_obs_period          =  7
        ,e_field_unit_of_obs         =  8
        ,e_field_construction_method =  9
        ,e_field_published_reference = 10
        ,e_field_comments            = 11
        ,e_field_min_age             = 12
        ,e_field_max_age             = 13
        ,e_field_select_period       = 14
        ,e_field_max_select_age      = 15
        ,e_field_num_decimals        = 16
        ,e_field_values              = 17
        ,e_field_hash_value          = 18
        ,e_field_end_table           = 9999
        };

    // Helper methods for IO: all of them throw std::runtime_error on failure
    // and mention the field name in the error message.
    //
    // They also check that the optional value provided as the output parameter
    // for reading data into is not initialized yet as it's an error to have
    // duplicate fields in our format.

    static
    void read_string
            (boost::optional<std::string>& ostr
            ,char const* name
            ,std::ifstream& ifs
            ,uint16_t length
            );

    template<typename T>
    static
    T do_read_number(char const* name, std::ifstream& ifs);

    template<typename T>
    static
    void read_number
            (boost::optional<T>& onum
            ,char const* name
            ,std::ifstream& ifs
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
            ,char const* name
            ,std::ifstream& ifs
            ,uint16_t length
            );

    // This one is different from the generic methods above as it's only used
    // for the specific values_ field and not any arbitrary vector.
    void read_values(std::ifstream& ifs, uint16_t length);

    // Compute the expected number of values from minimum and maximum age
    // values and the select period and max select age if specified.
    //
    // Throws if minimum or maximum ares are not defined or are invalid.
    unsigned get_expected_number_of_values() const;

    // Validate all the fields, throw an exception if any are invalid.
    void validate();

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

    boost::optional<uint8_t>
        type_;
};

namespace
{

// Throw an error indicating duplicate occurrence of some field if the first
// argument is true.
inline
void throw_if_duplicate_field(bool do_throw, char const* name)
{
    if(do_throw)
        {
        std::ostringstream oss;
        oss << "duplicate occurrence of the field '" << name << "'";
        throw std::runtime_error(oss.str());
        }
}

// Throw an error indicating that some field is missing if the first argument
// is true.
template<typename T>
inline
void throw_if_missing_field(boost::optional<T> const& o, char const* name)
{
    if(!o)
        {
        std::ostringstream oss;
        oss << "required field '" << name << "' was not specified";
        throw std::runtime_error(oss.str());
        }
}

} // anonymous namespace

void table_impl::read_string
        (boost::optional<std::string>& ostr
        ,char const* name
        ,std::ifstream& ifs
        ,uint16_t length
        )
{
    throw_if_duplicate_field(ostr.is_initialized(), name);

    std::string str;
    str.resize(length);
    ifs.read(&str[0], length);
    if(ifs.gcount() != length)
        {
        std::ostringstream oss;
        oss << "failed to read all " << length << " bytes of the field '"
            << name << "'";
        throw std::runtime_error(oss.str());
        }

    ostr = str;
}

template<typename T>
T table_impl::do_read_number(char const* name, std::ifstream& ifs)
{
    T num;
    ifs.read(reinterpret_cast<char*>(&num), sizeof(T));
    if(ifs.gcount() != sizeof(T))
        {
        std::ostringstream oss;
        oss << "failed to read field '" << name << "'";
        throw std::runtime_error(oss.str());
        }

    return from_little_endian(num);
}

template<typename T>
void table_impl::read_number
        (boost::optional<T>& onum
        ,char const* name
        ,std::ifstream& ifs
        ,uint16_t length
        )
{
    throw_if_duplicate_field(onum.is_initialized(), name);

    if(length != sizeof(T))
        {
        std::ostringstream oss;
        oss << "unexpected length " << length
            << " for the field '" << name << "', expected " << sizeof(T);
        throw std::runtime_error(oss.str());
        }

    onum = do_read_number<T>(name, ifs);
}

void table_impl::read_number_before_values
        (boost::optional<uint16_t>& onum
        ,char const* name
        ,std::ifstream& ifs
        ,uint16_t length
        )
{
    if(!values_.empty())
        {
        std::ostringstream oss;
        oss << "field '" << name << "' must occur before the values";
        throw std::runtime_error(oss.str());
        }

    read_number(onum, name, ifs, length);
}

unsigned table_impl::get_expected_number_of_values() const
{
    throw_if_missing_field(min_age_, "minimum age");
    throw_if_missing_field(max_age_, "maximum age");

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

void table_impl::read_values(std::ifstream& ifs, uint16_t /* length */)
{
    throw_if_duplicate_field(!values_.empty(), "values");

    // 2 byte length field can only represent values up to 2^16, i.e. only up
    // to 2^16/2^3 == 8192 double-sized elements, which is not enough for the
    // tables occurring in real-world. Because of this we don't trust the
    // length field from the file at all but deduce the number of values from
    // the previously specified age-related fields instead.
    unsigned const num_values = get_expected_number_of_values();

    values_.resize(num_values);
    ifs.read(reinterpret_cast<char*>(&values_[0]), num_values*sizeof(double));
    if(ifs.gcount() != num_values*sizeof(double))
        {
        throw std::runtime_error("failed to read the values");
        }

    typedef std::vector<double>::iterator dvit;
    for(dvit it = values_.begin(); it != values_.end(); ++it)
        {
        *it = from_little_endian(*it);
        }
}

void table_impl::validate()
{
    // Check that the fields we absolutely need were specified.
    throw_if_missing_field(number_, "table number");
    throw_if_missing_field(type_, "table type");

    // Check that we have the values: this also ensures that we have the
    // correct minimum and maximum age as this is verified when filling in the
    // values.
    if(values_.empty())
        {
        throw std::runtime_error("no values defined");
        }

    // Validate the type and check that the select period has or hasn't been
    // given, depending on it.
    switch(char const ch = static_cast<char>(*type_))
        {
        case 'A':
        case 'D':
            if(get_value_or(select_period_, 0))
                {
                std::ostringstream oss;
                oss << "select period cannot be specified for a table of type "
                    << "'" << ch << "'";
                throw std::runtime_error(oss.str());
                }
            if(get_value_or(max_select_age_, 0) && *max_select_age_ != *max_age_)
                {
                std::ostringstream oss;
                oss << "maximum select age " << *max_select_age_
                    << " different from the maximum age " << *max_age_
                    << " cannot be specified for a table of type"
                       " '" << ch << "'";
                throw std::runtime_error(oss.str());
                }
            break;

        case 'S':
            if(!get_value_or(select_period_, 0))
                {
                throw std::runtime_error
                    ("select period must be specified for a select and ultimate table"
                    );
                }
            break;

        default:
            std::ostringstream oss;
            oss << "unknown table type '" << ch << "'";
            throw std::runtime_error(oss.str());
        }

    // We have a reasonable default for this value, so don't complain if it's
    // absent.
    if(!num_decimals_)
        {
        num_decimals_ = 6;
        }
}

table_impl::table_impl(std::ifstream& ifs, uint32_t offset)
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
        // prefer to do for efficiency: this would fail at the very end of the
        // file).
        if(record_type == e_field_end_table)
            {
            validate();
            return;
            }

        uint16_t const
            length = do_read_number<uint16_t>("record length", ifs);

        switch(record_type)
            {
            case e_field_table_name:
                read_string(name_, "name", ifs, length);
                break;
            case e_field_table_number:
                read_number(number_, "table number", ifs, length);
                break;
            case e_field_table_type:
                read_number(type_, "table type", ifs, length);
                break;
            case e_field_contributor:
                read_string(contributor_, "contributor", ifs, length);
                break;
            case e_field_data_source:
                read_string(data_source_, "data source", ifs, length);
                break;
            case e_field_data_volume:
                read_string(data_volume_, "data volume", ifs, length);
                break;
            case e_field_obs_period:
                read_string(obs_period_, "observation period", ifs, length);
                break;
            case e_field_unit_of_obs:
                read_string(unit_of_obs_, "unit of observation", ifs, length);
                break;
            case e_field_construction_method:
                read_string(construction_method_, "construction method", ifs, length);
                break;
            case e_field_published_reference:
                read_string(published_reference_, "published reference", ifs, length);
                break;
            case e_field_comments:
                read_string(comments_, "comments", ifs, length);
                break;
            case e_field_min_age:
                read_number(min_age_, "minimum age", ifs, length);
                break;
            case e_field_max_age:
                read_number(max_age_, "maximum age", ifs, length);
                break;
            case e_field_select_period:
                read_number_before_values(select_period_, "select period", ifs, length);
                break;
            case e_field_max_select_age:
                read_number_before_values(max_select_age_, "maximum select age", ifs, length);
                break;
            case e_field_num_decimals:
                read_number(num_decimals_, "number of decimal places", ifs, length);
                break;
            case e_field_values:
                read_values(ifs, length);
                break;
            case e_field_hash_value:
                read_number(hash_value_, "hash value", ifs, length);
                break;
            default:
                std::ostringstream oss;
                oss << "unknown field type " << record_type;
                throw std::runtime_error(oss.str());
            }
        }
}

void table_impl::save_as_text(fs::path const& file) const
{
    throw std::runtime_error("NIY");
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

    typedef std::vector<double>::const_iterator cdvit;
    for(cdvit it = values_.begin(); it != values_.end(); ++it)
        {
        oss << std::setw(value_width) << *it;
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
    throw std::runtime_error("NIY");
}

void table::save_as_text(fs::path const& file) const
{
    impl_->save_as_text(file);
}

void table::name(std::string const& n)
{
    return impl_->name(n);
}

table::Number table::number() const
{
    return impl_->number();
}

std::string const& table::name() const
{
    return impl_->name();
}

unsigned long table::compute_hash_value() const
{
    return impl_->compute_hash_value();
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
    void read_index(fs::path const& path);

    // We don't currently use the name stored in the index, so this struct
    // doesn't include it.
    struct IndexEntry
    {
        IndexEntry
            (table::Number number
            ,uint32_t offset
            ,boost::shared_ptr<table_impl> table
            )
            :number_(number)
            ,offset_(offset)
            ,table_(table)
        {
        }

        table::Number number_;

        // This field is ignored for the tables added to the database after
        // reading the original index file and is only used for loading the
        // existing tables from the original input file.
        uint32_t offset_;

        // table pointer may be empty for the tables present in the input
        // database file but not loaded yet.
        mutable boost::shared_ptr<table_impl> table_;
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
        ,boost::shared_ptr<table_impl> table = boost::shared_ptr<table_impl>()
        );

    // Return the table corresponding to the given index entry, loading it from
    // the database file if this hadn't been done yet.
    table do_get_table(IndexEntry const& entry) const;

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

// Helper function opening the stream for reading the given file in binary
// mode and throwing an exception on error.
void open_binary_file(fs::ifstream& ifs, fs::path const& path)
{
    ifs.open(path, std::ios_base::in | std::ios_base::binary);
    if(!ifs)
        {
        std::ostringstream oss;
        oss << "File '" << path << "' could not be opened for reading.";
        throw std::runtime_error(oss.str());
        }
}

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
    ,boost::shared_ptr<table_impl> table
    )
{
    index_.push_back(IndexEntry(number, offset, table));

    // We expect an insertion to be made as the map shouldn't contain this
    // number yet, but can't generate the appropriate error message here if it
    // does, so let the caller do it.
    return index_by_number_.insert(std::make_pair(number, index_.size())).second;
}

void database_impl::read_index(fs::path const& path)
{
    fs::path const index_path = fs::change_extension(path, ".ndx");

    fs::ifstream index_ifs;
    open_binary_file(index_ifs, index_path);

    // An index record is composed of:
    //
    //  -  4 byte table number
    //  - 50 byte table name (ignored here)
    //  -  4 byte offset of the table in the database file
    //
    // The numbers are in little endian format.
    enum
        {e_pos_number = 0
        ,e_pos_name   = 4
        ,e_pos_offset = 54
        ,e_pos_max    = 58
        };

    char index_record[e_pos_max] = {0};

    for(;;)
        {
        index_ifs.read(index_record, e_pos_max);
        if(index_ifs.gcount() != e_pos_max)
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
            number = from_bytes<uint32_t>(&index_record[e_pos_number]);
        uint32_t const
            offset = from_bytes<uint32_t>(&index_record[e_pos_offset]);

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

table database_impl::do_get_table(IndexEntry const& entry) const
{
    if(!entry.table_)
        {
        try
            {
            entry.table_.reset(new table_impl(database_ifs_, entry.offset_));
            }
        catch(std::runtime_error const& e)
            {
            std::ostringstream oss;
            oss << "Error reading table " << entry.number_.value()
                << " from the offset " << entry.offset_
                << " in the database '" << path_ << "': " << e.what();
            throw std::runtime_error(oss.str());
            }

        if(entry.table_->number() != entry.number_)
            {
            std::ostringstream oss;
            oss << "database '" << path_ << "' is corrupt: "
                   "table number " << entry.table_->number().value()
                << " is inconsistent with its number in the index ("
                << entry.number_.value() << ")";
            throw std::runtime_error(oss.str());
            }
        }

    return table(entry.table_);
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
    throw std::runtime_error("NIY");
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

} // namespace soa_binary_format
