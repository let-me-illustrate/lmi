// Document class for plain text.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: text_doc.cpp,v 1.3 2005-12-01 04:06:34 chicares Exp $

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

// INELEGANT !! This wants to be a template in a base class or at
// least in a separate header.
//
// Somewhat naively, assume that the first view of the appropriate
// type is the one that contains the authoritative data.
//
wxTextCtrl& TextEditDocument::DominantViewWindow() const
{
    TextEditView const* view = 0;
    wxList const& vl = GetViews();
    for(wxList::const_iterator i = vl.begin(); i != vl.end(); ++i)
        {
        wxObject const* p = *i;
        LMI_ASSERT(0 != p);
        if(p->IsKindOf(CLASSINFO(TextEditView)))
            {
            view = dynamic_cast<TextEditView const*>(p);
            break;
            }
        }
    if(!view)
        {
        fatal_error() << "Text view not found." << LMI_FLUSH;
        }
    if(!view->text_window_)
        {
        fatal_error() << "Text window not found." << LMI_FLUSH;
        }
    return *view->text_window_;
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
        is_modified = is_modified || DominantViewWindow().IsModified();
        }
    return is_modified;
}

void TextEditDocument::Modify(bool mod)
{
    wxDocument::Modify(mod);
    if(!mod)
        {
        DominantViewWindow().DiscardEdits();
        }
}

bool TextEditDocument::OnOpenDocument(wxString const& filename)
{
    if(!DominantViewWindow().LoadFile(filename))
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
    if(!DominantViewWindow().SaveFile(filename))
        {
        return false;
        }
    Modify(false);
    return true;
}

