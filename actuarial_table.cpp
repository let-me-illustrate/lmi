// Actuarial tables from SOA database.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "actuarial_table.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "deserialize_cast.hpp"
#include "materially_equal.hpp"
#include "miscellany.hpp"
#include "oecumenic_enumerations.hpp"   // methuselah
#include "path_utility.hpp"             // fs::path inserter
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <boost/cstdint.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>

#include <xmlwrapp/nodes_view.h>

#include <algorithm>                    // std::max(), std::min()
#include <cctype>                       // std::toupper()
#include <climits>                      // CHAR_BIT
#include <iomanip>                      // std::setprecision()
#include <ios>
#include <istream>
#include <iterator>                     // std::distance()
#include <limits>

namespace
{
    // TODO ?? Add unit tests for tables with length
    //   - more than 2^15 but less than 2^16; and
    //   - exactly 2^16
    //   - more than 2^16

    /// The binary format that the SOA uses for its published tables
    /// has only a sixteen-bit integer to represent a table's length
    /// in bytes. This length must be read as unsigned because it may
    /// be 2^15 or greater, but less than 2^16.

    typedef boost::uint16_t soa_table_length_type;
    int const soa_table_length_max =
        std::numeric_limits<soa_table_length_type>::max()
        ;

    /// Use the value -1 as an initializer because it is invalid for
    /// all integral data fields, then assert that it doesn't equal
    /// the value ultimately returned. Cast it to the argument type
    /// in order to avoid warnings for unsigned types.

    template<typename T>
    T read_datum(std::istream& is, T& t, boost::int16_t nominal_length)
    {
        LMI_ASSERT(sizeof(T) == nominal_length);
        T const invalid(static_cast<T>(-1));
        t = invalid;
        char z[sizeof(T)];
        is.read(z, sizeof(T));
        t = deserialize_cast<T>(z);
        LMI_ASSERT(invalid != t);
        return t;
    }
} // Unnamed namespace.

actuarial_table_base::actuarial_table_base()
    :table_type_     (-1)
    ,min_age_        (-1)
    ,max_age_        (-1)
    ,select_period_  (-1)
    ,max_select_age_ (-1)
{
}

actuarial_table_base::~actuarial_table_base()
{
}

/// Read a given number of values for a given issue age.

std::vector<double> actuarial_table_base::values(int issue_age, int length) const
{
    return specific_values(issue_age, length);
}

/// Read a given number of values for a given issue age, using a
/// nondefault lookup method.
///
/// Assertions require that arguments be sane on entry, regardless of
/// method: method-specific adjustments are not permitted to render
/// sane what was insane ab ovo.

std::vector<double> actuarial_table_base::values_elaborated
    (int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      inforce_duration
    ,int                      reset_duration
    ) const
{
    LMI_ASSERT(min_age_ <= issue_age && issue_age <= max_age_);
    LMI_ASSERT(0 <= length && length <= 1 + max_age_ - issue_age);
    LMI_ASSERT(0 <= inforce_duration);
    LMI_ASSERT(inforce_duration < 1 + max_age_ - issue_age);
    LMI_ASSERT(reset_duration <= inforce_duration);

    if('S' != table_type_)
        {
        return specific_values(issue_age, length);
        }

    switch(method)
        {
        case e_reenter_at_inforce_duration:
            {
            int const delta = inforce_duration;
            std::vector<double> v = specific_values
                (issue_age + delta
                ,length    - delta
                );
            v.insert(v.begin(), delta, 0.0);
            return v;
            }
            // break;
        case e_reenter_upon_rate_reset:
            {
            int const age_setback_limit = issue_age - min_age_;
            int const delta = std::max(reset_duration, -age_setback_limit);
            std::vector<double> v = specific_values
                (issue_age + delta
                ,length    - delta
                );
            if(delta < 0)
                {
                v.erase(v.begin(), v.begin() - delta);
                }
            else
                {
                v.insert(v.begin(), delta, 0.0);
                }
            return v;
            }
            // break;
        case e_reenter_never: // Fall through.
        default:
            {
            fatal_error()
                << "Table-lookup method "
                << method
                << " is not valid in this context."
                << LMI_FLUSH
                ;
            throw "Unreachable--silences a compiler diagnostic.";
            }
        }
}

xml_actuarial_table::xml_actuarial_table(std::string const& filename, int table_number)
{
    // SOA !! This is temporary code for API compatibility with soa_actuarial_table.
    // It should be changed so that the constructor takes only a single
    // argument, filename of the XML table file.
    std::string xmlfile(filename, 0, filename.rfind('.'));
    xmlfile += "_";
    xmlfile += value_cast<std::string>(table_number);
    xmlfile += ".xtable";
    load_xml_table(xmlfile);
}

xml_actuarial_table::~xml_actuarial_table()
{
}

void xml_actuarial_table::load_xml_table(std::string const& filename)
{
    xml_lmi::dom_parser parser(filename);
    xml::element root(parser.root_node("table"));

    // SOA !! Implement loading of multi-dimensional tables as well.
    // SOA !! Upgrade xmlwrapp:
    // XMLWRAPP !! This should be const_iterator, but xmlwrapp < 0.7 lacks the
    // necessary operator!= for comparing const and non-const iterators.
    xml::node::iterator i;

    if (root.end() != (i = root.find("aggregate")))
        {
        load_xml_aggregate_table(*i);
        }
    else if (root.end() != (i = root.find("duration")))
        {
        load_xml_duration_table(*i);
        }
    else if (root.end() != (i = root.find("select")))
        {
        load_xml_select_table(*i);
        }
    else if (root.end() != (i = root.find("select-and-ultimate")))
        {
        load_xml_select_and_ultimate_table(*i);
        }
    else
        {
        fatal_error()
            << "Required data element not found."
            << LMI_FLUSH
            ;
        }
}

void xml_actuarial_table::load_xml_table_with_ages
    (xml::element const& node
    ,std::vector<double>& data
    ,int& min_age
    ,int& max_age
    )
{
    xml::const_nodes_view const values = node.elements("value");

    data.reserve(std::distance(values.begin(), values.end()));

    min_age = max_age = -1;

    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi i = values.begin(); i != values.end(); ++i)
        {
        data.push_back(value_cast<double>(xml_lmi::get_content(*i)));
        int age;
        if(!xml_lmi::get_attr(*i, "age", age))
            {
            fatal_error()
                << "XML <value> node doesn't have 'age' attribute."
                << LMI_FLUSH
                ;
            }
        if(-1 == min_age)
            {
            min_age = age;
            }
        else
            {
            if(max_age + 1 != age)
                {
                fatal_error()
                    << "XML <value> node has 'age' attribute value '"
                    << age
                    << "' but '"
                    << max_age + 1
                    << "' was expected."
                    << LMI_FLUSH
                    ;
                }
            }
        max_age = age;
        }

    LMI_ASSERT(data.size() == size_t(max_age - min_age + 1));
}

void xml_actuarial_table::load_xml_aggregate_table(xml::element const& node)
{
    load_xml_table_with_ages
        (node
        ,data_
        ,min_age_
        ,max_age_
        );

    table_type_ = e_table_aggregate;
}

void xml_actuarial_table::load_xml_duration_table(xml::element const& node)
{
    xml::const_nodes_view const values = node.elements("value");

    data_.reserve(std::distance(values.begin(), values.end()));

    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi i = values.begin(); i != values.end(); ++i)
        {
        data_.push_back(value_cast<double>(xml_lmi::get_content(*i)));
        }

    table_type_ = e_table_duration;
}

void xml_actuarial_table::load_xml_select_table(xml::element const& node)
{
    xml::const_nodes_view const rows = node.elements("row");

    if(!xml_lmi::get_attr(node, "period", select_period_))
        {
        fatal_error()
            << "XML <select> node doesn't have 'period' attribute."
            << LMI_FLUSH
            ;
        }

    data_.reserve(std::distance(rows.begin(), rows.end()) * select_period_);

    min_age_ = max_age_ = -1;

    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi i = rows.begin(); i != rows.end(); ++i)
        {
        int age;
        if(!xml_lmi::get_attr(*i, "age", age))
            {
            fatal_error()
                << "XML <row> node doesn't have 'age' attribute."
                << LMI_FLUSH
                ;
            }
        if(-1 == min_age_)
            {
            min_age_ = age;
            }
        else
            {
            if(max_age_ + 1 != age)
                {
                fatal_error()
                    << "XML <row> node has 'age' attribute value '"
                    << age
                    << "' but '"
                    << max_age_ + 1
                    << "' was expected."
                    << LMI_FLUSH
                    ;
                }
            }
        max_age_ = age;

        xml::const_nodes_view const values = i->elements("value");
        if(select_period_ != std::distance(values.begin(), values.end()))
            {
            fatal_error()
                << "XML <row> node has "
                << std::distance(values.begin(), values.end())
                << " values but select period is "
                << select_period_
                << "."
                << LMI_FLUSH
                ;
            }

        for(cnvi v = values.begin(); v != values.end(); ++v)
            {
            data_.push_back(value_cast<double>(xml_lmi::get_content(*v)));
            }
        }

    max_select_age_ = max_age_;

    LMI_ASSERT(data_.size() == size_t((max_age_ - min_age_ + 1) * select_period_));

    // Use the same type for select and select & ultimate tables: selected
    // table is just a special case of the latter where max_age_ ==
    // max_select_age_ and the ultimate table is empty.
    table_type_ = e_table_select_and_ultimate;
}

void xml_actuarial_table::load_xml_select_and_ultimate_table(xml::element const& node)
{
    load_xml_select_table(*xml_lmi::retrieve_element(node, "select"));

    int ultimate_min_age;
    load_xml_table_with_ages
        (*xml_lmi::retrieve_element(node, "ultimate")
        ,ultimate_
        ,ultimate_min_age
        ,max_age_
        );

    if(ultimate_min_age != min_age_ + select_period_)
        {
        fatal_error()
            << "Ultimate table should have min. age "
            << min_age_
            << ", but has "
            << ultimate_min_age
            << "."
            << LMI_FLUSH
            ;
        }

    table_type_ = e_table_select_and_ultimate;
}

std::vector<double> xml_actuarial_table::specific_values
    (int issue_age
    ,int length
    ) const
{
    if(table_type_ != e_table_duration)
        {
        // min_age_ and max_age_ are invalid for duration tables
        LMI_ASSERT(min_age_ <= issue_age && issue_age <= max_age_);
        LMI_ASSERT(0 <= length && length <= 1 + max_age_ - issue_age);
        }

    std::vector<double> v;
    switch(table_type_)
        {
        case e_table_aggregate:
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
        case e_table_duration:
            {
            v = std::vector<double>
                (data_.begin()
                ,data_.begin() + length
                );
            }
            break;
        case e_table_select_and_ultimate:
            {
            v.resize(length);
            std::vector<double>::iterator cursor = v.begin();

            int row_to_start_at;

            // Write select portion:
            if(issue_age < max_select_age_ + select_period_)
                {
                row_to_start_at = (std::min(max_select_age_, issue_age) - min_age_);
                int offset_in_row = std::max(0, issue_age - max_select_age_);
                int k = offset_in_row
                    +   row_to_start_at * select_period_
                    ;
                for(int i = offset_in_row; cursor != v.end() && i < select_period_; i++, k++)
                    *(cursor++) = data_[k];
                }
            else
                {
                const int min_ultimate_age = min_age_ + select_period_;
                row_to_start_at = issue_age - min_ultimate_age;
                }

            // And ultimate:
            for(int k = row_to_start_at; cursor != v.end(); k++)
                *(cursor++) = ultimate_[k];
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

soa_actuarial_table::soa_actuarial_table(std::string const& filename, int table_number)
    :filename_       (filename)
    ,table_number_   (table_number)
    ,table_offset_   (-1)
{
    if(table_number_ <= 0)
        {
        fatal_error()
            << "There is no table number "
            << table_number_
            << " in file '"
            << filename_
            << "'."
            << LMI_FLUSH
            ;
        }

    find_table();
    parse_table();
}

soa_actuarial_table::~soa_actuarial_table()
{
}

/// Find the table specified by table_number_.
///
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

void soa_actuarial_table::find_table()
{
    LMI_ASSERT(0 != table_number_);

    fs::path index_path(filename_);
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

    // TODO ?? Assert endianness too? SOA tables are not portable;
    // probably they can easily be read only on x86 hardware.

    BOOST_STATIC_ASSERT(8 == CHAR_BIT);
    BOOST_STATIC_ASSERT(4 == sizeof(int));
    BOOST_STATIC_ASSERT(2 == sizeof(short int));

    // 27.4.3.2/2 requires that this be interpreted as invalid.
    // Reinitialize it here for robustness, even though the ctor
    // already initializes it in the same way.
    table_offset_ = std::streampos(-1);

    int const index_record_length(58);
    char index_record[index_record_length] = {0};

    BOOST_STATIC_ASSERT(sizeof(boost::int32_t) <= sizeof(int));
    while(index_ifs)
        {
        int index_table_number =
            deserialize_cast<boost::int32_t>(index_record)
            ;
        if(table_number_ == index_table_number)
            {
            char* p = 54 + index_record;
            int z = deserialize_cast<boost::int32_t>(p);
            table_offset_ = std::streampos(z);
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

    if(std::streampos(-1) == table_offset_)
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

/// Read a table, parsing its header and values.
///
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

void soa_actuarial_table::parse_table()
{
    LMI_ASSERT(-1 == table_type_    );
    LMI_ASSERT(-1 == min_age_       );
    LMI_ASSERT(-1 == max_age_       );
    LMI_ASSERT(-1 == select_period_ );
    LMI_ASSERT(-1 == max_select_age_);

    fs::path data_path(filename_);
    data_path = fs::change_extension(data_path, ".dat");
    fs::ifstream data_ifs(data_path, ios_in_binary());
    if(!data_ifs)
        {
        fatal_error()
            << "File '"
            << data_path
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    data_ifs.seekg(table_offset_, std::ios::beg);
    LMI_ASSERT(table_offset_ == data_ifs.tellg());

    while(data_ifs)
        {
        boost::int16_t record_type;
        read_datum(data_ifs, record_type, sizeof(boost::int16_t));

        soa_table_length_type nominal_length;
        read_datum(data_ifs, nominal_length, sizeof(boost::int16_t));

        switch(record_type)
            {
            case 2: // 4-byte integer: Table number.
                {
                boost::int32_t z;
                read_datum(data_ifs, z, nominal_length);
                LMI_ASSERT(z == table_number_);
                }
                break;
            case 3: // [unsigned] char: Table type.
                {
                // Meaning: {A, D, S} --> {age, duration, select}.
                // SOA apparently permits upper or lower case.
                LMI_ASSERT(-1 == table_type_);
                unsigned char z;
                read_datum(data_ifs, z, nominal_length);
                z = static_cast<unsigned char>(std::toupper(z));
                LMI_ASSERT('A' == z || 'D' == z || 'S' == z);
                table_type_ = z;
                }
                break;
            case 12: // 2-byte integer: Minimum age.
                {
                LMI_ASSERT(-1 == min_age_);
                boost::int16_t z;
                read_datum(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                min_age_ = z;
                }
                break;
            case 13: // 2-byte integer: Maximum age.
                {
                LMI_ASSERT(-1 == max_age_);
                boost::int16_t z;
                read_datum(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                max_age_ = z;
                }
                break;
            case 14: // 2-byte integer: Select period.
                {
                LMI_ASSERT(-1 == select_period_);
                boost::int16_t z;
                read_datum(data_ifs, z, nominal_length);
                LMI_ASSERT(0 <= z && z <= methuselah);
                select_period_ = z;
                }
                break;
            case 15: // 2-byte integer: Maximum select age.
                {
                LMI_ASSERT(-1 == max_select_age_);
                boost::int16_t z;
                read_datum(data_ifs, z, nominal_length);
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
    LMI_ASSERT(-1 != table_type_    );
    LMI_ASSERT(-1 != min_age_       );
    LMI_ASSERT(-1 != max_age_       );
    LMI_ASSERT(-1 != select_period_ );
    LMI_ASSERT(-1 != max_select_age_);
}

/// Read values (record type 17) from a table.
///
/// The number of values equals the nominal length, in the SOA
/// implementation. That means that no table can have more than 4096
/// values, which is a draconian restriction: 100 x 100 tables are
/// common enough in real-world practice.
///
/// However, the actual number of values can always be deduced
/// correctly from context. And the context is always known when the
/// values are read, because the SOA implementation always writes the
/// values after all records that identify the context. Therefore, the
/// nominal length can be disregarded for record type 17, and any
/// desired number of values written. If the actual number of values
/// exceeds 4096, then this implementation handles them correctly, but
/// the SOA implementation does not.
///
/// GWC's email of Wednesday, December 16, 1998 5:56 PM to the author
/// of the SOA implementation proposed a patch to overcome this
/// limitation, but it was not accepted, and the limitation persists
/// in later 32-bit versions of the software distributed by the SOA
/// even as this is written on 2005-01-13.
///
/// One might suppose that the select period for tables that are not
/// select couldn't be nonzero, but the SOA publishes tables that
/// don't honor that invariant; this implementation imposes it after
/// the fact.
///
/// If max_select_age_ is given as zero, then it's apparently to be
/// taken as unlimited, so its value should be max_age_; this
/// implementation makes it so after the fact.

void soa_actuarial_table::read_values(std::istream& is, int nominal_length)
{
    if('S' != table_type_)
        {
        select_period_ = 0;
        }

    LMI_ASSERT(min_age_ <= max_age_);

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
        data_[j] = deserialize_cast<double>(z);
        }
}

/// Read a given number of values for a given issue age.
///
/// For table type "Duration", SOA software in effect treats min_age_
/// as the index origin. That doesn't matter to this function, except
/// that its 'issue_age' argument must be in [min_age_, max_age_] as
/// for other table types. Requiring that precondition to be satisfied
/// even though 'issue_age' is otherwise ignored seems superfluous but
/// harmless; the goal here is not to redesign the SOA software but to
/// emulate it.
///
/// If the issue-age argument exceeds max_select_age_ for a table of
/// type 'S' ("Select"), then rates are looked up as though the issue
/// age were reduced to the maximum select age, and the select
/// duration correspondingly increased, i.e.:
///   map [x]+j to [max_select_age_]+(x-max_select_age_)+j
/// This behavior is useful for the 'e_reenter_at_inforce_duration'
/// method. For other methods, it's the least unreasonable behavior
/// that doesn't throw a runtime exception. Alternatively, it might be
/// considered preferable to throw an exception, in case permitting
/// issue age to exceed max_select_age_ is an inadvertent mistake.

std::vector<double> soa_actuarial_table::specific_values
    (int issue_age
    ,int length
    ) const
{
    LMI_ASSERT(min_age_ <= issue_age && issue_age <= max_age_);
    LMI_ASSERT(0 <= length && length <= 1 + max_age_ - issue_age);

    std::vector<double> v;
    switch(table_type_)
        {
        case e_table_aggregate:
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
        case e_table_duration:
            {
            v = std::vector<double>
                (data_.begin()
                ,data_.begin() + length
                );
            }
            break;
        case e_table_select_and_ultimate:
            {
            int const stride = 1 + select_period_;
            int k =
                    std::max(0, issue_age - max_select_age_)
                +   (std::min(max_select_age_, issue_age) - min_age_) * stride
                ;
            v.resize(length);
            for(int j = 0; j < length; ++j, ++k)
                {
                v[j] = data_.at(k);
                if
                    (   j + issue_age < max_select_age_ + select_period_
                    &&  select_period_ <= j
                    )
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

#if defined LMI_USE_XML_TABLES
namespace
{
    bool almost_equal_doubles(std::vector<double> const& a, std::vector<double> const& b)
    {
        if(a.size() != b.size())
            return false;

        size_t const size = a.size();
        for(size_t i = 0; i < size; i++)
            {
            if(!materially_equal(a[i], b[i], 2.0E-15L))
                {
                warning()
                    << i << " i\n"
                    << value_cast<std::string>(a[i]) << " ... a[i]\n"
                    << value_cast<std::string>(b[i]) << " ... b[i]\n"
                    << std::setprecision(20)
                    << a[i] << " a[i]\n"
                    << b[i] << " b[i]\n"
                    << value_cast<std::string>((b[i] - a[i]) / b[i]) << " (b[i] - a[i]) / b[i]\n"
                    << LMI_FLUSH
                    ;
                return false;
                }
            }

        return true;
    }
} // Unnamed namespace.
#endif // defined LMI_USE_XML_TABLES

std::vector<double> actuarial_table_rates
    (std::string const& table_filename
    ,int                table_number
    ,int                issue_age
    ,int                length
    )
{
#if defined LMI_USE_XML_TABLES
    xml_actuarial_table     z(table_filename, table_number);
    soa_actuarial_table z_soa(table_filename, table_number);

    std::vector<double> values    (z.values(issue_age, length));
    std::vector<double> values_soa(z_soa.values(issue_age, length));

    // SOA !! Temporarily verify correctness of XML implementation,
    // remove this once satisfied
    LMI_ASSERT(almost_equal_doubles(values, values_soa));
    return values;
#else  // !defined LMI_USE_XML_TABLES
// SOA !! Ultimately, there will be only one class:
    actuarial_table z(table_filename, table_number);
    return z.values(issue_age, length);
#endif // !defined LMI_USE_XML_TABLES
}

std::vector<double> actuarial_table_rates_elaborated
    (std::string const&       table_filename
    ,int                      table_number
    ,int                      issue_age
    ,int                      length
    ,e_actuarial_table_method method
    ,int                      inforce_duration
    ,int                      reset_duration
    )
{
#if defined LMI_USE_XML_TABLES
    xml_actuarial_table     z(table_filename, table_number);
    soa_actuarial_table z_soa(table_filename, table_number);

    std::vector<double> values(z.values_elaborated
        (issue_age
        ,length
        ,method
        ,inforce_duration
        ,reset_duration
        ));
    std::vector<double> values_soa(z_soa.values_elaborated
        (issue_age
        ,length
        ,method
        ,inforce_duration
        ,reset_duration
        ));

    // SOA !! Temporarily verify correctness of XML implementation,
    // remove this once satisfied
    LMI_ASSERT(almost_equal_doubles(values, values_soa));
    return values;
#else  // !defined LMI_USE_XML_TABLES
// SOA !! Ultimately, there will be only one class:
    actuarial_table z(table_filename, table_number);
    return z.values_elaborated
        (issue_age
        ,length
        ,method
        ,inforce_duration
        ,reset_duration
        );
#endif // !defined LMI_USE_XML_TABLES
}

