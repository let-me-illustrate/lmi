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

// $Id: html_doc.hpp,v 1.1.1.1 2004-05-15 19:58:38 chicares Exp $

// Since html windows have their own functions for loading and saving
// files, override OnOpenDocument() and OnSaveDocument() instead of
// LoadObject and SaveObject.

#ifndef html_doc_hpp
#define html_doc_hpp

#include "config.hpp"

#include <wx/docview.h>

class WXDLLEXPORT wxHtmlWindow;

class HtmlDocument
    :public wxDocument
{
  public:
    HtmlDocument();
    ~HtmlDocument();

  private:
    bool OnOpenDocument(wxString const& filename);

    wxHtmlWindow* DominantViewWindow() const;

    DECLARE_DYNAMIC_CLASS(HtmlDocument)
};

#endif // html_doc_hpp

