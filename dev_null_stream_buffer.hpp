// Stream buffer that discards output like the null device.
//
// Copyright (C) 2007, 2008 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: dev_null_stream_buffer.hpp,v 1.2 2008-01-01 18:29:39 chicares Exp $

#ifndef dev_null_stream_buffer_hpp
#define dev_null_stream_buffer_hpp

#include "config.hpp"

#include <boost/utility.hpp>

#include <streambuf>
#include <string>

/// Class dev_null_stream_buffer is intended for redirecting output as
/// though to a null device. It is a derived work based on Dietmar
/// Kühl's article
///   http://groups.google.com/groups?selm=82mbke$l5c$1@nnrp1.deja.com
/// which bears no copyright notice, as is usual in usenet.
///
/// GWC modified this class in 2007, and in any later year, as
/// described in 'ChangeLog'; any defect in it should not reflect on
/// Dietmar Kuehl's reputation.

template<typename CharType, typename traits = std::char_traits<CharType> >
class dev_null_stream_buffer
    :public std::streambuf
    ,private boost::noncopyable
{
  public:
    dev_null_stream_buffer()
        {
        setp(buffer_, buffer_ + buffer_size_);
        }
    virtual ~dev_null_stream_buffer()
        {}

  private:
    virtual int_type overflow(int_type c)
        {
        setp(buffer_, buffer_ + buffer_size_);
        return traits_type::not_eof(c);
        }

    static int const buffer_size_ = 1024;
    char buffer_[buffer_size_];
};

#endif // dev_null_stream_buffer_hpp

