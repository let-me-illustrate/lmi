// Show progress on lengthy operations--wx interface.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: progress_meter_wx.cpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "progress_meter.hpp"

#include <wx/app.h> // wxTheApp
#include <wx/progdlg.h>

#include <sstream>

namespace
{
// Implicitly-declared special member functions do the right thing.

// Virtuals are private because no one has any business accessing
// them--not even derived classes, because deriving from this concrete
// class is not contemplated.

class concrete_progress_meter
    :public progress_meter
{
    enum
        {progress_dialog_style =
                wxPD_APP_MODAL
            |   wxPD_AUTO_HIDE
            |   wxPD_CAN_ABORT
            |   wxPD_ELAPSED_TIME
            |   wxPD_ESTIMATED_TIME
            |   wxPD_REMAINING_TIME
        };

  public:
    concrete_progress_meter(int max_count, std::string const& title);
    virtual ~concrete_progress_meter();

  private:
    // progress_meter overrides.
    virtual std::string progress_message() const;
    virtual bool show_progress_message() const;

    // Modified by show_progress_message() const.
    mutable wxProgressDialog progress_dialog;
};

concrete_progress_meter::concrete_progress_meter
    (int max_count
    ,std::string const& title
    )
    :progress_meter(max_count, title)
    ,progress_dialog
        (title
        ,progress_message().c_str()
        ,max_count
        ,wxTheApp->GetTopWindow()
        ,progress_dialog_style
        )
{
}

concrete_progress_meter::~concrete_progress_meter()
{
}

std::string concrete_progress_meter::progress_message() const
{
    std::ostringstream oss;
    oss << "Completed " << count() << " of " << max_count();
    return oss.str();
}

bool concrete_progress_meter::show_progress_message() const
{
    return progress_dialog.Update(count(), progress_message().c_str());
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

