// Single-choice popup menu: a wxGetSingleChoiceIndex alternative.
//
// Copyright (C) 2006 Gregory W. Chicares.
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

// $Id: single_choice_popup_menu.cpp,v 1.1 2006-02-28 13:35:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "single_choice_popup_menu.hpp"

#include <wx/gdicmn.h> // wxDefaultPosition, wxSize
#include <wx/utils.h>  // wxGetMousePosition()

SingleChoicePopupMenu::SingleChoicePopupMenu
    (wxArrayString const& choices
    ,wxString const&      title
    ,wxWindow*            parent
    )
    :wxWindow        (parent, -1, wxDefaultPosition, wxSize(0, 0))
    ,selection_index_(-1)
{
    if(!title.IsEmpty())
        {
        menu_.SetTitle(title);
        }
    for(unsigned int j = 0; j < choices.GetCount(); ++j)
        {
        wxString s = choices[j];
        if(-1 == s.Find('&'))
            {
            s.Prepend('&');
            }
        menu_.Append(j, s);
        }
    Connect
        (0
        ,choices.GetCount()
        ,wxEVT_COMMAND_MENU_SELECTED
        ,reinterpret_cast<wxObjectEventFunction>
            (&SingleChoicePopupMenu::OnMenuChoice
            )
        );
}

SingleChoicePopupMenu::~SingleChoicePopupMenu()
{
}

// WX !! Can't be const because PopupMenu() isn't.
int SingleChoicePopupMenu::Choose()
{
    PopupMenu(&menu_, ScreenToClient(wxGetMousePosition()));
    return selection_index_;
}

void SingleChoicePopupMenu::OnMenuChoice(wxCommandEvent& e)
{
    selection_index_ = e.GetId();
}

