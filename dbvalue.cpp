// Product-database entity.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#include "dbvalue.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "dbnames.hpp"
#include "et_vector.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "math_functions.hpp"           // lesser_of()
#include "print_matrix.hpp"
#include "value_cast.hpp"
#include "xml_serialize.hpp"

#include <algorithm>
#include <functional>                   // multiplies
#include <limits>                       // numeric_limits
#include <numeric>                      // accumulate()
#include <ostream>

namespace
{
int const ScalarDims[e_number_of_axes] = {1, 1, 1, 1, 1, 1, 1};
int const MaxPossibleElements = std::numeric_limits<int>::max();
} // Unnamed namespace.

/// Default ctor.

database_entity::database_entity()
    :key_          {0}
    ,axis_lengths_ (e_number_of_axes, 1)
    ,data_values_  (1)
{
    assert_invariants();
}

/// Handy ctor for writing programs to generate '.database' files.

database_entity::database_entity
    (int                key
    ,int                ndims
    ,int const*         dims
    ,double const*      data
    ,std::string const& gloss
    )
    :key_          {key}
    ,gloss_        {gloss}
{
    axis_lengths_ .assign(dims, dims + ndims);
    data_values_  .assign(data, data + getndata());
    assert_invariants();
}

database_entity::database_entity
    (int                        key
    ,std::vector<int> const&    dims
    ,std::vector<double> const& data
    ,std::string const&         gloss
    )
    :key_          {key}
    ,axis_lengths_ {dims}
    ,data_values_  {data}
    ,gloss_        {gloss}
{
    assert_invariants();
}

/// Handy ctor for scalar data.

database_entity::database_entity
    (int                key
    ,double             datum
    ,std::string const& gloss
    )
    :key_          {key}
    ,gloss_        {gloss}
{
    axis_lengths_ .assign(ScalarDims, ScalarDims + e_number_of_axes);
    data_values_  .push_back(datum);
    assert_invariants();
}

#if 0
/// Deliberately undefined copy assignment operator.
///
/// 'input_test.cpp' contains the following strange code, which seems
/// to have been written by accident and demonstrates a design flaw:
///
///   DBDictionary& dictionary = ...
///   ...
///   dictionary.datum("MaturityAge") = database_entity
///       (DB_StatVxQ
///       ,e_number_of_axes
///       ,dims_stat
///       ,stat
///       );
///   BOOST_TEST_THROW
///       (db.query(DB_MaturityAge)
///       ,std::runtime_error
///       ,"Assertion '1 == v.extent()' failed."
///       );
///
/// There are two different types of keys:
///  - strings like "MaturityAge", and
///  - enumerators like DB_MaturityAge or DB_StatVxQ,
/// and no invariant that preserves their mutual correspondence.
///
/// The first line in the example above sets
///   dictionary["MaturityAge"]
/// but
///   dictionary["StatVxQ"]
/// would have been set instead if it had been written thus:
///   dictionary.Add(...the temporary created above...)
///
/// The copy-ctor implementation below would produce an error in that
/// unit test. Defining it detects that problem, and its assertion
/// doesn't seem to fire in any other case. However, what is asserted
/// is not a sensible invariant of class database_entity. More likely
/// the real problem is that std::map::value_type is
///   std::pair<key_type const, mapped_type>
/// but the present class is akin to
///   struct{key_type k; mapped_type t;};
/// and its key is thus not const. Probably this situation arose
/// because the original implementation predated any usable STL.

database_entity& database_entity::operator=(database_entity const& z)
{
    LMI_ASSERT(0 == key_ || z.key_ == key_);
    axis_lengths_ = z.axis_lengths_;
    data_values_  = z.data_values_;
    gloss_        = z.gloss_;
    return *this;
}
#endif // 0

bool database_entity::operator==(database_entity const& z) const
{
#if 0
// PETE causes an 'ambiguous overload' error for vector 'v0==v1'.
    return
           key_          == z.key_
        && axis_lengths_ == z.axis_lengths_
        && data_values_  == z.data_values_
        && gloss_        == z.gloss_
        ;
#endif // 0
    return
           key_          == z.key_
        && std::operator==(axis_lengths_, z.axis_lengths_)
        && std::operator==(data_values_ , z.data_values_ )
        && gloss_        == z.gloss_
        ;
}

/// Change dimensions.
///
/// Preconditions:
///   - argument specifies the expected number of axes;
///   - each axis in the argument has a permissible value;
///   - data size would not be excessive.
///
/// Postconditions: all ctor postconditions are satisfied.

void database_entity::reshape(std::vector<int> const& new_dims)
{
    LMI_ASSERT(e_number_of_axes == new_dims.size());
    LMI_ASSERT(1 == new_dims[0] || e_max_dim_gender    == new_dims[0]);
    LMI_ASSERT(1 == new_dims[1] || e_max_dim_uw_class  == new_dims[1]);
    LMI_ASSERT(1 == new_dims[2] || e_max_dim_smoking   == new_dims[2]);
    LMI_ASSERT(1 == new_dims[3] || e_max_dim_issue_age == new_dims[3]);
    LMI_ASSERT(1 == new_dims[4] || e_max_dim_uw_basis  == new_dims[4]);
    LMI_ASSERT(1 == new_dims[5] || e_max_dim_state     == new_dims[5]);
    LMI_ASSERT(1 <= new_dims[6] && new_dims[6] <= e_max_dim_duration);

    // Number of times we'll go through the assignment loop.
    int n_iter = getndata(new_dims);

    // Create a new instance of this class having the same key but the
    // desired dimensions, for convenient use of operator[]().
    std::vector<double> new_data(n_iter);
    database_entity new_object(key(), new_dims, new_data);

    std::vector<int> dst_max_idx(e_number_of_axes);
    assign(dst_max_idx, new_dims - 1);

    std::vector<int> src_max_idx(e_number_of_axes);
    assign(src_max_idx, axis_lengths_ - 1);

    std::vector<int> dst_idx(e_number_of_axes); // indexes new_object
    std::vector<int> src_idx(e_number_of_axes); // indexes '*this'

    std::vector<int> working_idx(e_number_of_axes);
    for(int j = 0; j < n_iter; ++j)
        {
        int z = j;
        std::vector<int>::const_iterator i = new_dims.begin();
        std::vector<int>::iterator w = working_idx.begin();
        while(i != new_dims.end())
            {
            LMI_ASSERT(0 != *i);
            *w = z % *i;
            z /= *i;
            ++i;
            ++w;
            }
        LMI_ASSERT(0 == z);

        // limit dst and source indexes to those that actually vary
        assign(dst_idx, apply_binary(lesser_of<int>(), working_idx, dst_max_idx));
        assign(src_idx, apply_binary(lesser_of<int>(), working_idx, src_max_idx));
        new_object[dst_idx] = operator[](src_idx);
        }

    axis_lengths_ = new_dims;
    data_values_  = new_object.data_values_;
    assert_invariants();
}

/// Indexing operator for reshape() and product editor only.
///
/// Two indexing operators are provided. This one's argument includes
/// the number of durations--which, as far as the product editor is
/// concerned, is much like the other axes. However, for illustration
/// production, product_database::query() handles the last (duration)
/// axis, replicating the last value as needed to extend to maturity.

double& database_entity::operator[](std::vector<int> const& index)
{
    assert_invariants();
    LMI_ASSERT(e_number_of_axes == index.size());

    int z = 0;
    for(int j = 0; j < e_number_of_axes; ++j)
        {
        if(1 != axis_lengths_[j])
            {
            LMI_ASSERT(index[j] < axis_lengths_[j]);
            z = z * axis_lengths_[j] + index[j];
            }
        }
    if(static_cast<int>(data_values_.size()) <= z)
        {
        z = 0;
        alarum()
            << "Trying to index database item '"
            << GetDBNames()[key_].ShortName
            << "' past end of data."
            << LMI_FLUSH
            ;
        }
    return data_values_[z];
}

/// Indexing operator for illustration production.

double const* database_entity::operator[](database_index const& idx) const
{
    auto const& index(idx.index_array());
    LMI_ASSERT(e_number_of_axes == 1 + index.size());

    int z = 0;
    for(int j = 0; j < e_number_of_axes - 1; ++j)
        {
        if(1 != axis_lengths_[j])
            {
            LMI_ASSERT(index[j] < axis_lengths_[j]);
            z = z * axis_lengths_[j] + index[j];
            }
        }
    z *= axis_lengths_.back();
    if(static_cast<int>(data_values_.size()) <= z)
        {
        z = 0;
        alarum()
            << "Trying to index database item '"
            << GetDBNames()[key_].ShortName
            << "' past end of data."
            << LMI_FLUSH
            ;
        }
    return &data_values_[z];
}

int database_entity::key() const
{
    return key_;
}

/// Dimension along the duration axis.

int database_entity::extent() const
{
    return axis_lengths_.at(e_axis_duration);
}

std::vector<int> const& database_entity::axis_lengths() const
{
    LMI_ASSERT(e_number_of_axes == axis_lengths_.size());
    return axis_lengths_;
}

std::vector<double> const& database_entity::data_values() const
{
    return data_values_;
}

std::ostream& database_entity::write(std::ostream& os) const
{
    os
        << '"' << GetDBNames()[key_].LongName << '"'
        << '\n'
        << "  name='" << GetDBNames()[key_].ShortName << "'"
        << '\n'
        ;
    if(!gloss_.empty())
        {
        os << "  gloss: " << gloss_ << '\n';
        }

    if(1 == getndata())
        {
        os << "  scalar";
        }
    else
        {
        os << "  varies by:";
        if(1 != axis_lengths_[0]) os <<    " gender[" << axis_lengths_[0] << ']';
        if(1 != axis_lengths_[1]) os <<  " uw_class[" << axis_lengths_[1] << ']';
        if(1 != axis_lengths_[2]) os <<   " smoking[" << axis_lengths_[2] << ']';
        if(1 != axis_lengths_[3]) os << " issue_age[" << axis_lengths_[3] << ']';
        if(1 != axis_lengths_[4]) os <<  " uw_basis[" << axis_lengths_[4] << ']';
        if(1 != axis_lengths_[5]) os <<     " state[" << axis_lengths_[5] << ']';
        if(1 != axis_lengths_[6]) os <<  " duration[" << axis_lengths_[6] << ']';
        }

    os << '\n';
    print_matrix(os, data_values_, axis_lengths_);
    os << '\n';
    return os;
}

void database_entity::assert_invariants() const
{
    LMI_ASSERT(!contains(axis_lengths_, 0));
    LMI_ASSERT(getndata() == static_cast<int>(data_values_.size()));
    LMI_ASSERT
        (   0 < static_cast<int>(data_values_.size())
        &&      static_cast<int>(data_values_.size()) < MaxPossibleElements
        );
    LMI_ASSERT(DB_FIRST <= key_ && key_ < DB_LAST);
    LMI_ASSERT(e_number_of_axes == axis_lengths_.size());

    std::vector<int> const& max_dims(maximum_database_dimensions());
    LMI_ASSERT(e_number_of_axes == max_dims.size());
    std::vector<int>::const_iterator ai = axis_lengths_.begin();
    std::vector<int>::const_iterator mi = max_dims.begin();

    while(ai != axis_lengths_.end()) // not duration!
        {
        if(*ai != 1 && *ai != *mi && *ai != axis_lengths_.back())
            {
            alarum()
                << "Database item '"
                << GetDBNames()[key_].ShortName
                << "' has invalid length "
                << *ai
                << " in a dimension where "
                << *mi
                << " was expected."
                << LMI_FLUSH
                ;
            }
        ++ai;
        ++mi;
        }

    if(max_dims.back() < axis_lengths_.back())
            {
            alarum()
                << "Database item '"
                << GetDBNames()[key_].ShortName
                << "' has invalid duration."
                << LMI_FLUSH
                ;
            }
}

/// Calculate number of data required by lengths of object's axes.

int database_entity::getndata() const
{
    try
        {
        return getndata(axis_lengths_);
        }
    catch(...)
        {
        report_exception();
        alarum()
            << "Database item '"
            << GetDBNames()[key_].ShortName
            << "' has invalid dimensions."
            << LMI_FLUSH
            ;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Calculate number of data required by lengths of given axes.
///
/// Use a double-precision accumulator internally to avoid overflow.
///
/// Throw if the result equals zero or exceeds MaxPossibleElements.
///
/// Otherwise, return the result, cast to int. The enforced range
/// contraints guarantee that the cast preserves value.

int database_entity::getndata(std::vector<int> const& z)
{
    double n = std::accumulate(z.begin(), z.end(), 1.0, std::multiplies<double>());

    if(MaxPossibleElements < n)
        {
        alarum()
            << "There are " << n
            << " data, but at most " << MaxPossibleElements
            << " are permitted."
            << LMI_FLUSH
            ;
        }

    if(n <= 0)
        {
        alarum() << "Number of data must exceed zero." << LMI_FLUSH;
        }

    LMI_ASSERT(MaxPossibleElements <= std::numeric_limits<int>::max());
    // Redundant but cheap guarantee that the cast preserves value:
    LMI_ASSERT(1.0 <= n && n <= MaxPossibleElements);
    return static_cast<int>(n);
}

void database_entity::read(xml::element const& e)
{
    key_ = db_key_from_name(e.get_name());
    xml_serialize::get_element(e, "axis_lengths", axis_lengths_);
    xml_serialize::get_element(e, "data_values" , data_values_ );
    xml_serialize::get_element(e, "gloss"       , gloss_       );

    assert_invariants();
}

void database_entity::write(xml::element& e) const
{
    assert_invariants();

    xml_serialize::set_element(e, "axis_lengths", axis_lengths_);
    xml_serialize::set_element(e, "data_values" , data_values_ );
    xml_serialize::set_element(e, "gloss"       , gloss_       );
}

std::vector<int> const& maximum_database_dimensions()
{
    static int const d[e_number_of_axes] =
        {e_max_dim_gender
        ,e_max_dim_uw_class
        ,e_max_dim_smoking
        ,e_max_dim_issue_age
        ,e_max_dim_uw_basis
        ,e_max_dim_state
        ,e_max_dim_duration
        };
    static std::vector<int> const z(d, d + e_number_of_axes);
    return z;
}
