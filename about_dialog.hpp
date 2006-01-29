// State application's purpose and show GPL notices.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: about_dialog.hpp,v 1.2 2006-01-29 13:52:00 chicares Exp $

#ifndef about_dialog_hpp
#define about_dialog_hpp

#include "config.hpp"

#include <boost/utility.hpp>

#include <wx/dialog.h>

class AboutDialog
    :public wxDialog
    ,private boost::noncopyable
{
  public:
    AboutDialog(wxWindow* parent);
    virtual ~AboutDialog();

    // TODO ?? Consider Sutter's advice to make member functions
    // either public and nonvirtual, or nonpublic and virtual.
    //
    // wxDialog overrides.
    virtual int ShowModal();

  private:
    // WX !! Shouldn't OnOK() be virtual in wxDialog? If it were, then
    // wouldn't overriding it here mean that this class wouldn't need
    // an explicit event table?
    void OnOK(wxCommandEvent&);

    DECLARE_EVENT_TABLE()
};

#endif // about_dialog_hpp

