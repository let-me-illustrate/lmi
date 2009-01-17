// Icon provider for wx interface.
//
// Copyright (C) 2008, 2009 Gregory W. Chicares.
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

// $Id: icon_monger.cpp,v 1.14 2009-01-17 21:44:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "icon_monger.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "handle_exceptions.hpp"
#include "map_lookup.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <wx/image.h>

#include <sstream>

/// Map 'wxART_' id's to icon names.
///
/// Statements are ordered by icon name, not by 'wxART_' id.

icon_monger::icon_monger()
{
    icon_names_by_wx_id_[wxART_COPY        ] = "copy"   ;
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
/// First, if the icon sought is known to be used only by wx html help
/// and is not provided by lmi, then return a null bitmap, causing wx
/// to use its own builtin icon.
///
/// Otherwise, try to find an icon of the requested size. If none is
/// found, then try to find an icon of default size and scale it--but
/// complain even if that succeeds, because the result of scaling may
/// be quite unappealing.
///
/// Diagnosed failures are presented merely as warnings because they
/// do not make the system impossible to use.

wxBitmap icon_monger::CreateBitmap
    (wxArtID const&     id
    ,wxArtClient const& client
    ,wxSize const&      size
    )
{
    bool is_used_only_by_wx_html_help =
           wxART_ADD_BOOKMARK    == id
        || wxART_DEL_BOOKMARK    == id
        || wxART_GO_BACK         == id
        || wxART_GO_DOWN         == id
        || wxART_GO_FORWARD      == id
        || wxART_GO_TO_PARENT    == id
        || wxART_GO_UP           == id
        || wxART_HELP_BOOK       == id
        || wxART_HELP_FOLDER     == id
        || wxART_HELP_PAGE       == id
        || wxART_HELP_SETTINGS   == id
        || wxART_HELP_SIDE_PANEL == id
        ;
    if(is_used_only_by_wx_html_help)
        {
        return wxNullBitmap;
        }

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
            report_exception();
            }
        }

#if wxCHECK_VERSION(2,9,0)
    wxSize const desired_size = desired_icon_size(client, size);
#else  // !wxCHECK_VERSION(2,9,0)
    // See:
    //   http://lists.nongnu.org/archive/html/lmi/2008-10/msg00022.html
    wxSize const desired_size =
        wxART_HELP == id && wxART_HELP_BROWSER == client
        ? wxSize(16, 16)
        : desired_icon_size(client, size)
        ;
#endif // !wxCHECK_VERSION(2,9,0)

    std::ostringstream oss;
    oss << AddDataDir(icon_name) << '-' << desired_size.GetWidth();
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
        warning()
            << "Image '"
            << icon_path.string()
            << "' of size "
            << image.GetWidth()
            << " by "
            << image.GetHeight()
            << " has been scaled because no bitmap of requested size "
            << desired_size.GetWidth()
            << " by "
            << desired_size.GetHeight()
            << " was found."
            << LMI_FLUSH
            ;
        image.Rescale
            (desired_size.GetWidth()
            ,desired_size.GetHeight()
            ,wxIMAGE_QUALITY_HIGH
            );
        }

    return wxBitmap(image);
}

