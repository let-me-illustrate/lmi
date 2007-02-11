// Stratified charges manager.
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

// $Id: tier_view.cpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "tier_view.hpp"

#include "multidimgrid_safe.tpp"

#include "multidimgrid_any.hpp"
#include "multidimgrid_tools.hpp"
#include "stratified_charges.xpp"
#include "tier_document.hpp"
#include "tier_view_editor.hpp"
#include "wx_new.hpp"

#include <wx/window.h>

#include <sstream>

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

    static tier_entity_info const static_tier_entity_info_array[n]
        = { LMI_TIERED_CHARGE_NAMES };

    static std::vector<tier_entity_info> const static_tier_entity_infos
        (static_tier_entity_info_array + 0
        ,static_tier_entity_info_array + n
        );

    return static_tier_entity_infos;
}

} // unnamed namespace

IMPLEMENT_DYNAMIC_CLASS(TierView, TreeGridViewBase)

BEGIN_EVENT_TABLE(TierView, TreeGridViewBase)
    EVT_TREE_SEL_CHANGED(wxID_ANY, TierView::UponTreeSelectionChange)
END_EVENT_TABLE()

TierView::TierView()
    :TreeGridViewBase()
    ,table_adapter_(new TierTableAdapter())
{
}

TierView::~TierView()
{
}

wxTreeCtrl* TierView::CreateTreeCtrl(wxWindow* panel)
{
    return new(wx) AutoResizingTreeCtrl
        (panel
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT
        );
}

MultiDimGrid* TierView::CreateGridCtrl(wxWindow* panel)
{
    return new(wx) TierEditorGrid(panel, table_adapter_);
}

void TierView::SetupControls()
{
    std::vector<tier_entity_info> const& entities = get_tier_entity_infos();
    std::map<e_stratified, wxTreeItemId> index_to_id;

    wxTreeCtrl& tree = GetTreeCtrl();

    for(std::size_t i = 0; i < entities.size(); ++i)
        {
        tier_entity_info const& entity = entities[i];
        if(entity.index == entity.parent_index)
            {
            wxTreeItemId id = tree.AddRoot("");
            index_to_id[entity.index] = id;
            }
        else
            {
            wxTreeItemId id = tree.AppendItem
                (index_to_id[entity.parent_index]
                ,entity.short_name
                ,-1
                ,-1
                ,new(wx) TierTreeItemData(i, entity.long_name)
                );
            index_to_id[entity.index] = id;
            }
        }

    // Force BestSize to be recalculated, since we have added new items
    GetTreeCtrl().InvalidateBestSize();
}

wxIcon TierView::Icon() const
{
    return IconFromXmlResource("tier_view_icon");
}

wxMenuBar* TierView::MenuBar() const
{
    return MenuBarFromXmlResource("tier_view_menu");
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
    return dynamic_cast<TierDocument&>(*GetDocument());
}

void TierView::UponTreeSelectionChange(wxTreeEvent& event)
{
    wxTreeCtrl& tree = GetTreeCtrl();
    TierTreeItemData* item_data = dynamic_cast<TierTreeItemData*>
        (tree.GetItemData(event.GetItem())
        );

    if(item_data)
        {
        std::size_t index = item_data->get_id();

        bool is_topic = tree.GetChildrenCount(event.GetItem());

        std::ostringstream oss;
        if(is_topic)
            oss << "[+] ";
        oss << item_data->get_description();
        SetLabel(oss.str());

        std::vector<tier_entity_info> const& entities
            = get_tier_entity_infos();

        if(is_topic)
            {
            table_adapter_->SetTierEntity(tier_entity_adapter());
            }
        else
            {
            stratified_entity& entity
                = *document().get_stratified_entity(entities[index].index);
            table_adapter_->SetTierEntity
                (tier_entity_adapter(entity.limits_, entity.values_)
                );
            }

        MultiDimGrid& grid = GetGridCtrl();

        grid.Enable(!is_topic);
        grid.RefreshTableFull();
        }
}


