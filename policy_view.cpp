// Product data manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "policy_view.hpp"

#include "alert.hpp"
#include "policy_document.hpp"
#include "safely_dereference_as.hpp"

#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/window.h>
#include <wx/xrc/xmlres.h>

IMPLEMENT_DYNAMIC_CLASS(PolicyView, ProductEditorView)

wxWindow* PolicyView::CreateChildWindow()
{
    wxWindow* frame = GetFrame();
    wxPanel* main_panel = wxXmlResource::Get()->LoadPanel
        (frame
        ,"policy_view_panel"
        );
    if(!main_panel)
        {
        alarum() << "Unable to load xml resource." << LMI_FLUSH;
        }

    for(auto const& i : document().values())
        {
        wxTextCtrl* text_ctrl = dynamic_cast<wxTextCtrl*>
            (wxWindow::FindWindowById
                (wxXmlResource::GetXRCID(i.first.c_str())
                ,frame
                )
            );
        if(!text_ctrl)
            {
            alarum()
                << "Required text control '"
                << i.first
                << "' not found."
                << LMI_FLUSH
                ;
            }
        controls_[i.first] = text_ctrl;
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
    for(auto const& i : controls())
        {
        if(i.second->IsModified())
            {
            return true;
            }
        }
    return false;
}

void PolicyView::DiscardEdits()
{
    for(auto const& i : controls())
        {
        i.second->DiscardEdits();
        }
}
