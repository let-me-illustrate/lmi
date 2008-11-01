// Icon provider for wx interface.
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

// $Id: icon_monger.cpp,v 1.8 2008-11-01 15:48:18 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "icon_monger.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "map_lookup.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <wx/image.h>

#include <sstream>

icon_monger::icon_monger()
{
    icon_names_by_wx_id_[wxART_QUIT        ] = "exit"   ;
    icon_names_by_wx_id_[wxART_HELP        ] = "help"   ;
    icon_names_by_wx_id_[wxART_NEW         ] = "new"    ;
    icon_names_by_wx_id_[wxART_FILE_OPEN   ] = "open"   ;
    icon_names_by_wx_id_[wxART_PRINT       ] = "print"  ;
    icon_names_by_wx_id_[wxART_FILE_SAVE   ] = "save"   ;
    icon_names_by_wx_id_[wxART_FILE_SAVE_AS] = "save-as";
}

icon_monger::~icon_monger()
{
}

namespace
{
/// Return desired icon size.
///
/// Most often, the 'size' argument is wxDefaultSize, and the returned
/// size therefore depends only on the client that requested the icon.
///
/// For platforms with standard interface guidelines, GetSizeHint()
/// does the right thing; but for msw it just returns 16 by 15 because
/// there's no standard practice, so hardcoded sizes are given here.

wxSize desired_icon_size
    (wxArtClient const& client
    ,wxSize const&      size
    )
{
    wxSize z(wxDefaultSize != size ? size : wxArtProvider::GetSizeHint(client));
#if !defined LMI_MSW
    return z;
#else  // defined LMI_MSW
    // For the nonce, inhibit rescaling: it's not attractive enough.
    return wxSize(24, 24);
    if     (wxART_MENU    == client) {return wxSize(16, 16);}
    else if(wxART_TOOLBAR == client) {return wxSize(24, 24);}
    else                             {return z;}
#endif // defined LMI_MSW
}
} // Unnamed namespace.

/// Provide the most suitable icon in the given context.
///
/// Builtin wxArtID values are converted to fitting filenames, e.g.:
///   wxART_FOO_BAR --> foo-bar.png    [default size]
///   wxART_FOO_BAR --> foo-bar-16.png [16 pixels square]
///
/// First, try to find an icon of the requested size. If none is
/// found, then try to find an icon of default size and scale it.
/// Inability to find an icon file is not an error when a builtin
/// icon is available.
///
/// Diagnosed failures are presented merely as warnings because they
/// do not make the system impossible to use.

wxBitmap icon_monger::CreateBitmap
    (wxArtID const&     id
    ,wxArtClient const& client
    ,wxSize const&      size
    )
{
    std::string icon_name(id.c_str());
    bool is_builtin = 0 == icon_name.find("wxART_");
    if(is_builtin)
        {
        try
            {
            icon_name = map_lookup(icon_names_by_wx_id_, icon_name);
            }
        catch(...)
            {
            ; // Do nothing.
            }
        }

    wxSize const desired_size = desired_icon_size(client, size);

    std::ostringstream oss;
    oss << AddDataDir(icon_name) << '-' << desired_size.x;
    fs::path icon_path(oss.str() + ".png");
    if(!fs::exists(icon_path))
        {
        icon_path = AddDataDir(icon_name) + ".png";
        }
    if(!fs::exists(icon_path))
        {
        if(!is_builtin)
            {
            warning()
                << "Unable to find '"
                << icon_path.string()
                << "'. Try reinstalling."
                << LMI_FLUSH
                ;
            }
        return wxNullBitmap;
        }

    wxImage image(icon_path.string().c_str(), wxBITMAP_TYPE_PNG);
    if(!image.IsOk())
        {
        warning()
            << "Unable to load image '"
            << icon_path.string()
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        return wxNullBitmap;
        }

    if(desired_size != wxSize(image.GetWidth(), image.GetHeight()))
        {
        image.Rescale(desired_size.x, desired_size.y, wxIMAGE_QUALITY_HIGH);
        }

    return wxBitmap(image);
}

