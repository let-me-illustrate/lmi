// Product-database entity.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "dbvalue.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "dbnames.hpp"
#include "math_functors.hpp" // greater_of(), lesser_of()
#include "print_matrix.hpp"
#include "value_cast.hpp"
#include "xml_serialize.hpp"

#include <algorithm>
#include <functional>
#include <iterator>          // std::advance()
#include <limits>            // std::numeric_limits
#include <numeric>
#include <ostream>

static int const ScalarDims[TDBValue::e_number_of_axes] = {1, 1, 1, 1, 1, 1, 1};
static int const MaxPossibleElements = std::numeric_limits<int>::max();

std::vector<int> const& TDBValue::maximum_dimensions()
{
    static int const d[e_number_of_axes] =
        {e_max_dim_gender
        ,e_max_dim_class
        ,e_max_dim_smoking
        ,e_max_dim_issue_age
        ,e_max_dim_uw_basis
        ,e_max_dim_state
        ,e_max_dim_duration
        };
    static std::vector<int> const z(d, d + e_number_of_axes);
    return z;
}

//============================================================================
bool TDBValue::Equivalent(TDBValue const& a, TDBValue const& b)
{
    return(a.axis_lengths_ == b.axis_lengths_ && a.data_values_ == b.data_values_);
}

//============================================================================
bool TDBValue::VariesByState(TDBValue const& z)
{
    LMI_ASSERT(5 < z.axis_lengths_.size());
    return 1 != z.axis_lengths_[5];
}

//============================================================================
TDBValue::TDBValue()
    :key_          (0)
    ,axis_lengths_ (e_number_of_axes)
{
}

/// Handy ctor for writing programs to generate '.database' files.

TDBValue::TDBValue
    (int                key
    ,int                ndims
    ,int const*         dims
    ,double const*      data
    ,std::string const& gloss
    )
    :key_          (key)
    ,gloss_        (gloss)
{
    axis_lengths_ .assign(dims, dims + ndims);
    data_values_  .assign(data, data + getndata());

    ParanoidCheck();
}

//============================================================================
TDBValue::TDBValue
    (int                        key
    ,std::vector<int> const&    dims
    ,std::vector<double> const& data
    ,std::string const&         gloss
    )
    :key_          (key)
    ,axis_lengths_ (dims)
    ,data_values_  (data)
    ,gloss_        (gloss)
{
    ParanoidCheck();
}

/// Handy ctor for scalar data.

TDBValue::TDBValue
    (int                key
    ,double             datum
    ,std::string const& gloss
    )
    :key_          (key)
    ,gloss_        (gloss)
{
    axis_lengths_ .assign(ScalarDims, ScalarDims + e_number_of_axes);
    data_values_  .push_back(datum);
}

//============================================================================
TDBValue::TDBValue(TDBValue const& z)
    :obstruct_slicing<TDBValue>()
    ,key_          (z.key_)
    ,axis_lengths_ (z.axis_lengths_)
    ,data_values_  (z.data_values_)
    ,gloss_        (z.gloss_)
{
}

//============================================================================
TDBValue& TDBValue::operator=(TDBValue const& z)
{
    if(this != &z)
        {
        key_          = z.key_;
        axis_lengths_ = z.axis_lengths_;
        data_values_  = z.data_values_;
        gloss_        = z.gloss_;
        }
    return *this;
}

//============================================================================
TDBValue::~TDBValue()
{
}

//============================================================================
void TDBValue::ParanoidCheck() const
{
    if
        (
            axis_lengths_.end()
        !=  std::find
            (axis_lengths_.begin()
            ,axis_lengths_.end()
            ,0
            )
        )
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key_].ShortName
            << "' with key "
            << key_
            << " has zero in at least one dimension."
            << LMI_FLUSH
            ;
        }

    LMI_ASSERT(getndata() == static_cast<int>(data_values_.size()));
    LMI_ASSERT(DB_FIRST <= key_ && key_ < DB_LAST);
    LMI_ASSERT(e_number_of_axes == axis_lengths_.size());
}

//============================================================================
int TDBValue::getndata() const
{
    LMI_ASSERT(!axis_lengths_.empty());

    // Calculate number of elements required from lengths of axes.
    // Use a double for this purpose so that we can detect whether
    // the required number exceeds the maximum addressable number,
    // because a double has a wider range than an integer type.
    double n = std::accumulate
        (axis_lengths_.begin()
        ,axis_lengths_.end()
        ,1.0
        ,std::multiplies<double>()
        );

    // Meaningful iff a long int is bigger than an int.
    if(MaxPossibleElements < n)
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key_].ShortName
            << "' with key "
            << key_
            << " contains more than the maximum possible number of elements."
            << LMI_FLUSH
            ;
        }

    if(0 == n)
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key_].ShortName
            << "' with key "
            << key_
            << " has no data."
            << LMI_FLUSH
            ;
        }

    // Because MaxPossibleElements < n, this cast cannot lose information.
    return static_cast<int>(n);
}

//============================================================================
double& TDBValue::operator[](std::vector<int> const& idx)
{
    LMI_ASSERT(e_number_of_axes == idx.size());

    if(e_number_of_axes != axis_lengths_.size())
        {
        fatal_error()
            << "Trying to index database with key "
            << key_
            << ": "
            << "e_number_of_axes is " << e_number_of_axes
            << ", and axis_lengths.size() is " << axis_lengths_.size()
            << ", but those quantities must be equal."
            << LMI_FLUSH
            ;
        }

    int z = 0;
    // TODO ?? Can we use an STL algorithm instead?
    for(unsigned int j = 0; j < axis_lengths_.size(); j++)
        {
        if(1 != axis_lengths_[j])
            {
            LMI_ASSERT(idx[j] < axis_lengths_[j]);
            z = z * axis_lengths_[j] + idx[j];
            }
        }
// TODO ?? erase    z *= axis_lengths_.back();
    if(static_cast<int>(data_values_.size()) <= z)
        {
        z = 0;
        fatal_error()
            << "Trying to index database item with key "
            << key_
            << " past end of data."
            << LMI_FLUSH
            ;
        }
    return data_values_[z];
}

//============================================================================
double const* TDBValue::operator[](TDBIndex const& idx) const
{
    std::vector<double> index(idx.GetIdx());

    LMI_ASSERT(0 < axis_lengths_.size());
    int z = 0;
    // TODO ?? Can we use an STL algorithm instead?
    for(unsigned int j = 0; j < axis_lengths_.size() - 1; j++)
        {
        if(1 != axis_lengths_[j])
            {
            LMI_ASSERT(index[j] < axis_lengths_[j]);
            z = z * axis_lengths_[j] + static_cast<int>(index[j]);
            }
        }
    z *= axis_lengths_.back();
    if(static_cast<int>(data_values_.size()) <= z)
        {
        z = 0;
        fatal_error()
            << "Trying to index database item with key "
            << key_
            << " past end of data."
            << LMI_FLUSH
            ;
        }
    return &data_values_[z];
}

//============================================================================
void TDBValue::Reshape(std::vector<int> const& dims)
{
    // Create a new instance of this class having the same
    // key but the desired number of axes
    std::vector<double> new_data
        (
        std::accumulate
            (dims.begin()
            ,dims.end()
            ,1
            ,std::multiplies<int>()
            )
        );
    TDBValue new_object
        (GetKey()
        ,dims
        ,new_data
        );

    // ET !! std::vector<int> max_dims_used = max(axis_lengths_, dims);
    // ...and then expunge this comment:
    // greater length of src or dst along each axis
    std::vector<int> max_dims_used(e_number_of_axes);
    std::transform
        (axis_lengths_.begin()
        ,axis_lengths_.end()
        ,dims.begin()
        ,max_dims_used.begin()
        ,greater_of<int>()
        );
    // TODO ?? Oops--erase above std::transform() call--want only dst axes.
    max_dims_used = dims;

    // Number of times we'll go through the assignment loop.
    // TODO ?? prolly should use max_dims_used instead of dims here (they're the same).
    int n_iter = std::accumulate
        (dims.begin()
        ,dims.end()
        ,1
        ,std::multiplies<int>()
        );

    // ET !! std::vector<int> dst_max_idx = dims - 1;
    // ...and then expunge this comment:
    // max index of dst along each axis
    std::vector<int> dst_max_idx(dims);
    std::transform
        (dst_max_idx.begin()
        ,dst_max_idx.end()
        ,dst_max_idx.begin()
        ,std::bind2nd(std::minus<int>(), 1)
        );
    // ET !! std::vector<int> src_max_idx = axis_lengths_ - 1;
    // ...and then expunge this comment:
    // max index of src along each axis
    std::vector<int> src_max_idx(axis_lengths_);
    std::transform
        (src_max_idx.begin()
        ,src_max_idx.end()
        ,src_max_idx.begin()
        ,std::bind2nd(std::minus<int>(), 1)
        );

    // indexes new_object
    std::vector<int> dst_idx(e_number_of_axes);
    // indexes '*this'
    std::vector<int> src_idx(e_number_of_axes);

    std::vector<int> working_idx(e_number_of_axes);
    for(int j = 0; j < n_iter; j++)
        {
        int z = j;
        std::vector<int>::const_iterator i = max_dims_used.begin();
        std::vector<int>::iterator w = working_idx.begin();
        while(i != max_dims_used.end())
            {
            LMI_ASSERT(0 != *i);
            *w = z % *i;
            z /= *i;
            i++;
            w++;
            }
        LMI_ASSERT(0 == z);

        // ET !! dst_idx = min(working_idx, dst_max_idx)
        // ET !! src_idx = min(working_idx, src_max_idx)
        // ...and then expunge this comment:
        // limit dst and source indexes to those that actually vary
        std::transform
            (working_idx.begin()
            ,working_idx.end()
            ,dst_max_idx.begin()
            ,dst_idx.begin()
            ,lesser_of<int>()
            );
        std::transform
            (working_idx.begin()
            ,working_idx.end()
            ,src_max_idx.begin()
            ,src_idx.begin()
            ,lesser_of<int>()
            );
        new_object[dst_idx] = operator[](src_idx);
        }

// erase    (*this) = new_object;
    axis_lengths_ = dims;
    data_values_ = new_object.data_values_;
}

//============================================================================
std::ostream& TDBValue::write(std::ostream& os) const
{
    os
        << '"' << GetDBNames()[key_].LongName << '"'
        << '\n'
        << "  name='" << GetDBNames()[key_].ShortName << "'"
        << " key=" << key_
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

//============================================================================
// TODO ?? Combine this with ParanoidCheck()?
bool TDBValue::AreAllAxesOK() const
{
    bool rc = true;
    std::vector<int> const& max_dims(maximum_dimensions());
    LMI_ASSERT(axis_lengths_.size() == max_dims.size());
    std::vector<int>::const_iterator ai = axis_lengths_.begin();
    std::vector<int>::const_iterator mi = max_dims.begin();

    while(ai != axis_lengths_.end()) // not duration!
        {
        if(*ai != 1 && *ai != *mi && *ai != axis_lengths_.back())
            {
            warning()
                << "Database item '"
                << GetDBNames()[key_].ShortName
                << "' with key "
                << key_
                << " has invalid length in at least one dimension."
                << LMI_FLUSH
                ;
            rc = false;
            }
        ai++;
        mi++;
        }

    if(max_dims.back() < axis_lengths_.back())
            {
            warning()
                << "Database item '"
                << GetDBNames()[key_].ShortName
                << "' with key "
                << key_
                << " has invalid duration."
                << LMI_FLUSH
                ;
            rc = false;
            }
    return rc;
}

// potential inlines

//============================================================================
int TDBValue::GetKey() const
{
    return key_;
}

//============================================================================
int TDBValue::GetNDims() const
{
    return axis_lengths_.size();
}

//============================================================================
int TDBValue::GetLength() const
{
    LMI_ASSERT(0 < axis_lengths_.size());
    return axis_lengths_.back();
}

//============================================================================
int TDBValue::GetLength(int axis) const
{
    LMI_ASSERT(0 <= axis && axis < static_cast<int>(axis_lengths_.size()));
    return axis_lengths_[axis];
}

//============================================================================
std::vector<int> const& TDBValue::GetAxisLengths() const
{
    return axis_lengths_;
}

/*
Implementation
    duration should be ***last*** axis
        for a C array with dimensions d0, d1, d2
            int a[d0][d1][d2]
        index
            a[i0][i1][i2]
        is
            *( ((int*)a) + ((((i0*d1)+i1)*s2)+i2) )
        so we can do
            int* p =( ((int*)a) + (((i0*d1)+i1)*s2) )
            for(int j = length; 0 < j; j--)
                *output++ = *p++;

    bands on...each item?
    scalar marker?
*/

void TDBValue::read(xml::element const& e)
{
    std::string short_name;
    xml_serialize::get_element(e, "key"         , short_name   );
    key_ = db_key_from_name(short_name);
    xml_serialize::get_element(e, "axis_lengths", axis_lengths_);
    xml_serialize::get_element(e, "data_values" , data_values_ );
    xml_serialize::get_element(e, "gloss"       , gloss_       );

    LMI_ASSERT(getndata() == static_cast<int>(data_values_.size()));
    LMI_ASSERT
        (   0 < static_cast<int>(data_values_.size())
        &&      static_cast<int>(data_values_.size()) < MaxPossibleElements
        );
    AreAllAxesOK();
}

void TDBValue::write(xml::element& e) const
{
    LMI_ASSERT(getndata() == static_cast<int>(data_values_.size()));
    LMI_ASSERT
        (   0 < static_cast<int>(data_values_.size())
        &&      static_cast<int>(data_values_.size()) < MaxPossibleElements
        );
    AreAllAxesOK();

    xml_serialize::set_element(e, "key"         , db_name_from_key(key_));
    xml_serialize::set_element(e, "axis_lengths", axis_lengths_);
    xml_serialize::set_element(e, "data_values" , data_values_ );
    xml_serialize::set_element(e, "gloss"       , gloss_       );
}

