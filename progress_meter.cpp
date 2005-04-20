// Show progress on lengthy operations, optionally letting users cancel them.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: progress_meter.cpp,v 1.3 2005-04-20 14:19:34 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include <stdexcept>

progress_meter_creator_type progress_meter_creator = 0;

boost::shared_ptr<progress_meter> create_progress_meter
    (int max_count
    ,std::string const& title
    )
{
    if(0 == progress_meter_creator)
        {
        throw std::logic_error
            ("create_progress_meter: "
            "function pointer not yet uninitialized."
            );
        }

    return progress_meter_creator(max_count, title);
}

bool set_progress_meter_creator(progress_meter_creator_type f)
{
    if(0 != progress_meter_creator)
        {
        throw std::logic_error
            ("set_progress_meter_creator: "
            "function pointer already initialized--must not be reset."
            );
        }

    progress_meter_creator = f;
    return true;
}

progress_meter::progress_meter(int max_count, std::string const& title)
    :count_     (0)
    ,max_count_ (max_count)
    ,title_     (title)
{
}

progress_meter::~progress_meter()
{
}

bool progress_meter::reflect_progress()
{
    if(max_count_ <= count_)
        {
        throw std::logic_error("progress_meter: max_count_ exceeded.");
        }
    ++count_;
    return show_progress_message();
}

int progress_meter::count() const
{
    return count_;
}

int progress_meter::max_count() const
{
    return max_count_;
}

