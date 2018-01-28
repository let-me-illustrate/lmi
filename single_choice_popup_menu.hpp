// Single-choice popup menu: a wxGetSingleChoiceIndex alternative.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#ifndef single_choice_popup_menu_hpp
#define single_choice_popup_menu_hpp

#include "config.hpp"

#include "wx_utility.hpp"

#include <wx/arrstr.h>                  // wxArrayString
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/window.h>

/// Design notes for class SingleChoicePopupMenu.
///
/// A wxGetSingleChoiceIndex alternative. No distracting dialog frame.
/// No needless OK button--requires only a single click or keystroke.
///
/// WX !! Warning: the first character in 'title' (if specified) seems
/// to become an accelerator, interfering with any menuitem that uses
/// the same character as its own accelerator.

class SingleChoicePopupMenu
    :public wxWindow
{
  public:
    SingleChoicePopupMenu
        (wxArrayString const& choices
        ,wxString const&      title  = wxEmptyString
        ,wxWindow*            parent = &TopWindow()
        );

    ~SingleChoicePopupMenu() override = default;

    int Choose();

  private:
    void UponMenuChoice(wxCommandEvent&);

    wxMenu menu_;
    int selection_index_;
};

#endif // single_choice_popup_menu_hpp

