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

// $Id: text_doc.cpp,v 1.1.1.1 2004-05-15 19:59:25 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/doc.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2002 to factor the text-document
// class out of a larger file, and in the later years given in the
// copyright notice above.

#include "pchfile.hpp"

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "text_doc.hpp"

#include "text_view.hpp"

#include <wx/log.h>
#include <wx/textctrl.h>

IMPLEMENT_DYNAMIC_CLASS(TextEditDocument, wxDocument)

TextEditDocument::TextEditDocument()
    :wxDocument()
{
}

TextEditDocument::~TextEditDocument()
{
}

// TODO ?? This wants to be a template in a base class or at least
// in a separate header.
//
// Somewhat naively, assume that the first view of the appropriate
// type is the one that contains the authoritative data.
wxTextCtrl* TextEditDocument::DominantViewWindow() const
{
    TextEditView* view = 0;
    while(wxList::compatibility_iterator node = GetViews().GetFirst())
        {
        if(node->GetData()->IsKindOf(CLASSINFO(TextEditView)))
            {
            view = dynamic_cast<TextEditView*>(node->GetData());
            break;
            }
        node = node->GetNext();
        }
    if(!view)
        {
// WX!! When wx handles exceptions more gracefully, throw here:
        wxLogError("TextEditDocument::DominantViewWindow(): null view.");
        }
    if(!view->text_window_)
        {
// WX!! When wx handles exceptions more gracefully, throw here:
        wxLogError("TextEditDocument::DominantViewWindow(): null window.");
        }
    return view->text_window_;
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
        is_modified = is_modified || DominantViewWindow()->IsModified();
        }
    return is_modified;
}

void TextEditDocument::Modify(bool mod)
{
    wxDocument::Modify(mod);
    if(!mod)
        {
        DominantViewWindow()->DiscardEdits();
        }
}

bool TextEditDocument::OnOpenDocument(wxString const& filename)
{
    if(!DominantViewWindow()->LoadFile(filename))
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
    if(!DominantViewWindow()->SaveFile(filename))
        {
        return false;
        }
    Modify(false);
    return true;
}

