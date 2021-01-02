// Rounding rules manager.
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

#include "rounding_view.hpp"

#include "alert.hpp"
#include "rounding_document.hpp"
#include "rounding_view_editor.hpp"     // class RoundingButtons
#include "safely_dereference_as.hpp"

#include <wx/panel.h>
#include <wx/window.h>
#include <wx/xrc/xmlres.h>

IMPLEMENT_DYNAMIC_CLASS(RoundingView, ProductEditorView)

wxWindow* RoundingView::CreateChildWindow()
{
    wxWindow* frame = GetFrame();
    wxPanel* main_panel = wxXmlResource::Get()->LoadPanel
        (frame
        ,"rounding_view_panel"
        );
    if(!main_panel)
        {
        alarum() << "Unable to load xml resource." << LMI_FLUSH;
        }

    for(auto const& i : document().values())
        {
        RoundingButtons* control = dynamic_cast<RoundingButtons*>
            (wxWindow::FindWindowById
                (wxXmlResource::GetXRCID(i.first.c_str())
                ,frame
                )
            );
        if(!control)
            {
            alarum()
                << "Required text control '"
                << i.first
                << "' not found."
                << LMI_FLUSH
                ;
            }
        controls_[i.first] = control;
        }
    return main_panel;
}

char const* RoundingView::icon_xrc_resource() const
{
    return "rounding_view_icon";
}

char const* RoundingView::menubar_xrc_resource() const
{
    return "rounding_view_menu";
}

RoundingDocument& RoundingView::document() const
{
    return safely_dereference_as<RoundingDocument>(GetDocument());
}

bool RoundingView::IsModified() const
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

void RoundingView::DiscardEdits()
{
    for(auto const& i : controls())
        {
        i.second->DiscardEdits();
        }
}
