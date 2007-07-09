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

// $Id: progress_meter_cli.cpp,v 1.15 2007-07-09 14:05:14 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include "alert.hpp"
#include "dev_null_stream_buffer.hpp"

#include <iostream>
#include <ostream>
#include <sstream>

namespace
{
std::streambuf* select_streambuf(progress_meter::enum_display_mode display_mode)
{
    std::streambuf* z = 0;
    switch(display_mode)
        {
        case progress_meter::e_normal_display:
            {
            z = std::cout.rdbuf();
            }
            break;
        case progress_meter::e_quiet_display:
            {
            static dev_null_stream_buffer<char> no_output;
            z = &no_output;
            }
            break;
        case progress_meter::e_unit_test_mode:
            {
            z = progress_meter_unit_test_stream().rdbuf();
            }
            break;
        default:
            {
            fatal_error()
                << "Case "
                << display_mode
                << " not found."
                << LMI_FLUSH
                ;
            }
        }
    return z;
}

// Implicitly-declared special member functions do the right thing.

// Virtuals are private because no one has any business accessing
// them--not even derived classes, because deriving from this concrete
// class is not contemplated.

class concrete_progress_meter
    :public progress_meter
{
  public:
    concrete_progress_meter
        (int                max_count
        ,std::string const& title
        ,enum_display_mode
        );

    virtual ~concrete_progress_meter();

  private:
    // progress_meter required implementation.
    virtual std::string progress_message() const;
    virtual bool show_progress_message();
    virtual void culminate_ui();

    std::ostream os_;
};

concrete_progress_meter::concrete_progress_meter
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode display_mode
    )
    :progress_meter(max_count, title, display_mode)
    ,os_           (select_streambuf(display_mode))
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

bool concrete_progress_meter::show_progress_message()
{
    os_ << progress_message() << std::flush;
    return true;
}

void concrete_progress_meter::culminate_ui()
{
// Move contents of dtor's try-block here.
}

boost::shared_ptr<progress_meter> concrete_progress_meter_creator
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode display_mode
    )
{
    return boost::shared_ptr<progress_meter>
        (new concrete_progress_meter(max_count, title, display_mode)
        );
}

volatile bool ensure_setup = set_progress_meter_creator
    (concrete_progress_meter_creator
    );
} // Unnamed namespace.

