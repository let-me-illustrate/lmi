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

// $Id: icon_monger.cpp,v 1.2 2008-10-02 01:44:38 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "icon_monger.hpp"

#include "data_directory.hpp"

#include <wx/image.h>

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

wxSize desired_icon_size(wxArtClient const& client, wxSize const& size)
{
    if(wxDefaultSize == size)
        {
#ifdef LMI_MSW
        if(client == wxART_MENU)
            {
            return wxSize(16, 16);
            }
        if(client == wxART_TOOLBAR)
            {
            return wxSize(24, 24);
            }
#endif
        return wxArtProvider::GetSizeHint(client);
        }
    else
        {
        return size;
        }
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
/// Failure to find an icon file is not an error when a builtin
/// icon is available.

wxBitmap icon_monger::CreateBitmap
    (wxArtID const&     id
    ,wxArtClient const& client
    ,wxSize const&      size
    )
{
    wxString iconname;
    if(id.StartsWith("wxART_", &iconname))
        {
        iconname.MakeLower();
        iconname.Replace("_", "-");
        }
    else
        {
        iconname = id;
        }

    wxSize const desired_size = desired_icon_size(client, size);
    std::string const basename = AddDataDir(iconname.c_str());

    wxString file = wxString::Format("%s-%d.png", basename.c_str(), desired_size.x);
    if(!wxFileExists(file))
        {
        file = basename + ".png";
        }

    if(!wxFileExists(file))
        {
        return wxNullBitmap;
        }

    wxImage image(file, wxBITMAP_TYPE_PNG);
    if(!image.IsOk())
        {
        return wxNullBitmap;
        }

    if(image.GetWidth() != desired_size.x || image.GetHeight() != desired_size.y)
        {
        image.Rescale(desired_size.x, desired_size.y, wxIMAGE_QUALITY_HIGH);
        }

    return wxBitmap(image);
}

