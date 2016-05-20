// Helpers for working with output files in wx test suite.
//
// Copyright (C) 2015, 2016 Gregory W. Chicares.
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

#ifndef wx_test_output_hpp
#define wx_test_output_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <boost/filesystem/operations.hpp>

/// Class helping to check for the expected output file existence.
///
/// This class takes care of ensuring that the file doesn't exist when it is
/// constructed and provides a way to check for the existence of the file
/// later. It also cleans up the file when it is destroyed.

class output_file_existence_checker
    :private lmi::uncopyable<output_file_existence_checker>
{
  public:
    output_file_existence_checker(fs::path const& path)
        :path_(path)
        {
        fs::remove(path_);
        }

    bool exists() const
        {
        return fs::exists(path_);
        }

    ~output_file_existence_checker()
        {
        try
            {
            fs::remove(path_);
            }
        catch(...)
            {
            }
        }

  private:
    fs::path path_;
};

#endif // wx_test_output_hpp
