// Show progress on lengthy operations--wx interface.
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

#include "pchfile_wx.hpp"

#include "progress_meter.hpp"

#include "force_linking.hpp"
#include "wx_utility.hpp"               // TopWindow()

#include <wx/progdlg.h>
#include <wx/utils.h>                   // wxMilliSleep()

#include <ios>                          // fixed, ios_base::precision()
#include <sstream>

LMI_FORCE_LINKING_IN_SITU(progress_meter_wx)

namespace
{
// Implicitly-declared special member functions do the right thing.

// Virtuals are private because no one has any business accessing
// them--not even derived classes, because deriving from this concrete
// class is not contemplated.

class concrete_progress_meter
    :public progress_meter
{
    static constexpr int progress_dialog_style
        {   wxPD_APP_MODAL
        |   wxPD_AUTO_HIDE
        |   wxPD_CAN_ABORT
        |   wxPD_ELAPSED_TIME
        |   wxPD_ESTIMATED_TIME
        |   wxPD_REMAINING_TIME
        |   wxPD_SMOOTH
        };

  public:
    concrete_progress_meter
        (int                max_count
        ,std::string const& title
        ,enum_display_mode
        );

    ~concrete_progress_meter() override = default;

  private:
    // progress_meter overrides.
    void do_dawdle(int seconds) override;
    // progress_meter required implementation.
    std::string progress_message() const override;
    bool show_progress_message() override;
    void culminate_ui() override;

// Temporarily overridden while wx issues are being addressed:
//    wxProgressDialog progress_dialog_;
    wxGenericProgressDialog progress_dialog_;
};

// A wxProgressDialog created with maximum = 0 is displayed even
// though that's apparently just a nuisance. Calling Update(0) right
// after creation seems to be a reasonable workaround. This situation
// no longer occurs in practice as this is written in 2017-11, though
// formerly it did arise with an earlier version of the calculation
// summary, where some number (determined at runtime) of columns were
// updated in a context where that number might be zero. It doesn't
// make much sense to change wx to treat this as a special case: the
// wxProgressDialog implementation calls neither ShowModal() nor
// Show(), so it is reasonable always to show the dialog. It might
// make sense to treat this as a special case in this ctor, but it
// doesn't seem to be worth the bother.

concrete_progress_meter::concrete_progress_meter
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode display_mode
    )
    :progress_meter(max_count, title, display_mode)
    ,progress_dialog_
        (title
        ,progress_message()
        ,max_count
        ,&TopWindow()
        ,progress_dialog_style
        )
{
    if(0 == max_count)
        {
        progress_dialog_.Update(0);
        }
}

/// Sleep for the number of seconds given in the argument.
///
/// Sleep only for a tenth of a second at a time, to ensure that the
/// application remains responsive--see:
///   https://lists.nongnu.org/archive/html/lmi/2013-11/msg00006.html
/// Count down the remaining delay to reassure end users that activity
/// is taking place.

void concrete_progress_meter::do_dawdle(int seconds)
{
    std::ostringstream oss;
    oss.precision(1);
    oss << std::fixed;
    for(int i = 10 * seconds; 0 < i && !progress_dialog_.WasCancelled(); --i)
        {
        wxMilliSleep(100);
        oss.clear();
        oss.str("");
        oss << "Waiting " << 0.1 * i << " seconds";
        progress_dialog_.Update(count(), oss.str());
        }
}

std::string concrete_progress_meter::progress_message() const
{
    std::ostringstream oss;
    oss << "Completed " << count() << " of " << max_count();
    return oss.str();
}

bool concrete_progress_meter::show_progress_message()
{
    return progress_dialog_.Update(count(), progress_message());
}

void concrete_progress_meter::culminate_ui()
{
}

std::unique_ptr<progress_meter> concrete_progress_meter_creator
    (int                               max_count
    ,std::string const&                title
    ,progress_meter::enum_display_mode mode
    )
{
    return std::make_unique<concrete_progress_meter>(max_count, title, mode);
}

bool volatile ensure_setup = set_progress_meter_creator
    (concrete_progress_meter_creator
    );
} // Unnamed namespace.
