// Document class for html.
//
// Copyright (C) 2002, 2003, 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: html_doc.cpp,v 1.1.1.1 2004-05-15 19:58:38 chicares Exp $

#include "pchfile.hpp"

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "html_doc.hpp"

#include "html_view.hpp"

#include <wx/html/htmlwin.h>
#include <wx/log.h>

IMPLEMENT_DYNAMIC_CLASS(HtmlDocument, wxDocument)

HtmlDocument::HtmlDocument()
{
}

HtmlDocument::~HtmlDocument()
{
}

// Some wx samples do things like
//
//    if(!view || !view->window || !view->window->LoadPage(filename))
//
// which is cumbersome and error prone. It seems better to factor
// such potentially-repeated tests into a function that signals an
// error if the view and its associated child window should exist but
// do not.
//
// TODO ?? This wants to be a template in a base class or at least
// in a separate header.
//
// Somewhat naively, assume that the first view of the appropriate
// type is the one that contains the authoritative data.
wxHtmlWindow* HtmlDocument::DominantViewWindow() const
{
    HtmlView* view = 0;
    while(wxList::compatibility_iterator node = GetViews().GetFirst())
        {
        if(node->GetData()->IsKindOf(CLASSINFO(HtmlView)))
            {
            view = dynamic_cast<HtmlView*>(node->GetData());
            break;
            }
        node = node->GetNext();
        }
    if(!view)
        {
// WX!! When wx handles exceptions more gracefully, throw here:
        wxLogError("HtmlDocument::DominantViewWindow(): null view.");
        }
    if(!view->html_window_)
        {
// WX!! When wx handles exceptions more gracefully, throw here:
        wxLogError("HtmlDocument::DominantViewWindow(): null window.");
        }
    return view->html_window_;
}

bool HtmlDocument::OnOpenDocument(wxString const& filename)
{
    if(!DominantViewWindow()->LoadPage(filename))
        {
        return false;
        }

    SetFilename(filename, true);
    Modify(false);
    UpdateAllViews();
    return true;
}

