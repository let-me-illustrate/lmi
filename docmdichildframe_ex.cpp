// Customize implementation details of library class wxDocMDIChildFrame.
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

// $Id: docmdichildframe_ex.cpp,v 1.1 2005-03-11 03:09:22 chicares Exp $

#include "docmdichildframe_ex.hpp"

#include <wx/menu.h>

// FSF !! Expunge this when wx is enhanced as specified below.
#ifdef __WXMSW__
#   include <wx/msw/wrapwin.h>
#endif // __WXMSW__

#include <stdexcept>

IMPLEMENT_CLASS(DocMDIChildFrameEx, wxDocMDIChildFrame)

BEGIN_EVENT_TABLE(DocMDIChildFrameEx, wxDocMDIChildFrame)
    EVT_ACTIVATE(DocMDIChildFrameEx::OnActivate)
    EVT_MENU_HIGHLIGHT_ALL(DocMDIChildFrameEx::OnMenuHighlight)
END_EVENT_TABLE()

DocMDIChildFrameEx::DocMDIChildFrameEx
    (wxDocument*       doc
    ,wxView*           view
    ,wxMDIParentFrame* parent
    ,wxWindowID        id
    ,wxString   const& title
    ,wxPoint    const& pos
    ,wxSize     const& size
    ,long int          style
    ,wxString   const& name
    )
    :wxDocMDIChildFrame(doc, view, parent, id, title, pos, size, style, name)
    ,status_bar_sought_from_menu_highlight_handler_(false)
{
}

DocMDIChildFrameEx::~DocMDIChildFrameEx()
{
}

wxStatusBar* DocMDIChildFrameEx::GetStatusBar() const
{
    if(!status_bar_sought_from_menu_highlight_handler_)
        {
        return wxDocMDIChildFrame::GetStatusBar();
        }

    wxStatusBar* status_bar = wxDocMDIChildFrame::GetStatusBar();
    if(status_bar)
        {
        return status_bar;
        }

    wxFrame* parent_frame = dynamic_cast<wxFrame*>(GetParent());
    if(parent_frame)
        {
        return parent_frame->GetStatusBar();
        }

    return 0;
}

void DocMDIChildFrameEx::OnActivate(wxActivateEvent& event)
{
    wxDocMDIChildFrame::OnActivate(event);
    SetMdiWindowMenu();
}

void DocMDIChildFrameEx::OnMenuHighlight(wxMenuEvent& event)
{
    try
        {
        status_bar_sought_from_menu_highlight_handler_ = true;
        if(GetStatusBar())
            {
            wxDocMDIChildFrame::OnMenuHighlight(event);
            }
        status_bar_sought_from_menu_highlight_handler_ = false;
        }
    catch(...)
        {
        status_bar_sought_from_menu_highlight_handler_ = false;
        throw;
        }
}

void DocMDIChildFrameEx::SetMdiWindowMenu() const
{
// FSF !! Need this in the wx library for GNU/linux.
#ifdef __WXMSW__
    wxMDIParentFrame* parent_frame = dynamic_cast<wxMDIParentFrame*>(GetParent());
    if(!parent_frame)
        {
        throw std::runtime_error("MDI child frame has no parent.");
        }

    wxMDIClientWindow* client_window = parent_frame->GetClientWindow();
    if(!client_window)
        {
        throw std::runtime_error("Child frame's parent has no client window.");
        }
    HWND client_handle = (HWND)client_window->GetHandle();

    wxMenuBar* menu_bar = GetMenuBar();
    if(!menu_bar)
        {
        return;
        }

    int window_menu_index = menu_bar->FindMenu("Window");
    if(wxNOT_FOUND == window_menu_index)
        {
        return;
        }

    wxMenu* window_menu = menu_bar->GetMenu(window_menu_index);
    HMENU window_menu_handle = (HMENU)window_menu->GetHMenu();

    ::SendMessage(client_handle, WM_MDISETMENU, 0, (LPARAM)window_menu_handle);
    ::DrawMenuBar(client_handle);
#endif // __WXMSW__
}

