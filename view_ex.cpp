// Enhanced wxView class.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2004 to create an abstract
// enhanced view class, and in the later years given in the copyright
// notice above.

#include "pchfile_wx.hpp"

#include "view_ex.hpp"

// WX !! The application object's header must be included here because
// view creation is not performed by the framework--instead, wxWindows
// treats that as the user's responsibility. The framework defines
// wxView::OnCreate() to do nothing but {return true;}, so one would
// have expected it to be a pure virtual. Unfortunately, this seems to
// mean that a view class cannot be physically decoupled from the main
// application header, and indeed there are no wxView derivatives in
// the wx framework.

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "docmanager_ex.hpp"
#include "path.hpp"
#include "safely_dereference_as.hpp"
#include "skeleton.hpp"                 // Skeleton::CreateChildFrame()
#include "wx_new.hpp"

#include <wx/app.h>                     // GetInstance()
#include <wx/dc.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <cstdlib>                      // exit(), EXIT_FAILURE

IMPLEMENT_ABSTRACT_CLASS(ViewEx, wxView)

DocManagerEx& ViewEx::DocManager() const
{
    return safely_dereference_as<DocManagerEx>(GetDocumentManager());
}

wxFrame& ViewEx::FrameWindow() const
{
    return safely_dereference_as<wxFrame>(GetFrame());
}

wxIcon ViewEx::Icon() const
{
    return IconFromXmlResource(icon_xrc_resource());
}

/// Elsewhere, the result of wxXmlResource::Get()->LoadX is checked
/// before doing anything with it. However, LoadIcon() returns a
/// reference, not a pointer, because it's a graphics object:
///   https://lists.nongnu.org/archive/html/lmi/2009-03/msg00077.html
/// and therefore it returns a default-constructed wxIcon on failure.

wxIcon ViewEx::IconFromXmlResource(char const* z) const
{
    wxIcon icon = wxXmlResource::Get()->LoadIcon(z);
    if(!icon.Ok())
        {
        warning() << "Invalid icon; using default." << LMI_FLUSH;
        }
    return icon;
}

wxMenuBar* ViewEx::MenuBar() const
{
    return MenuBarFromXmlResource(menubar_xrc_resource());
}

/// If 'new(wx) wxMenuBar' fails, then the program would crash except
/// for the explicit test below. Use std::exit() instead of wxExit()
/// because wxExit() itself can lead to a crash.

wxMenuBar* ViewEx::MenuBarFromXmlResource(char const* z) const
{
    wxMenuBar* menubar = wxXmlResource::Get()->LoadMenuBar(z);
    if(!menubar)
        {
        warning() << "Invalid menubar; using default." << LMI_FLUSH;
        menubar = new(wx) wxMenuBar;
        if(!menubar)
            {
            warning()
                << "Terminating abnormally:"
                << " failed to construct default menubar."
                << LMI_FLUSH
                ;
            std::exit(EXIT_FAILURE);
            }
        }
    return menubar;
}

bool ViewEx::OnClose(bool delete_window)
{
    if(!GetDocument() || !GetDocument()->Close())
        {
        return false;
        }

    DocManager().DissociateFileHistoryFromFileMenu(FrameWindow().GetMenuBar());

    Activate(false);

    if(delete_window)
        {
        delete GetFrame();
        }

    return true;
}

/// Initialize a document view in an MDI window.
///
/// Completely replaces wxView::OnCreate(), whose implementation is
/// merely {return true;} as of wx-3.1 .
///
/// This default implementation simply forwards to DoOnCreate().
///
/// WX !! The documentation for wxDocMDIChildFrame::OnActivate() says
///   "You may need to override (but still call) this function in
///   order to set the keyboard focus for your subwindow."
/// At least for msw, creation does not trigger 'activation': more
/// precisely, WM_MDICREATE does not send WM_MDIACTIVATE, and
/// OnActivate() doesn't get called when the window is created, so the
/// documented method doesn't work--whereas setting the focus upon
/// creation here does work.

bool ViewEx::OnCreate(wxDocument* doc, long int flags)
{
    return DoOnCreate(doc, flags);
}

/// Default implementation for OnCreate().
///
/// Some derived classes call this function in an OnCreate() override,
/// e.g. to pop up an input dialog and create a view only if the
/// dialog is not cancelled.

bool ViewEx::DoOnCreate(wxDocument* doc, long int)
{
    Skeleton& app = safely_dereference_as<Skeleton>(wxApp::GetInstance());
    app.CreateChildFrame(doc, this);

    DocManager().AssociateFileHistoryWithFileMenu(FrameWindow().GetMenuBar());
    GetFrame()->SetLabel("Loading document...");

    wxWindow* child = CreateChildWindow();
    LMI_ASSERT(child);
    LMI_ASSERT(child->GetParent() == GetFrame());
    child->SetFocus();

    // WX !! This should be done inside the library.
#if defined __X__
    // The X Window Toolkit seems to require a forced resize.
    int x, y;
    GetFrame()->GetSize(&x, &y);
    GetFrame()->SetSize(wxDefaultCoord, wxDefaultCoord, x, y);
#endif // defined __X__

    GetFrame()->Show(true);
    Activate(true);

    return true;
}

void ViewEx::OnDraw(wxDC*)
{
}

/// A pathless name, useful for naming output files.
///
/// Using wxDocument::GetUserReadableName() means the name can be
/// altered by calling wxDocument::SetTitle(). By default, the title
/// is the filename with no path. The call to filename() guarantees that
/// the result is pathless, even if e.g. the title has been set to the
/// document's full filepath. If filename() is empty, then a name that
/// recognizably should never be uttered is returned.

std::string ViewEx::base_filename() const
{
    std::string t(GetDocument()->GetUserReadableName().ToStdString(wxConvUTF8));
    fs::path path(t);
    return path.has_filename() ? path.filename().string() : std::string("Hastur");
}
