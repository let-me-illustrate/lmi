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

// $Id: html_view.hpp,v 1.1.1.1 2004-05-15 19:58:43 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2003 to create a standalone
// view class customized for html documents, and in the later years
// given in the copyright notice above.

#ifndef html_view_hpp
#define html_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

class WXDLLEXPORT wxHtmlWindow;

class HtmlView
    :public ViewEx
{
    friend class HtmlDocument;

  public:
    HtmlView();
    ~HtmlView();

  private:
    HtmlView(HtmlView const&);
    HtmlView& operator=(HtmlView const&);

    wxWindow* CreateChildWindow();
    wxPrintout* OnCreatePrintout();

    wxIcon const& Icon() const;
    wxMenuBar* MenuBar() const;

    wxHtmlWindow* html_window_;

    DECLARE_DYNAMIC_CLASS(HtmlView)
    DECLARE_EVENT_TABLE()
};

#endif // html_view_hpp

