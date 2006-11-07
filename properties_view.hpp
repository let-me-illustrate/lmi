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

#ifndef properties_view_hpp
#define properties_view_hpp

#include "config.hpp"

#include "mvc_view.hpp"

/// Design notes for class PropertiesView.
///
/// This concrete class implements a default MvcView designed for
/// certain configurable settings values. Currently it is only used to
/// alter calculation summary columns list.
///
/// Implicitly-declared special member functions do the right thing.

class PropertiesView
    :public MvcView
{
  public:
    PropertiesView();
    virtual ~PropertiesView();

  private:
    // base class pure virtuals to override
    virtual char const* DoBookControlName () const;
    virtual char const* DoMainDialogName  () const;
    virtual char const* DoResourceFileName() const;
};

#endif // properties_view_hpp

