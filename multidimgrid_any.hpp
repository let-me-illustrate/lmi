// A control for editing multidimensional data and supporting classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef multidimgrid_any_hpp
#define multidimgrid_any_hpp

/// The MultiDimGrid control can be used to edit N-dimensional data tables.
/// The data is represented by MultiDimTable which is a table indexed by
/// arbitrary many axis. Each axis has a finite, discrete number of values
/// which can be
///      - of string type, in which case they must be listed explicitely
///      - of int type in which case the values are specified by a range
///        min..max with the given step (1 by default)
///      - of any other custom type, in that case axis must provide conversion
///        from/to the string representation of a value
///
/// MultiDimTable is an ABC and has to be implemented by the application to
/// provide the link between its internal data representation and the GUI
/// control.
///
/// As an added twist, each of the classes here is available in two versions:
///      - type-unsafe but template-less version using std::any and thus
///        capable of containing any, possibly even heterogeneous, values;
///        these classes are called FooAny below
///      - type-safe version requiring to specify the value and axis types
///        during compile-time: this is more restrictive but is much safer so,
///        whenever possible, these Foo classes should be used.

/// Adjustment windows in MultiDimGrid
///
/// Adjustment window is bound to an axis and is used to modify (adjust) axis
/// value range, which in return will adjust the domain of the data stored
/// in the underlying table. It is an optional control and should be used
/// only for mutable axis.
///
/// Adjustment window is created by a MultiDimAxis and is owned by MultiDimGrid
///
/// There are two processes involving adjustment windows:
///   - refreshing of axis value range and its adjustment window
///     to stay syncronised with MultiDimGrid actual value-domain. It is
///     triggered by MultiDimGrid::ApplyAxisAdjustment().
///   - applying adjustments made by the user through an adjustment control to
///     the  MultiDimTable. Triggered by MultiDimGrid::RefreshAxisAdjustment().
/// Both MultiDimGrid::ApplyAxisAdjustment() and
/// MultiDimGrid::RefreshAxisAdjustment() are two step functions.
/// MultiDimGrid::ApplyAxisAdjustment():
///   - apply adjustment from adjustment window to its axis
///     MultiDimAxisAny::ApplyAdjustment()
///   - apply adjustment from the axis to the data-table
///     MultiDimTableAny::ApplyAxisAdjustment()
///
/// MultiDimGrid::RefreshAxisAdjustment() :
///   - refresh the values shown for an axis
///     MultiDimTableAny::RefreshAxisAdjustment()
///   - refresh adjustment window from the axis
///     MultiDimAxisAny::RefreshAdjustment()

/// Axis choice windows in MultiDimGrid
///
/// Axis choice is UI window allowing the user to choose a specific value for
/// the axis. It is created by the MultiDimAxis and is required.
///
/// "Mutable axis" needs to update its choice control to reflect changes
/// in its value range.
///
/// The default implementation of axis choice control is a drop down list of
/// axis values. When a value is chosen the control should trigger
/// MultiDimGrid::FixAxisValue() function to notify the grid about axis value
/// change.

#include "config.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"

#include <wx/choice.h>
#include <wx/grid.h>

#include <any>
#include <cstddef>                      // size_t
#include <memory>                       // shared_ptr
#include <string>
#include <utility>                      // pair
#include <vector>

class MultiDimAxisAny;
class MultiDimAxisAnyChoice;
class MultiDimGrid;
class MultiDimTableAny;
class WXDLLIMPEXP_FWD_CORE wxBoxSizer;
class WXDLLIMPEXP_FWD_CORE wxGridBagSizer;

/// Type-unsafe classes using std::any as value type
/// --------------------------------------------------

/// Design notes for MultiDimAxisAny
///
/// Base class representing any axis in a multidimensional data table.
///
/// This is an ABC, concrete classes derived from it are
///   - MultiDimEnumAxis for sets of strings
///   - MultiDimIntAxis for ranges of integers
///
/// Other classes (e.g. a date axis) may be derived by the user code if
/// necessary.
///
/// MultiDimAxisAny(name). Parameter name is the name of this axis
/// (also returned by GetName()). It is shown in the UI for axis choice
/// and so it should be user-readable (in particular translated if needed).
///
/// GetCardinality() returns the number of values of this axis. This is
/// used to show the appropriate number of columns/rows for this axis.
/// The number of values should be strictly greater than 1.
///
/// GetLabel() returns the user-readable string representation of the N-th
/// axis value. It is used for the column or row labels in the UI and
/// so should be translated if appropriate.
///
/// GetValue() returns the underlying N-th axis value. The label shown in
/// the UI corresponds to a value of the underlying data type which is used
/// internally and this function is used to establish this correspondence.
///
/// CreateChoiceControl() creates a new GUI control which will be used to let
/// user to select a value of this axis. The control will be disabled when
/// this axis is selected as one of the grid axes. The base class version
/// returns a wxChoice-based control and fills it using value labels, but
/// this behaviour could be overriden to implement any other kind of logic.
/// When the selection in this control changes, MultiDimGrid::FixAxisValue()
/// should be called to refresh the display (this is done automatically by
/// the choice control created by the default implementation but has to be done
/// manually otherwise).
///
/// UpdateChoiceControl() updates the values in the choice control
/// of this axis. This function is called when there are changes made to the axis
/// itself. When adjustable axis is changed the MultiDimGrid will call
/// this function asking the axis to reflect the changes in the choice control.
/// The default implementation does the refresh by simply clearing and
/// repopulating the choice control. If CreateChoiceControl() function returns
/// any other custom widget and the axis is adjustable then this function
/// should be manually overriden.
///
/// CreateAdjustControl() creates a GUI element used to let user to restrain
/// axis values shown in the grid (when the axis is selected as a grid axis).
/// This is useful when axis value set is too large to be shown on the screen.
/// A simple example: an integer axis with values in some range, the adjustment
/// control then should be used to restrain shown upper and lower bounds, or
/// maybe to change step size for the axis values shown to the user).
/// The default version of the function returns NULL indicating that this axis
/// (and its value range) is immutable.
///
/// ApplyAdjustment() reads and applies an adjustment from adjustment window
/// of this axis. This function is called when user changes axis value range
/// (via this axis adjustment window). It is a part of action-chain performed
/// by MultiDimGrid upon a user action:
///   - from adjustment control to axis object (this function)
///   - from axis object to the data table object
///     (MultiDimTableAny::ApplyAxisAdjustment)
///   - some updates on grid internal widgets to refresh the shown data
/// The function is responsible for updating the internal axis value range
/// from the adjustment control. The function returns true if the update
/// has taken place, false if no changes detected.
///
/// RefreshAdjustment() refreshes the adjustment window of the axis. This
/// function is called when the axis object is updated from inside the code and
/// the adjustment window needs to be synchronized with its axis current state.
/// It is a part of chain of refreshes made by MultiDimGrid when
/// the underlying data table is changed and axes need to be updated.
/// This function returns true if the update has taken place, false if everything
/// was up-to-date.

class MultiDimAxisAny
{
  public:
    MultiDimAxisAny(std::string const& name);
    virtual ~MultiDimAxisAny() = default;

    std::string const&   GetName() const;

    virtual unsigned int GetCardinality() const = 0;
    virtual std::string  GetLabel(unsigned int n) const = 0;
    virtual std::any     GetValue(unsigned int n) const = 0;

    virtual MultiDimAxisAnyChoice* CreateChoiceControl
        (MultiDimGrid&
        ,MultiDimTableAny&
        );
    virtual void UpdateChoiceControl(MultiDimAxisAnyChoice& choice) const;

    virtual wxWindow* CreateAdjustControl(MultiDimGrid&, MultiDimTableAny&);
    virtual bool ApplyAdjustment(wxWindow&, unsigned int axis_id);
    virtual bool RefreshAdjustment(wxWindow&, unsigned int axis_id);

  private:
    MultiDimAxisAny(MultiDimAxisAny const&) = delete;
    MultiDimAxisAny& operator=(MultiDimAxisAny const&) = delete;

    /// Name of the axis used throughout the MultiDimGrid functions
    std::string const name_;
};

inline MultiDimAxisAny::MultiDimAxisAny(std::string const& name)
    :name_ {name}
{
}

inline std::string const& MultiDimAxisAny::GetName() const
{
    return name_;
}

/// Design notes for MultiDimTableAny
///
/// The table abstracts the data shown in and edited by the grid.
/// It is a bridge between the internal data representation in the application
/// and the GUI control, allowing to clearly separate them.
/// It is an ABC which must be implemented to provide access to real data.
/// It also contains the information about the axis used to form its data
/// domain range.
///
/// If type of the table values is known at compile-time you should use
/// MultiDimTable class instead.
///
/// See also MultiDimAxisAny, MultiDimGrid classes.
///
/// GetAxesAny() returns the axes objects for that table. It could be called
/// multiple times - once for every MultiDimGrid using this table as its data
/// source. N-th axis returned represents nth dimension and holds value range
/// for that dimension. This function redirects to pure virtual DoGetAxesAny().
///
/// ApplyAxisAdjustment() reads from the axis object and apply
/// any adjustment to the data table. Function is a part of the update chain
/// that happen when user changes axis value range at runtime. This function
/// is responsible for reading new value range from the axis object and
/// applying it to the data table value domain. Parameters:
///   - axis the axis object containing axis value range adjustments
///   - n the index of the axis
///   - returns true if the update has taken place, false if everything
///     was up-to-date
///
/// RefreshAxisAdjustment() refreshes the axis object to reflect
/// the data table domain value range. Function is a part of the adjustment
/// refresh chain called by MultiDimGrid to synchronize the structures
/// with the underlying data table updated in some way. Parameters:
///   - axis   the axis object that should be synced with the table
///   - n      the index of the axis
///   - return true if the update has taken place, false if everything was
///     up-to-date
///
/// GetValueAny() and SetValueAny() provide general access for table values.
/// The coordinates vector contains all the dimension values whether the table
/// depends (varies) on them or not. If the table does not depend
/// on a dimension, then the corresponding coordinate value should be empty.
/// Parameters:
///   - coords the vector of coordinates
///   - value  the value to set (only for SetValueAny)
///   - return the value for these coordinates (only for GetValueAny)
///
/// CanChangeVariationWith() returns true if the table variation on a dimension
/// could be changed.
///
/// MakeVaryByDimension() make table data depend on the dimension. Be sure
/// to call it only for a dimension for which CanChangeVariationWith() is true.
///
/// VariesByDimension() returns true if the table depends on the specified
/// dimension.

class MultiDimTableAny
{
  public:
    /// Coordinates for an element of the table
    typedef std::vector<std::any> Coords;
    typedef std::shared_ptr<MultiDimAxisAny> AxisAnyPtr;
    typedef std::vector<AxisAnyPtr> AxesAny;

    MultiDimTableAny() = default;
    virtual ~MultiDimTableAny() = default;

    /// Return the number of dimensions in this table.
    unsigned int GetDimension() const;
    /// Create new set of table axes.
    AxesAny GetAxesAny();

    bool ApplyAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);
    bool RefreshAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);

    /// Get/set if the data varies with (depend on) n-th axis.
    virtual bool CanChangeVariationWith(unsigned int n) const = 0;
    virtual void MakeVaryByDimension(unsigned int n, bool varies) = 0;
    virtual bool VariesByDimension(unsigned int n) const = 0;

    std::any GetValueAny(Coords const& coords) const;
    void     SetValueAny(Coords const& coords, std::any const& value);

    /// Value conversion functions to be overriden in derived classes.
    virtual std::any    StringToValue(std::string const& value) const = 0;
    virtual std::string ValueToString(std::any const& value) const = 0;

  protected:
    virtual AxesAny DoGetAxesAny() = 0;
    virtual unsigned int DoGetDimension() const = 0;
    virtual std::any DoGetValueAny(Coords const&) const = 0;
    virtual void     DoSetValueAny(Coords const&, std::any const&) = 0;

    virtual bool DoApplyAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);
    virtual bool DoRefreshAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);

    void EnsureValidDimensionIndex(unsigned int n) const;
};

inline void MultiDimTableAny::EnsureValidDimensionIndex(unsigned int n) const
{
    LMI_ASSERT(n < GetDimension());
}

inline MultiDimTableAny::AxesAny MultiDimTableAny::GetAxesAny()
{
    return DoGetAxesAny();
}

inline unsigned int MultiDimTableAny::GetDimension() const
{
    return DoGetDimension();
}

inline bool MultiDimTableAny::ApplyAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    EnsureValidDimensionIndex(n);
    return DoApplyAxisAdjustment(axis, n);
}
inline bool MultiDimTableAny::RefreshAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    EnsureValidDimensionIndex(n);
    return DoRefreshAxisAdjustment(axis, n);
}
inline std::any MultiDimTableAny::GetValueAny(Coords const& coords) const
{
    if(coords.size() != GetDimension())
        {
        alarum() << "Incorrect dimension." << LMI_FLUSH;
        }
    return DoGetValueAny(coords);
}
inline void MultiDimTableAny::SetValueAny
    (Coords   const& coords
    ,std::any const& value
    )
{
    if(coords.size() != GetDimension())
        {
        alarum() << "Incorrect dimension." << LMI_FLUSH;
        }
    DoSetValueAny(coords, value);
}

/// Design notes for MultiDimGrid
///
/// The control for editing N-dimensional data table.
///
/// Note: the following is not valid until we introduce some user-interface
/// for the class and then implement type-safe approach for the class.
/// This control is not type-safe and should be only used if the grid structure
/// can be only determined dynamically. Otherwise, a type-safe MultiDimGridN
/// control declared below should be used.
///
/// MultiDimGrid() and Create(): Constructors. Create() is a creation function
/// which must be called if the object had been constructed using the default
/// empty constructor.
///   - parent the parent window of the control, must not be NULL
///   - table  the table to use, must not be NULL
///   - id     the control id for wx
///   - pos    the initial control position
///   - size   the initial control size
///
/// FixAxisValue(axis, sel): Called when the value of the given axis changes.
/// This should be only called for the axis which is currently not shown in
/// the grid as there is no any fixed value for the grid axis. It is meant
/// to be called from MultiDimAxisAny axis value selection control.
///   - axis the name of the axis whose value has changed
///   - sel  index of the new axis value
/// See also MultiDimAxisAny::CreateChoiceControl()
///
/// RefreshTableData(): Refresh the data shown in the wxGrid control
/// of the widget.
/// Call this function if you need to refresh the data shown to the user
/// in the grid control.
/// It redirects to DoRefreshTableData(). If one suspect to call this function
/// (directly or indirectly) multiple times as a part of chain of refreshes,
/// it could be protected by GridRefreshTableDataGuard.
/// See also GridRefreshTableDataGuard
///
/// RefreshTableAxis(name): Refresh table axis by its name
///   - name   axis name
///   - return true if updated, false if no update was needed
///
/// RefreshTableAxis(): Refresh every table axis
///   - return true if any axis was updated, false if no update was needed
///
/// RefreshAxisVaries(name): Refreshes the axis and its widgets in case
/// the data variation with that dimension was changed.
///   - name   axis name
///   - return true if any axis was updated, false if no update was needed
/// RefreshTableFull(): Perform a full refresh of the MultiDimGrid widgets.
/// Refresh both table axis status and the underlying data.
///   - return true if any axis was updated, false if no update was needed
///
/// ApplyAxisAdjustment(name): Call the axis adjustment update chain.
/// Convenience function to call from Adjustment axis control to trigger
/// axis and/or table update in case of a new adjustment was done by user.
///   - name   name of the axis to apply the adjustments of
///   - return true if any axis was updated, false if no update was needed
/// See also RefreshAxisAdjustment, DoApplyAxisAdjustment,
/// MultiDimAxisAny::ApplyAdjustment, MultiDimTableAny::ApplyAxisAdjustment
///
/// RefreshAxisAdjustment(name): Call the axis adjustment refresh/sync chain.
/// Convenience function to call when the table object data domain was
/// changed in some way and the corresponding axis and its adjustment widget
/// need to be synced with it.
///   - name   name of the axis to apply the adjustments of
///   - return true if any axis was updated, false if no update was needed
/// See also ApplyAxisAdjustment, DoRefreshAxisAdjustment,
/// MultiDimAxisAny::RefreshAdjustment, MultiDimTableAny::RefreshAxisAdjustment
///
/// SetXAxisColor(color) and SetYAxisColor(color): Set the X and Y axis
/// highlighting color.
/// Color used to highlight X/Y axis selection controls, vertical/horizontal
/// labels in the data table and selected axis controls.
///
/// SetGridAxisSelection(firstAxis, secondAxis): Select axis to be shown
/// in the grid part of the widget.
/// Specify two different axis to be shown in the data grid. If a non-existing
/// axis index specified or both indexes are the same function returns false.
///   - firstAxis  Index of the X axis
///   - secondAxis Index of the Y axis
///   - return     true on success
///
/// GetAxisIndexByName(axisName): Return the axis index by its name.
///   - axisName the name of the axis.
///   - return   wxNOT_FOUND if not found, otherwise axis index (0..n-1)
///
/// DoGetAxisIndexByName(axisName): Return the axis index by its name.
/// Contrary to GetAxisIndexByName() it throws in case of unknown name
/// (it does NOT return wxNOT_FOUND)
///   - axisName name of the axis to find
///   - return   axis index

class MultiDimGrid
    :public  wxPanel
    ,private wxGridTableBase
{
    friend class GridRefreshTableDataGuard;

  public:
    /// Default constructor, use Create() to really create the control.
    MultiDimGrid();
    ~MultiDimGrid() override;

    MultiDimGrid
        (wxWindow* parent
        ,std::shared_ptr<MultiDimTableAny> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );
    bool Create
        (wxWindow* parent
        ,std::shared_ptr<MultiDimTableAny> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );

    void FixAxisValue(std::string const& axis, std::any const& sel);
    void RefreshTableData();
    bool RefreshTableAxis(std::string const& name);
    bool RefreshTableAxis();
    bool RefreshAxisVaries(std::string const& name);

    bool RefreshTableFull();
    bool ApplyAxisAdjustment(std::string const& name);
    bool RefreshAxisAdjustment(std::string const& name);
    void SetXAxisColor(wxColor const& color);
    void SetYAxisColor(wxColor const& color);

    bool SetGridAxisSelection(std::pair<int,int> const&);
    std::pair<int,int> GetGridAxisSelection() const;

  protected:
    // Distinguish between X axis and Y axis.
    enum enum_axis_x_or_y
        {e_axis_x = wxID_HIGHEST + 1
        ,e_axis_y
        };

    // Use these functions to access table_ and grid_ pointers. Note that these
    // getters will throw if the underlying pointer is NULL.
    MultiDimTableAny& table() const;
    wxGrid& grid() const;

  private:
    MultiDimGrid(MultiDimGrid const&) = delete;
    MultiDimGrid& operator=(MultiDimGrid const&) = delete;

    /// Implementation of wxGridTableBase interface. The widget serves as a data
    /// source for the wxGrid component.
    ///
    /// Note: once the wxGrid class support LabelAttributeProvider() we
    /// should add color highlighting of selected axis in here.

    // wxGridTableBase overrides.
    int GetNumberCols() override;
    int GetNumberRows() override;
    bool IsEmptyCell(int row, int col) override;
    wxString GetValue(int row, int col) override;
    void SetValue(int row, int col, wxString const& value) override;
    wxString GetColLabelValue(int col) override;
    wxString GetRowLabelValue(int row) override;

    unsigned int EnsureIndexIsPositive(int) const;

  protected:
    virtual std::string DoGetColLabelValue(unsigned int col) const;
    virtual std::string DoGetRowLabelValue(unsigned int row) const;
    virtual unsigned int DoGetNumberCols() const;
    virtual unsigned int DoGetNumberRows() const;
    virtual std::string DoGetValue(unsigned int row, unsigned int col) const;
    virtual void DoSetValue
        (unsigned int row
        ,unsigned int col
        ,std::string const&
        );

    /// Array of std::any values
    typedef MultiDimTableAny::Coords Coords;

    /// Helper function used by SetValue() and GetValue() functions
    /// to fill the private coordinates vector with correct values.
    Coords const& PrepareFixedCoords(int row, int col) const;

  private:
    /// Shared pointer to an axis object
    typedef std::shared_ptr<MultiDimAxisAny> AxisPtr;
    /// Container of (pointers to) axis objects.
    typedef std::vector<AxisPtr> Axis;
    /// Common part of all ctors
    void Init();
    /// Shared pointer to the data table.
    std::shared_ptr<MultiDimTableAny> table_;
    /// Array (std::vector) of shared pointer to axis.
    Axis axis_;
    /// Cache variable - number of dimensions
    unsigned int dimension_;
    /// Sizer containing axis selection controls (X and Y) and axis controls
    wxGridBagSizer* axis_sizer_;
    /// Index of the selected axis to be displayed as the X axis in the grid
    int first_grid_axis_;
    /// Index of the selected axis to be displayed as the Y axis in the grid
    int second_grid_axis_;
    /// Array of current axis fixed values, changed every time user changes
    /// fixed value for an axis.
    Coords axis_fixed_values_;
    /// Array of axis coordinates. Used only to pass current coordinates
    /// to the underlying data table. Note that when displaying table the
    /// values for selected axis change as we walk through the grid cells
    /// to retrieve its values.
    mutable Coords axis_fixed_coords_;

    /// Creates axis selection controls for axis X and Y
    wxChoice* CreateGridAxisSelection
        (enum_axis_x_or_y
        ,std::string const& label
        ,wxColor const& selectedColor
        );
    /// Places the axis label control into the widget
    void SetAxisLabel(int axis_id, wxWindow&);
    /// Places the axis fixed value choice control into the widget
    void SetAxisChoiceControl(int axis_id, MultiDimAxisAnyChoice&);
    /// Places the axis value range selection control into the widget
    void SetAxisAdjustControl(int axis_id, wxWindow*);
    /// Places the data variation checkbox into the widget
    void SetAxisVariesControl(unsigned int axisId);

    bool SetGridAxisSelection(int firstAxis, int secondAxis);
    /// Return the axis selected for the axisId selection (X or Y)
    int GetGridAxisSelection(enum_axis_x_or_y);
    /// Selects axis changing corresponding wxChoice selections
    /// and refreshing the data grid, etc.
    void DoSetGridAxisSelection();

    /// Used by DoSetGridAxisSelection() to refresh the axis selection
    /// and to reposition various elements inside the widget if needed.
    void DoSetGridAxisSelection(enum_axis_x_or_y, int selection);

    /// Populates the axis selection controls for axis X and Y
    void PopulateGridAxisSelection();

    /// Populates one axis selection controls for axis X or Y
    void PopulateGridAxisSelection(enum_axis_x_or_y);

    int GetAxisIndexByName(std::string const& axisName);
    inline unsigned int DoGetAxisIndexByName(std::string const& axisName);

    /// Return the axis for n-th dimension.
    /// Throws std::range_error if the dimension is out of range.
    ///   - n the index of the axis (between 0 and N-1 inclusive)
    MultiDimAxisAny&       GetAxis(unsigned int n);
    MultiDimAxisAny const& GetAxis(unsigned int n) const;

    /// This is the actual function that performs the refresh.
    void DoRefreshTableData();

    bool DoRefreshAxisVaries(unsigned int n);

    bool DoRefreshTableAxis(unsigned int n);

    /// Autoselects two first non-disabled axis
    bool AutoselectGridAxis();

    /// Helper for the AutoselectGridAxis() function
    std::pair<int,int> SuggestGridAxisSelection() const;

    bool DoApplyAxisAdjustment(unsigned int n);
    bool DoRefreshAxisAdjustment(unsigned int n);

    /// Event handlers.
    void UponSwitchSelectedAxis(wxCommandEvent& event);
    void UponAxisVariesToggle(wxCommandEvent& event);

    /// Actually handle the axis selection switch
    void DoOnSwitchSelectedAxis(enum_axis_x_or_y);

    /// accessor for first_axis_choice_ and second_axis_choice_
    wxChoice& GetAxisChoiceControl(enum_axis_x_or_y);

    bool IsFullyConstructed() const;

    /// Various GUI components of the widget

    /// Data grid
    wxGrid* grid_;

    /// Drop down menu for the X and Y axes selection.
    wxChoice* first_axis_choice_;
    wxChoice* second_axis_choice_;

    /// We do not have to care about these window destruction, since it is
    /// being taken care of by WX.
    typedef std::vector<wxWindow*>   Windows;
    typedef std::vector<MultiDimAxisAnyChoice*> AxisChoiceWindows;
    typedef std::vector<wxCheckBox*> CheckBoxes;

    /// Array of axis labels
    Windows axis_labels_;

    /// Array of axis choice controls, could be null if an axis is empty
    AxisChoiceWindows axis_choice_wins_;

    /// Array of axis adjust windows, could be null if an axis is "read-only"
    Windows axis_adjust_wins_;

    /// Array of data variation checkboxes
    /// controlling whether or not the data varies along the axis
    CheckBoxes axis_varies_checkboxes_;

    /// Color used to highlight X axis selections
    wxColor selected_first_color_;

    /// Color used to highlight Y axis selections
    wxColor selected_second_color_;

    /// Refresh counter
    unsigned int table_data_refresh_counter_;

    DECLARE_EVENT_TABLE()
};

inline MultiDimGrid::MultiDimGrid()
{
    Init();
}
inline MultiDimGrid::MultiDimGrid
    (wxWindow* parent
    ,std::shared_ptr<MultiDimTableAny> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
{
    Init();
    Create(parent, table, id, pos, size);
}
inline unsigned int MultiDimGrid::DoGetAxisIndexByName(std::string const& name)
{
    int n = GetAxisIndexByName(name);
    if(n == wxNOT_FOUND || static_cast<int>(dimension_) <= n)
        {
        alarum() << "Unknown axis '" << name << "'." << LMI_FLUSH;
        }
    return static_cast<unsigned int>(n);
}
inline bool MultiDimGrid::RefreshTableAxis(std::string const& name)
{
    return DoRefreshTableAxis(DoGetAxisIndexByName(name));
}
inline bool MultiDimGrid::RefreshAxisVaries(std::string const& name)
{
    return DoRefreshAxisVaries(DoGetAxisIndexByName(name));
}
inline bool MultiDimGrid::ApplyAxisAdjustment(std::string const& name)
{
    return DoApplyAxisAdjustment(DoGetAxisIndexByName(name));
}
inline bool MultiDimGrid::RefreshAxisAdjustment(std::string const& name)
{
    return DoRefreshAxisAdjustment(DoGetAxisIndexByName(name));
}

/// Design notes for MultiDimAxisAnyChoice
///
/// Axis choice control, to allow user to select value
/// for the (not-selected) axis.
///
/// See also MultiDimAxisAny::CreateChoiceControl()
///
/// SelectionChanged(): Trigger selection update. It calls
/// MultiDimGrid::FixAxisValue()
///
/// GetGrid(): Return the grid object to be notified of any axis value changes
///
/// EVGENIY !! Is a comment useful here? Is there something nonobvious
/// to say about what the function does or why?
/// UponSelectionChange(event): Selection change event handler
///
/// PopulateChoiceList(): Fill the control with axis value labels

class MultiDimAxisAnyChoice
    :public wxChoice
{
    friend class MultiDimAxisAny;

  public:
    void SelectionChanged();

  protected:
    MultiDimAxisAnyChoice(MultiDimAxisAny const& axis, MultiDimGrid& grid);

    void UponSelectionChange(wxCommandEvent& event);
    void PopulateChoiceList();

  private:
    MultiDimAxisAnyChoice(MultiDimAxisAnyChoice const&) = delete;
    MultiDimAxisAnyChoice& operator=(MultiDimAxisAnyChoice const&) = delete;

    // the axis that this object controls
    MultiDimAxisAny const& axis_;

    MultiDimGrid& GetGrid();

    DECLARE_EVENT_TABLE()
};

inline MultiDimGrid& MultiDimAxisAnyChoice::GetGrid()
{
    return *static_cast<MultiDimGrid*>(GetParent());
}

#endif // multidimgrid_any_hpp
