// Show progress on lengthy operations--common gateway interface.
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

// $Id: progress_meter_cgi.cpp,v 1.1 2005-04-20 21:04:40 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

namespace
{
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
    // progress_meter overrides.
    virtual std::string progress_message() const;
    virtual bool show_progress_message() const;
};

concrete_progress_meter::concrete_progress_meter
    (int max_count
    ,std::string const& title
    )
    :progress_meter(max_count, title)
{
}

concrete_progress_meter::~concrete_progress_meter()
{
}

std::string concrete_progress_meter::progress_message() const
{
    return "";
}

bool concrete_progress_meter::show_progress_message() const
{
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

