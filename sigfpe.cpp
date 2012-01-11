// Floating-point exception handler.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

// This is not portable because a signal handler isn't guaranteed to
// work if it does anything other than set a volatile global or static
// variable, without calling any standard library function other than
// std::signal().
//
// This has several problems with gcc on msw. It might trap the first
// SIGFPE only. The unit-testing code below apparently cannot be made
// to work. This file is retained because it might be useful on other
// platforms (on which it hasn't been tested).

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "sigfpe.hpp"

#include "alert.hpp"

#include <cfloat> // DBL_MAX
#include <csignal>
#include <string>

void floating_point_error_handler(int)
{
    hobsons_choice() << "Floating point error." << LMI_FLUSH;

    if(SIG_ERR == std::signal(SIGFPE, floating_point_error_handler))
        {
        fatal_error()
            << "Cannot reinstall floating point error signal handler."
            << LMI_FLUSH
            ;
        }
}

// TODO ?? Make this a standalone unit test.
//
void fpe_handler_test()
{
//    volatile int i0 = 0;
//    volatile int i1 = 1;
//    volatile int i2;
//
// CPU Integer divide by zero.
// This one doesn't seem recoverable--handler goes into loop.
//  i2 = i1 / i0;
//  i2 = i1 % i0;

    volatile double d0 = 0.0;
    volatile double d1 = 1.0;
    volatile double dm = DBL_MAX;
    volatile double d2;

// FPU divide by zero. (Note: 0/0 is invalid operation, not divide by 0.)
    d2 = d1 / d0;

// FPU arithmetic overflow. TODO ?? Is this right, and does it work?
    d2 = dm;
    d2 *= dm;

// FPU arithmetic underflow. TODO ?? Isn't this overflow? Does this work?
    d2 = (d1 / dm) / dm;

// FPU precision loss. TODO ?? Is this right, and does it work?
    d2 = d1 / dm;
    d2 /= dm;

// SIGFPE explicitly raised.
    std::raise(SIGFPE);

// FPU invalid operation.
    d2 = d0 / d0;
}

