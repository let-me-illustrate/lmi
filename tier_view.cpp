// Stratified charges manager.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "tier_view.hpp"

#include "multidimgrid_any.hpp"
#include "multidimgrid_safe.tpp"
#include "multidimgrid_tools.hpp"
#include "safely_dereference_as.hpp"
#include "stratified_charges.hpp"
#include "stratified_charges.xpp"
#include "tier_document.hpp"
#include "tier_view_editor.hpp"
#include "wx_new.hpp"

#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/window.h>

namespace
{
struct tier_entity_info
{
    e_stratified index;
    e_stratified parent_index;
    char const*  short_name;
    char const*  long_name;
};

std::vector<tier_entity_info> const& get_tier_entity_infos()
{
    static int const n = e_stratified_last;

    static tier_entity_info const static_tier_entity_info_array[n] =
        { LMI_TIERED_CHARGE_NAMES };

    static std::vector<tier_entity_info> const static_tier_entity_infos
        (static_tier_entity_info_array + 0
        ,static_tier_entity_info_array + n
        );

    return static_tier_entity_infos;
}

/// Stores additional information in a wxTree node

class tier_tree_item_data
  :public wxTreeItemData
{
  public:
    tier_tree_item_data(tier_entity_info const&);
    ~tier_tree_item_data() override = default;

    tier_entity_info const& entity_info() const;

  private:
    tier_entity_info const& entity_info_;
};

tier_tree_item_data::tier_tree_item_data(tier_entity_info const& entity_info)
    :wxTreeItemData {}
    ,entity_info_   {entity_info}
{
}

tier_entity_info const& tier_tree_item_data::entity_info() const
{
    return entity_info_;
}
} // Unnamed namespace.

IMPLEMENT_DYNAMIC_CLASS(TierView, TreeGridViewBase)

BEGIN_EVENT_TABLE(TierView, TreeGridViewBase)
    EVT_TREE_SEL_CHANGED(wxID_ANY, TierView::UponTreeSelectionChange)
END_EVENT_TABLE()

TierView::TierView()
    :TreeGridViewBase {}
    ,table_adapter_   {new TierTableAdapter()}
{
}

TierView::~TierView() = default;

wxTreeCtrl* TierView::CreateTreeCtrl(wxWindow* parent)
{
    return new(wx) AutoResizingTreeCtrl
        (parent
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT
        );
}

MultiDimGrid* TierView::CreateGridCtrl(wxWindow* parent)
{
    return new(wx) TierEditorGrid(parent, table_adapter_);
}

void TierView::SetupControls()
{
    std::map<e_stratified,wxTreeItemId> index_to_id;

    wxTreeCtrl& tree_ctrl = tree();

    for(auto const& entity : get_tier_entity_infos())
        {
        if(entity.index == entity.parent_index)
            {
            wxTreeItemId id = tree_ctrl.AddRoot("");
            index_to_id[entity.index] = id;
            }
        else
            {
            wxTreeItemId id = tree_ctrl.AppendItem
                (index_to_id[entity.parent_index]
                ,entity.short_name
                ,-1
                ,-1
                ,new(wx) tier_tree_item_data(entity)
                );
            index_to_id[entity.index] = id;
            }
        }

    // Force BestSize to be recalculated, since we have added new items
    tree_ctrl.InvalidateBestSize();
}

char const* TierView::icon_xrc_resource() const
{
    return "tier_view_icon";
}

char const* TierView::menubar_xrc_resource() const
{
    return "tier_view_menu";
}

bool TierView::IsModified() const
{
    return table_adapter_->IsModified();
}

void TierView::DiscardEdits()
{
    table_adapter_->SetModified(false);
}

TierDocument& TierView::document() const
{
    return safely_dereference_as<TierDocument>(GetDocument());
}

void TierView::UponTreeSelectionChange(wxTreeEvent& event)
{
    wxTreeCtrl& tree_ctrl = tree();
    tier_tree_item_data* item_data = dynamic_cast<tier_tree_item_data*>
        (tree_ctrl.GetItemData(event.GetItem())
        );

    if(item_data)
        {
        bool is_topic = tree_ctrl.GetChildrenCount(event.GetItem());

        set_grid_label_text(item_data->entity_info().long_name);

        if(is_topic)
            {
            table_adapter_->SetTierEntity(tier_entity_adapter());
            }
        else
            {
            stratified_entity& entity = document().get_stratified_entity
                (item_data->entity_info().index
                );
            table_adapter_->SetTierEntity
                (tier_entity_adapter(entity.limits_, entity.values_)
                );
            }

        MultiDimGrid& grid_ctrl = grid();

        wxSizer* sizer = grid_ctrl.GetContainingSizer();
        LMI_ASSERT(sizer);
        sizer->Show(&grid_ctrl, !is_topic);
        sizer->Layout();

        grid_ctrl.RefreshTableFull();
        }
}
