// Stream and stream buffer that discard output like the null device.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#include "null_stream.hpp"
#include "uncopyable_lmi.hpp"

#include <ostream>
#include <streambuf>
#include <string>

/// Redirect output as though to a null device.
///
/// This class is a derived work based on Dietmar K�hl's article
///   http://groups.google.com/groups?selm=82mbke$l5c$1@nnrp1.deja.com
/// which bears no copyright notice, as is usual in usenet.
///
/// GWC modified this class in 2007, and in any later year, as
/// described in 'ChangeLog'; any defect in it should not reflect on
/// Dietmar Kuehl's reputation.

template<typename CharType, typename traits = std::char_traits<CharType> >
class dev_null_stream_buffer
    :public  std::streambuf
    ,private lmi::uncopyable<dev_null_stream_buffer<CharType,traits> >
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

std::streambuf& null_streambuf()
{
    static dev_null_stream_buffer<char> z;
    return z;
}

std::ostream& null_stream()
{
    static std::ostream z(&null_streambuf());
    return z;
}

