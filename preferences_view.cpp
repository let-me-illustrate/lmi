// MVC View for user preferences.
//
// Copyright (C) 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: preferences_view.cpp,v 1.4 2008-12-27 02:56:52 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "preferences_view.hpp"

#include "configurable_settings.hpp"

PreferencesView::PreferencesView()
{
}

PreferencesView::~PreferencesView()
{
}

char const* PreferencesView::DoBookControlName() const
{
    return "preferences_notebook";
}

char const* PreferencesView::DoMainDialogName() const
{
    return "dialog_containing_preferences_notebook";
}

char const* PreferencesView::DoResourceFileName() const
{
    return "preferences_view.xrc";
}

