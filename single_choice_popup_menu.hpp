// Single-choice popup menu: a wxGetSingleChoiceIndex alternative.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef single_choice_popup_menu_hpp
#define single_choice_popup_menu_hpp

#include "config.hpp"

#include "wx_utility.hpp"               // TopWindow()

#include <wx/arrstr.h>                  // wxArrayString
#include <wx/menu.h>
#include <wx/string.h>

/// Design notes for class SingleChoicePopupMenu.
///
/// A wxGetSingleChoiceIndex alternative. No distracting dialog frame.
/// No needless OK button--requires only a single click or keystroke.

class SingleChoicePopupMenu
{
  public:
    SingleChoicePopupMenu
        (wxArrayString const& choices
        ,wxString const&      title  = wxEmptyString
        ,wxTopLevelWindow&    parent = TopWindow()
        );

    int Choose();

  private:
    wxMenu menu_;
    wxTopLevelWindow& parent_;
};

#endif // single_choice_popup_menu_hpp
