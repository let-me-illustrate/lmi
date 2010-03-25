// Database entity type.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_dbvalue.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "dbnames.hpp"
#include "math_functors.hpp" // greater_of(), lesser_of
#include "print_matrix.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <functional>
#include <istream>
#include <limits>       // numeric_limits<>
#include <numeric>
#include <ostream>

#ifndef stlstrm_hpp
#define stlstrm_hpp

// This misbegotten thing used to be a distinct header, but now we've
// extirpated it everywhere else. TODO ?? Stop using it here too.

// Stream STL containers with Joshua Rowe's object streaming.

#include "ihs_fpios.hpp"
namespace JRPS = JOSHUA_ROWE_PERSISTENT_STREAMS;

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

template<typename T>
JRPS::JrPs_ipstream& operator>> (JRPS::JrPs_ipstream& ips, std::vector<T>& x)
{
    x.erase(x.begin(), x.end());
    typename std::vector<T>::size_type vector_size;
    ips >> vector_size;
    x.reserve(vector_size);
    typename std::vector<T>::value_type z;
    for(typename std::vector<T>::size_type j = 0; j < vector_size; j++)
        {
        ips >> z;
        x.push_back(z);
        }
    assert(vector_size == x.size());
    return ips;
}

template<typename T>
JRPS::JrPs_opstream& operator<< (JRPS::JrPs_opstream& ops, std::vector<T> const& x)
{
    ops << x.size();
    for(typename std::vector<T>::const_iterator i = x.begin(); i < x.end(); i++)
        {
        ops << *i;
        }
    return ops;
}

#endif // stlstrm_hpp

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
    return(a.axis_lengths == b.axis_lengths && a.data_values == b.data_values);
}

//============================================================================
bool TDBValue::VariesByState(TDBValue const& z)
{
    LMI_ASSERT(5 < z.axis_lengths.size());
    return 1 != z.axis_lengths[5];
}

//============================================================================
TDBValue::TDBValue()
    :key(0)
    ,axis_lengths(e_number_of_axes)
{
}

//============================================================================
TDBValue::TDBValue
    (int           a_key
    ,int           a_ndims
    ,int const*    a_dims
    ,double const* a_data
    )
    :key(a_key)
{
    axis_lengths    .assign(a_dims, a_dims + a_ndims);
    data_values     .assign(a_data, a_data + getndata());

    ParanoidCheck();
}

//============================================================================
TDBValue::TDBValue
    (int                        a_key
    ,std::vector<int> const&    a_dims
    ,std::vector<double> const& a_data
    )
    :key(a_key)
    ,axis_lengths(a_dims)
    ,data_values(a_data)
{
    ParanoidCheck();
}

//============================================================================
// scalar only
TDBValue::TDBValue
    (int    a_key
    ,double a_datum
    )
    :key(a_key)
{
    axis_lengths    .assign(ScalarDims, ScalarDims + e_number_of_axes);
    data_values     .push_back(a_datum);
}

//============================================================================
TDBValue::TDBValue
    (int                      a_key
    ,int                      a_ndims
    ,int const*               a_dims
    ,double const*            a_data
    ,std::vector<std::string> a_extra_axes_names
    ,std::vector<double>      a_extra_axes_values
    )
    :key(a_key)
    ,extra_axes_names(a_extra_axes_names)
    ,extra_axes_values(a_extra_axes_values)
{
    axis_lengths    .assign(a_dims, a_dims + a_ndims);
    data_values     .assign(a_data, a_data + getndata());

    ParanoidCheck();
}

//============================================================================
TDBValue::TDBValue(TDBValue const& obj)
    :JRPS::JrPs_pstreamable()
    ,key(obj.key)
    ,axis_lengths(obj.axis_lengths)
    ,data_values(obj.data_values)
{
}

//============================================================================
TDBValue& TDBValue::operator=(TDBValue const& obj)
{
    if(this != &obj)
        {
        key             = obj.key;
        axis_lengths    = obj.axis_lengths;
        data_values     = obj.data_values;
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
            axis_lengths.end()
        !=  std::find
            (axis_lengths.begin()
            ,axis_lengths.end()
            ,0
            )
        )
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key].ShortName
            << "' with key "
            << key
            << " has zero in at least one dimension."
            << LMI_FLUSH
            ;
        }

    LMI_ASSERT(getndata() == static_cast<int>(data_values.size()));
    LMI_ASSERT(DB_FIRST <= key && key < DB_LAST);
    LMI_ASSERT(e_number_of_axes == axis_lengths.size());
}

//============================================================================
int TDBValue::getndata() const
{
    LMI_ASSERT(!axis_lengths.empty());

    // Calculate number of elements required from lengths of axes.
    // Use a double for this purpose so that we can detect whether
    // the required number exceeds the maximum addressable number,
    // because a double has a wider range than an integer type.
    double n = std::accumulate
        (axis_lengths.begin()
        ,axis_lengths.end()
        ,1.0
        ,std::multiplies<double>()
        );

    // Meaningful iff a long int is bigger than an int.
    if(MaxPossibleElements < n)
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key].ShortName
            << "' with key "
            << key
            << " contains more than the maximum possible number of elements."
            << LMI_FLUSH
            ;
        }

    if(0 == n)
        {
        fatal_error()
            << "Database item '"
            << GetDBNames()[key].ShortName
            << "' with key "
            << key
            << " has no data."
            << LMI_FLUSH
            ;
        }

    // Because MaxPossibleElements < n, this cast cannot lose information.
    return static_cast<int>(n);
}

//============================================================================
double& TDBValue::operator[](std::vector<int> const& a_idx)
{
    LMI_ASSERT(e_number_of_axes == a_idx.size());

    if(e_number_of_axes != axis_lengths.size())
        {
        fatal_error()
            << "Trying to index database with key "
            << key
            << ": "
            << "e_number_of_axes is " << e_number_of_axes
            << ", and axis_lengths.size() is " << axis_lengths.size()
            << ", but those quantities must be equal."
            << LMI_FLUSH
            ;
        }

    int z = 0;
    // TODO ?? Can we use an STL algorithm instead?
    for(unsigned int j = 0; j < axis_lengths.size(); j++)
        {
        if(1 != axis_lengths[j])
            {
            LMI_ASSERT(a_idx[j] < axis_lengths[j]);
            // TODO ?? Here gcc warns of assignment to unsigned int from double.
            z = z * axis_lengths[j] + a_idx[j];
            }
        }
// TODO ?? erase    z *= axis_lengths.back();
    if(static_cast<int>(data_values.size()) <= z)
        {
        z = 0;
        fatal_error()
            << "Trying to index database item with key "
            << key
            << " past end of data."
            << LMI_FLUSH
            ;
        }
    return data_values[z];
}

//============================================================================
double const* TDBValue::operator[](TDBIndex const& a_idx) const
{
/*
            tiered_product<double>()
                (assets
                ,TieredMEBands
                ,TieredMECharges
                )

        std::vector<double>::difference_type dist(0);
        distance(CSVBegin, CSVEnd, dist);

    // The first six axes are fixed.
    // Preprocess the others.
    // TODO ?? use std::find(), lower_bound, tiered_product...

    std::vector<double>     extra_axes_values;
    std::vector<e_IdxType>  extra_axes_types;
*/

    std::vector<double>idx(a_idx.GetIdx());
// TODO ?? FixupIndex(idx); // Doesn't work.

    LMI_ASSERT(0 < axis_lengths.size());
    int z = 0;
    // TODO ?? Can we use an STL algorithm instead?
    for(unsigned int j = 0; j < axis_lengths.size() - 1; j++)
        {
        if(1 != axis_lengths[j])
            {
            LMI_ASSERT(idx[j] < axis_lengths[j]);
            z = z * axis_lengths[j] + static_cast<int>(idx[j]);
            }
        }
    z *= axis_lengths.back();
    if(static_cast<int>(data_values.size()) <= z)
        {
        z = 0;
        fatal_error()
            << "Trying to index database item with key "
            << key
            << " past end of data."
            << LMI_FLUSH
            ;
        }
    return &data_values[z];
}

//============================================================================
void TDBValue::Reshape(std::vector<int> const& a_dims)
{
    // Create a new instance of this class having the same
    // key but the desired number of axes
    std::vector<double> new_data
        (
        std::accumulate
            (a_dims.begin()
            ,a_dims.end()
            ,1
            ,std::multiplies<int>()
            )
        );
    TDBValue new_object
        (GetKey()
        ,a_dims
        ,new_data
        );

    // ET !! std::vector<int> max_dims_used = max(axis_lengths, a_dims);
    // ...and then expunge this comment:
    // greater length of src or dst along each axis
    std::vector<int> max_dims_used(e_number_of_axes);
    std::transform
        (axis_lengths.begin()
        ,axis_lengths.end()
        ,a_dims.begin()
        ,max_dims_used.begin()
        ,greater_of<int>()
        );
    // TODO ?? Oops--erase above std::transform() call--want only dst axes.
    max_dims_used = a_dims;

    // Number of times we'll go through the assignment loop.
    // TODO ?? prolly should use max_dims_used instead of a_dims here (they're the same).
    int n_iter = std::accumulate
        (a_dims.begin()
        ,a_dims.end()
        ,1
        ,std::multiplies<int>()
        );

    // ET !! std::vector<int> dst_max_idx = a_dims - 1;
    // ...and then expunge this comment:
    // max index of dst along each axis
    std::vector<int> dst_max_idx(a_dims);
    std::transform
        (dst_max_idx.begin()
        ,dst_max_idx.end()
        ,dst_max_idx.begin()
        ,std::bind2nd(std::minus<int>(), 1)
        );
    // ET !! std::vector<int> src_max_idx = axis_lengths - 1;
    // ...and then expunge this comment:
    // max index of src along each axis
    std::vector<int> src_max_idx(axis_lengths);
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
    axis_lengths = a_dims;
    data_values = new_object.data_values;
}

//============================================================================
std::ostream& TDBValue::write(std::ostream& os) const
{
    os
        << '"' << GetDBNames()[key].LongName << '"'
        << '\n'
        << "  name='" << GetDBNames()[key].ShortName << "'"
        << " key=" << key
        << '\n'
        ;
    if(1 == getndata())
        {
        os << "  scalar";
        }
    else
        {
        os << "  varies by:";
        if(1 != axis_lengths[0]) os <<    " gender[" << axis_lengths[0] << ']';
        if(1 != axis_lengths[1]) os <<  " uw_class[" << axis_lengths[1] << ']';
        if(1 != axis_lengths[2]) os <<   " smoking[" << axis_lengths[2] << ']';
        if(1 != axis_lengths[3]) os << " issue_age[" << axis_lengths[3] << ']';
        if(1 != axis_lengths[4]) os <<  " uw_basis[" << axis_lengths[4] << ']';
        if(1 != axis_lengths[5]) os <<     " state[" << axis_lengths[5] << ']';
        if(1 != axis_lengths[6]) os <<  " duration[" << axis_lengths[6] << ']';
        }
    os << '\n';
    print_matrix(os, data_values, axis_lengths);
    os << '\n';
    return os;
}

//============================================================================
void TDBValue::FixupIndex(std::vector<double>& idx) const
{
    std::vector<double>::iterator           i_idx = idx.begin();
    // debugging
    if(idx.size() <= e_number_of_axes)
        {
        return;
        }
    std::advance(i_idx, e_number_of_axes - 1);
//  std::vector<double>::const_iterator     i_val = extra_axes_values.begin();
    std::vector<e_IdxType>::const_iterator  i_typ = extra_axes_types.begin();
    while(i_idx < idx.end())
        {
        switch(*i_typ)
            {
            case e_Offset:
                {
                // do nothing
                }
                break;
            case e_Discrete:
                {
                std::vector<double>::const_iterator z = std::find
                    (extra_axes_values.begin()
                    ,extra_axes_values.end()
                    ,*i_idx
                    );
                *i_idx = z - extra_axes_values.begin();
                }
                break;
            case e_LowBound:
                {
                std::vector<double>::const_iterator z = std::lower_bound
                    (extra_axes_values.begin()
                    ,extra_axes_values.end()
                    ,*i_idx
                    );
                *i_idx = z - extra_axes_values.begin();
                }
                break;

            case e_Incremental:
                {
                // TODO ?? What to do here?
                }
                break;
            default:
                fatal_error()
                    << "Unknown axis type "
                    << *i_typ
                    << '.'
                    << LMI_FLUSH
                    ;
                break;
            }
        i_idx++;
//      i_val++; // TODO ?? Need std::vector of vectors?
        i_typ++;
        }
}

//============================================================================
// TODO ?? Combine this with ParanoidCheck()?
bool TDBValue::AreAllAxesOK() const
{
    bool rc = true;
    std::vector<int> const& max_dims(maximum_dimensions());
    LMI_ASSERT(axis_lengths.size() == max_dims.size());
    std::vector<int>::const_iterator ai = axis_lengths.begin();
    std::vector<int>::const_iterator mi = max_dims.begin();

    while(ai != axis_lengths.end()) // not duration!
        {
        if(*ai != 1 && *ai != *mi && *ai != axis_lengths.back())
            {
            warning()
                << "Database item '"
                << GetDBNames()[key].ShortName
                << "' with key "
                << key
                << " has invalid length in at least one dimension."
                << LMI_FLUSH
                ;
            rc = false;
            }
        ai++;
        mi++;
        }

    if(max_dims.back() < axis_lengths.back())
            {
            warning()
                << "Database item '"
                << GetDBNames()[key].ShortName
                << "' with key "
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
    return key;
}

//============================================================================
// TODO ?? Isn't the following function useless?
int TDBValue::GetNDims() const
{
    return axis_lengths.size();
}

//============================================================================
int TDBValue::GetLength() const
{
    LMI_ASSERT(0 < axis_lengths.size());
    return axis_lengths.back();
}

//============================================================================
int TDBValue::GetLength(int a_axis) const
{
    LMI_ASSERT(0 <= a_axis && a_axis < static_cast<int>(axis_lengths.size()));
    return axis_lengths[a_axis];
}

//============================================================================
std::vector<int> const& TDBValue::GetAxisLengths() const
{
    return axis_lengths;
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

//===========================================================================
std::istream& operator>>(std::istream& is, TDBValue&)
{
    // SOMEDAY !! Someday we should implement this.
    fatal_error()
        << "operator>>(std::istream&, TDBValue&) not implemented."
        << LMI_FLUSH
        ;
    return is;
}

//===========================================================================
std::ostream& operator<<(std::ostream& os, TDBValue const& z)
{
    return z.write(os);
}

// Streaming implementation

TDBValue::TDBValue(JRPS::JrPs_pstreamableInit)
    :key(0)
    ,axis_lengths(e_number_of_axes)
{
}

JRPS::JrPs_pstreamable* TDBValue::jrps_build()
{
    return new TDBValue(JRPS::JrPs_pstreamableinit);
}

JRPS::JrPs_pstreamreg RegTDBValue
    ("TDBValue"
    ,TDBValue::jrps_build
    ,JRPS_PSTREAM_DELTA(TDBValue)
    );

// TODO ?? Couldn't templates handle this?
JRPS::JrPs_opstream& operator<< (JRPS::JrPs_opstream& os, TDBValue const* p)
{
    return os << (JRPS::JrPs_pstreamable const*)p;
}

JRPS::JrPs_ipstream& operator>> (JRPS::JrPs_ipstream& is, TDBValue*& p)
{
    return is >> (void const*&)p;
}

void* TDBValue::read(JRPS::JrPs_ipstream& is)
{
    int version;
    is >> version;
    if(version < StreamingVersion)
        {
        fatal_error()
            << "Program supports input versions up to "
            << StreamingVersion
            << " but input file is version "
            << version
            << " ."
            << LMI_FLUSH
            ;
        }

    is >> key;
    is >> axis_lengths;
    is >> extra_axes_values;
    is >> extra_axes_names;
    is >> data_values;

    LMI_ASSERT(getndata() == static_cast<int>(data_values.size()));
    LMI_ASSERT
        (   0 < static_cast<int>(data_values.size())
        &&      static_cast<int>(data_values.size()) < MaxPossibleElements
        );

    return this;
}

void TDBValue::write(JRPS::JrPs_opstream& os) const
{
    os << StreamingVersion;

    LMI_ASSERT(getndata() == static_cast<int>(data_values.size()));
    LMI_ASSERT
        (   0 < static_cast<int>(data_values.size())
        &&      static_cast<int>(data_values.size()) < MaxPossibleElements
        );
    AreAllAxesOK();

    os << key;
    os << axis_lengths;
    os << extra_axes_values;
    os << extra_axes_names;
    os << data_values;
}

char const* TDBValue::streamableName() const
{
    return "TDBValue";
}

