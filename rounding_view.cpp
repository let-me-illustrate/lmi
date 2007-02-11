// Rounding rules manager.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: rounding_view.cpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "rounding_view.hpp"
#include "rounding_view_editor.hpp"

#include "alert.hpp"
#include "rounding_document.hpp"
#include "wx_new.hpp"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <wx/xrc/xmlres.h>

#include <sstream>

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
        fatal_error() << "Unable to load an xml resource" << LMI_FLUSH;

    typedef RoundingDocument::values_type::const_iterator value_const_iterator;
    for
        (value_const_iterator cit = document().values().begin()
        ,end = document().values().end()
        ;cit != end
        ;++cit
        )
        {
        RoundingButtons* control = dynamic_cast<RoundingButtons*>
            (wxWindow::FindWindowById(XRCID(cit->first.c_str()), frame)
            );
        if(!control)
            fatal_error()
                << "A required text control ["
                << cit->first
                << "] was not found"
                << LMI_FLUSH
                ;
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
    return dynamic_cast<RoundingDocument&>(*GetDocument());
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
            return true;
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

