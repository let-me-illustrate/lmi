// Rounding rules manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "rounding_view.hpp"

#include "alert.hpp"
#include "rounding_document.hpp"
#include "rounding_view_editor.hpp" // class RoundingButtons
#include "safely_dereference_as.hpp"

#include <wx/icon.h>
#include <wx/panel.h>
#include <wx/window.h>
#include <wx/xrc/xmlres.h>

IMPLEMENT_DYNAMIC_CLASS(RoundingView, ProductEditorView)

RoundingView::RoundingView()
    :ProductEditorView()
    ,controls_()
{
}

RoundingView::~RoundingView()
{
}

wxWindow* RoundingView::CreateChildWindow()
{
    wxWindow* frame = GetFrame();
    wxPanel* main_panel = wxXmlResource::Get()->LoadPanel
        (frame
        ,"rounding_view_panel"
        );
    if(!main_panel)
        {
        fatal_error() << "Unable to load xml resource." << LMI_FLUSH;
        }

    typedef RoundingDocument::values_type::const_iterator value_const_iterator;
    for
        (value_const_iterator cit = document().values().begin()
        ,end = document().values().end()
        ;cit != end
        ;++cit
        )
        {
        RoundingButtons* control = dynamic_cast<RoundingButtons*>
            (wxWindow::FindWindowById
                (wxXmlResource::GetXRCID(cit->first.c_str())
                ,frame
                )
            );
        if(!control)
            {
            fatal_error()
                << "Required text control '"
                << cit->first
                << "' not found."
                << LMI_FLUSH
                ;
            }
        controls_[cit->first] = control;
        }
    return main_panel;
}

wxIcon RoundingView::Icon() const
{
    return IconFromXmlResource("rounding_view_icon");
}

wxMenuBar* RoundingView::MenuBar() const
{
    return MenuBarFromXmlResource("rounding_view_menu");
}

RoundingDocument& RoundingView::document() const
{
    return safely_dereference_as<RoundingDocument>(GetDocument());
}

bool RoundingView::IsModified() const
{
    for
        (controls_type::const_iterator it = controls().begin()
        ,end = controls().end()
        ;it != end
        ;++it
        )
        {
        if(it->second->IsModified())
            {
            return true;
            }
        }
    return false;
}

void RoundingView::DiscardEdits()
{
    for
        (controls_type::const_iterator it = controls().begin()
        ,end = controls().end()
        ;it != end
        ;++it
        )
        {
        it->second->DiscardEdits();
        }
}

