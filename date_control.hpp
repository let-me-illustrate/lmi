// Calendar-date control derived from wxTextCtrl.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: date_control.hpp,v 1.1 2005-03-11 03:19:31 chicares Exp $

// TODO ?? Replace this with wxDatePickerCtrl.

#ifndef date_control_hpp
#define date_control_hpp

#include "config.hpp"

#include <boost/utility.hpp>

#include <wx/textctrl.h>

class DateControl
    :public wxTextCtrl
    ,private boost::noncopyable
{
  public:
    DateControl();
    DateControl
        (wxWindow*          parent
        ,wxWindowID         id
        ,wxString    const& value     = ""
        ,wxPoint     const& pos       = wxDefaultPosition
        ,wxSize      const& size      = wxDefaultSize
        ,long               style     = 0
        ,wxValidator const& validator = wxDefaultValidator
        ,wxString    const& name      = wxTextCtrlNameStr
        );
    virtual ~DateControl();

    // wxTextCtrl overrides.
    // WX !! The documentation says that GetValue() isn't virtual, but
    // the implementation says it is: the documentation seems wrong.
    virtual wxString GetValue() const;
    virtual void SetValue(const wxString& value);

    DECLARE_DYNAMIC_CLASS(DateControl)
};

#endif // date_control_hpp

