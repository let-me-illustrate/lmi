// Stream and stream buffer that discard output like the null device.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "null_stream.hpp"

#include <ios>
#include <ostream>
#include <streambuf>
#include <string>

/// Redirect output as though to a null device.
///
/// This class is a derived work based on Dietmar Kühl's article
///   http://groups.google.com/groups?selm=82mbke$l5c$1@nnrp1.deja.com
/// which bears no copyright notice, as is usual in usenet.
///
/// GWC modified this class in 2007, and in any later year as
/// described in `git log`; any defect in it should not reflect on
/// Dietmar Kühl's reputation.

template<typename CharType, typename traits = std::char_traits<CharType>>
class dev_null_stream_buffer
    :public std::streambuf
{
  public:
    dev_null_stream_buffer()
        {
        setp(buffer_, buffer_ + buffer_size_);
        }
    ~dev_null_stream_buffer() override = default;

  private:
    dev_null_stream_buffer(dev_null_stream_buffer const&) = delete;
    dev_null_stream_buffer& operator=(dev_null_stream_buffer const&) = delete;

    int_type overflow(int_type c) override
        {
        setp(buffer_, buffer_ + buffer_size_);
        return traits_type::not_eof(c);
        }

    static int const buffer_size_ = 1024;
    char buffer_[buffer_size_];
};

std::streambuf& null_streambuf()
{
    static dev_null_stream_buffer<char> z;
    return z;
}
