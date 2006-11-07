// Properties dialog view for configurable settings.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "properties_view.hpp"

#include "configurable_settings.hpp"

PropertiesView::PropertiesView()
{
}

PropertiesView::~PropertiesView()
{
}

char const* PropertiesView::DoBookControlName() const
{
    return "properties_notebook";
}

char const* PropertiesView::DoMainDialogName() const
{
    return "dialog_containing_properties_notebook";
}

char const* PropertiesView::DoResourceFileName() const
{
    return configurable_settings::instance().skin_filename().c_str();
}

