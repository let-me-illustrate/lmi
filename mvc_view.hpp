// MVC View base class.
//
// Copyright (C) 2006, 2007 Gregory W. Chicares.
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

// $Id: mvc_view.hpp,v 1.3 2007-01-27 00:00:51 wboutin Exp $

#ifndef mvc_view_hpp
#define mvc_view_hpp

#include "config.hpp"

/// Design notes for class MvcView.
///
/// This abstract class presents the data a Controller needs to manage
/// a View. There is little such data because the View is expressed in
/// xml resources.
///
/// BookControlName(): Name of book control contained in the main
/// dialog.
///
/// MainDialogName(): Name of main dialog. At present, the Controller
/// expects a wxDialog that contains a wxBookCtrlBase.
///
/// ResourceFileName(): Name of file containing wxxrc resources.
///
/// Implicitly-declared special member functions do the right thing.

class MvcView
{
  public:
    MvcView();
    virtual ~MvcView();

    char const* BookControlName () const;
    char const* MainDialogName  () const;
    char const* ResourceFileName() const;

  private:
    virtual char const* DoBookControlName () const = 0;
    virtual char const* DoMainDialogName  () const = 0;
    virtual char const* DoResourceFileName() const = 0;
};

#endif // mvc_view_hpp

