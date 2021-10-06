// Global settings--unit test.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "global_settings.hpp"

#include "path.hpp"
#include "test_tools.hpp"

void test_directory_exceptions()
{
    // Test an exception for each 'directory' member to make sure
    // the member context (e.g., "Data directory") is correctly
    // reported.

    // String values assigned to 'directory' data members must be
    // valid "directory paths". In particular:

    // Such string values must not be empty.

    LMI_TEST_THROW
        (global_settings::instance().set_data_directory("")
        ,std::runtime_error
        ,"Data directory must not be empty."
        );

    // Such string values must be validly-formed paths.
    //
    // SOMEDAY !! boost::filesystem detected that "?" is not a
    // valid directory name, at least for msw. Should that useful
    // validation be regained by adding code to lmi, now that
    // std::filesystem doesn't validate paths semantically? If not,
    // then this test adds no value.

    LMI_TEST_THROW
        (global_settings::instance().set_data_directory("?")
        ,std::runtime_error
        ,"Data directory '?' not found."
        );

    // Such string values must name paths that exist in the filesystem.

    LMI_TEST_THROW
        (global_settings::instance().set_data_directory("UnLiKeLyNaMe")
        ,std::runtime_error
        ,"Data directory 'UnLiKeLyNaMe' not found."
        );

    // Such string values must name directories, not normal files.

    fs::path const p(__FILE__"_test_file");
    fs::ofstream ofs(p);
    ofs << p.string() << std::endl;
    ofs.close();
    LMI_TEST_THROW
        (global_settings::instance().set_data_directory(p.string())
        ,std::runtime_error
        ,"Data directory '" + p.string() + "' is not a directory."
        );
    fs::remove(p);
}

int test_main(int, char*[])
{
    // Initial values of 'directory' data members must be valid: the
    // operations tested here are required not to throw.

    global_settings::instance().set_data_directory
        (global_settings::instance().data_directory().string()
        );

    fs::path path(global_settings::instance().data_directory());

    // The data directory is supposed to exist.
    LMI_TEST(fs::is_directory(path));

    // Certain other operations are required to throw.

    test_directory_exceptions();

    // 'ash_nazg' implies 'mellon'.
    global_settings::instance().set_mellon(false);
    LMI_TEST(!global_settings::instance().mellon());

    global_settings::instance().set_ash_nazg(true);
    LMI_TEST( global_settings::instance().mellon());

    return 0;
}
