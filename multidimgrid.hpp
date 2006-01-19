/**
   @file multidimgrid.hpp
   @brief A control for editing multidimensional data and supporting classes.
   @author Vadim Zeitlin
   @version $Id: multidimgrid.hpp,v 1.1.2.2 2006-01-19 02:21:10 zeitlin Exp $
   @date 2005-10-19

   The MultiDimGrid control can be used to edit N-dimensional data tables.
   The data is represented by MultiDimTable which is a table indexed by
   arbitrary many axis. Each axis has a finite, discrete number of values
   which can be
        - either of string type, in which case they must be listed explicitely
        - or of int type in which case the values are specified by a range
          min..max with the given step (1 by default)

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
 */

#ifndef MULTIDIMGRID_HPP_
#define MULTIDIMGRID_HPP_

#include "config.hpp"

#include <wx/control.h>
#include <wx/scrolwin.h>
#include <wx/grid.h>

#include <vector>
#include <stdexcept>


#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>


/// Some forward declarations to avoid unnecessary header inclusions
class WXDLLEXPORT wxBoxSizer;
class WXDLLEXPORT wxStaticBoxSizer;
class WXDLLEXPORT wxFlexGridSizer;
class WXDLLEXPORT wxChoice;


/**
   @def MAX_MULTIDIMGRID_AXISCARDINALITY

   This constant specifies the maximal number of arguments to
   MultiDimEnumAxis::MakeArray() method.

   Note that this doesn't affect at all the max number of elements in the
   array passed to MultiDimEnumAxis ctor which is unlimited.
 */
#ifndef MAX_MULTIDIMGRID_AXISCARDINALITY
    #define MAX_MULTIDIMGRID_AXISCARDINALITY 10
#endif

/**
   @def MAX_MULTIDIMGRID_AXIS

   This constant specifies the number of type safe MultiDimGridN classes
   declared in this header. It may be set in the compiler options to any
   values but the compilation may be slower if it is very large.

   Note that this doesn't affect at all the number of axis in MultiDimGridAny
   which is unlimited.
 */
#ifndef MAX_MULTIDIMGRID_AXIS
    #define MAX_MULTIDIMGRID_AXIS 10
#endif

// forward declarations
class MultiDimAxisAny;
class MultiDimTableAny;
class MultiDimGridAny;

// ----------------------------------------------------------------------------
// type-unsafe classes using boost::any
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
       @return the control to select value of this axis, must not be @c NULL
     */
    virtual wxWindow *GetChoiceControl(MultiDimGridAny *grid) const;

    /**
       Create the control to adjust the values shown for this axis in the grid.

       Unlike GetChoiceControl(), this control is used for the axis only when
       it is selected as one of the two axis to be used in the grid. This
       control can be used to allow the user to configure the values shown in
       the grid dynamically (e.g. show only a subset of them: imagine an
       integer axis, then we could have an option to show only odd or only
       even integers).

       The default version just returns @c NULL meaning that no run-time
       configuration is possible. If a control is created and returned by this
       function, the UpdateGridAxis() method of the @a grid object should be
       called whenever something changes in it

       @param grid the grid control this axis is used with
       @return the control shown for a selected axis or @c NULL if none
     */
    virtual wxWindow *GetAdjustControl(MultiDimGridAny *grid) const
    {
        return NULL;
    }

    /**
       Reset any possible adjustment done to the axis by its adjustment control

       This method is called by the MultiDimGrid when an axis looses selection
       and isn't shown no more in the data grid. The grid resets it allow
       the user to choose value for the axis from the set of not-filtered values

       @param win   Window pointer previously created by GetAdjustControl()
                    It is passed to the user only by convenience.
       @sa GetAdjustControl()
     */
    virtual void ResetAdjustment(wxWindow * WXUNUSED(win)) {}

    /**
       Read and apply adjustment from its adjustment control.

       Method is called once the axis becomes selected and is now shown
       in the data grid. This method should trigger the adjustment window to
       reapply any filters needed to properly show axis values in the grid.
       Adjustment filters are persistent in the sense that adjustments
       are not discarded between the axis selections. Therefore need for the method.
       Typically the user will implement AdjustmentControl as a widget
       with some user input fields. This method should force the window
       to reread those input fields and reapply adjustment filters once the axis
       is reselected.

       @sa GetAdjustControl()

       @param win axis adjustment window previously constructed by GetAdjustControl()
     */
    virtual void ApplyAdjustment(wxWindow * WXUNUSED(win)) {}

    /**
       Virtual destructor for the base class.

     */
    virtual ~MultiDimAxisAny() { }

private:
    const wxString m_name;

    DECLARE_NO_COPY_CLASS(MultiDimAxisAny)
};


/**
   Class helper for an adjustable axis implementation.

   To implement an adjustable axis, derive this class and implement
   its pure-virtual methods and a couple of non-pure-virtual ones:
     - DoGetAdjustControl() - method that creates the adjustment window
     - DoApplyAdjustment() - reads adjustment control values and applies the adjustment
     - DoResetAdjustment() - resets any adjustments previously applied to the axis
     - GetCardinality() - returns the total number of values if not adjusted
                          (@sa IsAdjusted()), or the number of filtered ones
                          if adjusted. Note that as in
                          MultiDimAxisAny::GetCardinality()
                          it has to return a value greater or equal to 1
     - GetValue() - user should override it to return the corresponding
                    filtered value when axis is adjusted (@sa IsAdjusted())
     - GetLabel() - user should override it to return the corresponding
                    filtered value label when axis is adjusted (@sa IsAdjusted())
 */
template <class AxisAdjustControl>
class MultiDimAdjustableAxisAny : public MultiDimAxisAny
{
public:
    /**
       Call to DoGetAdjustControl and  save window pointer into m_win.

       Do not override this method, override DoGetAdjustControl instead.
     */
    virtual wxWindow *GetAdjustControl( MultiDimGridAny *grid ) const
    {
        return DoGetAdjustControl( grid );
    }

    /**
       This method sets internal adjustment flag to true, and then forward
       to the user overriden DoApplyAdjustment method.

       Do not override this method, override DoApplyAdjustment instead.
     */
    virtual void ApplyAdjustment( wxWindow * win )
    {
        wxASSERT_MSG( !win || dynamic_cast< AxisAdjustControl * >( win ),
            _T("The axis adjustment control given has incorrect type") );
        DoApplyAdjustment( static_cast< AxisAdjustControl * >( win ) );
    }

    /**
       This method calls the user overriden DoResetAdjustment method
       and then sets internal adjustment flag to false.

       Do not override this method, override DoResetAdjustment instead.
     */
    virtual void ResetAdjustment( wxWindow * win )
    {
        wxASSERT_MSG( !win || dynamic_cast< AxisAdjustControl * >( win ),
            _T("The axis adjustment control given has incorrect type") );
        DoResetAdjustment( static_cast< AxisAdjustControl * >( win ) );
    }

protected:
    /**
       Default empty constructor.

       This ctor is protected because this class can't be instantiated.
     */
    MultiDimAdjustableAxisAny( wxString const & name )
        : MultiDimAxisAny(name)
    {
    }

    /**
       The method to override in the derived user class.

       @sa MultiDimAxisAny::GetAdjustControl()
     */
    virtual
    AxisAdjustControl * DoGetAdjustControl( MultiDimGridAny *grid ) const = 0;

    /**
       Method to be overrided in the user class if needed.

       Should reset the values in the axis after its deselection by the user
       and loss of any adjustments.
     */
    virtual void DoResetAdjustment( AxisAdjustControl * win ) = 0;

    /**
       The method to override in the derived user class.

       @sa MultiDimAxisAny::ApplyAdjustment()
     */
    virtual
    void DoApplyAdjustment( AxisAdjustControl * win ) = 0;
};


/**
   The table abstracts the data shown in and edited by the grid.

   It is a bridge between the internal data representation in the application
   and the GUI control, allowing to clearly separate them.

   It is an ABC which must be implemented to provide access to real data.

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
       General accessor for table value.

       Throws an std::exception if the array cardinality is different from
       GetDimension() or boost::bad_any_cast exception if the array arguments
       have wrong types.

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

       Throws an std::exception if the array cardinality is different from
       GetDimension() or boost::bad_any_cast exception if the array arguments
       have wrong types.

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
};


/**
   The control allowing editing N-dimensional data table.

   This control is not type-safe and should be only used if the grid structure
   can be only determined dynamically. Otherwise, a type-safe MultiDimGridN
   control declared below should be used.
 */
class MultiDimGridAny : public wxScrolledWindow,
                        private wxGridTableBase
{
public:
    /// Container of (pointers to) axis objects.
    typedef std::vector< boost::shared_ptr<MultiDimAxisAny> > Axis;


    /**
       Default constructor, use Create() to really create the control.
     */
    MultiDimGridAny()
    {
        Init();
    }

    /**
       Creates a new grid control.

       @param parent the parent window of the control, must not be @c NULL
       @param table the table to use, must not be @c NULL
       @param axis the axis to be used for displaying/editing table (number of
                                axis must be the same as @a table dimension)
       @param id the control id for wx
       @param pos the initial control position
       @param size the initial control size
     */
    MultiDimGridAny(wxWindow *parent,
                    boost::shared_ptr<MultiDimTableAny> const & table,
                    Axis const & axis,
                    wxWindowID id = wxID_ANY,
                    wxPoint const & pos = wxDefaultPosition,
                    wxSize const & size = wxDefaultSize)
    {
        Init();

        Create(parent, table, axis, id, pos, size);
    }

    /**
       Virtual destructor.
     */
    virtual ~MultiDimGridAny()
    {
    }

    /**
       Creation function which must be called if the object had been
       consturcted using the default constructor.

       @param parent the parent window of the control, must not be @c NULL
       @param table the table to use, must not be @c NULL
       @param axis the axis to be used for displaying/editing table (number of
                                    axis must be the same as @a table dimension)
       @param id the control id for wx
       @param pos the initial control position
       @param size the initial control size
       @return true if the window was created successfully, false otherwise
     */
    bool Create(wxWindow *parent,
                boost::shared_ptr<MultiDimTableAny> const & table,
                Axis const & axis,
                wxWindowID id = wxID_ANY,
                wxPoint const & pos = wxDefaultPosition,
                wxSize const & size = wxDefaultSize);


    /**
       Return the axis for n-th dimension.

       Throws std::range_error if the dimension is out of range.

       @param n the index of the axis (between 0 and N-1 inclusive)
     */
    const MultiDimAxisAny& GetAxis( unsigned int n ) const;

    /**
       Return the axis index by its name.

       Returns -1 (wxAXIS_NOT_FOUND) if no such axis found

       @param axisName the name of the axis.
     */
    int GetAxisIndexByName( const wxString & axisName );

    /**
       Called when the value of the given axis changes.

       This should be only called for the axis which is currently not shown in
       the grid as there is no any fixed value for the grid axis. It is meant
       to be called from MultiDimAxisAny axis value selection control.

       @sa MultiDimAxisAny::GetChoiceControl()

       @param axis the name of the axis whose value has changed
       @param sel index of the new axis value
     */
    void FixAxisValue(const wxString& axis, const boost::any & sel);

    /**
       Update the columns or rows shown in the grid for the given axis.

       This should be only called for one of the two axis which is currently
       shown in the grid. It is meant to be called from MultiDimAxisAny
       axis value adjusting control, e.g. when the number of possible axis
       values changes.

       @sa MultiDimAxisAny::GetAdjustControl()

       @param axis the name of the axis which changed
     */
    void UpdateGridAxis(const wxString& axis);

    /**
       Select axis to be shown in the grid part of the widget.

       Specify two different axes to be shown in the data grid. If a non-existing
       axis index specified or both indexes are the same function returns false.

       @param firstAxis Index of the X axis
       @param secondAxis Index of the Y axis
       @return true on success
     */
    bool SetAxisSelection( int firstAxis, int secondAxis );

    /**
       Refresh underlying data table content.

       Helper function, that refreshes the data table to reflect any data change.
       In the current architechture axis are read-only, therefore there is no
       RefreshAxis() function in the interface.
     */
    void RefreshTableData();

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
    /**
       Build the vector of axis to be used for this grid.

       All parameters after the first one must be of type
       "boost::shared_ptr<MultiDimAxisAny> *" and there must be exactly
       N of them.

       @param N the number of pointers following it
     */
    static Axis
    MakeAxisVector(unsigned int N, ...)
    {
        Axis axis(N);

        va_list argptr;
        va_start(argptr, N);

        for ( unsigned n = 0; n < N; ++n )
        {
            void *p = va_arg(argptr, void *);
            axis[n] = boost::shared_ptr<MultiDimAxisAny>(
                static_cast<MultiDimAxisAny *>(p));
        }

        va_end(argptr);

        return axis;
    }

protected:
    /// Common part of all ctors
    void Init();

    /// Shared pointer to the data table.
    boost::shared_ptr<MultiDimTableAny> m_table;

    /// Array (std::vector) of shared pointer to axes.
    Axis m_axis;

    /// Cache variable - number of dimensions.
    unsigned int m_dimension;

    /// Top-level sizer of the widget
    wxBoxSizer * m_gridSizer;

    /// sizer containing axis selection controls (X and Y) and axis controls
    wxFlexGridSizer * m_axisSizer;


    /// Array of boost::any values
    typedef MultiDimTableAny::Coords Coords;
    /**
       Array of current axis fixed values, changed every time user changes
       fixed value for an axis.
     */
    Coords m_axisFixedValues;
    /**
       Array of axis coordinates. Used only to pass current coordinates
       to the underlying data table. Note that when displaying table the
       values for selected axes change as we walk through the grid cells
       to retreive its values.
     */
    Coords m_axisFixedCoords;

    /// Index of the selected axis to be displayed as the X axis in the grid
    int m_firstGridAxis;

    /// Index of the selected axis to be displayed as the Y axis in the grid
    int m_secondGridAxis;

    /// Creates axis selection controls for axes X and Y
    wxChoice * CreateAxisSelection( int id, wxString const & label,
                                    wxColour const & selectedColour );

    /// Places the axis label control into the widget
    void SetAxisLabel( int axisId, wxWindow * newWin );

    /// Places the axis fixed value choice control into the widget
    void SetAxisChoiceControl( int axisId, wxWindow * newWinm );

    /// Places the axis value range selection control into the widget
    void SetAxisAdjustControl( int axisId, wxWindow * newWin );

    /**
       Selects axes changing corresponding wxChoice selections
       and refreshing the data grid, etc.
     */
    void DoSetAxisSelection();

private:
    /// Show axis adjustment control (if any) under the given axis selection (X, Y)
    void ShowAxisAdjustControls();

    /**
       @name Various wx components of the widget
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

    /// Array of axis adjust windows, could be null if an axis does not allow adjustments
    Windows m_axisAdjustWins;

    //@}

    /// Color used to highlight X axis selections
    wxColour m_selectedFirstColour;

    /// Color used to highlight Y axis selections
    wxColour m_selectedSecondColour;

private:
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

    /**
       Helper function used by SetValue() and GetValue() functions
       to fill the private coordinates vector with correct values.

       @sa MultiDimAxisAny::GetValue()
       @sa MultiDimAxisAny::SetValue()
     */
    void PrepareFixedCoords( int row, int col );

private:
    /// Helper event hook monitoring axis selection changes
    void OnAxisChoiceSwitch( wxCommandEvent & event );

    DECLARE_NO_COPY_CLASS( MultiDimGridAny )
    DECLARE_EVENT_TABLE()

};

// ----------------------------------------------------------------------------
// type-safe classes
// ----------------------------------------------------------------------------

/**
   Represents an axis whose values is a set of strings.

   The string values typically correspond to an enum internally, hence the
   name of the class.

   Template parameter E is the enum type which corresponds to the values of
   this axis.
 */
template <typename E>
class MultiDimEnumAxis : public MultiDimAxisAny
{
public:
    /// Type of the values for this axis
    typedef E ValueType;


    /**
       Creates the axis with the given set of possible values.

       @param name the name of the axis, for GetName() implementation
       @param values all possible values for this axis
     */
    MultiDimEnumAxis(const wxString& name, const wxArrayString& values)
        : MultiDimAxisAny(name),
          m_values(values)
    {
    }

#ifdef DOXYGEN
    /**
       Helper for passing the values to constructor argument.

       This function takes N strings and returns an array with N elements.

       Note that in reality there is not a single function but a family of
       overloaded functions taking up to MAX_MULTIDIMGRID_AXISCARDINALITY
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
    BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXISCARDINALITY, MAKE_ARRAY_n, ~)

    #undef MAKE_ARRAY_n
#endif // DOXYGEN/!DOXYGEN

    virtual unsigned int GetCardinality() const { return m_values.size(); }
    virtual wxString GetLabel(unsigned int n) const { return m_values[n]; }
    virtual boost::any GetValue(unsigned int n) const
    {
        return boost::any(static_cast<ValueType>(n));
    }

private:
    wxString m_name;
    wxArrayString m_values;
};

/**
   Represents an axis whose values is a range of integeres.
 */
class MultiDimIntAxis : public MultiDimAxisAny
{
public:
    /// Type of the values for this axis
    typedef int ValueType;


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
    MultiDimIntAxis(const wxString& name,
                    int minValue,
                    int maxValue,
                    int step = 1)
        : MultiDimAxisAny(name),
          m_min(minValue),
          m_max(maxValue),
          m_step(step)
    {
        wxASSERT_MSG( minValue <= maxValue,
                      _T("minValue have to less or equal to maxValue") );
        wxASSERT_MSG( step >= 1,
                      _T("step has to be at least 1") );
    }


    virtual unsigned int GetCardinality() const
    {
        return ( m_max - m_min + m_step - 1 ) / m_step;
    }

    virtual wxString GetLabel(unsigned int n) const
    {
        return wxString::Format(_T("%d"), m_min + n * m_step);
    }

    virtual boost::any GetValue(unsigned int n) const
    {
        return boost::any( static_cast<int>( m_min + n * m_step ) );
    }

protected:
    int m_min,
        m_max,
        m_step;
};

/**
   Conversion helper for MultiDimTableN template classes.

   Implements convertion between ValueType and wxString.
   To support a new data type in the MultiDimTable
   one should specialise this template for the desired type.
   See int template specialisation for an example.
 */
template <typename T>
class MultiDimTableTypeTraits;

// this #if branch is for documentation purposes only, the code inside it is
// pseudo-code, see below for the real (but less clear) thing
#ifdef DOXYGEN

/**
   An example of MultiDimTableTypeTraits specification for the int type.

   It shows also the contract for every specialisation of the helper.

   One could use boost::lexical to implement a generic conversion
   using lexical casts (lexical cast - cast that uses standard iostream
   facilities as a conversion black box).
 */
template <>
class MultiDimTableTypeTraits<int>
{
public:
    /// Convert value respresented by a string into ValueType.
    int FromString( const wxString & str ) const
    {
        long value;
        if( str.ToLong( &value ) )
            return value;
        return -1;
    }
    /// Create a string representation of a value
    wxString ToString( int value ) const
    {
        wxString res;
        res << value;
        return res;
    }
};

#else // !DOXYGEN

/// Helper macro implementing MultiDimTableTypeTraits for a given integral type
#define MULTI_DTABLE_TTRAITS_INTEGRAL( ValueType, FromMethod, FromType, ErrFromValue ) \
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

MULTI_DTABLE_TTRAITS_INTEGRAL( int, ToLong, long, -1 );
MULTI_DTABLE_TTRAITS_INTEGRAL( unsigned int, ToULong, unsigned long, 0 );
MULTI_DTABLE_TTRAITS_INTEGRAL( long, ToLong, long, -1 );
MULTI_DTABLE_TTRAITS_INTEGRAL( unsigned long, ToULong, unsigned long, 0 );
MULTI_DTABLE_TTRAITS_INTEGRAL( double, ToDouble, double, -1 );

#endif // DOXYGEN/!DOXYGEN


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
   Multi-dimensional grid allowing to edit the values of the specified type
   depending on the given axis.

   There is no class MultiDimGridN in reality, only classes MultiDimGrid0,
   MultiDimGrid1, ... and so on up to MAX_MULTIDIMGRID_AXIS which is
   sufficiently large by default but may be predefined to be even larger if
   this is not enough.

   Template parameters are:
        - T the type of the grid values
        - AN the type of the N-th axis (@b not its value) which should have a
             default ctor and a nested ValueType typedef for the type it uses
 */
template <typename T, typename A1, ... , typename AN>
class MultiDimGridN : public MultiDimGridAny
{
public:
    /// The type of the table class which must be used with this grid.
    typedef MultiDimTableN<T,
                           typename A1::ValueType,
                           ...,
                           typename AN::ValueType> Table;

    /**
       Creates a new grid control.

       @param parent the parent window of the control, must not be @c NULL
       @param table the table to use, must not be @c NULL
       @param id the control id for wx
       @param pos the initial control position
       @param size the initial control size
     */
    MultiDimGridN(wxWindow *parent,
                  boost::shared_ptr<Table> const & table,
                  wxWindowID id = wxID_ANY,
                  wxPoint const & pos = wxDefaultPosition,
                  wxSize const & size = wxDefaultSize)
        : MultiDimGridAny(parent,
                          table,
                          MakeAxisVector(new A1, ..., new AN),
                          id,
                          pos,
                          size)
    {
    }
};

#else // !DOXYGEN

// helper macro used to generate Set/GetValue pseudo-code above
#define MDGRID_PARAMS_(z, n, unused)                                          \
    BOOST_PP_COMMA_IF(n) boost::any_cast<V##n>(coords[n])                     \

// helper macro used to declare MultiDimTableN and MultiDimGridN classes for
// given N
#define MDGRID_DECLARE_FOR_(z, n, unused)                                     \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
class MultiDimTable##n : public MultiDimTableAny                              \
{                                                                             \
public:                                                                       \
    typedef T ValueType;                                                      \
                                                                              \
    virtual T GetValue(BOOST_PP_ENUM_BINARY_PARAMS(n, V, v)) const = 0;       \
    virtual void                                                              \
    SetValue(BOOST_PP_ENUM_BINARY_PARAMS(n, V, v), const T& value) = 0;       \
                                                                              \
    virtual unsigned int GetDimension() const { return n; }                   \
                                                                              \
protected:                                                                    \
    virtual boost::any DoGetValue(const Coords& coords) const                 \
    {                                                                         \
        return GetValue(BOOST_PP_REPEAT(n, MDGRID_PARAMS_, ~));               \
    }                                                                         \
                                                                              \
    virtual void DoSetValue(const Coords& coords, const boost::any& value)    \
    {                                                                         \
        SetValue(BOOST_PP_REPEAT(n, MDGRID_PARAMS_, ~),                       \
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
};                                                                            \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename A)>                    \
class MultiDimGrid##n : public MultiDimGridAny                                \
{                                                                             \
public:                                                                       \
    typedef MultiDimTable##n                                                  \
            <                                                                 \
                T,                                                            \
                BOOST_PP_ENUM_BINARY_PARAMS(n, typename A, ::ValueType        \
                                            BOOST_PP_INTERCEPT)               \
            > Table;                                                          \
                                                                              \
    MultiDimGrid##n(wxWindow *parent,                                         \
                    boost::shared_ptr<Table> const & table,                   \
                    wxWindowID id = wxID_ANY,                                 \
                    wxPoint const & pos = wxDefaultPosition,                  \
                    wxSize const & size = wxDefaultSize)                      \
        : MultiDimGridAny(parent,                                             \
                          table,                                              \
                          MakeAxisVector(n, BOOST_PP_ENUM_PARAMS(n, new A)),  \
                          id,                                                 \
                          pos,                                                \
                          size)                                               \
    {                                                                         \
    }                                                                         \
};

// real code declaring MultiDimGridN classes: note that it doesn't make sense
// to use MultiDimGridN for N < 3
BOOST_PP_REPEAT_FROM_TO(3, MAX_MULTIDIMGRID_AXIS, MDGRID_DECLARE_FOR_, ~)

#undef MDGRID_DECLARE_FOR_
#undef MDGRID_PARAMS_
#undef MULTI_DTABLE_TTRAITS_INTEGRAL

#endif // DOXYGEN/!DOXYGEN

#endif // MULTIDIMGRID_HPP_

