// A control for editing multidimensional data and supporting classes.
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

// $Id: multidimgrid_any.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "multidimgrid_any.hpp"

#include "wx_new.hpp"

#include <wx/checkbox.h>
#include <wx/colour.h>
#include <wx/gbsizer.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/window.h>
#include <wx/wupdlock.h>

#include <sstream>

/// MultiDimAxisAny methods implementation
/// --------------------------------------

wxWindow* MultiDimAxisAny::GetAdjustControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& table
    )
{
    return NULL;
}

bool MultiDimAxisAny::ApplyAdjustment
    (wxWindow*
    ,unsigned int
    )
{
    return false;
}

bool MultiDimAxisAny::RefreshAdjustment
    (wxWindow*
    ,unsigned int
    )
{
    return false;
}

wxWindow* MultiDimAxisAny::GetChoiceControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& table
    )
{
    return new(wx) MultiDimAxisAnyChoice(*this, grid);
}

void MultiDimAxisAny::UpdateChoiceControl(wxWindow& choice_control) const
{
    MultiDimAxisAnyChoice* control
        = dynamic_cast<MultiDimAxisAnyChoice*>(&choice_control);
    if(!control)
        {
        warning() << "Wrong choice Control type" << LMI_FLUSH;
        return;
        }
    control->PopulateChoiceList();
}

/// MultiDimTableAny methods implementation
/// ---------------------------------------

bool MultiDimTableAny::DoApplyAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    return false;
}

bool MultiDimTableAny::DoRefreshAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
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
///   - MultiDimGridGrid tunes the scrolling behaviour of
///     the standard wxGrid widget.
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
        ,long style = wxWANTS_CHARS
        ,wxString const& name = wxPanelNameStr
        );
    virtual ~MultiDimGridGrid();

    virtual wxSize DoGetBestSize() const;
};

inline MultiDimGridGrid::MultiDimGridGrid
    (wxWindow* parent
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    ,long style
    ,wxString const& name
    )
    :wxGrid(parent, id, pos, size, style, name)
{
}

MultiDimGridGrid::~MultiDimGridGrid()
{
}

wxSize MultiDimGridGrid::DoGetBestSize() const
{
    // unfortunately GetCol/RowLabelSize() and GetColumnWidth()/GetRowHeight()
    // are not const in wx up to 2.8 so we need the const_cast here
    MultiDimGridGrid * const self = const_cast<MultiDimGridGrid *>(this);

    // 100 is the default minimal size of a widget - taken from wx headers.
    int width  = std::max(self->GetRowLabelSize() + self->GetColumnWidth(0), 100);
    int height = std::max(self->GetColLabelSize() + self->GetRowHeight(0), 100);

    // The following is from the wx header file for wxScrollWindow:
    // Round up to a multiple the scroll rate NOTE: this still doesn't get rid
    // of the scrollbars, is there any magic incantaion for that?
    int xpu, ypu;
    GetScrollPixelsPerUnit(&xpu, &ypu);
    if (xpu)
        {
        width  += 1 + xpu - (width  % xpu);
        }
    if (ypu)
        {
        height += 1 + ypu - (height % ypu);
        }

    return wxSize(width, height);
}

} // unnamed namespace

/// GridRefreshTableDataGuard: Table refresh guard class
///
/// This class prevents unneeded redrawing of the table to take place.
/// Under the hood it counts the number of contexts calling Table::Update
/// method. It calls for Table::DoUpdate only when that counter drops to zero,
/// which means that we are exiting the outermost context which called for
/// an update and it is the place to do it once for all the calls.

class GridRefreshTableDataGuard
  :private boost::noncopyable
{
  public:
    /// Construct guard for the counter, and use releaser at last exit
    GridRefreshTableDataGuard(MultiDimGrid&);
    ~GridRefreshTableDataGuard();

    void Release();

  private:
    MultiDimGrid* grid_;
};

inline GridRefreshTableDataGuard::GridRefreshTableDataGuard(MultiDimGrid& grid)
    :grid_(&grid)
{
    ++grid_->table_data_refresh_counter_;
}

inline GridRefreshTableDataGuard::~GridRefreshTableDataGuard()
{
    Release();
}

inline void GridRefreshTableDataGuard::Release()
{
    if(grid_ && !--grid_->table_data_refresh_counter_)
        {
        grid_->DoRefreshTableData();
        }

    grid_ = NULL;
}


/// MultiDimGrid methods implementation
/// -----------------------------------

// Two values to distinguish between X axis and Y axis
enum {
    ID_FIRST_AXIS_CHOICE = wxID_HIGHEST + 1,
    ID_SECOND_AXIS_CHOICE
};

BEGIN_EVENT_TABLE(MultiDimGrid, wxScrolledWindow)
    EVT_CHOICE   (ID_FIRST_AXIS_CHOICE  ,MultiDimGrid::OnSwitchSelectedAxis)
    EVT_CHOICE   (ID_SECOND_AXIS_CHOICE ,MultiDimGrid::OnSwitchSelectedAxis)
    EVT_CHECKBOX (wxID_ANY              ,MultiDimGrid::OnAxisVariesToggle  )
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

} // unnamed namespace

void MultiDimGrid::Init()
{
    first_grid_axis_ = wxNOT_FOUND;
    second_grid_axis_ = wxNOT_FOUND;

    // setting default colour to both X and Y axis
    selected_first_color_ = GetForegroundColour();
    selected_second_color_ = GetForegroundColour();

    // intialize refresh counter
    table_data_refresh_counter_ = 0;
}

bool MultiDimGrid::Create
    (wxWindow* parent
    ,boost::shared_ptr<MultiDimTableAny> const& atable
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
{
    // setting self as a wxPanel
    wxPanel::Create(parent, id, pos, size, wxWANTS_CHARS | wxVSCROLL);

    // setting internal variables
    table_ = atable;
    dimension_ = table_->GetDimension();

    // pospone the table data refresh until we exit Create() function
    // being sure that we are ready for a refresh
    GridRefreshTableDataGuard guard(*this);

    axis_.clear();
    axis_.resize(dimension_);
    for(unsigned int a = 0; a < dimension_; ++a)
        {
        axis_[a] = AxisPtr(table_->GetAxisAny(a));
        }

    if(atable->GetDimension() != axis_.size())
        fatal_error()
            << "Table size and number of axis do not match"
            << LMI_FLUSH
            ;

    // initializing fixed values with empty ones
    axis_fixed_values_.resize(dimension_);
    axis_fixed_coords_.resize(dimension_);

    // reserve space in axis choice control containers of the widget
    axis_labels_.resize(dimension_);
    axis_choice_wins_.resize(dimension_);
    axis_adjust_wins_.resize(dimension_);
    axis_varies_checkboxes_.resize(dimension_);


    // WX !! without the following we don't get any scrollbars at all
    // we only want the vertical scrollbar enabled
    SetScrollbars(0, 20, 0, 50);

    wxStaticBoxSizer* sizer
        = new(wx) wxStaticBoxSizer(wxHORIZONTAL, this, "Axis");

    // wxGridBagSizer(vgap, hgap)
    axis_sizer_ = new(wx) wxGridBagSizer(MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP);

    // space between X and Y axis selection controls and axis controls
    axis_sizer_->SetEmptyCellSize(wxSize(MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP));

    sizer->Add
        (axis_sizer_
        ,wxSizerFlags()
            .Expand()
            .Proportion(1)
            .Border(wxRIGHT | wxLEFT, MDGRID_SIZER_HGAP)
        );

    // Data table grid component
    grid_ = new(wx) MultiDimGridGrid(this, wxID_ANY, wxDefaultPosition);
    grid_->SetTable(this, false);

    // main sizer contains axis controls in the left part and the grid in the right
    grid_sizer_ = new(wx) wxBoxSizer(wxHORIZONTAL);
    grid_sizer_->Add
        (sizer
        ,wxSizerFlags()
            .Expand()
            .Border(wxLEFT | wxRIGHT, MDGRID_SIZER_HGAP)
        );
    grid_sizer_->Add
        (grid_
        ,wxSizerFlags()
            .Proportion(1)
            .Right()
            .Expand()
            .Border(wxALL, 1)
        );

    SetSizer(grid_sizer_);

    // create and add axis choice controls
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        SetAxisLabel(i, new(wx) wxStaticText(this, wxID_ANY, axis_[i]->GetName()));
        SetAxisChoiceControl(i, axis_[i]->GetChoiceControl(*this, *table_));
        SetAxisVariesControl(i);
        SetAxisAdjustControl(i, axis_[i]->GetAdjustControl(*this, *table_));
        }

    first_axis_choice_ = CreateGridAxisSelection
        (ID_FIRST_AXIS_CHOICE
        ,"X axis"
        ,selected_first_color_
        );
    second_axis_choice_ = CreateGridAxisSelection
        (ID_SECOND_AXIS_CHOICE
        ,"Y axis"
        ,selected_second_color_
        );

    axis_sizer_->Add
        (new(wx) wxStaticText(this, wxID_ANY, " ")
        ,wxGBPosition(MDGridGetAxisLabelRow(dimension_), 0)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags().GetFlags()
        );

    RefreshTableFull();

    Layout();

    return true;
}

void MultiDimGrid::FixAxisValue
    (std::string const& axisName
    ,boost::any const& value
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
    grid_->SetTable(grid_->GetTable(), false);
    grid_->ForceRefresh();
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
            updated = true;
        }

    // TODO ?? this method should check for changes in the table
    // data structure (number of axis, types of axis), and refresh accordingly
    if(AutoselectGridAxis())
        updated = true;

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
    first_axis_choice_->Show(dimension_ > 0);
    second_axis_choice_->Show(dimension_ > 1);

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
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        if(table_->VariesByDimension(i))
            {
            if(newFirst == wxNOT_FOUND)
                {
                if(static_cast<int>(i) != newSecond)
                    {
                    newFirst = static_cast<int>(i);
                    if(newSecond != wxNOT_FOUND)
                        break;
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
    return std::make_pair(newFirst, newSecond);
}

bool MultiDimGrid::DoRefreshTableAxis(unsigned int n)
{
    // Refresh table data only once
    GridRefreshTableDataGuard guard(*this);
    bool updated = false;
    if(DoRefreshAxisVaries(n))
        updated = true;
    if(DoRefreshAxisAdjustment(n))
        updated = true;
    return updated;
}

bool MultiDimGrid::DoRefreshAxisVaries(unsigned int axis_id)
{
    bool updated = false;

    bool varies = table_->VariesByDimension(axis_id);
    bool canChange = table_->CanChangeVariationWith(axis_id);

    wxCheckBox* box = axis_varies_checkboxes_[axis_id];
    if(box)
        {
        if(varies != box->GetValue())
            {
            box->SetValue(varies);
            updated = true;
            }

        if(canChange != box->Enable())
            updated = true;
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
    if(!table_->VariesByDimension(n))
        {
        return false;
        }

    MultiDimAxisAny& axis = *axis_[n];
    wxWindow* adjustWin = axis_adjust_wins_[n];

    bool updated = false;
    if(table_->RefreshAxisAdjustment(axis, n))
        updated = true;
    if(axis.RefreshAdjustment(adjustWin, n))
        updated = true;

    if(updated)
    {
        if(axis_choice_wins_[n])
            {
            axis.UpdateChoiceControl(*axis_choice_wins_[n]);
            }
        RefreshTableData();
    }
    return updated;
}

bool MultiDimGrid::DoApplyAxisAdjustment(unsigned int n)
{
    if(!table_->VariesByDimension(n))
        {
        return false;
        }
    MultiDimAxisAny& axis = *axis_[n];
    wxWindow* adjustWin = axis_adjust_wins_[n];

    bool updated = false;

    if(axis.ApplyAdjustment(adjustWin, n))
        updated = true;
    if(table_->ApplyAxisAdjustment(axis, n))
        updated = true;

    if(updated)
        {
        if(axis_choice_wins_[n])
            axis.UpdateChoiceControl(*axis_choice_wins_[n]);
        RefreshTableData();
        }
    return updated;
}

void MultiDimGrid::SetXAxisColour(wxColour const& color)
{
    selected_first_color_ = color;
    first_axis_choice_->SetForegroundColour(color);
    // WX !! In the future wx releases wxGrid might add support
    // for label coloring.
    // grid_->SetColLabelColour(colour);

    // Update select axis labels
    DoSetGridAxisSelection();
}

void MultiDimGrid::SetYAxisColour(wxColour const& colour)
{
    selected_second_color_ = colour;
    second_axis_choice_->SetForegroundColour(colour);
    // WX !! In the future wx releases wxGrid might add support
    // for label coloring.
    // grid_->SetRowLabelColour(colour);

    // Update select axis labels
    DoSetGridAxisSelection();
}

wxChoice* MultiDimGrid::CreateGridAxisSelection
    (int id
    ,std::string const& label
    ,wxColour const& selected_color
    )
{
    // wxChoice will grow to its default size if all choice strings are empty,
    // therefore we don't pass the empty string ("") but a space instead (" ")
    wxString only_empty_choice = " ";

    wxChoice* win = new(wx) wxChoice
        (this
        ,id
        ,wxDefaultPosition
        ,wxDefaultSize
        ,1
        ,&only_empty_choice
        );

    win->SetOwnForegroundColour(selected_color);

    unsigned int row
        = (id == ID_FIRST_AXIS_CHOICE)
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

void MultiDimGrid::SetAxisLabel(int axis_id, wxWindow* window)
{
    axis_labels_[axis_id] = window;
    axis_sizer_->Add
        (window
        ,wxGBPosition(MDGridGetAxisLabelRow(axis_id), MDGRID_LABEL_COL)
        ,wxGBSpan(1, 1)
        ,wxSizerFlags()
            .Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL)
            .Border(wxRIGHT, 16)
            .GetFlags()
        );
}

void MultiDimGrid::SetAxisChoiceControl(int axis_id, wxWindow* window)
{
    axis_choice_wins_[axis_id] = window;
    axis_sizer_->Add
        (window
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
    wxCheckBox* win = new(wx) wxCheckBox(this, wxID_ANY, "");
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
        warning() << "Select different axes" << LMI_FLUSH;
        return false;
        }
    bool update =  first_grid_axis_ != firstAxis
                || second_grid_axis_ != secondAxis
                ;
    first_grid_axis_ = firstAxis;
    second_grid_axis_ = secondAxis;
    DoSetGridAxisSelection();
    return update;
}

int MultiDimGrid::GetGridAxisSelection(int id)
{
    wxChoice* choice = (id == ID_FIRST_AXIS_CHOICE)
        ? first_axis_choice_
        : second_axis_choice_
        ;
    if(!choice)
        {
        // function is called when the control is not yet fully constructed
        return wxNOT_FOUND;
        }
    int oldSel = choice->GetSelection();
    // wxNOT_FOUND - nothing is selected
    // 0 - first empty item (" ") is selected => same as no selection at all
    if(oldSel == wxNOT_FOUND || oldSel == 0)
        return wxNOT_FOUND;

    return reinterpret_cast<int>(choice->GetClientData(oldSel));
}

void MultiDimGrid::DoSetGridAxisSelection(int axis_id, int selection)
{
    if(selection == GetGridAxisSelection(axis_id))
        return;

    wxChoice* choice = (axis_id == ID_FIRST_AXIS_CHOICE)
        ? first_axis_choice_
        : second_axis_choice_
        ;

    int old_selection = GetGridAxisSelection(axis_id);

    if(selection != old_selection)
        {
        for(unsigned int i = 1; i < choice->GetCount(); ++i)
            {
            int cdata = reinterpret_cast<int>(choice->GetClientData(i));
            if(cdata == selection)
                {
                choice->SetSelection(i);
                return;
                }
            }
        choice->SetSelection(0);
        RefreshTableData();
        }
}

void MultiDimGrid::DoSetGridAxisSelection()
{
    DoSetGridAxisSelection(ID_FIRST_AXIS_CHOICE, first_grid_axis_);
    DoSetGridAxisSelection(ID_SECOND_AXIS_CHOICE, second_grid_axis_);

    axis_fixed_coords_ = axis_fixed_values_;
    for(unsigned int i = 0; i < dimension_; ++i)
        {
        bool selected
            =  (static_cast<int>(i) == first_grid_axis_)
            || (static_cast<int>(i) == second_grid_axis_)
            ;

        // different colour to the selected axis
        wxColour color = GetForegroundColour();
        if(selected)
            {
            color = (static_cast<int>(i) == first_grid_axis_)
                ? selected_first_color_
                : selected_second_color_
                ;
            }
        axis_labels_[i]->SetOwnForegroundColour(color);

        // text does not get repainted after font and colour changes, force it
        axis_labels_[i]->Refresh();

        // disable selected window choice control
        wxWindow* choiceWin = axis_choice_wins_[i];
        if(choiceWin)
            {
            choiceWin->Enable(!selected && table_->VariesByDimension(i));
            }
        }
    RefreshTableData();
}

void MultiDimGrid::PopulateGridAxisSelection()
{
    PopulateGridAxisSelection(ID_FIRST_AXIS_CHOICE);
    PopulateGridAxisSelection(ID_SECOND_AXIS_CHOICE);
    Layout();
}

void MultiDimGrid::PopulateGridAxisSelection(unsigned int id)
{
    wxChoice* choice = (id == ID_FIRST_AXIS_CHOICE)
        ? first_axis_choice_
        : second_axis_choice_
        ;
    if(!choice)
        {
        // we are still constructing the MultiDimGrid control
        return;
        }

    wxWindowUpdateLocker update_locker(choice);

    int old_selection = GetGridAxisSelection(id);
    // set selection to empty line so that it does not change
    choice->SetSelection(0);
    int new_sel_index = wxNOT_FOUND;

    // remove every item, except the empty one (the first " ")
    for(int j = choice->GetCount() - 1; j >= 1; --j)
        {
        choice->Delete(j);
        }

    // repopulate the drop-down list with axis names
    std::size_t const size = axis_.size();
    for(unsigned int i = 0; i < size; ++i)
        {
        if(table_->VariesByDimension(i))
            {
            choice->Append
                (axis_[i]->GetName()
                ,reinterpret_cast<void*>(i)
                );
            if(static_cast<int>(i) == old_selection)
                {
                new_sel_index = choice->GetCount() - 1;
                }
            }
        }

    if(new_sel_index != wxNOT_FOUND)
        {
        choice->SetSelection(new_sel_index);
        }

    if(old_selection != GetGridAxisSelection(id))
        {
        DoOnSwitchSelectedAxis(id);
        }
}

int MultiDimGrid::GetAxisIndexByName(std::string const& axisName)
{
    for (unsigned int i = 0; i < dimension_; ++i)
        {
        if(axis_[i]->GetName() == axisName)
            {
            return i;
            }
        }
    return wxNOT_FOUND;
}

MultiDimAxisAny const& MultiDimGrid::GetAxis(unsigned int n) const
{
    if(n >= axis_.size())
        fatal_error() << "invalid axis index" << LMI_FLUSH;
    return *axis_[n];
}

MultiDimAxisAny& MultiDimGrid::GetAxis(unsigned int n)
{
    if(n >= axis_.size())
        fatal_error() << "invalid axis index" << LMI_FLUSH;
    return *axis_[n];
}

int MultiDimGrid::GetNumberRows()
{
    if(second_grid_axis_ == wxNOT_FOUND)
        return 1;
    return axis_[second_grid_axis_]->GetCardinality();
}

int MultiDimGrid::GetNumberCols()
{
    if (first_grid_axis_ == wxNOT_FOUND)
        return 1;
    return axis_[first_grid_axis_]->GetCardinality();
}

bool MultiDimGrid::IsEmptyCell(int row, int col)
{
    return false;
}

void MultiDimGrid::PrepareFixedCoords(int row, int col)
{
    if(first_grid_axis_ != wxNOT_FOUND)
        {
        axis_fixed_coords_[first_grid_axis_]
            = axis_[first_grid_axis_]->GetValue(col);
        }
    else
        {
        if(col != 0)
            fatal_error() << "No first grid axis selected" << LMI_FLUSH;
        }

    if(second_grid_axis_ != wxNOT_FOUND)
        {
        axis_fixed_coords_[second_grid_axis_] =
                    axis_[second_grid_axis_]->GetValue(row);
        }
    else
        {
        if(row != 0)
            fatal_error() << "No second grid axis selected" << LMI_FLUSH;
        }
}

wxString MultiDimGrid::GetValue(int row, int col)
{
    PrepareFixedCoords(row, col);
    boost::any value = table_->GetAnyValue(axis_fixed_coords_);
    return table_->ValueToString(value);
}

void MultiDimGrid::SetValue(int row, int col, wxString const& value)
{
    PrepareFixedCoords(row, col);
    table_->SetAnyValue(axis_fixed_coords_, table_->StringToValue(value));
}

wxString MultiDimGrid::GetRowLabelValue(int row)
{
    unsigned int const urow = static_cast<unsigned int>(row);
    if
        (  second_grid_axis_ != wxNOT_FOUND
        && urow < axis_[second_grid_axis_]->GetCardinality()
        )
        {
        return axis_[second_grid_axis_]->GetLabel(urow);
        }

    if(first_grid_axis_ != wxNOT_FOUND)
        {
        return axis_[first_grid_axis_]->GetName();
        }

    return "";
}

wxString MultiDimGrid::GetColLabelValue(int col)
{
    unsigned int const ucol = static_cast<unsigned int>(col);
    if
        (  first_grid_axis_ != wxNOT_FOUND
        && ucol < axis_[first_grid_axis_]->GetCardinality()
        )
        {
        return axis_[first_grid_axis_]->GetLabel(col);
        }

    if(second_grid_axis_ != wxNOT_FOUND)
        {
        return axis_[second_grid_axis_]->GetName();
        }

    return "";
}

void MultiDimGrid::OnAxisVariesToggle(wxCommandEvent& event)
{
    // find the checkbox triggered the event
    CheckBoxes::iterator it = std::find
        (axis_varies_checkboxes_.begin()
        ,axis_varies_checkboxes_.end()
        ,dynamic_cast<wxCheckBox*>(event.GetEventObject())
        );

    if(it == axis_varies_checkboxes_.end())
        {
        fatal_error() << "Unidentified event caught" << LMI_FLUSH;
        }

    std::size_t index = it - axis_varies_checkboxes_.begin();
    bool varies = axis_varies_checkboxes_[index]->GetValue();
    if(varies != table_->VariesByDimension(index))
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
            table_->MakeVaryByDimension(index, varies);
            DoRefreshAxisVaries(index);
            }
        else
            {
            // restore the varies checkbox value
            axis_varies_checkboxes_[index]->SetValue
                (table_->VariesByDimension(index)
                );
            }
        }
}

void MultiDimGrid::OnSwitchSelectedAxis(wxCommandEvent& event)
{
    int id = event.GetId();
    if(id != ID_FIRST_AXIS_CHOICE && id != ID_SECOND_AXIS_CHOICE)
        {
        fatal_error() << "Event from unknown control received" << LMI_FLUSH;
        }

    DoOnSwitchSelectedAxis(static_cast<unsigned int>(id));
}

void MultiDimGrid::DoOnSwitchSelectedAxis(unsigned int axis_id)
{
    int new_selection = GetGridAxisSelection(axis_id);

    if(axis_id == ID_FIRST_AXIS_CHOICE)
        {
        if(new_selection == second_grid_axis_ && new_selection != wxNOT_FOUND)
            {
            // means with the X axis we're hitting the Y axis - so switch them
            second_grid_axis_ = first_grid_axis_;
            }
        first_grid_axis_ = new_selection;
        }
    else
        {
        // here we have axis_id == ID_SECOND_AXIS_CHOICE
        if(new_selection == first_grid_axis_ && new_selection != wxNOT_FOUND)
            {
            // means with the Y axis we're hitting the X axis - so switch them
            first_grid_axis_ = second_grid_axis_;
            }
        second_grid_axis_ = new_selection;
        }
    DoSetGridAxisSelection();
}

/// MultiDimAxisAnyChoice methods implementation
/// --------------------------------------------
BEGIN_EVENT_TABLE(MultiDimAxisAnyChoice, wxChoice)
    EVT_CHOICE(wxID_ANY, MultiDimAxisAnyChoice::OnSelectionChange)
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
    std::string const selected_label
        = selection != wxNOT_FOUND ? GetString(selection) : "";

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

    if(!selected && axis_count > 0)
        {
        SetSelection(0);
        GetGrid().FixAxisValue(axis_.GetName(), axis_.GetValue(0));
        }
}

void MultiDimAxisAnyChoice::OnSelectionChange(wxCommandEvent&)
{
    SelectionChanged();
}

void MultiDimAxisAnyChoice::SelectionChanged()
{
    unsigned int sel = GetSelection();

    if(sel < 0 || sel >= axis_.GetCardinality())
        {
        fatal_error()
            << "The axis and its choice control are out of sync"
            << LMI_FLUSH;
        }

    GetGrid().FixAxisValue(axis_.GetName(), axis_.GetValue(sel));
}

