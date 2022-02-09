// Database dictionary manager.
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

#include "database_view.hpp"

#include "database_document.hpp"
#include "database_view_editor.hpp"
#include "dbnames.hpp"
#include "multidimgrid_any.hpp"
#include "multidimgrid_tools.hpp"
#include "safely_dereference_as.hpp"
#include "wx_new.hpp"

#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <wx/window.h>

#include <map>

namespace
{

/// Store information from struct db_names as wxTreeItemData.
///
/// EVGENIY !! Here are some changes we discussed on the mailing list:
///
///  - Rename 'id' to something like 'database_key' (and 'id_'
///    similarly).
///
///  - Change the type of 'id_' to (enum) e_database_key.
///
/// However, shouldn't we rewrite this class completely instead?
///
/// Its purpose is to represent the information in struct db_names,
/// while deriving from class wxTreeItemData. Yet it contains only two
/// of that struct's four members--these:
///    e_database_key      Idx;
///    char const*         LongName;
/// but not these:
///    e_database_key      ParentIdx;
///    char const*         ShortName;
/// The fields not included are of course accessed by indexing a
/// db_names object from the vector returned by this function:
///   LMI_SO std::vector<db_names> const& GetDBNames();
/// Even db_names::Idx is accessed that way, and I think our
/// discussions have raised the issue of whether that member's value
/// should be asserted to equal the value of the loop counter in
/// DatabaseView::SetupControls(). But why index by a sequential
/// loop counter instead of iterating across an available vector?
/// The former could break if enumerators ever fail to follow the
/// pattern 0, 1, 2, ... N-1; but iterating across a vector works
/// robustly without relying on any such assumption. And why have a
/// copy of 'LongName' here but not 'ShortName'?
///
/// I see two other designs to consider:
///
/// (1) This class holds only an e_database_key enum. That's enough to
/// find the corresponding struct db_names in the vector returned by
///   LMI_SO std::vector<db_names> const& GetDBNames();
/// and we can then access that struct's members directly. And we
/// don't have to know that description() here is 'LongName' there.
/// Then the only thing we have to worry about is mapping between
/// that enum and wxTreeItemData::GetId().
///
/// (2) This class holds a db_names struct. To make that work, I guess
/// we'd have to rewrite that struct (I mean, char* members? in 2007?
/// Is that required for static initialization to work?) so that its
/// implicitly-defined member functions do the right thing, and hold a
/// copy of it here as the itemdata. (I don't know whether that'd make
/// populating the tree too slow.)
///
/// Or maybe even a third:
///
/// (3) Mixin programming:
///
///   class database_tree_item_data
///       :public wxTreeItemData
///       ,public db_names
///   {}; // Is any implementation actually required?
///
/// Is (3) an abuse of inheritance? AFAICT it satisfies the LSP.
/// I think it's either elegant or abhorrent; I'm not sure which.

class database_tree_item_data
  :public wxTreeItemData
{
  public:
    database_tree_item_data(db_names const&);
    ~database_tree_item_data() override = default;

    db_names const& db_name() const {return db_names_;}

    std::pair<int,int> get_axes_selected() const;
    void set_axes_selected(std::pair<int,int> const&);

  private:
    db_names const& db_names_;
    std::pair<int,int> axes_selected_;
};

database_tree_item_data::database_tree_item_data(db_names const& names)
    :db_names_      {names}
    ,axes_selected_ {wxNOT_FOUND, wxNOT_FOUND}
{
}

std::pair<int,int> database_tree_item_data::get_axes_selected() const
{
    return axes_selected_;
}

void database_tree_item_data::set_axes_selected
    (std::pair<int,int> const& axes_selected
    )
{
    axes_selected_ = axes_selected;
}

} // Unnamed namespace.

IMPLEMENT_DYNAMIC_CLASS(DatabaseView, TreeGridViewBase)

BEGIN_EVENT_TABLE(DatabaseView, TreeGridViewBase)
    EVT_TREE_SEL_CHANGED(wxID_ANY, DatabaseView::UponTreeSelectionChange)
END_EVENT_TABLE()

DatabaseView::DatabaseView()
    :TreeGridViewBase {}
    ,table_adapter_   {new DatabaseTableAdapter()}
{
}

wxTreeCtrl* DatabaseView::CreateTreeCtrl(wxWindow* parent)
{
    return new(wx) AutoResizingTreeCtrl
        (parent
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT
        );
}

MultiDimGrid* DatabaseView::CreateGridCtrl(wxWindow* parent)
{
    LMI_ASSERT(table_adapter_);
    return new(wx) DatabaseEditorGrid(parent, table_adapter_);
}

void DatabaseView::SetupControls()
{
    std::map<e_database_key,wxTreeItemId> name_to_id;

    wxTreeCtrl& tree_ctrl = tree();

    for(auto const& name : GetDBNames())
        {
        if(DB_FIRST == name.Idx)
            {
            LMI_ASSERT(name.Idx == name.ParentIdx);
            wxTreeItemId id = tree_ctrl.AddRoot
                (""
                ,-1
                ,-1
                ,new(wx) database_tree_item_data(name)
                );
            name_to_id[name.Idx] = id;
            }
        else
            {
            LMI_ASSERT(name.Idx != name.ParentIdx);
            wxTreeItemId parent = name_to_id[name.ParentIdx];
            wxTreeItemId id = tree_ctrl.AppendItem
                (parent
                ,name.ShortName
                ,-1
                ,-1
                ,new(wx) database_tree_item_data(name)
                );
            name_to_id[name.Idx] = id;
            }
        }

    // Force BestSize to be recalculated, since we have added new items
    tree_ctrl.InvalidateBestSize();
}

char const* DatabaseView::icon_xrc_resource() const
{
    return "database_view_icon";
}

char const* DatabaseView::menubar_xrc_resource() const
{
    return "database_view_menu";
}

DatabaseDocument& DatabaseView::document() const
{
    return safely_dereference_as<DatabaseDocument>(GetDocument());
}

DatabaseTableAdapter& DatabaseView::table_adapter()
{
    return const_cast<DatabaseTableAdapter&>
        (const_cast<DatabaseView const*>(this)->table_adapter()
        );
}

DatabaseTableAdapter const& DatabaseView::table_adapter() const
{
    LMI_ASSERT(table_adapter_);
    return *table_adapter_;
}

bool DatabaseView::IsModified() const
{
    return table_adapter().IsModified();
}

void DatabaseView::DiscardEdits()
{
    table_adapter().SetModified(false);
}

void DatabaseView::UponTreeSelectionChange(wxTreeEvent& event)
{
    wxTreeCtrl& tree_ctrl = tree();
    MultiDimGrid& grid_ctrl = grid();

    // save the current selection
    wxTreeItemId const old_item = event.GetOldItem();
    if(old_item.IsOk())
        {
        database_tree_item_data* old_item_data =
            dynamic_cast<database_tree_item_data*>
                (tree_ctrl.GetItemData(old_item)
                );
        if(old_item_data)
            {
            old_item_data->set_axes_selected(grid_ctrl.GetGridAxisSelection());
            }
        }

    database_tree_item_data* item_data =
        dynamic_cast<database_tree_item_data*>
            (tree_ctrl.GetItemData(event.GetItem())
            );
    if(!item_data)
        {
        return;
        }

    table_adapter().SetTDBValue
        (&document().GetTDBValue(item_data->db_name().Idx)
        );

    bool is_topic = tree_ctrl.GetChildrenCount(event.GetItem());

    set_grid_label_text(item_data->db_name().LongName);

    wxSizer* sizer = grid_ctrl.GetContainingSizer();
    LMI_ASSERT(sizer);
    sizer->Show(&grid_ctrl, !is_topic);
    sizer->Layout();

    // restore axis selection if any
    grid_ctrl.SetGridAxisSelection(item_data->get_axes_selected());
    grid_ctrl.RefreshTableFull();
}
