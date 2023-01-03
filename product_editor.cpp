// Common code used in various product editor document/view classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "product_editor.hpp"

#include "multidimgrid_any.hpp"
#include "view_ex.tpp"
#include "wx_new.hpp"

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/window.h>

ProductEditorView& ProductEditorDocument::PredominantView() const
{
    return ::PredominantView<ProductEditorView>(*this);
}

bool ProductEditorDocument::IsModified() const
{
    if(wxDocument::IsModified())
        {
        return true;
        }

    if(GetViews().empty())
        {
        return false;
        }

    return PredominantView().IsModified();
}

void ProductEditorDocument::Modify(bool modified)
{
    wxDocument::Modify(modified);
    if(!modified && !GetViews().empty())
        {
        PredominantView().DiscardEdits();
        }
}

bool ProductEditorDocument::DoOpenDocument(wxString const& filename)
{
    try
        {
        ReadDocument(filename.ToStdString(wxConvUTF8));
        return true;
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error while loading document.\n"
            << "Filename: "
            << filename
            << "\n"
            << "Message: "
            << e.what()
            << LMI_FLUSH
            ;
        return false;
        }
}

bool ProductEditorDocument::DoSaveDocument(wxString const& filename)
{
    try
        {
        WriteDocument(filename.ToStdString(wxConvUTF8));
        return true;
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error while saving document.\n"
            << "Filename: "
            << filename
            << "\n"
            << "Message: "
            << e.what()
            << LMI_FLUSH
            ;
        return false;
        }
}

wxTreeCtrl& TreeGridViewBase::tree() const
{
    if(!tree_)
        {
        alarum() << "Tree control can't be null" << LMI_FLUSH;
        }
    return *tree_;
}

MultiDimGrid& TreeGridViewBase::grid() const
{
    if(!grid_)
        {
        alarum() << "Grid control can't be null" << LMI_FLUSH;
        }
    return *grid_;
}

void TreeGridViewBase::set_grid_label_text(std::string const& label)
{
    if(!grid_label_)
        {
        alarum() << "Grid control can't be null" << LMI_FLUSH;
        }
    grid_label_->SetLabel(label);
}

wxWindow* TreeGridViewBase::CreateChildWindow()
{
    wxPanel* main_panel = new(wx) wxPanel(GetFrame());

    tree_ = CreateTreeCtrl(main_panel);
    grid_ = CreateGridCtrl(main_panel);

    grid_label_ = new(wx) wxStaticText(main_panel, wxID_ANY, "");

    // Make label text is bold.
    wxFont font = main_panel->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    grid_label_->SetFont(font);

    wxBoxSizer* const sizer = new(wx) wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* const grid_sizer = new(wx) wxBoxSizer(wxVERTICAL);

    sizer->Add
        (tree_
        ,wxSizerFlags().Proportion(0).Expand()
        );
    sizer->Add
        (grid_sizer
        ,wxSizerFlags().Proportion(1).Expand()
        );
    grid_sizer->Add
        (grid_label_
        ,wxSizerFlags().Proportion(0).Expand().Border(wxALL, 8)
        );
    grid_sizer->Add
        (grid_
        ,wxSizerFlags().Proportion(1).Expand()
        );

    main_panel->SetSizer(sizer);

    SetupControls();

    main_panel->SetSize(GetFrame()->GetClientSize());

    // Hide the grid until a node in the tree is selected
    grid_sizer->Hide(grid_);

    return main_panel;
}
