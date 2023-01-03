// Single-choice popup menu: a wxGetSingleChoiceIndex alternative.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "single_choice_popup_menu.hpp"

#include "ssize_lmi.hpp"

#include <wx/evtloop.h>
#include <wx/toplevel.h>

SingleChoicePopupMenu::SingleChoicePopupMenu
    (wxArrayString const& choices
    ,wxString const&      title
    ,wxTopLevelWindow&    parent
    )
    :parent_ {parent}
{
    if(!title.IsEmpty())
        {
        menu_.SetTitle(title);
        }
    for(int j = 0; j < lmi::ssize(choices); ++j)
        {
        wxString s = choices[j];
        if(wxNOT_FOUND == s.Find('&'))
            {
            s.Prepend('&');
            }
        menu_.Append(j, s);
        }
}

// WX !! Can't be const because PopupMenu() isn't.
int SingleChoicePopupMenu::Choose()
{
    int const selection_index = parent_.GetPopupMenuSelectionFromUser(menu_);

    if(wxEventLoopBase* const loop = wxEventLoopBase::GetActive())
        {
        // This function can often be used to get users choice before starting
        // some time-consuming operation. Ensure that the area previously
        // covered by the menu shown by GetPopupMenuSelectionFromUser() is
        // repainted to avoid leaving it invalidated for a possibly long time.
        loop->YieldFor(wxEVT_CATEGORY_UI);
        }

    return selection_index != wxID_NONE ? selection_index : -1;
}
