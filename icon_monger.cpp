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

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "icon_monger.hpp"

#include "alert.hpp"
#include "data_directory.hpp"
#include "map_lookup.hpp"
#include "path_utility.hpp" // fs::path inserter

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <wx/image.h>

#include <sstream>

namespace
{
// SOMEDAY !! Write a "contains.hpp" header that implements this
// function for every standard container as well as std::string.
// Rationale: this usage represents half of our find() calls, and
// the idiom is verbose.
template<typename Key, typename Compare, typename Allocator>
bool contains(std::set<Key,Compare,Allocator> const& c, Key const& k)
{
    return c.end() != c.find(k);
}
} // Unnamed namespace.

/// Enumerate lmi icon names; map 'wxART_' id's to icon names.
///
/// The wxART mapping permits the use of simpler gnome-standard names:
/// e.g., "copy" rather than "wxART_COPY".
///
/// Typically, only one instance of this class would be created, in a
/// wxApp::OnInit() override. Creating more than one instance is safe,
/// though, and needn't be guarded against. Speed being insignificant,
/// associative-container members are populated directly here; there's
/// no compelling reason to make them static.

icon_monger::icon_monger()
{
    // Statements are ordered by icon name, not by 'wxART_' id.
    icon_names_by_wx_id_[wxART_COPY        ] = "copy"   ;
    icon_names_by_wx_id_[wxART_QUIT        ] = "exit"   ;
    icon_names_by_wx_id_[wxART_HELP        ] = "help"   ;
    icon_names_by_wx_id_[wxART_NEW         ] = "new"    ;
    icon_names_by_wx_id_[wxART_FILE_OPEN   ] = "open"   ;
    icon_names_by_wx_id_[wxART_PRINT       ] = "print"  ;
    icon_names_by_wx_id_[wxART_FILE_SAVE   ] = "save"   ;
    icon_names_by_wx_id_[wxART_FILE_SAVE_AS] = "save-as";

#if defined LMI_MSW
    // Stock icons explicitly provided by lmi, for msw only; for gtk,
    // themes would control.
    lmi_specific_icon_names_.insert("about"        );
    lmi_specific_icon_names_.insert("close"        );
    lmi_specific_icon_names_.insert("copy"         );
    lmi_specific_icon_names_.insert("delete-row"   );
    lmi_specific_icon_names_.insert("exit"         );
    lmi_specific_icon_names_.insert("help"         );
    lmi_specific_icon_names_.insert("insert-rows"  );
    lmi_specific_icon_names_.insert("new"          );
    lmi_specific_icon_names_.insert("open"         );
    lmi_specific_icon_names_.insert("print"        );
    lmi_specific_icon_names_.insert("print-options");
    lmi_specific_icon_names_.insert("print-preview");
    lmi_specific_icon_names_.insert("save"         );
    lmi_specific_icon_names_.insert("save-as"      );
    lmi_specific_icon_names_.insert("save-pdf"     );
#endif // defined LMI_MSW

    // Non-stock icons.
    lmi_specific_icon_names_.insert("copy-cell"               );
    lmi_specific_icon_names_.insert("default-cell"            );
    lmi_specific_icon_names_.insert("edit-case"               );
    lmi_specific_icon_names_.insert("edit-cell"               );
    lmi_specific_icon_names_.insert("edit-class"              );
    lmi_specific_icon_names_.insert("fixed-width"             );
    lmi_specific_icon_names_.insert("paste-case"              );
    lmi_specific_icon_names_.insert("preferences"             );
    lmi_specific_icon_names_.insert("print-case"              );
    lmi_specific_icon_names_.insert("run-case"                );
    lmi_specific_icon_names_.insert("run-cell"                );
    lmi_specific_icon_names_.insert("varying-width"           );
    lmi_specific_icon_names_.insert("window-cascade"          );
    lmi_specific_icon_names_.insert("window-next"             );
    lmi_specific_icon_names_.insert("window-previous"         );
    lmi_specific_icon_names_.insert("window-tile-horizontally");
    lmi_specific_icon_names_.insert("window-tile-vertically"  );
    lmi_specific_icon_names_.insert("write-spreadsheet"       );
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
/// Convert builtin wxArtID values to fitting filenames, e.g.:
///   wxART_FOO_BAR --> foo-bar.png    [default size]
///   wxART_FOO_BAR --> foo-bar-16.png [16 pixels square]
///
/// First, try to find an icon of the requested size. If none is
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
            ; // Do nothing. Not all wxART id's have lmi overrides.
            }
        }

    wxSize const desired_size = desired_icon_size(client, size);

    std::ostringstream oss;
    oss << AddDataDir(icon_name) << '-' << desired_size.GetWidth();
    fs::path icon_path(oss.str() + ".png");
    if(!fs::exists(icon_path))
        {
        icon_path = AddDataDir(icon_name) + ".png";
        }
    if(!fs::exists(icon_path))
        {
        if(!contains(lmi_specific_icon_names_, icon_name))
            {
            return wxNullBitmap;
            }
        else if(is_builtin)
            {
            warning()
                << "Unable to find icon '"
                << icon_path
                << "'. Try reinstalling."
                << "\nA builtin alternative will be used instead,"
                << " but it may be visually jarring."
                << LMI_FLUSH
                ;
            return wxNullBitmap;
            }
        else
            {
            warning()
                << "Unable to find icon '"
                << icon_path
                << "'. Try reinstalling."
                << "\nA blank icon will be used instead."
                << LMI_FLUSH
                ;
            return wxNullBitmap;
            }
        }

    wxImage image(icon_path.string().c_str(), wxBITMAP_TYPE_PNG);
    if(!image.IsOk())
        {
        warning()
            << "Unable to load image '"
            << icon_path
            << "'. Try reinstalling."
            << LMI_FLUSH
            ;
        return wxNullBitmap;
        }

    if(desired_size != wxSize(image.GetWidth(), image.GetHeight()))
        {
        warning()
            << "Image '"
            << icon_path
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

