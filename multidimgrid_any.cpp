// A control for editing multidimensional data and supporting classes.
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

#include "multidimgrid_any.hpp"

#include "alert.hpp"
#include "bourn_cast.hpp"
#include "wx_new.hpp"
#include "wx_workarounds.hpp"           // wx[GS]et.*groundColor()

#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/gbsizer.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/window.h>
#include <wx/wupdlock.h>

#include <algorithm>
#include <sstream>
#include <utility>                      // swap()

/// MultiDimAxisAny functions implementation
/// --------------------------------------

wxWindow* MultiDimAxisAny::CreateAdjustControl
    (MultiDimGrid&     // unused: grid
    ,MultiDimTableAny& // unused: table
    )
{
    return nullptr;
}

bool MultiDimAxisAny::ApplyAdjustment(wxWindow&, unsigned int)
{
    return false;
}

bool MultiDimAxisAny::RefreshAdjustment(wxWindow&, unsigned int)
{
    return false;
}

MultiDimAxisAnyChoice* MultiDimAxisAny::CreateChoiceControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& // unused: table
    )
{
    return new(wx) MultiDimAxisAnyChoice(*this, grid);
}

void MultiDimAxisAny::UpdateChoiceControl(MultiDimAxisAnyChoice& choice) const
{
    choice.PopulateChoiceList();
}

/// MultiDimTableAny functions implementation
/// ---------------------------------------

bool MultiDimTableAny::DoApplyAxisAdjustment
    (MultiDimAxisAny& // unused: axis
    ,unsigned int     // unused: n
    )
{
    return false;
}

bool MultiDimTableAny::DoRefreshAxisAdjustment
    (MultiDimAxisAny& // unused: axis
    ,unsigned int     // unused: n
    )
{
    return false;
}

#include <iostream>

namespace
{
/// MultiDimGridGrid is a customised version of wxGrid
///
/// It is used by MultiDimGrid.
///   - Widget size could be made as small as possible leaving only labels,
///     one row and one column visible.

class MultiDimGridGrid
  :public wxGrid
{
  public:
    MultiDimGridGrid
        (wxWindow*
        ,wxWindowID
        ,wxPoint const& = wxDefaultPosition
        ,wxSize const& = wxDefaultSize
        ,long int style = 0 // no wxWANTS_CHARS, as is wxGrid's default
        ,std::string const& name = wxPanelNameStr
        );
    ~MultiDimGridGrid() override = default;

    wxSize DoGetBestSize() const override;

  private:
    void UponKeyDown(wxKeyEvent& event);

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MultiDimGridGrid, wxGrid)
    EVT_KEY_DOWN (MultiDimGridGrid::UponKeyDown)
END_EVENT_TABLE()

inline MultiDimGridGrid::MultiDimGridGrid
    (wxWindow* parent
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    ,long int style
    ,std::string const& name
    )
    :wxGrid(parent, id, pos, size, style, name)
{
}

wxSize MultiDimGridGrid::DoGetBestSize() const
{
    // 100 is the default minimal size of a widget - taken from wx headers.
    int width  = std::max(GetRowLabelSize() + GetColSize(0), 100);
    int height = std::max(GetColLabelSize() + GetRowSize(0), 100);

    // The following is from the wx header file for wxScrollWindow:
    // Round up to a multiple the scroll rate NOTE: this still doesn't get rid
    // of the scrollbars, is there any magic incantaion for that?
    int xpu, ypu;
    GetScrollPixelsPerUnit(&xpu, &ypu);
    if(xpu)
        {
        width  += 1 + xpu - (width  % xpu);
        }
    if(ypu)
        {
        height += 1 + ypu - (height % ypu);
        }

    return wxSize(width, height);
}

void MultiDimGridGrid::UponKeyDown(wxKeyEvent& event)
{
    // wxGrid uses TAB to navigate between cells, but we prefer it to
    // change focus out of the grid control instead.
    //
    // Even though we don't pass wxWANTS_CHARS to wxGrid's ctor to prevent TAB
    // key down events from being sent to the window under wxMSW, they are
    // still sent by other wx ports, so we still have to override wxGrid's
    // default behavior.

    // WX !! Use wx-2.9's wxWindow::HandleHandleKeyEventForNavigation()
    if(event.GetKeyCode() == WXK_TAB)
        {
        int dir = event.ShiftDown()
            ? wxNavigationKeyEvent::IsBackward
            : wxNavigationKeyEvent::IsForward
            ;
        Navigate(dir | wxNavigationKeyEvent::FromTab);
        }
    else
        {
        event.Skip();
        }
}
} // Unnamed namespace.

/// GridRefreshTableDataGuard: Table refresh guard class
///
/// This class prevents unneeded redrawing of the table to take place.
/// Under the hood it counts the number of contexts calling Table::Update
/// function. It calls for Table::DoUpdate only when that counter drops to zero,
/// which means that we are exiting the outermost context which called for
/// an update and it is the place to do it once for all the calls.

class GridRefreshTableDataGuard
{
  public:
    /// Construct guard for the counter, and use releaser at last exit
    GridRefreshTableDataGuard(MultiDimGrid&);
    ~GridRefreshTableDataGuard();

  private:
    GridRefreshTableDataGuard(GridRefreshTableDataGuard const&) = delete;
    GridRefreshTableDataGuard& operator=(GridRefreshTableDataGuard const&) = delete;

    MultiDimGrid& grid_;
};

inline GridRefreshTableDataGuard::GridRefreshTableDataGuard(MultiDimGrid& grid)
    :grid_ {grid}
{
    ++grid_.table_data_refresh_counter_;
}

inline GridRefreshTableDataGuard::~GridRefreshTableDataGuard()
{
    if(!--grid_.table_data_refresh_counter_)
        {
        grid_.DoRefreshTableData();
        }
}

/// MultiDimGrid functions implementation
/// -----------------------------------

BEGIN_EVENT_TABLE(MultiDimGrid, wxPanel)
    EVT_CHOICE   (MultiDimGrid::e_axis_x ,MultiDimGrid::UponSwitchSelectedAxis)
    EVT_CHOICE   (MultiDimGrid::e_axis_y ,MultiDimGrid::UponSwitchSelectedAxis)
    EVT_CHECKBOX (wxID_ANY               ,MultiDimGrid::UponAxisVariesToggle  )
END_EVENT_TABLE()

/// Some constants to describe various element positions
enum
    {MDGRID_AXIS_X_ROW = 0         // the row for the X axis selection
    ,MDGRID_AXIS_Y_ROW = 1         // the row for the Y axis selection
    ,MDGRID_AXIS_SEPARATOR_ROW = 2 // the row separating axis selections from axis controls
    ,MDGRID_AXIS_ROW = 3           // first row for the axis controls
    ,MDGRID_LABEL_COL = 0          // column for the labels
    ,MDGRID_CHOICE_COL = 1         // column for the axis choice controls or axis selection dropdown
    ,MDGRID_CHOICE_HSPAN = 1       // horizontal span for the axis choice controls or axis selection dropdown
    ,MDGRID_ADJUST_COL = 0         // column for the axis adjustment controls
    ,MDGRID_ADJUST_HSPAN = 3       // horizontal span for the axis adjustment controls
    ,MDGRID_VARIES_COL = 2         // column for checkbox controlling product dependency on the axis
    ,MDGRID_SIZER_VGAP = 4         // sizer vertical cell spacing
    ,MDGRID_SIZER_HGAP = 8         // sizer horizontal cell spacing
    };

namespace
{
/// Helper functions for an axis row calculation
inline unsigned int MDGridGetAxisLabelRow(unsigned int n)
{
    return MDGRID_AXIS_ROW + n * 2;
}
inline unsigned int MDGridGetAxisChoiceRow(unsigned int n)
{
    return MDGRID_AXIS_ROW + n * 2;
}
inline unsigned int MDGridGetAxisVariesRow(unsigned int n)
{
    return MDGRID_AXIS_ROW + n * 2;
}
inline unsigned int MDGridGetAxisAdjustRow(unsigned int n)
{
    return MDGRID_AXIS_ROW + n * 2 + 1;
}
} // Unnamed namespace.

void MultiDimGrid::Init()
{
    first_grid_axis_ = wxNOT_FOUND;
    second_grid_axis_ = wxNOT_FOUND;

    // setting default color to both X and Y axis
    selected_first_color_ = GetForegroundColor();
    selected_second_color_ = GetForegroundColor();

    // intialize refresh counter
    table_data_refresh_counter_ = 0;

    // These pointers are used in IsFullyConstructed.
    first_axis_choice_ = nullptr;
    second_axis_choice_ = nullptr;
}

MultiDimGrid::~MultiDimGrid()
{
    // If we don't set grid() table to nullptr, then a crash might occur.
    // The reason is that MultiDimGrid has multiple base classes:
    // wxGridTableBase and wxPanel
    // By the time the wxPanel destructor is called
    // the wxGridTableBase base of this object might already be destroyed
    // which is really bad, because the grid() object still has a pointer
    // to (wxGridTableBase*)this and it might call it, which would result in
    // an undefined behavior.
    // The alternative is to rearrange the order of base classes of MultiDimGrid
    // so that wxGridTableBase part is destroyed _after_ wxPanel base.
    // But it is not a good idea to depend on the base classes order,
    // especially when a change would lead to non-obvious crashes.
    grid().SetTable(nullptr);
}

bool MultiDimGrid::Create
    (wxWindow* parent
    ,std::shared_ptr<MultiDimTableAny> const& atable
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
{
    // setting self as a wxPanel
    wxPanel::Create(parent, id, pos, size);

    // setting internal variables
    table_ = atable;
    if(!table_)
        {
        alarum() << "Table cannot be null." << LMI_FLUSH;
        }
    dimension_ = table().GetDimension();

    // postpone the table data refresh until we exit Create() function
    // being sure that we are ready for a refresh
    GridRefreshTableDataGuard guard(*this);

    axis_ = table().GetAxesAny();
    if(dimension_ != axis_.size())
        {
        alarum() << "Table size does not match number of axes." << LMI_FLUSH;
        }

    // initializing fixed values with empty ones
    axis_fixed_values_.resize(dimension_);
    axis_fixed_coords_.resize(dimension_);

    // reserve space in axis choice control containers of the widget
    axis_labels_.resize(dimension_);
    axis_choice_wins_.resize(dimension_);
    axis_adjust_wins_.resize(dimension_);
    axis_varies_checkboxes_.resize(dimension_);

    wxStaticBoxSizer* const sizer =
        new(wx) wxStaticBoxSizer(wxHORIZONTAL, this, "Axis");

    // wxGridBagSizer(vgap, hgap)
    axis_sizer_ = new(wx) wxGridBagSizer(MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP);

    // space between X and Y axis selection controls and axis controls
    axis_sizer_->SetEmptyCellSize(wxSize(MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP));

    // create and add axis choice controls
    first_axis_choice_ = CreateGridAxisSelection
        (e_axis_x
        ,"X axis"
        ,selected_first_color_
        );
    second_axis_choice_ = CreateGridAxisSelection
        (e_axis_y
        ,"Y axis"
        ,selected_second_color_
        );

    for(unsigned int i = 0; i < dimension_; ++i)
        {
        SetAxisLabel
            (i
            ,*new(wx) wxStaticText(this, wxID_ANY, axis_[i]->GetName())
            );
        SetAxisChoiceControl
            (i
            ,*axis_[i]->CreateChoiceControl(*this, table())
            );
        SetAxisVariesControl(i);
        SetAxisAdjustControl(i, axis_[i]->CreateAdjustControl(*this, table()));
        }

    sizer->Add
        (axis_sizer_
        ,wxSizerFlags()
            .Expand()
            .Proportion(1)
            .Border(wxRIGHT | wxLEFT, MDGRID_SIZER_HGAP)
        );

    // Data table grid component
    grid_ = new(wx) MultiDimGridGrid(this, wxID_ANY, wxDefaultPosition);
    grid().SetTable(this, false);

    // main sizer contains axis controls in the left part and the grid in the right
    wxBoxSizer* const grid_sizer = new(wx) wxBoxSizer(wxHORIZONTAL);
    grid_sizer->Add
        (sizer
        ,wxSizerFlags()
            .Expand()
            .Border(wxLEFT | wxRIGHT, MDGRID_SIZER_HGAP)
        );
    grid_sizer->Add
        (grid_
        ,wxSizerFlags()
            .Proportion(1)
            .Expand()
            .Border(wxALL, 1)
        );

    SetSizer(grid_sizer);

    RefreshTableFull();

    Layout();

    return true;
}

bool MultiDimGrid::IsFullyConstructed() const
{
    return first_axis_choice_ && second_axis_choice_;
}

MultiDimTableAny& MultiDimGrid::table() const
{
    LMI_ASSERT(table_);
    return *table_;
}

wxGrid& MultiDimGrid::grid() const
{
    LMI_ASSERT(grid_);
    return *grid_;
}

void MultiDimGrid::FixAxisValue
    (std::string const& axisName
    ,std::any    const& value
    )
{
    int sel = GetAxisIndexByName(axisName);
    if(sel != wxNOT_FOUND)
        {
        axis_fixed_values_[sel] = value;
        axis_fixed_coords_[sel] = value;

        RefreshTableData();
        }
}

void MultiDimGrid::RefreshTableData()
{
    /// this will call DoRefreshTableData()
    /// when the last refresh schedule expires
    GridRefreshTableDataGuard guard(*this);
}

void MultiDimGrid::DoRefreshTableData()
{
    wxWindowUpdateLocker update_locker(this);
    grid().SetTable(grid().GetTable(), false);

    // Automatically adjust the width of the column of row labels.
    //
    // Note that there's no point in doing the same for columns using
    // SetColLabelSize(), because that would only affect _height_ of
    // columns labels, but we need to adjust their width.
    grid().SetRowLabelSize(wxGRID_AUTOSIZE);

    // Adjust size of the data columns so that both the label and the
    // data fit in it. At the same time, we want to keep some sensible
    // minimal width so that columns with short labels (e.g. states, age)
    // aren't too narrow.
    int const cols = GetNumberCols();
    for(int i = 0; i < cols; ++i)
        {
            // set some minimal width for aesthetic reasons (otherwise
            // columns could be very narrow for axes like age or state):
            grid().SetColMinimalWidth(i, WXGRID_DEFAULT_COL_WIDTH);

            grid().AutoSizeColumn(i, false); // false: setAsMin
        }

    grid().ForceRefresh();
}

bool MultiDimGrid::RefreshTableAxis()
{
    // Refresh table data only once
    GridRefreshTableDataGuard guard(*this);
    bool updated = false;
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        // the order is important, otherwise it could optimize off
        // the call to DoRefreshTableAxis
        if(DoRefreshTableAxis(i))
            {
            updated = true;
            }
        }

    // TODO ?? this function should check for changes in the table
    // data structure (number of axis, types of axis), and refresh accordingly
    if(AutoselectGridAxis())
        {
        updated = true;
        }

    return updated;
}

bool MultiDimGrid::RefreshTableFull()
{
    wxWindowUpdateLocker update_locker(this);
    bool updated = RefreshTableAxis();
    RefreshTableData();
    return updated;
}

bool MultiDimGrid::AutoselectGridAxis()
{
    bool updated = false;
    first_axis_choice_ ->Show(0 < dimension_);
    second_axis_choice_->Show(1 < dimension_);

    if(first_grid_axis_ == wxNOT_FOUND || second_grid_axis_ == wxNOT_FOUND)
        {
        std::pair<int,int> gridSelection = SuggestGridAxisSelection();

        if  (  gridSelection.first != first_grid_axis_
            || gridSelection.second != second_grid_axis_
            )
            {
            updated = true;
            }
        SetGridAxisSelection(gridSelection.first, gridSelection.second);
        }
    return updated;
}

std::pair<int,int> MultiDimGrid::SuggestGridAxisSelection() const
{
    int newFirst = first_grid_axis_;
    int newSecond = second_grid_axis_;

    bool const canReorder = newFirst == wxNOT_FOUND || newSecond == wxNOT_FOUND;

    for(unsigned int i = 0; i < dimension_; ++i)
        {
        if(table().VariesByDimension(i))
            {
            if(newFirst == wxNOT_FOUND)
                {
                if(static_cast<int>(i) != newSecond)
                    {
                    newFirst = static_cast<int>(i);
                    if(newSecond != wxNOT_FOUND)
                        {
                        break;
                        }
                    }
                }
            else if(newSecond == wxNOT_FOUND)
                {
                if(static_cast<int>(i) != newFirst)
                    {
                    newSecond = static_cast<int>(i);
                    break;
                    }
                }
            }
        }

    if(canReorder)
        {
        // make sure the dimension with higher cardinality maps to the
        // Y axis of grid, so that the grid doesn't scroll horizontally
        // (or at least doesn't scroll too much):
        if(newFirst != wxNOT_FOUND && newSecond != wxNOT_FOUND)
            {
            if(axis_[newFirst]->GetCardinality() >
               axis_[newSecond]->GetCardinality())
                {
                std::swap(newFirst, newSecond);
                }
            }
        else if(newFirst != wxNOT_FOUND) // && newSecond == wxNOT_FOUND
            {
            // use Y axis by default
            std::swap(newFirst, newSecond);
            }
        }

    return std::make_pair(newFirst, newSecond);
}

bool MultiDimGrid::DoRefreshTableAxis(unsigned int n)
{
    // Refresh table data only once
    GridRefreshTableDataGuard guard(*this);
    bool updated = false;
    if(DoRefreshAxisVaries(n))
        {
        updated = true;
        }
    if(DoRefreshAxisAdjustment(n))
        {
        updated = true;
        }
    return updated;
}

bool MultiDimGrid::DoRefreshAxisVaries(unsigned int axis_id)
{
    bool updated = false;

    bool varies = table().VariesByDimension(axis_id);
    bool canChange = table().CanChangeVariationWith(axis_id);

    wxCheckBox* box = axis_varies_checkboxes_[axis_id];
    if(box)
        {
        if(varies != box->GetValue())
            {
            box->SetValue(varies);
            updated = true;
            }

        if(canChange != box->Enable())
            {
            updated = true;
            }
        box->Show(canChange || !varies);
    }

    wxWindow* win = axis_choice_wins_[axis_id];
    if(win)
        {
        win->Show(varies);
        }

    win = axis_adjust_wins_[axis_id];
    if(win)
        {
        win->Show(varies);
        }

    if(updated)
        {
        PopulateGridAxisSelection();
        DoSetGridAxisSelection();
        if(varies)
            {
            AutoselectGridAxis();
            }
        }

    return updated;
}

bool MultiDimGrid::DoRefreshAxisAdjustment(unsigned int n)
{
    if(!table().VariesByDimension(n))
        {
        return false;
        }

    MultiDimAxisAny& axis = *axis_[n];
    wxWindow* const adjust_window = axis_adjust_wins_[n];

    bool updated = false;
    if(table().RefreshAxisAdjustment(axis, n))
        {
        updated = true;
        }

    if(adjust_window && axis.RefreshAdjustment(*adjust_window, n))
        {
        updated = true;
        }

    if(updated)
        {
        MultiDimAxisAnyChoice* const choice_window = axis_choice_wins_[n];
        if(choice_window)
            {
            axis.UpdateChoiceControl(*choice_window);
            }
        RefreshTableData();
        }
    return updated;
}

bool MultiDimGrid::DoApplyAxisAdjustment(unsigned int n)
{
    if(!table().VariesByDimension(n))
        {
        return false;
        }

    MultiDimAxisAny& axis = *axis_[n];
    wxWindow* const adjust_window = axis_adjust_wins_[n];

    bool updated = false;

    if(adjust_window && axis.ApplyAdjustment(*adjust_window, n))
        {
        updated = true;
        }

    if(table().ApplyAxisAdjustment(axis, n))
        {
        updated = true;
        }

    if(updated)
        {
        MultiDimAxisAnyChoice* const choice_window = axis_choice_wins_[n];
        if(choice_window)
            {
            axis.UpdateChoiceControl(*choice_window);
            }
        RefreshTableData();
        }
    return updated;
}

void MultiDimGrid::SetXAxisColor(wxColor const& color)
{
    selected_first_color_ = color;
    first_axis_choice_->SetForegroundColor(color);
    // WX !! In the future wx releases wxGrid might add support
    // for label coloring.
    // grid().SetColLabelColor(color);

    // Update select axis labels
    DoSetGridAxisSelection();
}

void MultiDimGrid::SetYAxisColor(wxColor const& color)
{
    selected_second_color_ = color;
    second_axis_choice_->SetForegroundColor(color);
    // WX !! In the future wx releases wxGrid might add support
    // for label coloring.
    // grid().SetRowLabelColor(color);

    // Update select axis labels
    DoSetGridAxisSelection();
}

bool MultiDimGrid::SetGridAxisSelection
    (std::pair<int,int> const& selection
    )
{
    return SetGridAxisSelection(selection.first, selection.second);
}

std::pair<int,int> MultiDimGrid::GetGridAxisSelection() const
{
    return std::make_pair(first_grid_axis_, second_grid_axis_);
}

wxChoice* MultiDimGrid::CreateGridAxisSelection
    (enum_axis_x_or_y x_or_y
    ,std::string const& label
    ,wxColor const& selected_color
    )
{
    // wxChoice will grow to its default size if all choice strings are empty,
    // therefore we don't pass the empty string ("") but a space instead (" ")
    wxString only_empty_choice = " ";

    wxChoice* const win = new(wx) wxChoice
        (this
        ,x_or_y
        ,wxDefaultPosition
        ,wxDefaultSize
        ,1
        ,&only_empty_choice
        );

    win->SetOwnForegroundColor(selected_color);

    unsigned int row =
          (x_or_y == e_axis_x)
        ? MDGRID_AXIS_X_ROW
        : MDGRID_AXIS_Y_ROW
        ;
    axis_sizer_->Add
        (new(wx) wxStaticText(this, wxID_ANY, label)
        ,wxGBPosition(row, MDGRID_LABEL_COL)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags()
            .Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL)
            .Border(wxRIGHT, 16)
            .GetFlags()
        );

    axis_sizer_->Add
        (win
        ,wxGBPosition(row, MDGRID_CHOICE_COL)
        ,wxGBSpan(1, MDGRID_CHOICE_HSPAN)
        ,wxSizerFlags()
            .Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL)
            .Border(wxALL, 1)
            .Expand()
            .GetFlags()
        );
    return win;
}

void MultiDimGrid::SetAxisLabel(int axis_id, wxWindow& window)
{
    axis_labels_[axis_id] = &window;
    axis_sizer_->Add
        (&window
        ,wxGBPosition(MDGridGetAxisLabelRow(axis_id), MDGRID_LABEL_COL)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags()
            .Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL)
            .Border(wxRIGHT, 16)
            .GetFlags()
        );
}

void
MultiDimGrid::SetAxisChoiceControl(int axis_id, MultiDimAxisAnyChoice& window)
{
    axis_choice_wins_[axis_id] = &window;
    axis_sizer_->Add
        (&window
        ,wxGBPosition(MDGridGetAxisChoiceRow(axis_id), MDGRID_CHOICE_COL)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags()
            .Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL)
            .Border(wxALL, 1)
            .Expand()
            .GetFlags()
        );
}

void MultiDimGrid::SetAxisAdjustControl(int axis_id, wxWindow* window)
{
    axis_adjust_wins_[axis_id] = window;
    if(window)
        {
        axis_sizer_->Add
            (window
            ,wxGBPosition(MDGridGetAxisAdjustRow(axis_id), MDGRID_ADJUST_COL)
            ,wxGBSpan(1, MDGRID_ADJUST_HSPAN)
            ,wxSizerFlags()
                .Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL)
                .Border(wxALL, 1)
                .GetFlags()
            );
        }
}

void MultiDimGrid::SetAxisVariesControl(unsigned int axis_id)
{
    wxCheckBox* const win = new(wx) wxCheckBox(this, wxID_ANY, "");
    axis_sizer_->Add
        (win
        ,wxGBPosition(MDGridGetAxisVariesRow(axis_id), MDGRID_VARIES_COL)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags()
            .Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL)
            .Border(wxALL, 1)
            .GetFlags()
        );
    axis_varies_checkboxes_[axis_id] = win;
}

bool MultiDimGrid::SetGridAxisSelection(int firstAxis, int secondAxis)
{
    if(firstAxis == secondAxis && firstAxis != wxNOT_FOUND)
        {
        alarum() << "Select different axes." << LMI_FLUSH;
        return false;
        }
    bool update =
            first_grid_axis_  != firstAxis
        ||  second_grid_axis_ != secondAxis
        ;
    first_grid_axis_ = firstAxis;
    second_grid_axis_ = secondAxis;
    DoSetGridAxisSelection();
    return update;
}

int MultiDimGrid::GetGridAxisSelection(enum_axis_x_or_y x_or_y)
{
    if(!IsFullyConstructed())
        {
        // function is called when the control is not yet fully constructed
        return wxNOT_FOUND;
        }
    wxChoice& choice = GetAxisChoiceControl(x_or_y);
    int oldSel = choice.GetSelection();
    // wxNOT_FOUND - nothing is selected
    // 0 - first empty item (" ") is selected => same as no selection at all
    if(oldSel == wxNOT_FOUND || oldSel == 0)
        {
        return wxNOT_FOUND;
        }

    return bourn_cast<int>(wxPtrToUInt(choice.GetClientData(oldSel)));
}

void MultiDimGrid::DoSetGridAxisSelection(enum_axis_x_or_y x_or_y, int axis)
{
    if(axis == GetGridAxisSelection(x_or_y))
        {
        return;
        }

    wxChoice& choice = GetAxisChoiceControl(x_or_y);

    int previous_axis = GetGridAxisSelection(x_or_y);

    if(axis != previous_axis)
        {
        for(unsigned int i = 1; i < choice.GetCount(); ++i)
            {
            int cdata = bourn_cast<int>(wxPtrToUInt(choice.GetClientData(i)));
            if(cdata == axis)
                {
                choice.SetSelection(i);
                return;
                }
            }
        choice.SetSelection(0);
        RefreshTableData();
        }
}

wxChoice& MultiDimGrid::GetAxisChoiceControl(enum_axis_x_or_y x_or_y)
{
    return (x_or_y == e_axis_x)
        ? *first_axis_choice_
        : *second_axis_choice_
        ;
}

void MultiDimGrid::DoSetGridAxisSelection()
{
    DoSetGridAxisSelection(e_axis_x, first_grid_axis_);
    DoSetGridAxisSelection(e_axis_y, second_grid_axis_);

    axis_fixed_coords_ = axis_fixed_values_;
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        bool selected =
               (static_cast<int>(i) == first_grid_axis_)
            || (static_cast<int>(i) == second_grid_axis_)
            ;

        // different color to the selected axis
        wxColor color = GetForegroundColor();
        if(selected)
            {
            color = (static_cast<int>(i) == first_grid_axis_)
                ? selected_first_color_
                : selected_second_color_
                ;
            }
        axis_labels_[i]->SetOwnForegroundColor(color);

        // text does not get repainted after font and color changes, force it
        axis_labels_[i]->Refresh();

        // disable selected window choice control
        wxWindow* choiceWin = axis_choice_wins_[i];
        if(choiceWin)
            {
            choiceWin->Enable(!selected && table().VariesByDimension(i));
            }
        }
    RefreshTableData();
}

void MultiDimGrid::PopulateGridAxisSelection()
{
    PopulateGridAxisSelection(e_axis_x);
    PopulateGridAxisSelection(e_axis_y);
    Layout();
}

void MultiDimGrid::PopulateGridAxisSelection(enum_axis_x_or_y x_or_y)
{
    if(!IsFullyConstructed())
        {
        return;
        }

    wxChoice& choice = GetAxisChoiceControl(x_or_y);
    wxWindowUpdateLocker update_locker(&choice);

    int old_selection = GetGridAxisSelection(x_or_y);
    // set selection to empty line so that it does not change
    choice.SetSelection(0);
    int new_sel_index = wxNOT_FOUND;

    // remove every item, except the empty one (the first " ")
    for(int j = choice.GetCount() - 1; 1 <= j; --j)
        {
        choice.Delete(j);
        }

    // repopulate the drop-down list with axis names
    std::size_t const size = axis_.size();
    for(unsigned int i = 0; i < size; ++i)
        {
        if(table().VariesByDimension(i))
            {
            choice.Append
                (axis_[i]->GetName()
                ,reinterpret_cast<void*>(i)
                );
            if(static_cast<int>(i) == old_selection)
                {
                new_sel_index = choice.GetCount() - 1;
                }
            }
        }

    if(new_sel_index != wxNOT_FOUND)
        {
        choice.SetSelection(new_sel_index);
        }

    if(old_selection != GetGridAxisSelection(x_or_y))
        {
        DoOnSwitchSelectedAxis(x_or_y);
        }
}

int MultiDimGrid::GetAxisIndexByName(std::string const& axisName)
{
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        if(axis_[i]->GetName() == axisName)
            {
            return i;
            }
        }
    return wxNOT_FOUND;
}

// EVGENIY !! Given that we check array bounds here, shouldn't all
// occurrences of '*axis_[n]' be replaced by calls to this function?
// Alternatively, we could just replace '/[n]/.at(n)/'. (Originally,
// you had written an explicit test against the upper bound, but I've
// replaced that here with std::vector::at() calls.)

MultiDimAxisAny const& MultiDimGrid::GetAxis(unsigned int n) const
{
    return *axis_.at(n);
}

MultiDimAxisAny& MultiDimGrid::GetAxis(unsigned int n)
{
    return *axis_.at(n);
}

int MultiDimGrid::GetNumberRows()
{
    return DoGetNumberRows();
}

unsigned int MultiDimGrid::DoGetNumberRows() const
{
    if(second_grid_axis_ == wxNOT_FOUND)
        {
        return 1;
        }
    return axis_[second_grid_axis_]->GetCardinality();
}

int MultiDimGrid::GetNumberCols()
{
    return DoGetNumberCols();
}

unsigned int MultiDimGrid::DoGetNumberCols() const
{
    if(first_grid_axis_ == wxNOT_FOUND)
        {
        return 1;
        }
    return axis_[first_grid_axis_]->GetCardinality();
}

bool MultiDimGrid::IsEmptyCell
    (int // unused: row
    ,int // unused: col
    )
{
    return false;
}

MultiDimGrid::Coords const& MultiDimGrid::PrepareFixedCoords(int row, int col) const
{
    if(first_grid_axis_ != wxNOT_FOUND)
        {
        axis_fixed_coords_[first_grid_axis_] =
            axis_[first_grid_axis_]->GetValue(col);
        }
    else
        {
        if(col != 0)
            {
            alarum() << "No first grid axis selected." << LMI_FLUSH;
            }
        }

    if(second_grid_axis_ != wxNOT_FOUND)
        {
        axis_fixed_coords_[second_grid_axis_] =
                    axis_[second_grid_axis_]->GetValue(row);
        }
    else
        {
        if(row != 0)
            {
            alarum() << "No second grid axis selected." << LMI_FLUSH;
            }
        }
    return axis_fixed_coords_;
}

wxString MultiDimGrid::GetValue(int row, int col)
{
    try
        {
        return DoGetValue
            (EnsureIndexIsPositive(row)
            ,EnsureIndexIsPositive(col)
            );
        }
    catch(std::exception const& e)
        {
        warning()
            << "Error getting value: "
            << e.what()
            << LMI_FLUSH
            ;
        return "error";
        }
}

std::string MultiDimGrid::DoGetValue(unsigned int row, unsigned int col) const
{
    std::any value = table().GetValueAny(PrepareFixedCoords(row, col));
    return table().ValueToString(value);
}

void MultiDimGrid::SetValue(int row, int col, wxString const& value)
{
    try
        {
        DoSetValue
            (EnsureIndexIsPositive(row)
            ,EnsureIndexIsPositive(col)
            ,value.ToStdString(wxConvUTF8)
            );
        }
    catch(std::exception const& e)
        {
        std::string title;

        warning()
            << "Error setting value: "
            << e.what()
            << LMI_FLUSH
            ;
        }
}

void MultiDimGrid::DoSetValue
    (unsigned int row
    ,unsigned int col
    ,std::string const& value
    )
{
    table().SetValueAny
        (PrepareFixedCoords(row, col)
        ,table().StringToValue(std::string(value))
        );
}

wxString MultiDimGrid::GetRowLabelValue(int row)
{
    return DoGetRowLabelValue(EnsureIndexIsPositive(row));
}

std::string MultiDimGrid::DoGetRowLabelValue(unsigned int row) const
{
    if(second_grid_axis_ != wxNOT_FOUND)
        {
        if(row < axis_[second_grid_axis_]->GetCardinality())
            {
            return axis_[second_grid_axis_]->GetLabel(row);
            }
        }
    else if(first_grid_axis_ != wxNOT_FOUND)
        {
        return axis_[first_grid_axis_]->GetName();
        }

    return "";
}

wxString MultiDimGrid::GetColLabelValue(int col)
{
    return DoGetColLabelValue(EnsureIndexIsPositive(col));
}

std::string MultiDimGrid::DoGetColLabelValue(unsigned int col) const
{
    if(first_grid_axis_ != wxNOT_FOUND)
        {
        if(col < axis_[first_grid_axis_]->GetCardinality())
            {
            return axis_[first_grid_axis_]->GetLabel(col);
            }
        }
    else if(second_grid_axis_ != wxNOT_FOUND)
        {
        return axis_[second_grid_axis_]->GetName();
        }

    return "";
}

void MultiDimGrid::UponAxisVariesToggle(wxCommandEvent& event)
{
    // find the checkbox triggered the event
    CheckBoxes::iterator it = std::find
        (axis_varies_checkboxes_.begin()
        ,axis_varies_checkboxes_.end()
        ,dynamic_cast<wxCheckBox*>(event.GetEventObject())
        );

    if(it == axis_varies_checkboxes_.end())
        {
        alarum() << "Event received from unexpected control." << LMI_FLUSH;
        }

    unsigned int index = bourn_cast<unsigned int>(it - axis_varies_checkboxes_.begin());
    bool varies = axis_varies_checkboxes_[index]->GetValue();
    if(varies != table().VariesByDimension(index))
        {
        bool confirmed = true;
        if(!varies)
            {
            // we are going to disable that axis - warn user about data loss
            std::ostringstream oss;
            oss
                << "Disabling the axis '"
                << axis_[index]->GetName()
                << "' could cause data loss."
                << LMI_FLUSH
                ;
            int answer = wxMessageBox
                (oss.str()
                ,axis_[index]->GetName()
                ,wxOK | wxCANCEL | wxICON_EXCLAMATION
                ,this
                );
            confirmed = (answer == wxOK);
            }
        if(confirmed)
            {
            table().MakeVaryByDimension(index, varies);
            DoRefreshAxisVaries(index);
            }
        else
            {
            // restore the varies checkbox value
            axis_varies_checkboxes_[index]->SetValue
                (table().VariesByDimension(index)
                );
            }
        }
}

void MultiDimGrid::UponSwitchSelectedAxis(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id != e_axis_x && id != e_axis_y)
        {
        alarum() << "Event received from unexpected control." << LMI_FLUSH;
        }

    DoOnSwitchSelectedAxis(static_cast<enum_axis_x_or_y>(id));
}

void MultiDimGrid::DoOnSwitchSelectedAxis(enum_axis_x_or_y x_or_y)
{
    int new_selection = GetGridAxisSelection(x_or_y);

    if(x_or_y == e_axis_x)
        {
        if(new_selection == second_grid_axis_ && new_selection != wxNOT_FOUND)
            {
            // means with the X axis we're hitting the Y axis - so switch them
            second_grid_axis_ = first_grid_axis_;
            }
        first_grid_axis_ = new_selection;
        }
    else // x_or_y == e_axis_y
        {
        if(new_selection == first_grid_axis_ && new_selection != wxNOT_FOUND)
            {
            // means with the Y axis we're hitting the X axis - so switch them
            first_grid_axis_ = second_grid_axis_;
            }
        second_grid_axis_ = new_selection;
        }
    DoSetGridAxisSelection();
}

unsigned int MultiDimGrid::EnsureIndexIsPositive(int row_or_col) const
{
    if(row_or_col < 0)
        {
        alarum()
            << "Row or column index "
            << row_or_col
            << " is negative."
            << LMI_FLUSH
            ;
        }
    return static_cast<unsigned int>(row_or_col);
}

/// MultiDimAxisAnyChoice functions implementation
/// --------------------------------------------
BEGIN_EVENT_TABLE(MultiDimAxisAnyChoice, wxChoice)
    EVT_CHOICE(wxID_ANY, MultiDimAxisAnyChoice::UponSelectionChange)
END_EVENT_TABLE()

MultiDimAxisAnyChoice::MultiDimAxisAnyChoice
    (MultiDimAxisAny const& axis
    ,MultiDimGrid& grid
    )
    :wxChoice
        (&grid
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxArrayString()
        )
    ,axis_(axis)
{
    PopulateChoiceList();
}

void MultiDimAxisAnyChoice::PopulateChoiceList()
{
    int const selection = GetSelection();
    std::string const selected_label =
        selection != wxNOT_FOUND
        ? GetString(selection).ToStdString(wxConvUTF8)
        : std::string()
        ;

    Clear();

    unsigned int const axis_count = axis_.GetCardinality();

    bool selected = false;
    for(unsigned int i = 0; i < axis_count; ++i)
        {
        std::string const label = axis_.GetLabel(i);
        Append(label);
        if(label == selected_label)
            {
            SetSelection(i);
            GetGrid().FixAxisValue(axis_.GetName(), axis_.GetValue(i));
            selected = true;
            }
        }

    if(!selected && 0 < axis_count)
        {
        SetSelection(0);
        GetGrid().FixAxisValue(axis_.GetName(), axis_.GetValue(0));
        }
}

void MultiDimAxisAnyChoice::UponSelectionChange(wxCommandEvent&)
{
    SelectionChanged();
}

void MultiDimAxisAnyChoice::SelectionChanged()
{
    int const sel = GetSelection();
    if(!(0 <= sel && static_cast<unsigned int>(sel) < axis_.GetCardinality()))
        {
        alarum()
            << "The axis is inconsistent with its choice control."
            << LMI_FLUSH
            ;
        }

    GetGrid().FixAxisValue(axis_.GetName(), axis_.GetValue(sel));
}
