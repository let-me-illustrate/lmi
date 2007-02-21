// Common code used in various product editor document/view classes.
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

// $Id: product_editor.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#include "product_editor.hpp"

#include "multidimgrid_any.hpp"
#include "view_ex.tpp"
#include "wx_new.hpp"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/window.h>

ProductEditorDocument::ProductEditorDocument()
    :wxDocument()
{
}

ProductEditorDocument::~ProductEditorDocument()
{
}

ProductEditorView& ProductEditorDocument::PredominantView() const
{
    return ::PredominantView<ProductEditorView>(*this);
}

bool ProductEditorDocument::IsModified() const
{
    if(wxDocument::IsModified())
        return true;

    if(GetViews().empty())
        return false;

    return PredominantView().IsModified();
}

void ProductEditorDocument::Modify(bool modified)
{
    wxDocument::Modify(modified);
    if(!modified && !GetViews().empty())
        PredominantView().DiscardEdits();
}

bool ProductEditorDocument::OnOpenDocument(wxString const& filename)
{
    ReadDocument(filename);

    SetFilename(filename, true);
    Modify(false);
    UpdateAllViews();
    return true;
}

bool ProductEditorDocument::OnSaveDocument(wxString const& filename)
{
    WriteDocument(filename);

    Modify(false);
    return true;
}

ProductEditorView::ProductEditorView()
    :ViewEx()
{
}

ProductEditorView::~ProductEditorView()
{
}


TreeGridViewBase::TreeGridViewBase()
    :ProductEditorView()
{
}

TreeGridViewBase::~TreeGridViewBase()
{
}

wxWindow* TreeGridViewBase::CreateChildWindow()
{
    wxPanel* main_panel = new(wx) wxPanel(GetFrame());

    tree_ = CreateTreeCtrl(main_panel);
    grid_ = CreateGridCtrl(main_panel);

    grid_label_ = new(wx) wxStaticText(main_panel, wxID_ANY, "");

    // Title text bold and 1.5 bigger
    wxFont font = main_panel->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(font.GetPointSize() * 3 / 2);
    grid_label_->SetFont(font);

    wxBoxSizer* sizer = new(wx) wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* grid_sizer = new(wx) wxBoxSizer(wxVERTICAL);

    sizer->Add(tree_, wxSizerFlags().Proportion(0).Expand());
    sizer->Add(grid_sizer, wxSizerFlags().Proportion(1).Expand());
    grid_sizer->Add(grid_label_, wxSizerFlags().Proportion(0).Expand().Border(wxALL, 8));
    grid_sizer->Add(grid_, wxSizerFlags().Proportion(1).Expand());

    main_panel->SetSizer(sizer);

    SetupControls();

    main_panel->SetSize(GetFrame()->GetClientSize());

    return main_panel;
}

void TreeGridViewBase::SetLabel(std::string const& label)
{
    grid_label_->SetLabel(label);
}

