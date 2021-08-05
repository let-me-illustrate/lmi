// Stream and stream buffer that discard output like the null device.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

/// Reference to a static null stream--see caveat.
///
/// Caveat: The static object is in effect a global variable.
/// Replacing its streambuf by calling rdbuf(another_streambuf)
/// therefore has a global effect that is probably unwanted.
/// Therefore, prefer to create a local object instead, e.g.:
///   std::ostream local_os(&null_streambuf());
///   local_os << "written to oblivion";
///   local_os.rdbuf(std::cout.rdbuf); // effect is only local
///   local_os << "written to stdout";
///
/// This is only intended to be used as a default ostream& argument:
///   foo(std::ostream& os = null_stream());
/// for functions that never change the streambuf.

std::ostream& null_stream()
{
    static std::ostream z(&null_streambuf());
    z.setstate(std::ios::badbit);
    return z;
}
