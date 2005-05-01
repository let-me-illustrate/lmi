// Enhanced wxView class.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: view_ex.cpp,v 1.4 2005-05-01 00:50:28 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2004 to create an abstract
// enhanced view class, and in the later years given in the copyright
// notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "view_ex.hpp"

// WX !! Application object's header must be included here because
// view creation is not performed by the framework--instead, wxWindows
// treats that as the user's responsibility. The framework defines
// wxView::OnCreate() to do nothing but {return true;}, so one would
// have expected it to be a pure virtual. Unfortunately, this seems to
// mean that a view class cannot be physically decoupled from the main
// application header, and indeed there are no wxView derivatives in
// the framework.

#include "alert.hpp"
#include "docmanager_ex.hpp"
#include "main_wx.hpp" // wxGetApp()
#include "path_utility.hpp"

#include <wx/dc.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

IMPLEMENT_ABSTRACT_CLASS(ViewEx, wxView)

ViewEx::ViewEx()
    :wxView()
{
}

ViewEx::~ViewEx()
{
}

DocManagerEx& ViewEx::DocManager() const
{
    return dynamic_cast<DocManagerEx&>(*GetDocumentManager());
}

wxFrame& ViewEx::FrameWindow() const
{
    return dynamic_cast<wxFrame&>(*GetFrame());
}

// WX !! Elsewhere, the result of wxXmlResource::Get()->LoadX is
// checked before doing anything with it. But here, LoadIcon returns a
// reference, not a pointer. The library returns a default-constructed
// wxIcon on load failure. This seems inconsistent with the way other
// resources such as menubars and toolbars are treated.
wxIcon ViewEx::IconFromXmlResource(char const* z) const
{
    wxIcon icon = wxXmlResource::Get()->LoadIcon(z);
    if(!icon.Ok())
        {
        warning()
            << "IconFromXmlResource(): invalid icon; using default."
            << LMI_FLUSH
            ;
        }
    return icon;
}

wxMenuBar* ViewEx::MenuBarFromXmlResource(char const* z) const
{
    wxMenuBar* menubar = wxXmlResource::Get()->LoadMenuBar(z);
    if(!menubar)
        {
        warning()
            << "MenuBarFromXmlResource(): null menubar pointer."
            << LMI_FLUSH
            ;
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

// WX !! The wx documentation for wxDocMDIChildFrame::OnActivate() says
//   "You may need to override (but still call) this function in order
//   to set the keyboard focus for your subwindow."
// At least for msw, creation does not trigger 'activation': more
// precisely, WM_MDICREATE does not send WM_MDIACTIVATE, and
// OnActivate() doesn't get called when the window is created, so the
// documented method doesn't work--whereas setting the focus upon
// creation here does work.
//
bool ViewEx::OnCreate(wxDocument* doc, long)
{
    wxGetApp().CreateChildFrame(doc, this);
    DocManager().AssociateFileHistoryWithFileMenu(FrameWindow().GetMenuBar());
    GetFrame()->SetTitle("Loading document...");
    CreateChildWindow()->SetFocus();

    // WX !! This should be done inside the library.
#ifdef __X__
    // The X Window Toolkit seems to require a forced resize.
    int x, y;
    GetFrame()->GetSize(&x, &y);
    GetFrame()->SetSize(-1, -1, x, y);
#endif

    GetFrame()->Show(true);
    Activate(true);

    return true;
}

void ViewEx::OnDraw(wxDC*)
{
}

std::string ViewEx::base_filename() const
{
    std::string t = GetDocument()->GetFilename().c_str();
    if(0 == t.size())
        {
        t = GetDocument()->GetTitle().c_str();
        }
// TODO ?? This assertion fires for new documents. What's the best way
// to handle this? Doesn't wx give a new document *some* name?
//    LMI_ASSERT(0 != t.size());
    if(0 != t.size())
        {
        t = "Anonymous";
        }
    fs::path path(t);
    return path.leaf();
}

std::string ViewEx::serial_filename
    (int                serial_number
    ,std::string const& extension
    ) const
{
    return base_filename() + serialize_extension(serial_number, extension);
}

