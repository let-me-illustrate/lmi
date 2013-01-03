// Premium tax--unit test.
//
// Copyright (C) 2011, 2012, 2013 Gregory W. Chicares.
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

#include "premium_tax.hpp"

#include "path_utility.hpp" // initialize_filesystem()
#include "test_tools.hpp"

class premium_tax_test
{
  public:
    static void test()
        {
        test_something();
        }

  private:
    static void test_something();
};

/// Placeholder.

void premium_tax_test::test_something()
{
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.
    initialize_filesystem();

    premium_tax_test::test();

    return EXIT_SUCCESS;
}

