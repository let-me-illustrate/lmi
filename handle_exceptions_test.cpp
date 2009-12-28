// Handlers for exceptional situations--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: handle_exceptions_test.cpp,v 1.7 2008-12-27 02:56:42 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "handle_exceptions.hpp"

#include "test_tools.hpp"

#include <stdexcept>

int test_main(int, char*[])
{
    try
        {
        BOOST_TEST(true);
        throw std::runtime_error("  Test succeeded");
        BOOST_TEST(false); // Shouldn't be reached.
        }
    catch(...)
        {
        report_exception();
        }

    return 0;
}

