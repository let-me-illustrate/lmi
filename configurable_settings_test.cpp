// Configurable settings--unit test.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "configurable_settings.hpp"

#include "miscellany.hpp"   // ios_out_trunc_binary()
#include "path_utility.hpp" // initialize_filesystem()
#include "test_tools.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

class configurable_settings_test
{
  public:
    static void test()
        {
        test_normal_usage();
        test_writability();
        test_backward_compatibility();
        }

  private:
    static void test_normal_usage();
    static void test_writability();
    static void test_backward_compatibility();
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

/// Test each aspect of backward compatibility:
///  - missing 'version' attribute
///  - missing elements
///  - renamed elements
///  - a withdrawn element
///  - an obsolete skin name

void configurable_settings_test::test_backward_compatibility()
{
    fs::path const filename("eraseme");
    fs::ofstream ofs(filename, ios_out_trunc_binary());
    ofs
        << "<?xml version=\"1.0\"?>\n"
        << "<configurable_settings>\n"
        << "<custom_input_filename>[renamed]</custom_input_filename>\n"
        << "<custom_output_filename>[renamed]</custom_output_filename>\n"
        << "<xml_schema_filename>[withdrawn]</xml_schema_filename>\n"
        << "<skin_filename>xml_notebook.xrc</skin_filename>\n"
        << "</configurable_settings>\n"
        ;
    ofs.close();

    configurable_settings& c = configurable_settings::instance();
    c.xml_serializable<configurable_settings>::load(filename);
    BOOST_TEST_EQUAL("[renamed]"   , c.custom_input_0_filename());
    BOOST_TEST_EQUAL("custom.inix" , c.custom_input_1_filename());
    BOOST_TEST_EQUAL("[renamed]"   , c.custom_output_0_filename());
    BOOST_TEST_EQUAL("custom.out1" , c.custom_output_1_filename());
    BOOST_TEST_EQUAL("skin.xrc"    , c.skin_filename());
}

int test_main(int, char*[])
{
    // Absolute paths require "native" name-checking policy for msw.
    initialize_filesystem();

    configurable_settings_test::test();

    return EXIT_SUCCESS;
}

