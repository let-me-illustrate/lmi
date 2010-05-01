// Product-database entity.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include <limits>
#include <stdexcept>

// TODO ?? This must die.
namespace deprecated
{
    template<typename T, typename SIZE_T>
    void dupmem(T* dest, T const*const value, SIZE_T n)
    {
        for(SIZE_T j = 0; j < n; j++)
            dest[j] = value[j];
    }
} // namespace deprecated

//============================================================================
TDBValue::TDBValue()
    :key_  (0)
    ,ndims_(0)
    ,dims_ (0)
    ,ndata_(0)
    ,data_ (0)
{
}

//============================================================================
TDBValue::TDBValue
    (int                key
    ,int                ndims
    ,int const*         dims
    ,double const*      data
    ,std::string const& gloss
    )
    :key_   (key)
    ,ndims_ (ndims)
    ,gloss_ (gloss)
{
    dims_ = new int[ndims_];
    deprecated::dupmem(dims_, dims, ndims_);
    ndata_ = getndata();

    data_ = new double[ndata_];
    deprecated::dupmem(data_, data, ndata_);
}

//============================================================================
TDBValue::TDBValue(TDBValue const& z)
    :obstruct_slicing<TDBValue>()
    ,key_   (z.key_  )
    ,ndims_ (z.ndims_)
    ,ndata_ (z.ndata_)
{
    dims_ = new int[ndims_];
    deprecated::dupmem(dims_, z.dims_, ndims_);
    data_ = new double[ndata_];
    deprecated::dupmem(data_, z.data_, ndata_);
}

//============================================================================
TDBValue& TDBValue::operator=(TDBValue const& z)
{
    if(this != &z)
        {
        key_   = z.key_;
        ndata_ = z.ndata_;
        ndims_ = z.ndims_;
        delete[]dims_;
        delete[]data_;
        dims_ = new int[ndims_];
        deprecated::dupmem(dims_, z.dims_, ndims_);
        data_ = new double[ndata_];
        deprecated::dupmem(data_, z.data_, ndata_);
        }
    return *this;
}

//============================================================================
TDBValue::~TDBValue()
{
    delete[]dims_;
    delete[]data_;
}

//============================================================================
int TDBValue::getndata() const
{
    if(0 == ndims_)
        {
        return 0;
        }

    long int n = 1L;

    for(int j = 0; j < ndims_; j++)
        {
        n *= dims_[j];
        }

    if(std::numeric_limits<int>::max() < n)
        {
        fatal_error() << "Too much data in database." << LMI_FLUSH;
        }
    return static_cast<int>(n);
}

//============================================================================
double const* TDBValue::operator[](int const* idx) const
{
    int z = 0;
    for(int j = 0; j < ndims_ - 1; j++)
        {
        if(1 != dims_[j])
            {
            z = z * dims_[j] + idx[j];
            }
        }
    if(ndata_ <= z)
        {
        throw std::runtime_error
            ("Trying to index database item past end of data."
            );
        }
    return &data_[z];
}

/*
Implementation
    TODO ?? duration should be ***last*** axis
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

    TODO ?? must-be-scalar flag?
*/

