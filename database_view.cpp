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

// $Id: database_view.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "database_view.hpp"

#include "multidimgrid_safe.tpp"

#include "database_document.hpp"
#include "database_view_editor.hpp"
#include "dbnames.hpp"
#include "multidimgrid_any.hpp"
#include "multidimgrid_tools.hpp"
#include "wx_new.hpp"

#include <wx/window.h>

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
                ,new(wx) DatabaseTreeItemData(i, name.LongName)
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
    DatabaseTreeItemData* item_data = dynamic_cast<DatabaseTreeItemData*>
        (tree.GetItemData(event.GetItem())
        );

    if(item_data)
        {
        std::size_t index = item_data->GetId();

        table_adapter_->SetTDBValue(document().GetTDBValue(index));

        bool is_topic = tree.GetChildrenCount(event.GetItem());

        std::ostringstream oss;
        if(is_topic)
            oss << "[+] ";
        oss << item_data->GetDescription();
        SetLabel(oss.str());

        MultiDimGrid& grid = GetGridCtrl();

        grid.Enable(!is_topic);
        grid.RefreshTableFull();
        }
}
