// Show progress on lengthy operations--command-line interface.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: progress_meter_cli.cpp,v 1.8 2007-05-12 01:00:40 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include <boost/utility.hpp>

#include <iostream>
#include <ostream>
#include <streambuf>

namespace
{
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

// Implicitly-declared special member functions do the right thing.

// Virtuals are private because no one has any business accessing
// them--not even derived classes, because deriving from this concrete
// class is not contemplated.

class concrete_progress_meter
    :public progress_meter
{
  public:
    concrete_progress_meter(int max_count, std::string const& title);
    virtual ~concrete_progress_meter();

  private:
    mutable std::ostream os_;

    // progress_meter overrides.
    virtual std::string progress_message() const;
    virtual bool show_progress_message() const;
};

concrete_progress_meter::concrete_progress_meter
    (int max_count
    ,std::string const& title
    )
    :progress_meter(max_count, title)
    ,os_           (std::cout.rdbuf())
{
    os_ << title << std::flush;
}

concrete_progress_meter::~concrete_progress_meter()
{
    try
        {
        os_ << std::endl;
        }
    catch(...) {}
}

std::string concrete_progress_meter::progress_message() const
{
    return ".";
}

bool concrete_progress_meter::show_progress_message() const
{
    os_ << progress_message() << std::flush;
    return true;
}

boost::shared_ptr<progress_meter> concrete_progress_meter_creator
    (int max_count
    ,std::string const& title
    )
{
    return boost::shared_ptr<progress_meter>
        (new concrete_progress_meter(max_count, title)
        );
}

volatile bool ensure_setup = set_progress_meter_creator
    (concrete_progress_meter_creator
    );
} // Unnamed namespace.

