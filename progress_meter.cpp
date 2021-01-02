// Show progress on lengthy operations, optionally letting users cancel them.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "progress_meter.hpp"

#include "alert.hpp"
#include "timer.hpp"                    // lmi_sleep()

#include <exception>                    // uncaught_exceptions()
#include <sstream>

std::ostringstream& progress_meter_unit_test_stream()
{
    static std::ostringstream oss;
    return oss;
}

progress_meter_creator_type progress_meter_creator = nullptr;

std::unique_ptr<progress_meter> create_progress_meter
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode display_mode
    )
{
    if(nullptr == progress_meter_creator)
        {
        alarum() << "Function pointer not yet initialized." << LMI_FLUSH;
        }

    return progress_meter_creator(max_count, title, display_mode);
}

bool set_progress_meter_creator(progress_meter_creator_type f)
{
    if(nullptr != progress_meter_creator)
        {
        // TODO ?? Use 'callback.hpp' instead, and consider whether
        // this message can ever actually be displayed--either in its
        // present form, or in the 'callback.hpp' equivalent.
        alarum() << "Function pointer already initialized." << LMI_FLUSH;
        }

    progress_meter_creator = f;
    return true;
}

progress_meter::progress_meter
    (int                max_count
    ,std::string const& title
    ,enum_display_mode  display_mode
    )
    :count_         {0}
    ,max_count_     {max_count}
    ,title_         {title}
    ,display_mode_  {display_mode}
    ,was_cancelled_ {false}
{
}

progress_meter::~progress_meter()
{
    if(!std::uncaught_exceptions() && !are_postconditions_met())
        {
        safely_show_message("Please report this: culminate() not called.");
        }
}

void progress_meter::dawdle(int seconds)
{
    do_dawdle(seconds);
}

bool progress_meter::reflect_progress()
{
    if(max_count_ <= count_)
        {
        alarum() << "Progress meter maximum count exceeded." << LMI_FLUSH;
        }
    if(was_cancelled_)
        {
        alarum() << "Progress meter previously cancelled." << LMI_FLUSH;
        }
    ++count_;
    was_cancelled_ = !show_progress_message();
    return !was_cancelled_;
}

void progress_meter::culminate()
{
    culminate_ui();
    if(!are_postconditions_met())
        {
        alarum()
            << max_count_
            << " iterations expected, but only "
            << count_
            << " completed."
            << LMI_FLUSH
            ;
        }
}

int progress_meter::count() const
{
    return count_;
}

int progress_meter::max_count() const
{
    return max_count_;
}

/// Pause for the number of seconds given in the argument.
///
/// This default implementation calls lmi_sleep(). The overriding
/// implementation for a GUI library would naturally use a more
/// sophisticated technique.

void progress_meter::do_dawdle(int seconds)
{
    lmi_sleep(seconds);
}

bool progress_meter::are_postconditions_met() const
{
    return was_cancelled_ || max_count_ == count_;
}
