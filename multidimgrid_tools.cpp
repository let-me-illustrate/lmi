// Support classes for editing multidimensional data.
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

#include "multidimgrid_tools.hpp"

#include <wx/wupdlock.h>

/// AxisMaxBoundAdjusterBase implementation
/// ---------------------------------------

BEGIN_EVENT_TABLE(AxisMaxBoundAdjusterBase, wxChoice)
    EVT_CHOICE(wxID_ANY, AxisMaxBoundAdjusterBase::UponChange)
END_EVENT_TABLE()

AxisMaxBoundAdjusterBase::AxisMaxBoundAdjusterBase(MultiDimGrid& grid)
    :wxChoice
        (&grid
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxArrayString()
        )
{
}

void AxisMaxBoundAdjusterBase::UponChange(wxCommandEvent&)
{
    DoUponChange();
}

MultiDimGrid& AxisMaxBoundAdjusterBase::GetGrid()
{
    return *static_cast<MultiDimGrid*>(GetParent());
}

MultiDimGrid const& AxisMaxBoundAdjusterBase::GetGrid() const
{
    return *static_cast<MultiDimGrid const*>(GetParent());
}

/// AutoResizingTreeCtrl implementation
/// -----------------------------------

AutoResizingTreeCtrl::AutoResizingTreeCtrl
    (wxWindow* parent
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    ,long int style
    ,wxValidator const& validator
    )
    :wxTreeCtrl(parent, id, pos, size, style, validator)
{
}

AutoResizingTreeCtrl::~AutoResizingTreeCtrl() = default;

wxSize AutoResizingTreeCtrl::DoGetBestSize() const
{
    AutoResizingTreeCtrl& myself = const_cast<AutoResizingTreeCtrl&>(*this);
    wxWindowUpdateLocker update_locker(&myself);

    wxSize best_size(0, 0);

    wxTreeItemId const selection = GetSelection();
    wxTreeItemId const first_visible = GetFirstVisibleItem();

    wxTreeItemId const root = GetRootItem();
    myself.DoGetBestSizePrivate(best_size, root, true);

    // need some minimal size even for an empty tree
    if(best_size.x == 0 || best_size.y == 0)
        {
        wxSize min_size = wxTreeCtrl::DoGetBestSize();

        if(best_size.x == 0)
            {
            best_size.x = min_size.x;
            }
        if(best_size.y == 0)
            {
            best_size.y = min_size.y;
            }
        }
    best_size += GetSize() - GetClientSize();

    if(selection.IsOk())
        {
        myself.SelectItem(selection);
        }
    if(first_visible.IsOk())
        {
        myself.ScrollTo(first_visible);
        }

    CacheBestSize(best_size);

    return best_size;
}

void AutoResizingTreeCtrl::DoGetBestSizePrivate
    (wxSize& best_size
    ,wxTreeItemId const& node
    ,bool is_root
    )
{
    // This function recursively expands all the nodes in the tree and calculates
    // widest bounding rectangle width. It then collapses back originally
    // collapsed nodes and returns.
    if(!is_root)
        {
        wxRect rect;
        if(GetBoundingRect(node, rect, true))
            {
            // adjust width only. height does not matter
            best_size.x = std::max(best_size.x, rect.x + rect.width);
            }
        }

    if(node.IsOk() && GetChildrenCount(node))
        {
        bool originally_expanded = is_root || IsExpanded(node);
        if(!originally_expanded)
            {
            Expand(node);
            }

        wxTreeItemIdValue cookie;
        for
            (wxTreeItemId child = GetFirstChild(node, cookie)
            ;child.IsOk()
            ;child = GetNextChild(node, cookie)
            )
            {
            DoGetBestSizePrivate(best_size, child);
            }

        if(!originally_expanded)
            {
            Collapse(node);
            }
        }
}
