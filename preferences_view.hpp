// MVC View for user preferences.
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

#ifndef preferences_view_hpp
#define preferences_view_hpp

#include "config.hpp"

#include "mvc_view.hpp"

/// Design notes for class PreferencesView.
///
/// This concrete class implements an MVC View for user-configurable
/// settings. For now, only calculation-summary column selections are
/// presented.
///
/// Implicitly-declared special member functions do the right thing.

class PreferencesView
    :public MvcView
{
  public:
    PreferencesView() = default;
    ~PreferencesView() override = default;

  private:
    // MvcView required implementation.
    char const* DoBookControlName () const override;
    char const* DoMainDialogName  () const override;
    char const* DoResourceFileName() const override;
};

#endif // preferences_view_hpp
