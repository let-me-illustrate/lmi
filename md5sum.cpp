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

#include "pchfile.hpp"

#include "assert_lmi.hpp"
#include "md5.hpp"
#include "md5sum.hpp"

#include <cstddef>                      // size_t
#include <fstream>
#include <iomanip>                      // hex, setfill(), setw()
#include <ios>                          // ios_base, streamsize
#include <istream>
#include <sstream>
#include <stdexcept>

std::vector<md5sum_for_file> md5_read_checksum_stream
    (std::istream     & is
    ,std::string const& stream_description
    )
{
    std::vector<md5sum_for_file> result_vec;

    int line_number = 0;
    auto error_with_message = [&line_number, stream_description] (char const* message)
        {
        std::ostringstream oss;
        oss << "'" << stream_description << "': " << message << " at line " << line_number;
        return oss.str();
        };

    std::string line;
    while(std::getline(is, line))
        {
        ++line_number;

        // The minimal length: 32(md5sum) + 2(spaces) + 1(shortest file name)
        std::size_t const minimal_line_length = 35;

        // Ignore comment lines, which begin with a '#' character.
        // Empty lines are not allowed.
        if(!line.empty() && line.front() == '#')
            {
            continue;
            }

        if(line.size() < minimal_line_length)
            {
            throw std::runtime_error(error_with_message("line too short"));
            }

        auto const space_pos = line.find(' ');
        if(line.size() - 1 <= space_pos)
            {
            throw std::runtime_error(error_with_message("incorrect checksum line format"));
            }

        char const second_delimiter = line[space_pos + 1];
        md5_file_mode file_mode;
        if(second_delimiter == ' ')
            {
            file_mode = md5_file_mode::text;
            }
        else if(second_delimiter == '*')
            {
            file_mode = md5_file_mode::binary;
            }
        else
            {
            throw std::runtime_error(error_with_message("incorrect checksum line format"));
            }

        std::string md5sum = line.substr(0, space_pos);
        std::string file = line.substr(space_pos + 2);

        if(md5sum.size() != chars_per_formatted_hex_byte * md5len)
            {
            throw std::runtime_error(error_with_message("incorrect MD5 sum format"));
            }

        result_vec.emplace_back(std::move(file), std::move(md5sum), file_mode);
        }

    return result_vec;
}

std::vector<md5sum_for_file> md5_read_checksum_file(fs::path const& filename)
{
    auto const filename_string = filename.string();

    std::ifstream is(filename_string);
    if(!is)
        {
        std::ostringstream oss;
        oss << "'" << filename_string << "': no such file or directory";
        throw std::runtime_error(oss.str());
        }

    return md5_read_checksum_stream(is, filename_string);
}

std::string md5_calculate_stream_checksum
    (std::istream     & is
    ,std::string const& stream_description
    )
{
    std::vector<unsigned char> md5(md5len);

    // Note that block_size must be a multiple of 64 to use md5_process_block()
    // below.
    constexpr std::streamsize block_size = 4096;
    md5_ctx ctx;
    char buffer[block_size];
    std::streamsize read_count;

    // Initialize the computation context.
    md5_init_ctx(&ctx);

    // Iterate over full file contents.
    for(;;)
        {
        // We read the file in blocks of block_size bytes. One call of the
        // computation function processes the whole buffer so that with the
        // next round of the loop another block can be read.
        is.read(buffer, block_size);
        read_count = is.gcount();

        // If end of file is reached, end the loop.
        if(is.eof())
            {
            break;
            }

        if(read_count != block_size || !is)
            {
            std::ostringstream oss;
            oss
                << "'"
                << stream_description
                << "': failed to read data while computing md5sum"
                ;
            throw std::runtime_error(oss.str());
            }

        // Process buffer with block_size bytes. Note that
        // block_size % 64 == 0
        md5_process_block(buffer, block_size, &ctx);
        }

    // Add the last bytes if necessary.
    if(0 < read_count)
        {
        // Note that we have to use md5_process_bytes() and not the faster
        // md5_process_block() here because the read_count is not necessarily
        // a multiple of 64 here.
        md5_process_bytes(buffer, read_count, &ctx);
        }

    // Construct result in desired memory.
    md5_finish_ctx(&ctx, md5.data());

    return md5_hex_string(md5);
}

std::string md5_calculate_file_checksum
    (fs::path const& filename
    ,md5_file_mode   file_mode
    )
{
    auto const filename_string = filename.string();

    std::vector<unsigned char> md5(md5len);

    std::ios_base::openmode open_mode{std::ios_base::in};
    switch(file_mode)
        {
        case md5_file_mode::binary:
            open_mode |= std::ios_base::binary;
            break;
        case md5_file_mode::text:
            // Nothing to do.
            break;
        }

    std::ifstream is(filename_string, open_mode);
    if(!is)
        {
        std::ostringstream oss;
        oss << "'" << filename_string << "': no such file or directory";
        throw std::runtime_error(oss.str());
        }

    return md5_calculate_stream_checksum(is, filename_string);
}

std::string md5_hex_string(std::vector<unsigned char> const& vuc)
{
    LMI_ASSERT(md5len == vuc.size());
    std::stringstream oss;
    oss << std::hex;
    for(auto const& j : vuc)
        {
        oss
            << std::setw(chars_per_formatted_hex_byte)
            << std::setfill('0')
            << static_cast<int>(j)
            ;
        }
    return oss.str();
}
