// Icon provider for wx interface.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef icon_monger_hpp
#define icon_monger_hpp

#include "config.hpp"

#include <wx/artprov.h>
#include <wx/image.h>

#include <map>
#include <set>
#include <string>

/// Icon provider for wx interface.

class icon_monger
    :public wxArtProvider
{
  public:
    icon_monger();
    ~icon_monger() override = default;

  private:
    icon_monger(icon_monger const&) = delete;
    icon_monger& operator=(icon_monger const&) = delete;

    // Implement wxArtProvider function.
    wxBitmapBundle CreateBitmapBundle
        (wxArtID const&
        ,wxArtClient const&
        ,wxSize const&
        ) override;

    std::map<wxArtID,std::string> icon_names_by_wx_id_;
    std::set<std::string>         lmi_specific_icon_names_;
};

wxImage load_image(char const* file);

#endif // icon_monger_hpp
