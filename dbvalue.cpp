// Product database entity type.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: dbvalue.cpp,v 1.7 2008-12-27 02:56:40 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
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
}

//============================================================================
TDBValue::TDBValue()
    :key(0)
    ,ndims(0)
    ,dims(0)
    ,ndata(0)
    ,data(0)
{
}

//============================================================================
TDBValue::TDBValue
    (int     inputkey
    ,int     inputndims
    ,int*    inputdims
    ,double* inputdata
    )
    :key   (inputkey)
    ,ndims (inputndims)
{
    dims = new int[ndims];
    deprecated::dupmem(dims, inputdims, ndims);
    ndata = getndata();

    data = new double[ndata];
    deprecated::dupmem(data, inputdata, ndata);
}

//============================================================================
TDBValue::TDBValue(TDBValue const& rhs)
    :obstruct_slicing<TDBValue>()
    ,key   (rhs.key)
    ,ndims (rhs.ndims)
    ,ndata (rhs.ndata)
{
    dims = new int[ndims];
    deprecated::dupmem(dims, rhs.dims, ndims);
    data = new double[ndata];
    deprecated::dupmem(data, rhs.data, ndata);
}

//============================================================================
TDBValue& TDBValue::operator=(TDBValue const& rhs)
{
    if(this != &rhs)
        {
        key     = rhs.key;
        ndata   = rhs.ndata;
        ndims   = rhs.ndims;
        delete[]dims;
        delete[]data;
        dims = new int[ndims];
        deprecated::dupmem(dims, rhs.dims, ndims);
        data = new double[ndata];
        deprecated::dupmem(data, rhs.data, ndata);
        }
    return *this;
}

//============================================================================
TDBValue::~TDBValue()
{
    delete[]dims;
    delete[]data;
}

//============================================================================
int TDBValue::getndata()
{
    if(0 == ndims)
        {
        return 0;
        }

    long int n = 1L;

    for(int j = 0; j < ndims; j++)
        {
        n *= dims[j];
        }

    if(std::numeric_limits<int>::max() < n)
        {
        fatal_error() << "Too much data in database." << LMI_FLUSH;
        }
    return static_cast<int>(n);
}

//============================================================================
double* TDBValue::operator[](int const* idx) const
{
    int z = 0;
    for(int j = 0; j < ndims - 1; j++)
        {
        if(1 != dims[j])
            {
            z = z * dims[j] + idx[j];
            }
        }
    if(ndata <= z)
        {
        throw std::runtime_error
            ("Trying to index database item past end of data."
            );
        }
    return &data[z];
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

