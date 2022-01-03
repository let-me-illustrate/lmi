// Helpers for working with output files in wx test suite.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef wx_test_output_hpp
#define wx_test_output_hpp

#include "config.hpp"

#include "path.hpp"

/// Class helping to check for the expected output file existence.
///
/// This class takes care of ensuring that the file doesn't exist when it is
/// constructed and provides a way to check for the existence of the file
/// later. It also cleans up the file when it is destroyed.

class output_file_existence_checker
{
  public:
    output_file_existence_checker(fs::path const& path)
        :path_ {path}
        {
        fs::remove(path_);
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

    bool exists() const
        {
        return fs::exists(path_);
        }

    std::string path() const
        {
        return path_.string();
        }

    // Objects of this class can't be copied, because of side effects of its
    // dtor, but can be moved.
    output_file_existence_checker(output_file_existence_checker&&) = default;
    output_file_existence_checker& operator=(output_file_existence_checker&&) = default;

    output_file_existence_checker(output_file_existence_checker const&) = delete;
    output_file_existence_checker& operator=(output_file_existence_checker const&) = delete;

  private:
    fs::path path_;
};

#endif // wx_test_output_hpp
