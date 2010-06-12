// Configurable settings--unit test.
//
// Copyright (C) 2010 Gregory W. Chicares.
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

#include "configurable_settings.hpp"

#include "path_utility.hpp" // initialize_filesystem()
#include "test_tools.hpp"

class configurable_settings_test
{
  public:
    static void test()
        {
        test_normal_usage();
        test_writability();
        }

  private:
    static void test_normal_usage();
    static void test_writability();
};

/// Test for gross failure upon instantiation.

void configurable_settings_test::test_normal_usage()
{
    configurable_settings const& c = configurable_settings::instance();
    BOOST_TEST(!c.skin_filename().empty());
}

/// Save a copy of the file multiple times (because users might).

void configurable_settings_test::test_writability()
{
    configurable_settings const& c = configurable_settings::instance();
    std::string const filename("eraseme");
    c.xml_serializable<configurable_settings>::save(filename);
    c.xml_serializable<configurable_settings>::save(filename);
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.
    initialize_filesystem();

    configurable_settings_test::test();

    return EXIT_SUCCESS;
}

