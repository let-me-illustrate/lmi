// Database dictionary manager.
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

// $Id: database_view.cpp,v 1.10 2007-03-28 02:52:27 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database_view.hpp"

#include "database_document.hpp"
#include "database_view_editor.hpp"
#include "dbnames.hpp"
#include "multidimgrid_any.hpp"
#include "multidimgrid_safe.tpp"
#include "multidimgrid_tools.hpp"
#include "wx_new.hpp"

#include <wx/icon.h>
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
///  - Change the type of 'id_' to (enum) DatabaseNames.
///
/// However, shouldn't we rewrite this class completely instead?
///
/// Its purpose is to represent the information in struct db_names,
/// while deriving from class wxTreeItemData. Yet it contains only two
/// of that struct's four members--these:
///    DatabaseNames       Idx;
///    char const*         LongName;
/// but not these:
///    DatabaseNames       ParentIdx;
///    char const*         ShortName;
/// The fields not included are of course accessed by indexing a
/// db_names object from the vector returned by this function:
///   std::vector<db_names> const& LMI_SO GetDBNames();
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
/// (1) This class holds only a DatabaseNames enum. That's enough to
/// find the corresponding struct db_names in the vector returned by
///   std::vector<db_names> const& LMI_SO GetDBNames();
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
    database_tree_item_data(std::size_t id, std::string const& description);

    std::size_t id() const;
    std::string const& description() const;

  private:
    std::size_t id_;
    std::string description_;
};

database_tree_item_data::database_tree_item_data
    (std::size_t id
    ,std::string const& description
    )
    :wxTreeItemData()
    ,id_(id)
    ,description_(description)
{
}

std::size_t database_tree_item_data::id() const
{
    return id_;
}

std::string const& database_tree_item_data::description() const
{
    return description_;
}

} // Unnamed namespace.

IMPLEMENT_DYNAMIC_CLASS(DatabaseView, TreeGridViewBase)

BEGIN_EVENT_TABLE(DatabaseView, TreeGridViewBase)
    EVT_TREE_SEL_CHANGED(wxID_ANY, DatabaseView::UponTreeSelectionChange)
END_EVENT_TABLE()

DatabaseView::DatabaseView()
    :TreeGridViewBase()
    ,table_adapter_(new DatabaseTableAdapter())
{
}

DatabaseView::~DatabaseView()
{
}

// EVGENIY !! Here and anywhere else 'wxWindow* panel' occurs,
// does the name 'panel' suggest that the type should be wxPanel?

wxTreeCtrl* DatabaseView::CreateTreeCtrl(wxWindow* panel)
{
    return new(wx) AutoResizingTreeCtrl
        (panel
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT
        );
}

MultiDimGrid* DatabaseView::CreateGridCtrl(wxWindow* panel)
{
    return new(wx) DatabaseEditorGrid(panel, table_adapter_);
}

void DatabaseView::SetupControls()
{
    std::vector<db_names> const& names = GetDBNames();
    std::map<DatabaseNames, wxTreeItemId> name_to_id;

    wxTreeCtrl& tree = GetTreeCtrl();

    for(std::size_t i = 0; i < names.size(); ++i)
        {
        db_names const& name = names[i];
        if(name.Idx == name.ParentIdx)
            {
            wxTreeItemId id = tree.AddRoot("");
            name_to_id[name.Idx] = id;
            }
        else
            {
            wxTreeItemId parent = name_to_id[name.ParentIdx];
            wxTreeItemId id = tree.AppendItem
                (parent
                ,name.ShortName
                ,-1
                ,-1
                ,new(wx) database_tree_item_data(i, name.LongName)
                );
            name_to_id[name.Idx] = id;
            }
        }

    // Force BestSize to be recalculated, since we have added new items
    tree.InvalidateBestSize();
}

wxIcon DatabaseView::Icon() const
{
    return IconFromXmlResource("database_view_icon");
}

wxMenuBar* DatabaseView::MenuBar() const
{
    return MenuBarFromXmlResource("database_view_menu");
}

DatabaseDocument& DatabaseView::document() const
{
    return dynamic_cast<DatabaseDocument&>(*GetDocument());
}

bool DatabaseView::IsModified() const
{
    return table_adapter_->IsModified();
}

void DatabaseView::DiscardEdits()
{
    table_adapter_->SetModified(false);
}

void DatabaseView::UponTreeSelectionChange(wxTreeEvent& event)
{
    wxTreeCtrl& tree = GetTreeCtrl();
    database_tree_item_data* item_data = dynamic_cast<database_tree_item_data*>
        (tree.GetItemData(event.GetItem())
        );
    if(!item_data)
        {return;}

    std::size_t index = item_data->id();

    table_adapter_->SetTDBValue(document().GetTDBValue(index));

    bool is_topic = tree.GetChildrenCount(event.GetItem());

    SetLabel(item_data->description());

    MultiDimGrid& grid = GetGridCtrl();

    grid.Enable(!is_topic);
    grid.RefreshTableFull();
}
