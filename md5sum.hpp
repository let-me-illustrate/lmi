// Compute checksums of files or strings.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef md5sum_hpp
#define md5sum_hpp

#include "config.hpp"

#include <boost/filesystem/path.hpp>

#include <climits>                      // CHAR_BIT
#include <iosfwd>
#include <string>
#include <utility>                      // move()
#include <vector>

// The gnu libc md5 implementation seems to assume this:
static_assert(8 == CHAR_BIT || 16 == CHAR_BIT);
// so md5 output is 128 bits == 16 8-bit bytes or 8 16-bit bytes:
constexpr int md5len = 128 / CHAR_BIT;

constexpr int chars_per_formatted_hex_byte = CHAR_BIT / 4;

enum class md5_file_mode
{
    binary,
    text
};

struct md5sum_for_file
{
    md5sum_for_file(fs::path&& path, std::string&& sum, md5_file_mode mode)
        :filename {std::move(path)}
        ,md5sum   {std::move(sum)}
        ,file_mode{mode}
    {}

    fs::path      filename;
    std::string   md5sum;
    md5_file_mode file_mode{md5_file_mode::binary};
};

/// Reads the vector of structs with the file name and the md5 sum from the given input
/// stream.
///
/// Throws an std::runtime_error in case of an error.
///
/// The input stream must consist of lines with checksum and filename pairs and optional
/// comments introduced by '#' character at the beginning of the line.
/// Sample:
/// 595f44fec1e92a71d3e9e77456ba80d1  filetohashA.txt
/// 71f920fa275127a7b60fa4d4d41432a3  filetohashB.txt
/// 43c191bf6d6c3f263a8cd0efd4a058ab  filetohashC.txt
///
/// There must be two spaces or a space and an asterisk between each md5sum
/// value and filename to be compared (the second space indicates text mode,
/// the asterisk binary mode). Otherwise, a std::runtime_error will be thrown.
///
/// The stream_description parameter is only used in exceptions messages.

std::vector<md5sum_for_file> md5_read_checksum_stream
    (std::istream     & is
    ,std::string const& stream_description
    );

/// Reads the vector of structs with the file name and the md5 sum from the file.
///
/// Throws an std::runtime_error in case of an error.
///
/// Uses md5_read_checksum_stream to read the content of the file.

std::vector<md5sum_for_file> md5_read_checksum_file
    (fs::path const& filename
    );

/// Reads the content of the input stream and calculates the md5sum from it.
///
/// Throws an std::runtime_error in case of an error.

std::string md5_calculate_stream_checksum
    (std::istream     & is
    ,std::string const& stream_description
    );

/// Reads the content of the file and calculates the md5sum from it.
///
/// Throws an std::runtime_error in case of an error.

std::string md5_calculate_file_checksum
    (fs::path const& filename
    ,md5_file_mode   file_mode = md5_file_mode::binary
    );

/// Hex representation of an md5 sum as a string.

std::string md5_hex_string(std::vector<unsigned char> const&);

#endif // md5sum_hpp
