// Icon provider for wx interface.
//
// Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "icon_monger.hpp"

#include "alert.hpp"
#include "contains.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "path.hpp"
#include "path_utility.hpp"             // fs::path inserter

#include <wx/image.h>

#include <sstream>

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
    lmi_specific_icon_names_.insert("copy-census"             );
    lmi_specific_icon_names_.insert("default-cell"            );
    lmi_specific_icon_names_.insert("edit-case"               );
    lmi_specific_icon_names_.insert("edit-cell"               );
    lmi_specific_icon_names_.insert("edit-class"              );
    lmi_specific_icon_names_.insert("fixed-width"             );
    lmi_specific_icon_names_.insert("paste-census"            );
    lmi_specific_icon_names_.insert("preferences"             );
    lmi_specific_icon_names_.insert("print-case"              );
    lmi_specific_icon_names_.insert("print-case-pdf"          );
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
        auto const& i = icon_names_by_wx_id_.find(icon_name);
        if(i == icon_names_by_wx_id_.end())
            {
            // This is not an error as not all wxART id's have lmi overrides,
            // so just let the next art provider deal with it.
            return wxNullBitmap;
            }
        icon_name = i->second;
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
            warning()
                << "Unable to find icon '"
                << icon_name
                << "' for current theme. Please report this problem."
                << "\nA blank icon will be used instead."
                << LMI_FLUSH
                ;
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

    // WX !! Remove this little block when wx does this automatically.
    if(image.HasMask())
        {
        image.InitAlpha();
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

/// Load the image from the given file.
///
/// Look for the file in the current working directory, or, if that
/// fails, in lmi's data directory. Warn if it's not found in either
/// of those locations, or if it's found but cannot be loaded.
///
/// Diagnosed failures are presented merely as warnings so that quotes
/// can be produced even with a generic system built from the free
/// public source code only, with no (proprietary) images.

wxImage load_image(char const* file)
{
    fs::path image_path(file);
    if(!fs::exists(image_path))
        {
        image_path = AddDataDir(file);
        }
    if(!fs::exists(image_path))
        {
        warning()
            << "Unable to find image '"
            << image_path
            << "'. Try reinstalling."
            << "\nA blank image will be used instead."
            << LMI_FLUSH
            ;
        return wxImage();
        }

    wxImage image(image_path.string().c_str(), wxBITMAP_TYPE_PNG);
    if(!image.IsOk())
        {
        warning()
            << "Unable to load image '"
            << image_path
            << "'. Try reinstalling."
            << "\nA blank image will be used instead."
            << LMI_FLUSH
            ;
        return wxImage();
        }

    return image;
}
