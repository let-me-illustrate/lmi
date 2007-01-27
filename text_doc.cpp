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

// $Id: text_doc.cpp,v 1.6 2007-01-27 00:00:52 wboutin Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/doc.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-document
// class out of a larger file, and in the later years given in the
// copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "text_doc.hpp"

#include "view_ex.tpp"

#include "alert.hpp"
#include "text_view.hpp"

#include <wx/textctrl.h>

IMPLEMENT_DYNAMIC_CLASS(TextEditDocument, wxDocument)

TextEditDocument::TextEditDocument()
    :wxDocument()
{
}

TextEditDocument::~TextEditDocument()
{
}

wxTextCtrl& TextEditDocument::PredominantViewWindow() const
{
    return ::PredominantViewWindow<TextEditView,wxTextCtrl>
        (*this
        ,&TextEditView::text_window_
        );
}

// When this function is called, a view may or may not exist.
// The framework behaves this way:
//   if the view contains no modified data
//     destroy the view
// then call the present function:
//   if the document contains no modified data
//     destroy the document
//   else
//     prompt to save the document
bool TextEditDocument::IsModified() const
{
    bool is_modified = wxDocument::IsModified();
    if(GetViews().GetCount())
        {
        is_modified = is_modified || PredominantViewWindow().IsModified();
        }
    return is_modified;
}

void TextEditDocument::Modify(bool mod)
{
    wxDocument::Modify(mod);
    if(!mod)
        {
        PredominantViewWindow().DiscardEdits();
        }
}

bool TextEditDocument::OnOpenDocument(wxString const& filename)
{
    if(!PredominantViewWindow().LoadFile(filename))
        {
        return false;
        }

    SetFilename(filename, true);
    Modify(false);
    UpdateAllViews();
    return true;
}

bool TextEditDocument::OnSaveDocument(wxString const& filename)
{
    if(!PredominantViewWindow().SaveFile(filename))
        {
        return false;
        }
    Modify(false);
    return true;
}

