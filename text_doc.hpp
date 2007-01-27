// Document class for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: text_doc.hpp,v 1.6 2007-01-27 00:00:52 wboutin Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/doc.h (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-document
// class out of a larger file, and in the later years given in the
// copyright notice above.

// Because text windows have their own functions for loading and saving
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
    virtual ~TextEditDocument();

  private:
    // wxDocument overrides.
    virtual bool IsModified() const;
    virtual void Modify(bool);
    virtual bool OnOpenDocument(wxString const& filename);
    virtual bool OnSaveDocument(wxString const& filename);

    wxTextCtrl& PredominantViewWindow() const;

    DECLARE_DYNAMIC_CLASS(TextEditDocument)
};

#endif // text_doc_hpp

