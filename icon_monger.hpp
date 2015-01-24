// Icon provider for wx interface.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

// $Id$

#ifndef icon_monger_hpp
#define icon_monger_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <wx/artprov.h>

#include <map>
#include <set>
#include <string>

/// Icon provider for wx interface.

class icon_monger
    :public  wxArtProvider
    ,private lmi::uncopyable<icon_monger>
{
  public:
    icon_monger();
    virtual ~icon_monger();

  private:
    // wxArtProvider required implementation.
    virtual wxBitmap CreateBitmap
        (wxArtID const&
        ,wxArtClient const&
        ,wxSize const&
        );

    std::map<wxArtID,std::string> icon_names_by_wx_id_;
    std::set<std::string>         lmi_specific_icon_names_;
};

#endif // icon_monger_hpp

