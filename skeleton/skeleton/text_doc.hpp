// Document class for plain text.
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

// $Id: text_doc.hpp,v 1.1.1.1 2004-05-15 19:59:25 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/doc.h (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-document
// class out of a larger file, and in the later years given in the
// copyright notice above.

// Since text windows have their own functions for loading and saving
// files, override OnOpenDocument() and OnSaveDocument() instead of
// LoadObject and SaveObject.

#ifndef text_doc_hpp
#define text_doc_hpp

#include "config.hpp"

#include <wx/docview.h>

class WXDLLEXPORT wxTextCtrl;

class TextEditDocument
    :public wxDocument
{
  public:
    TextEditDocument();
    ~TextEditDocument();

  private:
    void Modify(bool mod);

    bool OnOpenDocument(wxString const& filename);
    bool OnSaveDocument(wxString const& filename);

    wxTextCtrl* DominantViewWindow() const;
    bool IsModified() const;
    
    DECLARE_DYNAMIC_CLASS(TextEditDocument)
};

#endif // text_doc_hpp

