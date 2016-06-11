// Product data manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "policy_view.hpp"

#include "alert.hpp"
#include "policy_document.hpp"
#include "safely_dereference_as.hpp"

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <wx/xrc/xmlres.h>

IMPLEMENT_DYNAMIC_CLASS(PolicyView, ProductEditorView)

PolicyView::PolicyView()
    :ProductEditorView()
    ,controls_()
{
}

PolicyView::~PolicyView()
{
}

wxWindow* PolicyView::CreateChildWindow()
{
    wxWindow* frame = GetFrame();
    wxPanel* main_panel = wxXmlResource::Get()->LoadPanel
        (frame
        ,"policy_view_panel"
        );
    if(!main_panel)
        {
        fatal_error() << "Unable to load xml resource." << LMI_FLUSH;
        }

    typedef PolicyDocument::values_type::const_iterator value_const_iterator;
    for
        (value_const_iterator cit = document().values().begin()
        ,end = document().values().end()
        ;cit != end
        ;++cit
        )
        {
        wxTextCtrl* text_ctrl = dynamic_cast<wxTextCtrl*>
            (wxWindow::FindWindowById
                (wxXmlResource::GetXRCID(cit->first.c_str())
                ,frame
                )
            );
        if(!text_ctrl)
            {
            fatal_error()
                << "Required text control '"
                << cit->first
                << "' not found."
                << LMI_FLUSH
                ;
            }
        controls_[cit->first] = text_ctrl;
        }

    return main_panel;
}

char const* PolicyView::icon_xrc_resource() const
{
    return "policy_view_icon";
}

char const* PolicyView::menubar_xrc_resource() const
{
    return "policy_view_menu";
}

PolicyDocument& PolicyView::document() const
{
    return safely_dereference_as<PolicyDocument>(GetDocument());
}

bool PolicyView::IsModified() const
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

void PolicyView::DiscardEdits()
{
    for
        (controls_type::iterator it = controls().begin()
        ,end = controls().end()
        ;it != end
        ;++it
        )
        {
        it->second->DiscardEdits();
        }
}
