// Document view for html.
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

// $Id: html_view.cpp,v 1.1.1.1 2004-05-15 19:58:38 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The originals were modified by GWC in 2003 to create a standalone
// view class customized for html documents, and in the later years
// given in the copyright notice above.

#include "pchfile.hpp"

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "html_view.hpp"

#include "html_doc.hpp"
#include "wx_new.hpp"

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/icon.h>
#include <wx/menu.h>

IMPLEMENT_DYNAMIC_CLASS(HtmlView, ViewEx)

BEGIN_EVENT_TABLE(HtmlView, ViewEx)
//    None for now....
END_EVENT_TABLE()

HtmlView::HtmlView()
    :ViewEx      ()
    ,html_window_(0)
{
}

HtmlView::~HtmlView()
{
}

wxWindow* HtmlView::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

wxIcon const& HtmlView::Icon() const
{
    static wxIcon const icon = wxICON(IDI_HTML_VIEW);
    return icon;
}

wxMenuBar* HtmlView::MenuBar() const
{
    return MenuBarFromXmlResource("html_view_menu");
}

wxPrintout* HtmlView::OnCreatePrintout()
{
    // WX!! The string argument seems pretty pointless: it simply
    // follows "Please wait while printing" in messagebox text. The
    // argument is documented as a "title", but the messagebox title
    // is immutably "Printing failed".
    wxHtmlPrintout* printout = new(wx) wxHtmlPrintout("");
    printout->SetHtmlFile(GetDocument()->GetFilename());
    return printout;
}

