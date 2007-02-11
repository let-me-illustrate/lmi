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

// $Id: multidimgrid_any.hpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

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
///      - type-unsafe but template-less version using boost::any and thus
///        capable of containing any, possibly even heterogeneous, values;
///        these classes are called FooAny below
///      - type-safe version requiring to specify the value and axis types
///        during compile-time: this is more restrictive but is much safer so,
///        whenever possible, these FooN (where N = 0, 1, ...) classes should
///        be used.


/// Adjustment windows in MultiDimGrid
///
/// Adjustment window is bound to an axis and is used to modify (adjust) axis
/// value range, which in return will adjust the domain of the data stored
/// in the underlying table. It is an optional control and should be used
/// only for mutable axis.
///
/// Adjustment window is created by a MultiDimAxis and is owned by MultiDimGrid
///
/// There are two processes invlving adjustment windows:
///   - refreshing of axis value range and its adjustment window
///     to stay syncronised with MultiDimGrid actual value-domain. It is
///     triggered by MultiDimGrid::ApplyAxisAdjustment().
///   - applying adjustments made by the user through an adjustment control to
///     the  MultiDimTable. Triggered by MultiDimGrid::RefreshAxisAdjustment().
/// Both MultiDimGrid::ApplyAxisAdjustment() and
/// MultiDimGrid::RefreshAxisAdjustment() are two step methods.
/// MultiDimGrid::ApplyAxisAdjustment() :
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
/// MultiDimGrid::FixAxisValue() method to notify the grid about axis value
/// change.

#include "config.hpp"

#include "alert.hpp"

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/string.h>

#include <vector>

// Some forward declarations to avoid unnecessary header inclusions
class WXDLLEXPORT wxBoxSizer;
class WXDLLEXPORT wxGridBagSizer;

// This class will be defined later in this header
class MultiDimAxisAny;
class MultiDimAxisAnyChoice;
class MultiDimGrid;
class MultiDimTableAny;

// This helper class will be defined in the cpp file
class GridRefreshTableDataGuard;

/// Type-unsafe classes using boost::any as value type
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
/// MultiDimAxisAny(name): the name of this axis, returned by GetName()
///
/// GetName(): This is the axis name shown in the UI for axis choice
/// and so it should be user-readable (in particular translated if needed).
///
/// GetCardinality(): Return the number of values of this axis. This is
/// used to show the appropriate number of columns/rows for this axis.
/// The number of values should be strictly greater than 1.
///
/// GetLabel(n): Return the user-readable string representation of the N-th
/// axis value.
/// This is used for the column or row labels in the UI and so should be
/// translated if appropriate.
///   - n is the value between 0 and GetCardinality()-1
///   - returns label for this value
///
/// GetValue(n): Return the underlying value for the given axis label.
/// The label shown in the UI corresponds to a value of the underlying data
/// type which is used internally and this function is used to establish
/// this correspondence.
///   - n the value between 0 and GetCardinality()-1
///   - returns the value corresponding to this index
///
/// GetChoiceControl(grid, table): Create the control for choosing the value
/// for this axis.
/// This method is used to create the GUI control allowing to choose fixed
/// value of this axis (when it is not selected as one of the grid axis).
/// The base class version return a wxChoice control and fills it with the
/// results of calls to GetLabel() but this may be overridden to create any
/// other kind of control.
/// When the selection in this control changes, FixAxisValue() method of
/// the grid object should be called to refresh the display (this is done
/// automatically by the combobox created by the default implementation but
/// has to be done manually otherwise).
///   - grid the grid control this axis is used with
///   - table data-table object, the creator of this axis, optional parameter
///     to help non-trivial implementations to keep track of table-axis links.
///     It is not used in default implementation
///   - returns the control to select value of this axis, must not be NULL
///
/// UpdateChoiceControl(choice_control): Update the content of the value
/// choice control of the axis.
/// This method is used to update the axis choice control when there are
/// any changes made to the axis itself. When adjustable axis is changed
/// the MultiDimGrid will cal this method asking the axis to reflect
/// the changes in the choice control widget.
/// The default wxChoice control implementation does the refresh by simply
/// repopulating the choice control. If GetChoiceControl() method returns
/// any other custoim widget and the axis is adjustable then this method
/// should be manually overriden.
///   - choice_control the axis choice control previously created
//      by GetChoiceControl()
///
/// GetAdjustControl(grid, table): Create the control to adjust the axis
/// value range.
/// This control is used by an axis when it allows the user to change its
/// value range in some way during the runtime (imagine an integer axis
/// with values in a range with variable upper bound).
/// The default version of the method returns @c NULL indicating
/// that the axis (and its value range) is immutable.
/// See also ApplyAdjustment, RefreshAdjustment
///   - grid the grid control this axis is used with
///   - table the underlying data table object
///   - returns the adjustment control for the axis or NULL if none
///
/// ApplyAdjustment(axis_window, n): Read and apply adjustment from
/// its adjustment control.
/// Method is called from the adjustment control (see also GetAdjustControl())
/// when the user wants to alter the axis value range. The MultiDimGrid triggers
/// a chain of updates:
///   - from adjustment control to axis object (this method)
///   - from axis object to the data table object
///     (MultiDimTableAny::ApplyAxisAdjustment)
///   - some updates on its internal widgets to refresh the shown data
/// The method is responsible for updating the internal axis value range
/// from the adjustment control.
///   - axis_window axis adjustment window previously constructed
///     by GetAdjustControl()
///   - n axis index in the data-table
///   - returns true if the update has taken place, false if everything
///     is up-to-date
/// See also GetAdjustControl, RefreshAdjustment,
/// MultiDimTableAny::ApplyAxisAdjustment, MultiDimGrid::ApplyAxisAdjustment
///
/// RefreshAdjustment(adjust_window, n): Refresh the adjustment control
/// synchronising it with the axis.
/// Method is called when the axis object is updated from inside the code
/// and the adjustment control needs to reread the axis current state and
/// reflect it in its widgets.
/// It is a part of chain of refreshes made by MultiDimGrid
/// when the underlying data table is changed in a way that needs
/// to update its axis.
///   - adjust_window axis adjustment control constructed with GetAdjustControl
///   - n the axis index in the data table
///   - returns true if the update has taken place, false if everything
///     was up-to-date
/// See also GetAdjustControl, ApplyAdjustment,
/// MultiDimTableAny::RefreshAxisAdjustment,MultiDimGrid::RefreshAxisAdjustment

class MultiDimAxisAny
  :private boost::noncopyable
{
  public:
    MultiDimAxisAny(std::string const& name);
    virtual ~MultiDimAxisAny() {}

    std::string const&   GetName() const;
    virtual unsigned int GetCardinality() const = 0;
    virtual std::string  GetLabel(unsigned int n) const = 0;
    virtual boost::any   GetValue(unsigned int n) const = 0;

    virtual wxWindow* GetChoiceControl
        (MultiDimGrid& grid
        ,MultiDimTableAny& table
        );

    virtual void UpdateChoiceControl(wxWindow& choice_control) const;

    virtual wxWindow* GetAdjustControl
        (MultiDimGrid& grid
        ,MultiDimTableAny& table
        );
    virtual bool ApplyAdjustment
        (wxWindow* adjust_window
        ,unsigned int n
        );
    virtual bool RefreshAdjustment
        (wxWindow* adjust_window
        ,unsigned int n
        );

  private:
    /// Name of the axis used throughout the MultiDimGrid methods
    std::string const name_;
};

inline MultiDimAxisAny::MultiDimAxisAny(std::string const& name)
    :name_(name)
{
}
inline std::string const& MultiDimAxisAny::GetName() const
{
    return name_;
}


/// Design notes for MultiDimTableAny
///
/// The table abstracts the data shown in and edited by the grid.
///
/// It is a bridge between the internal data representation in the application
/// and the GUI control, allowing to clearly separate them.
///
/// It is an ABC which must be implemented to provide access to real data.
///
/// It also contains the information about the axis used to form its data
/// domain range.
///
/// If the structure of the table (i.e. the number and type of the axis) is
/// known at compile-time you should use one of type-safe MultiDimTableN
/// classes instead of this one.
///
/// GetAxisAny(n): Return the n-th axis object for that table.
/// Method is called only once by the MDGrid to retrieve the axis object
/// representing nth dimension and holding value range for that dimension.
/// This method simply redirects to polymorphic method DoGetAxisAny()
///   - n index of the dimension represented by the axis
///   - returns pointer to the axis
/// See also MultiDimAxisAny, MultiDimAxis, MultiDimGrid, DoGetAxisAny
///
/// ApplyAxisAdjustment(axis, n): Read from the axis object and apply
/// any adjustment to the data table.
/// Method is a part of the update chain that happen when user changes axis
/// value range at runtime. This method is responsible for reading new value
/// range from the axis object and applying it to the data table value domain.
///   - axis the axis object containing axis value range adjustments
///   - n the index of the axis
///   - returns true if the update has taken place, false if everything
///     was up-to-date
/// See also GetAxisAny, RefreshAxisAdjustment,
/// MultiDimAxisAny::ApplyAdjustment, MultiDimGrid::ApplyAxisAdjustment
///
/// RefreshAxisAdjustment(axis, n): Refresh the axis object to reflect
/// the data table domain value range
/// Method is a part of the adjustment refresh chain called by MultiDimGrid
/// to synchronize the structures with the underlying data table updated
/// in some way.
///   - axis   the axis object that should be synced with the table
///   - n      the index of the axis
///   - return true if the update has taken place, false if everything was up-to-date
/// See also GetAxisAny, ApplyAxisAdjustment,
/// MultiDimAxisAny::RefreshAdjustment, MultiDimGrid::RefreshAxisAdjustment
///
/// GetAnyValue(coords): General accessor for table value.
/// Calls fatal_error() if the array cardinality is different from
/// GetDimension() or boost::bad_any_cast exception if the array arguments
/// have wrong types.
/// Note that the coordinates vector contains all the dimensions values whether
/// the table depends on them or not, but those values should be empty
/// if the table does not depend on the dimension (they will be simply ignored
/// by the function).
///   - coords the vector of coordinates
///   - return the value for these coordinates
///
/// SetAnyValue(coords, value): Modifier for the table values.
/// See GetAnyValue for function description and additional notes.
///   - coords the vector of coordinates
///   - value  the value to set
///   - return the value for these coordinates
///
/// DoGetValue(coords): Return the value for the given set of coordinates.
///   - coords the vector of coordinates of size GetDimension()
///   - return the value for these coordinates
///
/// DoSetValue(coords, value): Sets the value for the given coordinates.
///   - coords the vector of coordinates of size GetDimension()
///   - value  the value to be stored for these coordinates
///
/// DoGetAxisAny(n): Returns the n-th axis for the table
/// Abstract method to implement in the deriving classes.
///   - n      the axis number (0 <= n < GetDimension())
///   - return MultiDimAxisAny object that represents nth axis

class MultiDimTableAny
{
  public:
    /// Coordinates for an element of the table
    typedef std::vector<boost::any> Coords;

    MultiDimTableAny() {}
    virtual ~MultiDimTableAny() {}

    /// Return the number of dimensions in this table.
    virtual unsigned int GetDimension() const = 0;

    MultiDimAxisAny* GetAxisAny(unsigned int n);

    bool ApplyAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);
    bool RefreshAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);

    /// Return true if the data varies with that dimension
    virtual bool VariesByDimension(unsigned int n) const = 0;

    /// Enable/disable data variation with n-th dimension
    virtual void MakeVaryByDimension(unsigned int n, bool varies) = 0;

    /// Return true if variation with n-th axis could be changed
    virtual bool CanChangeVariationWith(unsigned int n) const = 0;

    boost::any GetAnyValue(Coords const& coords) const;
    void SetAnyValue(Coords const& coords, boost::any const& value);

    /// The method to be provided by the table to allow the conversion from
    /// the table values to the strings.
    virtual wxString ValueToString(boost::any const& value) const = 0;

    /// The method to be provided by the table to allow the conversion from
    /// strings to the the table values.
    virtual boost::any StringToValue(wxString const& value) const = 0;

  protected:
    virtual boost::any DoGetValue(Coords const& coords) const = 0;
    virtual void DoSetValue(Coords const& coords, boost::any const& value) = 0;
    virtual MultiDimAxisAny* DoGetAxisAny(unsigned int n) = 0;

    virtual bool DoApplyAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);
    virtual bool DoRefreshAxisAdjustment(MultiDimAxisAny& axis, unsigned int n);

    void EnsureValidDimensionIndex(unsigned int n) const;
};

inline void MultiDimTableAny::EnsureValidDimensionIndex(unsigned int n) const
{
    if(n >= GetDimension())
        fatal_error() << "incorrect dimension" << LMI_FLUSH;
}

inline MultiDimAxisAny* MultiDimTableAny::GetAxisAny(unsigned int n)
{
    EnsureValidDimensionIndex(n);
    return DoGetAxisAny(n);
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
inline boost::any MultiDimTableAny::GetAnyValue(Coords const& coords) const
{
    if(coords.size() != GetDimension())
        fatal_error() << "incorrect dimension" << LMI_FLUSH;
    return DoGetValue(coords);
}
inline void MultiDimTableAny::SetAnyValue
    (Coords const& coords
    ,boost::any const& value
    )
{
    if(coords.size() != GetDimension())
        fatal_error() << "incorrect dimension" << LMI_FLUSH;
    DoSetValue(coords, value);
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
/// See also MultiDimAxisAny::GetChoiceControl()
///
/// RefreshTableData(): Refresh the data shown in the wxGrid control
/// of the widget.
/// Call this method if you need to refresh the data shown to the user
/// in the grid control.
/// It redirects to DoRefreshTableData(). If one suspect to call this method
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
/// SetXAxisColour(colour) and SetYAxisColour(colour): Set the X and Y axis
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
  :public wxScrolledWindow
  ,protected wxGridTableBase
  ,private boost::noncopyable
{
  public:
    /// Default constructor, use Create() to really create the control.
    MultiDimGrid();
    virtual ~MultiDimGrid() {}

    MultiDimGrid
        (wxWindow* parent
        ,boost::shared_ptr<MultiDimTableAny> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );
    bool Create
        (wxWindow* parent
        ,boost::shared_ptr<MultiDimTableAny> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );

    void FixAxisValue(std::string const& axis, boost::any const& sel);
    void RefreshTableData();
    bool RefreshTableAxis(std::string const& name);
    bool RefreshTableAxis();
    bool RefreshAxisVaries(std::string const& name);

    bool RefreshTableFull();
    bool ApplyAxisAdjustment(std::string const& name);
    bool RefreshAxisAdjustment(std::string const& name);
    void SetXAxisColour(wxColour const& colour);
    void SetYAxisColour(wxColour const& colour);

protected:
    /// Shared pointer to an axis object
    typedef boost::shared_ptr<MultiDimAxisAny> AxisPtr;
    /// Container of (pointers to) axis objects.
    typedef std::vector<AxisPtr> Axis;
    /// Common part of all ctors
    void Init();
    /// Shared pointer to the data table.
    boost::shared_ptr<MultiDimTableAny> table_;
    /// Array (std::vector) of shared pointer to axis.
    Axis axis_;
    /// Cache variable - number of dimensions
    unsigned int dimension_;
    /// Top-level sizer of the widget
    wxBoxSizer* grid_sizer_;
    /// Sizer containing axis selection controls (X and Y) and axis controls
    wxGridBagSizer* axis_sizer_;
    /// Array of boost::any values
    typedef MultiDimTableAny::Coords Coords;
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
    Coords axis_fixed_coords_;

    /// Helper function used by SetValue() and GetValue() functions
    /// to fill the private coordinates vector with correct values.
    void PrepareFixedCoords(int row, int col);
    /// Creates axis selection controls for axis X and Y
    wxChoice* CreateGridAxisSelection
        (int id
        ,std::string const& label
        ,wxColour const& selectedColour
        );
    /// Places the axis label control into the widget
    void SetAxisLabel(int axisId, wxWindow* newWin);
    /// Places the axis fixed value choice control into the widget
    void SetAxisChoiceControl(int axisId, wxWindow* newWinm);
    /// Places the axis value range selection control into the widget
    void SetAxisAdjustControl(int axisId, wxWindow* newWin);
    /// Places the data variation checkbox into the widget
    void SetAxisVariesControl(unsigned int axisId);

    bool SetGridAxisSelection(int firstAxis, int secondAxis);
    /// Return the axis selected for the axisId selection (X or Y)
    int GetGridAxisSelection(int axisId);
    /// Selects axis changing corresponding wxChoice selections
    /// and refreshing the data grid, etc.
    void DoSetGridAxisSelection();

    /// Used by DoSetGridAxisSelection() to refresh the axis selection
    /// and to reposition various elements inside the widget if needed.
    void DoSetGridAxisSelection(int axisId, int selection);

    /// Populates the axis selection controls for axis X and Y
    void PopulateGridAxisSelection();

    /// Populates one axis selection controls for axis X or Y
    void PopulateGridAxisSelection(unsigned int id);

    int GetAxisIndexByName(std::string const& axisName);
    inline unsigned int DoGetAxisIndexByName(std::string const& axisName);

    /// Return the axis for n-th dimension.
    /// Throws std::range_error if the dimension is out of range.
    ///   - n the index of the axis (between 0 and N-1 inclusive)
    MultiDimAxisAny&       GetAxis(unsigned int n);
    MultiDimAxisAny const& GetAxis(unsigned int n) const;

    /// This is the actual method that performs the refresh.
    void DoRefreshTableData();

    bool DoRefreshAxisVaries(unsigned int n);

    bool DoRefreshTableAxis(unsigned int n);

    /// Autoselects two first non-disabled axis
    bool AutoselectGridAxis();

    /// Helper for the AutoselectGridAxis() method
    std::pair<int,int> SuggestGridAxisSelection() const;

    bool DoApplyAxisAdjustment(unsigned int n);
    bool DoRefreshAxisAdjustment(unsigned int n);

    /// wxGridTableBase part.
    ///
    /// Implementation of wxGridTableBase interface. The widget serves as a data
    /// source for the wxGrid component.
    ///
    /// Note: once the wxGrid class support LabelAttributeProvider() we
    /// should add color hightliting of selected axis in here.

    /// X axis cardinality
    virtual int GetNumberRows();

    /// Y axis cardinality
    virtual int GetNumberCols();

    /// Return always true for the data range given by axis
    virtual bool IsEmptyCell(int row, int col);

    /// Gets value of the cell from the underlying data table
    virtual wxString GetValue(int row, int col);

    /// Sets value of the cell into the underlying data table
    virtual void SetValue(int row, int col, wxString const& value);

    /// Returns the corresponding Y axis row label
    virtual wxString GetRowLabelValue(int row);

    /// Returns the corresponding X axis column label
    virtual wxString GetColLabelValue(int col);

private:
    /// Various GUI components of the widget

    /// Data grid
    wxGrid* grid_;

    /// Drop down menu for the X axis selection
    wxChoice* first_axis_choice_;
    /// Drop down menu for the Y axis selection
    wxChoice* second_axis_choice_;

    typedef std::vector<wxWindow*> Windows;

    /// Array of axis labels
    Windows axis_labels_;

    /// Array of axis choice controls, could be null if an axis is empty
    Windows axis_choice_wins_;

    /// Array of axis adjust windows, could be null if an axis is "read-only"
    Windows axis_adjust_wins_;

    /// See axis_varies_checkboxes_
    typedef std::vector<wxCheckBox*> CheckBoxes;

    /// Array of data variation checkboxes
    /// controlling whether or not the data varies along the axis
    CheckBoxes axis_varies_checkboxes_;

    /// Color used to highlight X axis selections
    wxColour selected_first_color_;

    /// Color used to highlight Y axis selections
    wxColour selected_second_color_;

    friend class GridRefreshTableDataGuard;
    /// Refresh counter
    unsigned int table_data_refresh_counter_;

    /// Monitor axis selection changes
    void OnSwitchSelectedAxis(wxCommandEvent& event);
    /// Actually handle the axis selection switch
    void DoOnSwitchSelectedAxis(unsigned int axisId);

    /// Monitor axis variation checkboxes
    void OnAxisVariesToggle(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

inline MultiDimGrid::MultiDimGrid()
{
    Init();
}
inline MultiDimGrid::MultiDimGrid
    (wxWindow* parent
    ,boost::shared_ptr<MultiDimTableAny> const& table
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
    if(n == wxNOT_FOUND || n >= static_cast<int>(dimension_))
        fatal_error() << "Unknown name" << LMI_FLUSH;
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
/// See also MultiDimAxisAny::GetChoiceControl()
///
/// SelectionChanged(): Trigger selection update. It calls
/// MultiDimGrid::FixAxisValue()
///
/// GetGrid(): Return the grid object to be notified of any axis value changes
///
/// OnSelectionChange(event): Selection change event handler
///
/// PopulateChoiceList(): Fill the control with axis value labels

class MultiDimAxisAnyChoice
  :public wxChoice
  ,private boost::noncopyable
{
  public:
    void SelectionChanged();

  protected:
    friend class MultiDimAxisAny;
    MultiDimAxisAnyChoice(MultiDimAxisAny const& axis, MultiDimGrid& grid);

    void OnSelectionChange(wxCommandEvent& event);
    void PopulateChoiceList();

  private:
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

