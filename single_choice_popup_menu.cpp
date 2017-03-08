// Single-choice popup menu: a wxGetSingleChoiceIndex alternative.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "single_choice_popup_menu.hpp"

#include <wx/gdicmn.h>                  // wxDefaultPosition, wxSize

SingleChoicePopupMenu::SingleChoicePopupMenu
    (wxArrayString const& choices
    ,wxString const&      title
    ,wxWindow*            parent
    )
    :wxWindow        (parent, wxID_ANY, wxDefaultPosition, wxSize(0, 0))
    ,selection_index_(-1)
{
    if(!title.IsEmpty())
        {
        menu_.SetTitle(title);
        }
    for(unsigned int j = 0; j < choices.GetCount(); ++j)
        {
        wxString s = choices[j];
        if(wxNOT_FOUND == s.Find('&'))
            {
            s.Prepend('&');
            }
        menu_.Append(j, s);
        }
    Connect
        (0
        ,choices.GetCount()
        ,wxEVT_COMMAND_MENU_SELECTED
#if wxCHECK_VERSION(2,6,0)
        ,wxCommandEventHandler(SingleChoicePopupMenu::UponMenuChoice)
#else  // !wxCHECK_VERSION(2,6,0)
        ,reinterpret_cast<wxObjectEventFunction>
            (&SingleChoicePopupMenu::UponMenuChoice
            )
#endif // !wxCHECK_VERSION(2,6,0)
        );
}

// WX !! Can't be const because PopupMenu() isn't.
int SingleChoicePopupMenu::Choose()
{
    PopupMenu(&menu_);
    return selection_index_;
}

void SingleChoicePopupMenu::UponMenuChoice(wxCommandEvent& e)
{
    selection_index_ = e.GetId();
}

