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

// $Id: icon_monger.cpp,v 1.1 2008-10-01 23:17:20 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "icon_monger.hpp"

#include "data_directory.hpp"

#include <wx/image.h>

namespace
{

/// Return desirable size of icons, given the client that requested the icon
/// and requested size (which should usually be unspecified)

wxSize GetRealSize(wxArtClient const& client, wxSize const& size)
{
    if(size == wxDefaultSize)
        {
        // on Windows, we don't want to use wxWidgets' 16x15 default size
        // for everything
#ifdef __WXMSW__
        if(client == wxART_MENU)
            return wxSize(16, 16);
        else if(client == wxART_TOOLBAR)
            return wxSize(24, 24);
#endif
        // fall back to defaults on other platforms and for things we don't
        // care about:
        return wxArtProvider::GetSizeHint(client);
        }
    else
        {
        return size;
        }
}

} // unnamed namespace

wxBitmap LMIArtProvider::CreateBitmap
    (wxArtID const& id
    ,wxArtClient const& client
    ,wxSize const& size
    )

{
    // convert wx's IDs in the form of "wxART_FOO_BAR" into nice filename
    // such as "foo-bar"
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

    const wxSize realsize = GetRealSize(client, size);
    const std::string basename = AddDataDir(iconname.c_str());

    // try to load icon of requested size; if it cannot be found, load default
    // size and scale it:
    wxString file = wxString::Format("%s-%d.png", basename.c_str(), realsize.x);
    if(!wxFileExists(file))
        file = basename + ".png";

    if(!wxFileExists(file))
        return wxNullBitmap; // icon not provided by us

    wxImage img(file, wxBITMAP_TYPE_PNG);
    if(!img.IsOk())
        return wxNullBitmap;

    if(img.GetWidth() != realsize.x || img.GetHeight() != realsize.y)
        img.Rescale(realsize.x, realsize.y, wxIMAGE_QUALITY_HIGH);

    return wxBitmap(img);
}

