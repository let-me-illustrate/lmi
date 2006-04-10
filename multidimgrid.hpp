/**
   @file multidimgrid.hpp
   @brief A control for editing multidimensional data and supporting classes.
   @author Vadim Zeitlin
   @version $Id: multidimgrid.hpp,v 1.1.2.6 2006-04-10 20:26:03 etarassov Exp $
   @date 2005-10-19

   The MultiDimGrid control can be used to edit N-dimensional data tables.
   The data is represented by MultiDimTable which is a table indexed by
   arbitrary many axis. Each axis has a finite, discrete number of values
   which can be
        - of string type, in which case they must be listed explicitely
        - of int type in which case the values are specified by a range
          min..max with the given step (1 by default)
        - of any other custom type, in that case axis must provide conversion
          from/to the string representation of a value

   MultiDimTable is an ABC and has to be implemented by the application to
   provide the link between its internal data representation and the GUI
   control.

   As an added twist, each of the classes here is available in two versions:
        - type-unsafe but template-less version using boost::any and thus
          capable of containing any, possibly even heterogeneous, values;
          these classes are called FooAny below
        - type-safe version requiring to specify the value and axis types
          during compile-time: this is more restrictive but is much safer so,
          whenever possible, these FooN (where N = 0, 1, ...) classes should be
          used.

   @page mdgrid-adjustment Adjustment windows in MultiDimGrid

   Adjustment window is bound to an axis and is used to modify (adjust) axis
   value range, which in return will adjust the domain of the data stored
   in the underlying table. It is an @b optional control and should be used
   only for mutable axis.

   Adjustment window is created by a MultiDimAxis and is owned by MultiDimGrid.

   There are two processes invlving adjustment windows:
   @li refreshing of axis value range and its adjustment window
       to stay syncronised with MultiDimGrid actual value-domain. It is triggered
       by MultiDimGrid::ApplyAxisAdjustment().
   @li applying adjustments made by the user through an adjustment control to
       the  MultiDimTable. Triggered by MultiDimGrid::RefreshAxisAdjustment().
   Both MultiDimGrid::ApplyAxisAdjustment() and
   MultiDimGrid::RefreshAxisAdjustment() are two step methods.
   MultiDimGrid::ApplyAxisAdjustment() :
   @li apply adjustment from adjustment window to its axis MultiDimAxisAny::ApplyAdjustment()
   @li apply adjustment from the axis to the data-table MultiDimTableAny::ApplyAxisAdjustment()

   MultiDimGrid::RefreshAxisAdjustment() :
   @li refresh the values shown for an axis MultiDimTableAny::RefreshAxisAdjustment()
   @li refresh adjustment window from the axis MultiDimAxisAny::RefreshAdjustment()

   @page mdgrid-axischoice Axis choice windows in MultiDimGrid

   Axis choice is UI window allowing the user to choose a specific value for
   the axis. It is created by the MultiDimAxis and is @b required.

   @ref mdgrid-adjustment "Mutable axis" needs to update its choice control to
   reflect changes in its value range.

   The default implementation of axis choice control is a drop down list of
   axis values. When a value is chosen the control should trigger
   MultiDimGrid::FixAxisValue() method to notify the grid about axis value
   change.
 */

#ifndef MULTIDIMGRID_HPP_
#define MULTIDIMGRID_HPP_

#include "config.hpp"

#include <wx/control.h>
#include <wx/scrolwin.h>
#include <wx/grid.h>
#include <wx/choice.h>

#include <vector>
#include <stdexcept>


#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>


// Some forward declarations to avoid unnecessary header inclusions
class WXDLLEXPORT wxBoxSizer;
class WXDLLEXPORT wxStaticBoxSizer;
class WXDLLEXPORT wxChoice;
class WXDLLEXPORT wxGridBagSizer;
// This class will be defined later in this header
class MultiDimAxisAnyChoice;

/**
   @def MAX_MULTIDIMGRID_MAKEARRAY

   This constant specifies the maximal number of arguments for the MakeArray
   method to accept.

   Note that this doesn't affect at all the max number of elements in the
   array passed to MultiDimEnumAxis ctor which is unlimited.
 */
#ifndef MAX_MULTIDIMGRID_MAKEARRAY
    #define MAX_MULTIDIMGRID_MAKEARRAY 10
#endif

/**
   @def MAX_MULTIDIMGRID_AXIS

   This constant specifies the number of type safe MultiDimGridN classes
   declared in this header. It may be set in the compiler options to any
   values but the compilation may be slower if it is very large.

   Note that this doesn't affect at all the number of axis in MultiDimGrid
   which is unlimited.
 */
#ifndef MAX_MULTIDIMGRID_AXIS
    #define MAX_MULTIDIMGRID_AXIS 10
#endif

// forward declarations
class MultiDimAxisAny;
class MultiDimTableAny;
class MultiDimGrid;

// ----------------------------------------------------------------------------
// type-unsafe classes using boost::any as value type
// ----------------------------------------------------------------------------

/**
   Base class representing any axis in a multidimensional data table.

   This is an ABC, concrete classes derived from it are
        - MultiDimEnumAxis for sets of strings
        - MultiDimIntAxis for ranges of integers

   Other classes (e.g. a date axis) may be derived by the user code if
   necessary.
 */
class MultiDimAxisAny
{
public:
    /**
       Create the axis object.

       @param name the name of this axis, returned by GetName()
     */
    MultiDimAxisAny(const wxString& name) : m_name(name) { }

    /**
       Return the axis name.

       This is the name shown in the UI for axis choice and so it should be
       user-readable (in particular translated if needed).

       The base class version simply returns the argument previously passed to
       constructor.
     */
    const wxString& GetName() const { return m_name; }

    /**
       Return the number of values of this axis.

       This is used to show the appropriate number of columns/rows for this
       axis.

       The number of values should be strictly greater than 1.
     */
    virtual unsigned int GetCardinality() const = 0;

    /**
       Return the user-readable string representation of the N-th axis value.

       This is used for the column or row labels in the UI and so should be
       translated if appropriate.

       @param n the value between 0 and GetCardinality()-1
       @return label for this value
     */
    virtual wxString GetLabel(unsigned int n) const = 0;

    /**
       Returns the underlying value for the given axis label.

       The label shown in the UI corresponds to a value of the underlying data
       type which is used internally and this function is used to establish
       this correspondence.

       @param n the value between 0 and GetCardinality()-1
       @return the value corresponding to this index
     */
    virtual boost::any GetValue(unsigned int n) const = 0;

    /**
       Create the control for choosing the value for this axis.

       This method is used to create the GUI control allowing to choose fixed
       value of this axis (when it is not selected as one of the grid axis).
       The base class version return a wxChoice control and fills it with the
       results of calls to GetLabel() but this may be overridden to create any
       other kind of control.

       When the selection in this control changes, FixAxisValue() method of
       the @a grid object should be called to refresh the display (this is
       done automatically by the combobox created by the default
       implementation but has to be done manually otherwise).

       @param grid the grid control this axis is used with
       @param table data-table object, the creator of this axis, optional
                    parameter to help non-trivial implementations to keep track
                    of table-axis links. It is not used in default implementation
       @return the control to select value of this axis, must not be @c NULL
     */
    virtual
    wxWindow *GetChoiceControl( MultiDimGrid & grid,
                                MultiDimTableAny & table );

    /**
       Update the content of the value choice control of the axis.

       This method is used to update the axis choice control when there are
       any changes made to the axis itself. When adjustable axis is changed
       the MultiDimGrid will cal this method asking the axis to reflect
       the changes in the choice control widget.

       The default wxChoice control implementation does the refresh by simply
       repopulating the choice control. If GetChoiceControl() method returns
       any other custoim widget and the axis is adjustable then this method
       should be manually overriden.

       @param choiceControl the axis choice control previously created by GetChoiceControl()
     */
    virtual
    void UpdateChoiceControl( wxWindow & choiceControl ) const;

    /**
       Create the control to adjust the axis value range.

       This control is used by an axis when it allows the user to change its
       value range in some way during the runtime (imagine an integer axis
       with values in a range with variable upper bound).

       The default version of the method returns @c NULL indicating
       that the axis (and its value range) is immutable.

       @sa ApplyAdjustment RefreshAdjustment

       @param grid the grid control this axis is used with
       @param table the underlying data table object
       @return the adjustment control for the axis or @c NULL if none
     */
    virtual wxWindow *GetAdjustControl( MultiDimGrid & grid,
                                        MultiDimTableAny & table )
    {
        return NULL;
    }

    /**
       Read and apply adjustment from its adjustment control.

       Method is called from the adjustment control (see also GetAdjustControl() )
       when the user wants to alter the axis value range. The MultiDimGrid
       triggers a chain of updates:
        - from adjustment control to axis object (this method)
        - from axis object to the data table object (MultiDimTableAny::ApplyAxisAdjustment)
        - some updates on its internal widgets to refresh the shown data

       The method is responsible for updating the internal axis value range
       from the adjustment control.

       @sa GetAdjustControl RefreshAdjustment
       @sa MultiDimTableAny::ApplyAxisAdjustment MultiDimGrid::ApplyAxisAdjustment

       @param axis adjustment window previously constructed by GetAdjustControl()
       @param n axis index in the data-table
       @return true if the update has taken place, false if everything is up-to-date
     */
    virtual
    bool ApplyAdjustment( wxWindow * WXUNUSED(adjustWin),
                          unsigned int WXUNUSED(n) )
    {   return false;   }

    /**
       Refresh the adjustment control synchronising it with the axis

       Method is called when the axis object is updated from inside the code
       and the adjustment control needs to reread the axis current state and
       reflect it in its widgets.

       It is a part of chain of refreshes made by MultiDimGrid
       when the underlying data table is changed in a way that needs
       to update its axis.

       @sa GetAdjustControl ApplyAdjustment
       @sa MultiDimTableAny::RefreshAxisAdjustment MultiDimGrid::RefreshAxisAdjustment

       @param adjustWin axis adjustment control constructed with GetAdjustControl
       @param n the axis index in the data table
       @return true if the update has taken place, false if everything was up-to-date
     */
    virtual
    bool RefreshAdjustment( wxWindow * WXUNUSED(adjustWin),
                            unsigned int WXUNUSED(n) )
    {   return false;   }

    /**
       Virtual destructor for the base class.
     */
    virtual ~MultiDimAxisAny() {}

private:
    /// Name of the axis used throughout the MultiDimGrid methods
    const wxString m_name;

    DECLARE_NO_COPY_CLASS(MultiDimAxisAny)
};

/**
   The table abstracts the data shown in and edited by the grid.

   It is a bridge between the internal data representation in the application
   and the GUI control, allowing to clearly separate them.

   It is an ABC which must be implemented to provide access to real data.

   It also contains the information about the axis used to form its data domain
   range.

   If the structure of the table (i.e. the number and type of the axis) is
   known at compile-time you should use one of type-safe MultiDimTableN
   classes instead of this one.
 */
class MultiDimTableAny
{
public:
    /// Coordinates for an element of the table
    typedef std::vector<boost::any> Coords;

    /**
       Default constructor doesn't do anything special.
     */
    MultiDimTableAny()
    {
    }

    /**
       Virtual destructor for the base class.
     */
    virtual ~MultiDimTableAny()
    {
    }

    /**
       Return the number of dimensions in this table.
     */
    virtual unsigned int GetDimension() const = 0;

    /**
       Return the nth axis object for that table.

       Method is called only once by the MDGrid to retrieve the axis object
       representing nth dimension and holding value range for that dimension.

       This method simply redirects to polymorphic method DoGetAxisAny()

       @sa MultiDimAxisAny MultiDimAxis MultiDimGrid
       @sa DoGetAxisAny

       @param n index of the dimension represented by the axis
       @return pointer to the axis
     */
    MultiDimAxisAny * GetAxisAny( unsigned int n )
    {
        if ( n >= GetDimension() )
            throw std::range_error("incorrect dimension");
        return DoGetAxisAny(n);
    }

    /**
       Read from the axis object and apply any adjustment to the data table

       Method is a part of the update chain that happen when user changes axis
       value range at runtime. This method is responsible for reading
       new value range from the axis object and applying it to the data table
       value domain.

       @sa GetAxisAny RefreshAxisAdjustment
       @sa MultiDimAxisAny::ApplyAdjustment MultiDimGrid::ApplyAxisAdjustment

       @param axis the axis object containing axis value range adjustments
       @param n the index of the axis
       @return true if the update has taken place, false if everything was up-to-date
     */
    bool ApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
    {
        if ( n >= GetDimension() )
            throw std::range_error("incorrect dimension");
        return DoApplyAxisAdjustment( axis, n );
    }

    /**
       Refresh the axis object to reflect the data table domain value range

       Method is a part of the adjustment refresh chain called by
       MultiDimGrid to synchronize the structures with the underlying data table
       updated in some way.

       @sa GetAxisAny ApplyAxisAdjustment
       @sa MultiDimAxisAny::RefreshAdjustment MultiDimGrid::RefreshAxisAdjustment

       @param axis the axis object that should be synced with the table
       @param n the index of the axis
       @return true if the update has taken place, false if everything was up-to-date
     */
    bool RefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
    {
        if ( n >= GetDimension() )
            throw std::range_error("incorrect dimension");
        return DoRefreshAxisAdjustment( axis, n );
    }

    /**
       Return true if the data varies with that dimension
     */
    virtual bool VariesByDimension( unsigned int n ) const = 0;

    /**
       Enable/disable data variation with n-th dimension
     */
    virtual void MakeVaryByDimension( unsigned int n, bool varies ) = 0;

    /**
       Return true if variation with n-th axis could be changed
     */
    virtual bool CanChangeVariationWith( unsigned int n ) const = 0;

    /**
       General accessor for table value.

       Throws an std::exception if the array cardinality is different from
       GetDimension() or boost::bad_any_cast exception if the array arguments
       have wrong types.
       Note that the coordinates vector contains all the dimensions values
       whether the table depends on them or not, but those values should
       be empty if the table does not depend on the dimension (they will be
       simply ignored by the function).

       @param coords the vector of coordinates
       @return the value for these coordinates
     */
    boost::any GetAnyValue(const Coords& coords) const
    {
        if ( coords.size() != GetDimension() )
            throw std::range_error("incorrect dimension");

        return DoGetValue(coords);
    }

    /**
       Modifier for the table values.

       See GetAnyValue for function description and additional notes.
       @sa GetAnyValue

       @param coords the vector of coordinates
       @param value the value to set
       @return the value for these coordinates
     */
    void SetAnyValue(const Coords& coords, const boost::any& value)
    {
        if ( coords.size() != GetDimension() )
            throw std::range_error("incorrect dimension");

        DoSetValue(coords, value);
    }

    /**
       The method to be provided by the table to allow the conversion from
       the table values to the strings.
     */
    virtual wxString ValueToString(const boost::any & value) const = 0;

    /**
       The method to be provided by the table to allow the conversion from
       strings to the the table values.
     */
    virtual boost::any StringToValue(const wxString & value) const = 0;

protected:
    /**
       Return the value for the given set of coordinates.

       @param coords the vector of coordinates of size GetDimension()
       @return the value for these coordinates
     */
    virtual boost::any DoGetValue(const Coords& coords) const = 0;

    /**
       Sets the value for the given coordinates.

       @param coords the vector of coordinates of size GetDimension()
       @param value the value to be stored for these coordinates
     */
    virtual void DoSetValue(const Coords& coords, const boost::any& value) = 0;

    /**
       Returns the nth axis for the table

       Abstract method to implement in the deriving classes.

       @param n the axis number (0 <= n < GetDimension())
       @return MultiDimAxisAny object that represents nth axis
     */
    virtual MultiDimAxisAny * DoGetAxisAny(unsigned int n) = 0;

    /**
       @sa ApplyAxisAdjustment
     */
    virtual
    bool DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
    {   return false;    }

    /**
       @sa DoRefreshAxisAdjustment
     */
    virtual
    bool DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
    {   return false;   }
};

// forward declaration
class RefreshCounter;

/**
   The control for editing N-dimensional data table.

   Note: the following is not valid until we introduce some user-interface
   for the class and then implement type-safe approach for the class.
   This control is not type-safe and should be only used if the grid structure
   can be only determined dynamically. Otherwise, a type-safe MultiDimGridN
   control declared below should be used.
 */
class MultiDimGrid : public wxScrolledWindow,
                     protected wxGridTableBase
{
public:
    /**
       Default constructor, use Create() to really create the control.
     */
    MultiDimGrid()
    {
        Init();
    }

    /**
       Creates a new grid control.

       @param parent the parent window of the control, must not be @c NULL
       @param table the table to use, must not be @c NULL
       @param id the control id for wx
       @param pos the initial control position
       @param size the initial control size
     */
    MultiDimGrid( wxWindow *parent,
                     boost::shared_ptr<MultiDimTableAny> const & table,
                     wxWindowID id = wxID_ANY,
                     wxPoint const & pos = wxDefaultPosition,
                     wxSize const & size = wxDefaultSize )
    {
        Init();

        Create(parent, table, id, pos, size);
    }

    /**
       Virtual destructor.
     */
    virtual ~MultiDimGrid()
    {}

    /**
       Creation function which must be called if the object had been
       constructed using the default empty constructor.

       @param parent the parent window of the control, must not be @c NULL
       @param table the table to use, must not be @c NULL
       @param id the control id for wx
       @param pos the initial control position
       @param size the initial control size
       @return true if the window was created successfully, false otherwise
     */
    bool Create( wxWindow *parent,
                 boost::shared_ptr<MultiDimTableAny> const & table,
                 wxWindowID id = wxID_ANY,
                 wxPoint const & pos = wxDefaultPosition,
                 wxSize const & size = wxDefaultSize );

    /**
       Called when the value of the given axis changes.

       This should be only called for the axis which is currently not shown in
       the grid as there is no any fixed value for the grid axis. It is meant
       to be called from MultiDimAxisAny axis value selection control.

       @sa MultiDimAxisAny::GetChoiceControl()

       @param axis the name of the axis whose value has changed
       @param sel index of the new axis value
     */
    void FixAxisValue( const wxString& axis, const boost::any & sel );

    /**
       Refresh the data shown in the wxGrid control of the widget.

       Call this method if you need to refresh the data shown to the user
       in the grid control.

       It redirects to DoRefreshTableData(). If one suspect to call this method
       (directly or indirectly) multiple times as a part of chain of refreshes,
       it could be protected by GridRefreshTableDataGuard.

       @sa GridRefreshTableDataGuard
     */
    void RefreshTableData();

    /**
       Refresh table axis by its name

       @sa DoRefreshTableAxis
       @param name axis name
       @return true if updated, false if no update was needed
     */
    bool RefreshTableAxis( wxString const & name )
    {   return DoRefreshTableAxis( DoGetAxisIndexByName( name ) );   }

    /**
       Refresh every table axis

       @return true if any axis was updated, false if no update was needed
     */
    bool RefreshTableAxis();

    /**
       Refreshes the axis and its widgets in case the data variation with
       that dimension was changed.

       @sa DoRefreshAxisVaries()
       @param name axis name
       @return true if any axis was updated, false if no update was needed
     */
    bool RefreshAxisVaries( wxString const & name )
    {   return DoRefreshAxisVaries( DoGetAxisIndexByName( name ) ); }

    /**
       Perform a full refresh of the MultiDimGrid widgets.

       Refresh both table axis status and the underlying data.

       @return true if any axis was updated, false if no update was needed
     */
    bool RefreshTableFull()
    {
        bool updated = false;
        Freeze();
        if( RefreshTableAxis() )
            updated = true;
        RefreshTableData();
        Thaw();
        return updated;
    }

    /**
       Call the axis adjustment update chain

       Convenience function to call from Adjustment axis control to trigger
       axis and/or table update in case of a new adjustment was done by user.


       @sa MultiDimAxisAny::ApplyAdjustment MultiDimTableAny::ApplyAxisAdjustment
       @sa RefreshAxisAdjustment DoApplyAxisAdjustment

       @param name name of the axis to apply the adjustments of
       @return true if any axis was updated, false if no update was needed
     */
     bool ApplyAxisAdjustment( wxString const & name )
     {  return DoApplyAxisAdjustment( DoGetAxisIndexByName( name ) );    }

    /**
       Call the axis adjustment refresh/sync chain

       Convenience function to call when the table object data domain was
       changed in some way and the corresponding axis and its adjustment widget
       need to be synced with it.

       @sa MultiDimAxisAny::RefreshAdjustment MultiDimTableAny::RefreshAxisAdjustment
       @sa ApplyAxisAdjustment DoRefreshAxisAdjustment

       @param name name of the axis to apply the adjustments of
       @return true if any axis was updated, false if no update was needed
     */
     bool RefreshAxisAdjustment( wxString const & name )
     {  return DoRefreshAxisAdjustment( DoGetAxisIndexByName( name ) ); }

    /**
       Set the X axis highlighting color.

       Color used to highlight X axis selection controls, vertical labels in
       the data table and selected axis controls.
     */
    void SetXAxisColour( wxColour const & colour );

    /**
       Set the Y axis highlighting color

       Color used to highlight Y axis selection controls, horizontal labels in
       the data table and selected axis controls.
     */
    void SetYAxisColour( wxColour const & colour );

protected:
    /// Shared pointer to an axis object
    typedef boost::shared_ptr<MultiDimAxisAny> AxisPtr;

    /// Container of (pointers to) axis objects.
    typedef std::vector<AxisPtr> Axis;

    /// Common part of all ctors
    void Init();

    /// Shared pointer to the data table.
    boost::shared_ptr<MultiDimTableAny> m_table;

    /// Array (std::vector) of shared pointer to axis.
    Axis m_axis;

    /// Cache variable - number of dimensions
    unsigned int m_dimension;

    /// Top-level sizer of the widget
    wxBoxSizer * m_gridSizer;

    /// Sizer containing axis selection controls (X and Y) and axis controls
    wxGridBagSizer * m_axisSizer;

    /// Array of boost::any values
    typedef MultiDimTableAny::Coords Coords;

    /// Index of the selected axis to be displayed as the X axis in the grid
    int m_firstGridAxis;

    /// Index of the selected axis to be displayed as the Y axis in the grid
    int m_secondGridAxis;

    /**
       Array of current axis fixed values, changed every time user changes
       fixed value for an axis.
     */
    Coords m_axisFixedValues;

    /**
       Array of axis coordinates. Used only to pass current coordinates
       to the underlying data table. Note that when displaying table the
       values for selected axis change as we walk through the grid cells
       to retrieve its values.
     */
    Coords m_axisFixedCoords;

    /**
       Helper function used by SetValue() and GetValue() functions
       to fill the private coordinates vector with correct values.

       @sa MultiDimAxisAny::GetValue()
       @sa MultiDimAxisAny::SetValue()
     */
    void PrepareFixedCoords( int row, int col );

    /// Creates axis selection controls for axis X and Y
    wxChoice * CreateGridAxisSelection( int id, wxString const & label,
                                        wxColour const & selectedColour );

    /// Places the axis label control into the widget
    void SetAxisLabel( int axisId, wxWindow * newWin );

    /// Places the axis fixed value choice control into the widget
    void SetAxisChoiceControl( int axisId, wxWindow * newWinm );

    /// Places the axis value range selection control into the widget
    void SetAxisAdjustControl( int axisId, wxWindow * newWin );

    /// Places the data variation checkbox into the widget
    void SetAxisVariesControl( unsigned int axisId );

    /**
       Select axis to be shown in the grid part of the widget.

       Specify two different axis to be shown in the data grid. If a non-existing
       axis index specified or both indexes are the same function returns false.

       @param firstAxis Index of the X axis
       @param secondAxis Index of the Y axis
       @return true on success
     */
    bool SetGridAxisSelection( int firstAxis, int secondAxis );

    /// Return the axis selected for the axisId selection (X or Y)
    int GetGridAxisSelection( int axisId );

    /**
       Selects axis changing corresponding wxChoice selections
       and refreshing the data grid, etc.
     */
    void DoSetGridAxisSelection();

    /**
       Used by DoSetGridAxisSelection() to refresh the axis selection
       and to reposition various elements inside the widget if needed.
     */
    void DoSetGridAxisSelection( int axisId, int selection );

    /// Populates the axis selection controls for axis X and Y
    void PopulateGridAxisSelection();

    /// Populates one axis selection controls for axis X or Y
    void PopulateGridAxisSelection( unsigned int id );

    /**
       Return the axis index by its name.

       @param axisName the name of the axis.
       @return wxNOT_FOUND if not found, otherwise axis index (0..n-1)
     */
    int GetAxisIndexByName( const wxString & axisName );

    /**
       Return the axis index by its name.

       Contrary to GetAxisIndexByName() it throws in case of unknown name
       (it does NOT return wxNOT_FOUND)

       @param axisName name of the axis to find
       @return axis index
     */
    inline
    unsigned int DoGetAxisIndexByName( const wxString & axisName );

    /**
       Return the axis for n-th dimension.

       Throws std::range_error if the dimension is out of range.

       @param n the index of the axis (between 0 and N-1 inclusive)
     */
    /// @{
    MultiDimAxisAny& GetAxis( unsigned int n );
    MultiDimAxisAny const & GetAxis( unsigned int n ) const;
    /// @}

    /// This is the actual method that performs the refresh. See also RefreshTableData()
    void DoRefreshTableData();

    /// @sa RefreshAxisVaries()
    bool DoRefreshAxisVaries( unsigned int n );

    /// @sa RefreshTableAxis()
    bool DoRefreshTableAxis( unsigned int n );

    /// Autoselects two first non-disabled axis
    bool AutoselectGridAxis();

    /// Helper for the AutoselectGridAxis() method
    std::pair<int,int> SuggestGridAxisSelection() const;

    /// @sa ApplyAxisAdjustment()
    bool DoApplyAxisAdjustment( unsigned int n );

    /// @sa RefreshAxisAdjustment()
    bool DoRefreshAxisAdjustment( unsigned int n );

    /**
       @name wxGridTableBase part.

       Implementation of wxGridTableBase interface. The widget serves as a data
       source for the wxGrid component.

       Note: once the wxGrid class support LabelAttributeProvider() we
       should add color hightliting of selected axis in here.
     */
    //@{

    /// X axis cardinality
    virtual int GetNumberRows();

    /// Y axis cardinality
    virtual int GetNumberCols();

    /// Return always true for the data range given by axis
    virtual bool IsEmptyCell( int row, int col );

    /// Gets value of the cell from the underlying data table
    virtual wxString GetValue( int row, int col );

    /// Sets value of the cell into the underlying data table
    virtual void SetValue( int row, int col, const wxString& value );

    /// Returns the corresponding Y axis row label
    virtual wxString GetRowLabelValue( int row );

    /// Returns the corresponding X axis column label
    virtual wxString GetColLabelValue( int col );

    //@}

private:
    /**
       @name Various gui components of the widget
     */
    //@{

    /// Data grid
    wxGrid *m_grid;

    /// Drop down menu for the X axis selection
    wxChoice *m_firstAxisChoice;
    /// Drop down menu for the Y axis selection
    wxChoice *m_secondAxisChoice;

    typedef std::vector<wxWindow *> Windows;

    /// Array of axis labels
    Windows m_axisLabels;

    /// Array of axis choice controls, could be null if an axis is empty
    Windows m_axisChoiceWins;

    /// Array of axis adjust windows, could be null if an axis is "read-only"
    Windows m_axisAdjustWins;

    /// @sa m_axisVariesCheckboxes
    typedef std::vector<wxCheckBox *> CheckBoxes;

    /**
        Array of data variation checkboxes
        controlling whether or not the data varies along the axis
      */
    CheckBoxes m_axisVariesCheckboxes;

    //@}

    /// Color used to highlight X axis selections
    wxColour m_selectedFirstColour;

    /// Color used to highlight Y axis selections
    wxColour m_selectedSecondColour;

    friend class GridRefreshTableDataGuard;
    /// Refresh counter
    unsigned int m_tableDataRefreshCounter;

    /// Monitor axis selection changes
    void OnSwitchSelectedAxis( wxCommandEvent & event );
    /// Actually handle the axis selection switch
    void DoOnSwitchSelectedAxis( unsigned int axisId );

    /// Monitor axis variation checkboxes
    void OnAxisVariesToggle( wxCommandEvent & event );

    DECLARE_NO_COPY_CLASS( MultiDimGrid )
    DECLARE_EVENT_TABLE()
};

inline
unsigned int MultiDimGrid::DoGetAxisIndexByName( const wxString & axisName )
{
    int n = GetAxisIndexByName( axisName );
    if( n == wxNOT_FOUND || n >= static_cast<int>( m_dimension ) )
        throw std::range_error( "Unknown name" );
    return static_cast<unsigned int>( n );
}


/**
   @class MultiDimAxisAnyChoice

   Axis choice control, to allow user to select value for the (not-selected) axis

   @sa MultiDimAxisAny::GetChoiceControl()
 */
class MultiDimAxisAnyChoice : public wxChoice
{
public:
    /// Trigger selection update. It calls MultiDimGrid::FixAxisValue()
    void SelectionChanged();

protected:
    friend class MultiDimAxisAny;
    MultiDimAxisAnyChoice( MultiDimAxisAny const & axis, MultiDimGrid & grid );

    /// Selection change event handler
    void OnSelectionChange( wxCommandEvent & event );

    /// Fill the control with axis value labels
    void PopulateChoiceList();

private:
    // the axis that this object controls
    MultiDimAxisAny const & m_axis;

    /// Return the grid object to be notified of any axis value changes
    MultiDimGrid & GetGrid()
    {   return *static_cast<MultiDimGrid *>( GetParent() );   }

    DECLARE_NO_COPY_CLASS( MultiDimAxisAnyChoice )
    DECLARE_EVENT_TABLE()
};

/**
   \defgroup mdgrid_type_safe MultiDim* type-safe classes

   Type-safe classes.

   These classes are provided to allow the type-safe use/checks when the
   axis and table types are known at compile time.

   Almost every untyped virtual method is reimplemented in those classes to
   redirect the work to its Do* twin virtual method that takes correctly typed
   parameters.
   You should not use untyped method versions (such as MultiDimAxis::GetValue(),
   MultiDimTableN::SetValue()) - use typed versions instead
   (MultiDimAxis::DoGetValue(), MultiDimTableN::DoSetValue()).
 */
/// @{

/**
   Typed version of MultiDimAxisAny.

   Use it as the base class for any specific custom axis with known value type.
   @sa MultiDimEnumAxis MultiDimIntAxis
 */
template<typename E>
class MultiDimAxis : public MultiDimAxisAny
{
public:
    /// Type of values for this axis
    typedef E ValueType;

    /// @param axis name
    MultiDimAxis( wxString const & name )
    : MultiDimAxisAny(name) { }

    /**
       Redirect the method to the typed method DoGetValue()

       This method should not be used at all, use DoGetValue() instead
     */
    virtual boost::any GetValue( unsigned int n ) const;

protected:
    /// Implement this function returning values of the type ValueType
    virtual ValueType DoGetValue( unsigned int n ) const = 0;
};

template<typename E>
boost::any MultiDimAxis<E>::GetValue(unsigned int n) const
{
    return boost::any(static_cast<ValueType>( DoGetValue( n ) ));
}

/**
   Represents an axis whose values is a set of strings.

   The string values typically correspond to an enum internally, hence the
   name of the class.

   Template parameter E is the enum type which corresponds to the values of
   this axis.
 */
template <typename E>
class MultiDimEnumAxis : public MultiDimAxis<E>
{
public:
    typedef MultiDimAxis<E> BaseClass;
    typedef typename BaseClass::ValueType ValueType;

    /**
       Creates the axis with the given set of possible values.

       @param name the name of the axis, for GetName() implementation
       @param values all possible values for this axis
     */
    MultiDimEnumAxis(const wxString& name, const wxArrayString& values)
        : MultiDimAxis<E>(name), m_values(values)
    {
    }

#ifdef DOXYGEN
    /**
       Helper for passing the values to constructor argument.

       This function takes N strings and returns an array with N elements.

       Note that in reality there is not a single function but a family of
       overloaded functions taking up to MAX_MULTIDIMGRID_MAKEARRAY
       parameters.

       @param s1 the label for the first value
       ...
       @param sN the label for the last value
     */
    static wxArrayString MakeArray(const wxString& s1, ..., const wxString& sN);
#else // !DOXYGEN
    #define MAKE_ARRAY_n(z, n, unused)                                        \
    static wxArrayString                                                      \
    MakeArray(BOOST_PP_ENUM_PARAMS(n, const wxString& s))                     \
    {                                                                         \
        const wxString strings[n] =                                           \
        {                                                                     \
            BOOST_PP_ENUM_PARAMS(n, s)                                        \
        };                                                                    \
        return wxArrayString(n, strings);                                     \
    }

    /**
       c++ standard does not allow empty static arrays therefore we should
       exclude MakeArray_0 case, starting from 1
     */
    BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_MAKEARRAY, MAKE_ARRAY_n, ~)

    #undef MAKE_ARRAY_n
#endif // DOXYGEN/!DOXYGEN

    /// @sa MultiDimAxisAny::GetCardinality()
    virtual unsigned int GetCardinality() const
    {   return m_values.size(); }

    /// @sa MultiDimAxisAny::GetLabel()
    virtual wxString GetLabel(unsigned int n) const
    {   return m_values[n]; }

    /// @sa MultiDimAxis::DoGetValue()
    virtual ValueType DoGetValue(unsigned int n) const
    {
        return static_cast<ValueType>( n );
    }

private:
    wxArrayString m_values;
};

/**
   Represents an axis whose values is a range of integeres.
 */
template<typename Integral>
class MultiDimIntegralAxis : public MultiDimAxis<Integral>
{
public:

    /**
       Creates the axis for the range @a minValue .. @a maxValue.

       @a minValue must be strictly inferior to @a maxValue, the ctor does not
       reorder them.

       @param name the name of the axis, for GetName() implementation
       @param minValue the minimal axis value (inclusive), e.g. 0
       @param maxValue the maximal axis value (inclusive), e.g. 100
       @param step only values offset from minValue by a multiple of step are
                        valid values; step must be strictly positive
     */
    MultiDimIntegralAxis( wxString const & name,
                          Integral minValue,
                          Integral maxValue,
                          Integral step )
        : MultiDimAxis<Integral>(name)
    {
        SetValues( minValue, maxValue, step );
    }

    MultiDimIntegralAxis( wxString const & name )
        : MultiDimAxis<Integral>(name)
    {
        SetValues( 0, 100, 1 );
    }

    /// @return the range lower bound (including)
    Integral GetMinValue() const
    {   return m_min; }

    /// @return the range upper bound (including)
    Integral GetMaxValue() const
    {   return m_max; }

    /// @return the step
    Integral GetStep() const
    {   return m_step; }

    /// Modifier for GetMinValue(), GetMaxValue() and GetStep()
    void SetValues( Integral minValue, Integral maxValue, Integral step )
    {
        wxASSERT_MSG( minValue <= maxValue,
                      _T("minValue has to be less than or equal to maxValue") );
        wxASSERT_MSG( step >= 1,
                      _T("step has to be at least 1") );
        m_min = minValue;
        m_max = maxValue;
        m_step = step;
    }

    /// @sa MultiDimAxisAny::GetCardinality()
    virtual unsigned int GetCardinality() const
    {
        return ( m_max - m_min + m_step ) / m_step;
    }

    /// @sa MultiDimAxisAny::GetLabel()
    virtual wxString GetLabel( unsigned int n ) const
    {
        return wxString::Format(_T("%d"), m_min + n * m_step );
    }

    /// @sa MultiDimAxis::DoGetValue()
    virtual Integral DoGetValue(unsigned int n) const
    {
        return m_min + n * m_step;
    }

protected:
    Integral m_min;
    Integral m_max;
    Integral m_step;
};

typedef MultiDimIntegralAxis<int> MultiDimIntAxis;
typedef MultiDimIntegralAxis<unsigned int> MultiDimUIntAxis;

/**
   Conversion helper for MultiDimTableN template classes.

   Implements convertion between ValueType and wxString.
   To support a new data type in the MultiDimTable
   one should specialise this template for the desired type.
   See int template specialisation for an example.

   One could use boost::lexical to implement a generic conversion
   using lexical casts (lexical cast - cast that uses standard iostream
   facilities as a conversion black box).
 */
template <typename ValueType>
class MultiDimTableTypeTraits
{
public:
    /// Convert value respresented by a string into ValueType.
    ValueType FromString( const wxString & str ) const
    {   return str;     }

    /// Create a string representation of a value
    wxString ToString( ValueType const & value ) const
    {   return value;   }
};

// Helper macro implementing MultiDimTableTypeTraits for a given integral type
#define MDTABLE_TTRAITS_INTEGRAL( ValueType, FromMethod, FromType, ErrFromValue ) \
template <>                                             \
class MultiDimTableTypeTraits<ValueType>                \
{                                                       \
public:                                                 \
    ValueType FromString( const wxString & str ) const  \
    {                                                   \
        FromType value;                                 \
        if( str.FromMethod( &value ) )                  \
            return value;                               \
        return ErrFromValue;                            \
    }                                                   \
    wxString ToString( ValueType value ) const          \
    {                                                   \
        wxString res;                                   \
        res << value;                                   \
        return res;                                     \
    }                                                   \
}

// sepcialisations of the MultiDimTableTypeTraits for some common types
MDTABLE_TTRAITS_INTEGRAL( int, ToLong, long, -1 );
MDTABLE_TTRAITS_INTEGRAL( unsigned int, ToULong, unsigned long, 0 );
MDTABLE_TTRAITS_INTEGRAL( long, ToLong, long, -1 );
MDTABLE_TTRAITS_INTEGRAL( unsigned long, ToULong, unsigned long, 0 );
MDTABLE_TTRAITS_INTEGRAL( double, ToDouble, double, -1 );

// this #if branch is for documentation purposes only, the code inside it is
// pseduo-code, see below for the real (but less clear) thing
#ifdef DOXYGEN

/**
   Type-safe N-dimensional table.

   There is no class MultiDimTableN in reality, only classes MultiDimTable0,
   MultiDimTable1, ... and so on up to MAX_MULTIDIMGRID_AXIS which is
   sufficiently large by default but may be predefined to be even larger if
   this is not enough.

   Template parameters are:
        - T the type of the grid values
        - VN the type of the values of the N-th axis
 */
template <typename T, typename V1, ... , typename VN>
class MultiDimTableN : public MultiDimTableAny
{
public:
    enum {
        /// Number of axis in the table
        AxisNumber = N
    };

    /**
       This function which must be overridden to provide read access to the
       table values.
     */
    virtual T GetValue(V1 a1, ..., VN an) const = 0;

    /**
       This function which must be overridden to provide write access to the
       table values.
     */
    virtual void SetValue(V1 a1, ..., VN an, const T& value) const = 0;


    virtual unsigned int GetDimension() const { return N; }

    /// for every M in 0..(N-1)
    virtual MultiDimAxis<VM> * GetAxisM() = 0;

protected:
    /// Implement base class pure virtual in terms of public GetValue()
    virtual boost::any DoGetValue(const Coords& coords) const
    {
        return GetValue(boost::any_cast<V1>(coords[0]), ...);
    }

    /// Implement base class pure virtual in terms of public SetValue()
    virtual void DoSetValue(const Coords& coords, const boost::any& value)
    {
        SetValue(any_cast<V1>(coords[0]), ..., any_cast<V1>(coords[N]),
                 any_cast<T>(value));
    }
private:
    /// internal instance of the converter object
    MultiDimTableTypeTraits<T> m_converter;

    /**
       @name String vs ValueType convertion helpers.
       Delegates the convertion between type ValueType and wxString
       to template class MultiDimTableTypeTraits<ValueType>
       To support new type one should instantiate the MultiDimTableTypeTraits
       for the type and implement the converting methods.
     */
    //@{
    virtual wxString ValueToString(const boost::any & value) const
    {
        try {
            return m_converter.ToString( boost::any_cast<T>( value ) );
        } catch( const boost::bad_any_cast & ) {
            return _T("invalid boost::any value type");
        }
    }
    virtual boost::any StringToValue(const wxString & str) const
    {
        return boost::any( static_cast<T>( m_converter.FromString( str ) ) );
    }
    //@}
};

/**
   Helper class to easily retrieve type information from a MultiDimTableN
   class.

   It retrieves the type of the Nth axis ot the table.

   It is specialized for every N in 1..MAX_MULTIDIMGRID_AXIS.
 */
template< class Table, int N >
struct MultiDimTableAxisValueType
{
    static unsigned int const N;
    typedef typename Table::AxisValueTypeN Type;
};

#else // !DOXYGEN

// helper macro used to generate Set/GetValue pseudo-code above
#define MDTABLE_PARAMS_(z, n, unused)                                   \
    BOOST_PP_COMMA_IF(n) boost::any_cast<V##n>(coords[n])               \

#define MDTABLE_DECLARE_GETAXIS_(z, n, unused)                          \
    virtual MultiDimAxis<V##n> * GetAxis##n() = 0;                      \

#define MDTABLE_SWITCH_GETAXIS_(z, n, unused)                           \
        case n: return GetAxis##n();                                    \

#define MDTABLE_AXISVALUETYPE_TYPEDEF_(z, n, unused)                    \
    typedef V##n AxisValueType##n;                                      \

// helper macro used to declare MultiDimTableN and MultiDimGridN classes for
// given N
#define MDTABLE_DECLARE_FOR_(z, n, unused)                                    \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
class MultiDimTable##n : public MultiDimTableAny                              \
{                                                                             \
public:                                                                       \
    typedef T ValueType;                                                      \
    static unsigned int const AxisNumber = n;                                 \
    BOOST_PP_REPEAT(n, MDTABLE_AXISVALUETYPE_TYPEDEF_, ~)  \
                                                                              \
    virtual T GetValue(BOOST_PP_ENUM_BINARY_PARAMS(n, V, v)) const = 0;       \
    virtual void                                                              \
    SetValue(BOOST_PP_ENUM_BINARY_PARAMS(n, V, v), const T& value) = 0;       \
                                                                              \
    virtual unsigned int GetDimension() const { return AxisNumber; }          \
                                                                              \
    BOOST_PP_REPEAT(n, MDTABLE_DECLARE_GETAXIS_, ~)  \
                                                                              \
    virtual MultiDimAxisAny * DoGetAxisAny(unsigned int nn)                   \
    {                                                                         \
        switch(nn)                                                            \
        {                                                                     \
            BOOST_PP_REPEAT(n, MDTABLE_SWITCH_GETAXIS_, ~) \
        }                                                                     \
        /* will never happen anyway */                                        \
        throw std::range_error("Invalid dimension");                          \
    }                                                                         \
                                                                              \
                                                                              \
protected:                                                                    \
    virtual boost::any DoGetValue(const Coords& coords) const                 \
    {                                                                         \
        return GetValue(BOOST_PP_REPEAT(n, MDTABLE_PARAMS_, ~));              \
    }                                                                         \
                                                                              \
    virtual void DoSetValue(const Coords& coords, const boost::any& value)    \
    {                                                                         \
        SetValue(BOOST_PP_REPEAT(n, MDTABLE_PARAMS_, ~),                      \
                 boost::any_cast<ValueType>(value));                          \
    }                                                                         \
private:                                                                      \
    MultiDimTableTypeTraits<T> m_converter;                                   \
    virtual wxString ValueToString(const boost::any & value) const            \
    {                                                                         \
        try {                                                                 \
            return m_converter.ToString( boost::any_cast<T>( value ) );       \
        } catch( const boost::bad_any_cast & ) {                              \
            return _T("invalid boost::any value type");                       \
        }                                                                     \
    }                                                                         \
    virtual boost::any StringToValue(const wxString & str) const              \
    {                                                                         \
        return boost::any( static_cast<T>( m_converter.FromString( str ) ) ); \
    }                                                                         \
};

// real code declaring MultiDimGridN classes
BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXIS, MDTABLE_DECLARE_FOR_, ~)

template< class Table, int M >
struct MultiDimTableAxisValueType;

#define MDTABLE_AXISVALUETYPE_FOR_(z, n, unused)                            \
                                                                            \
template< class Table >                                                     \
struct MultiDimTableAxisValueType< Table, n >                               \
{                                                                           \
    static unsigned int const N = n;                                        \
    typedef typename Table::AxisValueType##n Type;                          \
};

BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXIS, MDTABLE_AXISVALUETYPE_FOR_, ~)

#undef MDTABLE_AXISVALUETYPE_FOR_
#undef MDTABLE_DECLARE_GETAXIS_FOR_
#undef MDTABLE_SWITCH_GETAXIS_FOR_
#undef MDTABLE_AXISVALUETYPE_TYPEDEF_
#undef MDTABLE_DECLARE_FOR_
#undef MDTABLE_PARAMS_
#undef MDTABLE_TTRAITS_INTEGRAL

#endif // DOXYGEN/!DOXYGEN


/// @}

/**
   Adjustable Axis base class.

   Use this class as the base for you adjustable axis. This class defines
   type-safe methods to implement.

   This template uses BaseAxisType as its base class. The only constraint i that
   this base class has to have a constructor taking axis name as the only
   parameter.

   @param AdjustControl type of the adjustment control you will use for the axis
   @param BaseAxisType base class to use for the axis
 */
template<class AdjustControl, class BaseAxisType = MultiDimAxisAny>
class MultiDimAdjustableAxis : public BaseAxisType
{
    BOOST_STATIC_ASSERT((boost::is_base_of<MultiDimAxisAny,BaseAxisType>::value
                      || boost::is_same<MultiDimAxisAny,BaseAxisType>::value));

public:
    /// Adjustment control type
    typedef AdjustControl AxisAdjustControl;

    /**
       Redirects to type-safe method DoGetAdjustControl()

       Do not override this method, override DoGetAdjustControl instead.
     */
    virtual wxWindow * GetAdjustControl( MultiDimGrid & grid,
                                         MultiDimTableAny & table )
    {
        return DoGetAdjustControl( grid, table );
    }

    /// Redirects to the type-safe DoApplyAdjustment()
    virtual
    bool ApplyAdjustment( wxWindow * adjustWin,
                          unsigned int n )
    {
        AxisAdjustControl * win
            = dynamic_cast< AxisAdjustControl * >( adjustWin );
        wxASSERT_MSG( !adjustWin || win,
            _T("The axis adjustment control given has incorrect type") );
        return DoApplyAdjustment( win, n );
    }

    /// Redirects to the type-safe DoRefreshAdjustment()
    virtual
    bool RefreshAdjustment( wxWindow * adjustWin,
                            unsigned int n )
    {
        AxisAdjustControl * win
            = dynamic_cast< AxisAdjustControl * >( adjustWin );
        wxASSERT_MSG( !adjustWin || win,
            _T("The axis adjustment control given has incorrect type") );
        return DoRefreshAdjustment( win, n );
    }

protected:
    /**
       Default constructor.
       This ctor is protected because this class has to be derived from.
     */
    MultiDimAdjustableAxis( wxString const & name )
    : BaseAxisType(name)
    {
    }

    /**
       Type-safe method to override in the derived user class.

       @sa MultiDimAxisAny::GetAdjustControl()
     */
    virtual
    AxisAdjustControl * DoGetAdjustControl( MultiDimGrid & grid,
                                            MultiDimTableAny & table ) = 0;

    /**
       Type-safe method to override in the derived user class.

       @sa MultiDimAxisAny::ApplyAdjustment()
     */
    virtual
    bool DoApplyAdjustment( AxisAdjustControl * adjustWin,
                            unsigned int n ) = 0;

    /**
       Type-safe method to override in the derived user class.

       @sa MultiDimAxisAny::RefreshAdjustment()
     */
    virtual
    bool DoRefreshAdjustment( AxisAdjustControl * adjustWin,
                              unsigned int n ) = 0;
};

#endif // MULTIDIMGRID_HPP_

