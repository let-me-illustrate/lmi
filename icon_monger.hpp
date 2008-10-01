// Provider of icons for wx interface.
//
// Copyright (C) 2008 Gregory W. Chicares.
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

// $Id: icon_monger.hpp,v 1.1 2008-10-01 23:17:23 chicares Exp $

#ifndef icon_monger_hpp
#define icon_monger_hpp

#include "config.hpp"

#include <wx/artprov.h>

class LMIArtProvider : public wxArtProvider
{
  protected:
    virtual wxBitmap CreateBitmap
        (wxArtID const&     id
        ,wxArtClient const& client
        ,wxSize const&      size
        );
};

#endif // icon_monger_hpp

