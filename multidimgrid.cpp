/**
   @file multidimgrid.cpp
   @brief A control for editing multidimensional data and supporting classes.
   @author Vadim Zeitlin
   @version $Id: multidimgrid.cpp,v 1.1.2.6 2006-04-10 20:26:03 etarassov Exp $
   @date 2005-10-19

   Implementation of multidimensional data editor control
*/
#include "multidimgrid.hpp"
#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/window.h>
#include <wx/colour.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include <wx/intl.h>

#include <sstream>

// ----------------------------------------------------------
// MultiDimAxisAnyChoice implementation
// ----------------------------------------------------------
BEGIN_EVENT_TABLE(MultiDimAxisAnyChoice, wxChoice)
    EVT_CHOICE(wxID_ANY, MultiDimAxisAnyChoice::OnSelectionChange)
END_EVENT_TABLE()

MultiDimAxisAnyChoice::MultiDimAxisAnyChoice
(
    MultiDimAxisAny const & axis,
    MultiDimGrid & grid
)
: wxChoice(&grid, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString()),
  m_axis(axis)
{
    PopulateChoiceList();
}

void MultiDimAxisAnyChoice::PopulateChoiceList()
{
    int selection = GetSelection();
    wxString selectedLabel = selection != wxNOT_FOUND ?
                             GetString(selection) : _T("");

    Clear();

    unsigned int const numAxis = m_axis.GetCardinality();
    wxString label;
    bool selected = false;
    for ( unsigned int i = 0; i < numAxis; ++i )
    {
        label = m_axis.GetLabel( i );
        Append( label );
        if( label == selectedLabel )
        {
            SetSelection( i );
            GetGrid().FixAxisValue( m_axis.GetName(),  m_axis.GetValue( i ) );
            selected = true;
        }
    }

    if ( !selected && numAxis > 0 )
    {
        SetSelection( 0 );
        GetGrid().FixAxisValue( m_axis.GetName(),  m_axis.GetValue( 0 ) );
    }
}

void MultiDimAxisAnyChoice::OnSelectionChange( wxCommandEvent & WXUNUSED(event) )
{
    SelectionChanged();
}

void MultiDimAxisAnyChoice::SelectionChanged()
{
    unsigned int sel = GetSelection();

    wxASSERT_MSG( sel >= 0 && sel < m_axis.GetCardinality(),
                  _T("The axis and its choice control are out of sync") );

    GetGrid().FixAxisValue( m_axis.GetName(),  m_axis.GetValue( sel ) );
}

// -------------------------------------------------------
// Class MultiDimAxiAny
// -------------------------------------------------------
wxWindow * MultiDimAxisAny::GetChoiceControl( MultiDimGrid & grid,
                                              MultiDimTableAny & table )
{
    return new MultiDimAxisAnyChoice(*this, grid);
}

void MultiDimAxisAny::UpdateChoiceControl( wxWindow & choiceControl ) const
{
    wxCHECK_RET( dynamic_cast<MultiDimAxisAnyChoice*>(&choiceControl),
                 _T("Wrong choice Control type") );
    static_cast<MultiDimAxisAnyChoice&>(choiceControl).PopulateChoiceList();
}

// -------------------------------------------------------
// Class MultiDimGridGrid
// -------------------------------------------------------
// wxGrid customised to the needs of MultiDimGrid.
//  - It tunes the scrolling behaviour of the standard wxGrid widget.
//  - Widget size could be made as small as possible leaving only labels,
//    one row and one column visible.
// -------------------------------------------------------

class MultiDimGridGrid : public wxGrid
{
public:
    MultiDimGridGrid(
        wxWindow* parent,
        wxWindowID id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxWANTS_CHARS,
        const wxString& name = wxPanelNameStr )
    : wxGrid( parent, id, pos, size, style, name )
    {
    }
    virtual wxSize DoGetBestSize() const;
    virtual ~MultiDimGridGrid() {}
};


// -------------------------------------------------------
// MultiDimGridGrid implementation
// -------------------------------------------------------
wxSize MultiDimGridGrid::DoGetBestSize() const
{
    // TODO(ET): don't multiply by two -- take LabelWidth + Colum_0_Width
    // Its not already done because of a strange wxGrid behaviour - when
    // GetColumnWidth() is called it tryes to calculate sizes of all the columns
    // and uses som kind of internal cache which is not being properly purged
    // when the underlying table data provider changes. It results in a number
    // of alerts popping up after changing axis selection. That's why simply
    // multiplying by 2 seems to be the best temporary workaround.
    int width = 2 * m_rowLabelWidth;
    int height = 2 * m_colLabelHeight;

    // default minimal size of a widget - values taken from one of wx headers
    width = wxMax( width, 100 );
    height = wxMax( height, 100 );

    // NOTE(ET): does it really do anything usefull?
    // The following is from the wx header file for wxScrollWindow:
    // Round up to a multiple the scroll rate NOTE: this still doesn't get rid
    // of the scrollbars, is there any magic incantaion for that?
    int xpu, ypu;
    GetScrollPixelsPerUnit(&xpu, &ypu);
    if (xpu)
        width  += 1 + xpu - (width  % xpu);
    if (ypu)
        height += 1 + ypu - (height % ypu);

    return wxSize(width, height);
}

/**
   Table refresh guard classes.

   Needed to eliminate unwanted table data refreshes. It is implemented
   through counting of internal variable that triggers update when zeroed,
   meaning that every guard has released the counter and it is time to do the
   update which will take into accounts all the changes.
 */
class GridRefreshTableDataGuard
{
public:
    /// Construct guard for the counter, and use releaser at last exit
    GridRefreshTableDataGuard( MultiDimGrid & grid )
    : m_grid( &grid )
    {   ++m_grid->m_tableDataRefreshCounter;    }

    /// Free the guard. Its automatically called from the destructor
    void Release()
    {
        MultiDimGrid * grid = m_grid;
        m_grid = NULL;
        if( grid && !(--grid->m_tableDataRefreshCounter) )
            grid->DoRefreshTableData();
    }

    /// Automatically call Release()
    virtual ~GridRefreshTableDataGuard()
    {   Release(); }

private:
    MultiDimGrid * m_grid;

    DECLARE_NO_COPY_CLASS( GridRefreshTableDataGuard )
};

// ---------------------------------------
// Implementation of class MultiDimGrid
// ---------------------------------------

// Two values to distinguish between X axis and Y axis
enum {
    ID_FIRST_AXIS_CHOICE = wxID_HIGHEST + 1,
    ID_SECOND_AXIS_CHOICE
};

BEGIN_EVENT_TABLE(MultiDimGrid, wxScrolledWindow)
    EVT_CHOICE ( ID_FIRST_AXIS_CHOICE,  MultiDimGrid::OnSwitchSelectedAxis )
    EVT_CHOICE ( ID_SECOND_AXIS_CHOICE, MultiDimGrid::OnSwitchSelectedAxis )
    EVT_CHECKBOX ( wxID_ANY, MultiDimGrid::OnAxisVariesToggle )
END_EVENT_TABLE()

// Some constants to describe various element positions
enum
{
    // the row for the X axis selection
    MDGRID_AXIS_X_ROW = 0,
    // the row for the Y axis selection
    MDGRID_AXIS_Y_ROW = 1,
    // the row separating axis selections from axis controls
    MDGRID_AXIS_SEPARATOR_ROW = 2,
    // first row for the axis controls
    MDGRID_AXIS_ROW = 3,
    // column for the labels
    MDGRID_LABEL_COL = 0,
    // column for the axis choice controls or axis selection dropdown
    MDGRID_CHOICE_COL = 1,
    // horizontal span for the axis choice controls or axis selection dropdown
    MDGRID_CHOICE_HSPAN = 1,
    // column for the axis adjustment controls
    MDGRID_ADJUST_COL = 0,
    // horizontal span for the axis adjustment controls
    MDGRID_ADJUST_HSPAN = 3,
    // column for checkbox controlling product dependency on the axis
    MDGRID_VARIES_COL = 2,
    // sizer vertical cell spacing
    MDGRID_SIZER_VGAP = 4,
    // sizer horizontal cell spacing
    MDGRID_SIZER_HGAP = 8
};

namespace
{
/// Helper functions for an axis row calculation
///@{
inline unsigned int MDGridGetAxisLabelRow( unsigned int n )
{   return MDGRID_AXIS_ROW + n * 2; }
inline unsigned int MDGridGetAxisChoiceRow( unsigned int n )
{   return MDGRID_AXIS_ROW + n * 2; }
inline unsigned int MDGridGetAxisVariesRow( unsigned int n )
{   return MDGRID_AXIS_ROW + n * 2; }
inline unsigned int MDGridGetAxisAdjustRow( unsigned int n )
{   return MDGRID_AXIS_ROW + n * 2 + 1; }
///@{
}

void MultiDimGrid::Init()
{
    m_firstGridAxis = wxNOT_FOUND;
    m_secondGridAxis = wxNOT_FOUND;

    // setting default colour to both X and Y axis
    m_selectedFirstColour = GetForegroundColour();
    m_selectedSecondColour = GetForegroundColour();

    // intialize refresh counter
    m_tableDataRefreshCounter = 0;
}

bool
MultiDimGrid::Create(wxWindow *parent,
                        boost::shared_ptr<MultiDimTableAny> const & atable,
                        wxWindowID id,
                        wxPoint const & pos,
                        wxSize const & size)
{
    // setting self as a wxPanel
    // Note: wxVSCROLL does not appear to influent the wxPanel behaviour under msw
    // leaving it only for other platforms
    wxPanel::Create( parent, id, pos, size, wxWANTS_CHARS | wxVSCROLL );

    // setting internal variables
    m_table = atable;
    m_dimension = m_table->GetDimension();

    // pospone the table data refresh until we exit Create() function
    // being sure that we are ready for a refresh
    GridRefreshTableDataGuard guard( *this );

    m_axis.clear();
    m_axis.resize( m_dimension, AxisPtr() );
    for( unsigned int a = 0; a < m_dimension; ++a )
    {
        m_axis[a] = AxisPtr( m_table->GetAxisAny(a) );
    }

    wxASSERT_MSG( atable->GetDimension() == m_axis.size(),
            _T("Table size and number of axis do not match") );

    // initializing fixed values with empty ones
    m_axisFixedValues.resize( m_dimension, boost::any() );
    m_axisFixedCoords.resize( m_dimension, boost::any() );

    // reserve space in axis choice control containers of the widget
    m_axisLabels.resize( m_dimension, NULL );
    m_axisChoiceWins.resize( m_dimension, NULL );
    m_axisAdjustWins.resize( m_dimension, NULL );
    m_axisVariesCheckboxes.resize( m_dimension, NULL );


    // msw: without the following we don't get any scrollbars at all
    // we only want the vertical scrollbar enabled
    SetScrollbars( 0, 20, 0, 50 );

    wxStaticBoxSizer * stBoxSizer = new wxStaticBoxSizer(
            wxHORIZONTAL, this, _("Axis") );

    // wxGridBagSizer( vgap, hgap )
    m_axisSizer = new wxGridBagSizer( MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP );

    // space between X and Y axis selection controls and axis controls
    m_axisSizer->SetEmptyCellSize( wxSize( MDGRID_SIZER_VGAP, MDGRID_SIZER_HGAP ) );

    stBoxSizer->Add( m_axisSizer,
        wxSizerFlags().Expand().Proportion( 1 ).Border( wxRIGHT | wxLEFT, MDGRID_SIZER_HGAP ) );

    // Data table grid component
    m_grid = new MultiDimGridGrid( this, wxID_ANY, wxDefaultPosition );
    m_grid->SetTable( this, false );

    // main sizer contains axis controls in the left part and the grid in the right
    m_gridSizer = new wxBoxSizer( wxHORIZONTAL );
    m_gridSizer->Add( stBoxSizer, wxSizerFlags().Expand().Border( wxLEFT | wxRIGHT, MDGRID_SIZER_HGAP ) );
    m_gridSizer->Add( m_grid, wxSizerFlags().Proportion(1)
                                    .Right().Expand().Border(wxALL, 1) );

    SetSizer( m_gridSizer );
    m_gridSizer->SetSizeHints( this );

    // create and add axis choice controls
    for ( unsigned int i = 0; i < m_dimension; ++i )
    {
        SetAxisLabel( i, new wxStaticText( this, wxID_ANY, m_axis[i]->GetName() ) );
        SetAxisChoiceControl( i, m_axis[i]->GetChoiceControl( *this, *m_table ) );
        SetAxisVariesControl( i );
        SetAxisAdjustControl( i, m_axis[i]->GetAdjustControl( *this, *m_table ) );
    }

    m_firstAxisChoice = CreateGridAxisSelection( ID_FIRST_AXIS_CHOICE,
                                        _("X axis"), m_selectedFirstColour );
    m_secondAxisChoice = CreateGridAxisSelection( ID_SECOND_AXIS_CHOICE,
                                        _("Y axis"), m_selectedSecondColour );

    m_axisSizer->Add( new wxStaticText( this, wxID_ANY, _T(" ") ),
                      wxGBPosition( MDGridGetAxisLabelRow( m_dimension ), 0 ),
                      wxGBSpan( 1, 1 ),
                      wxSizerFlags().GetFlags() );

    RefreshTableFull();

    Layout();

    return true;
}

void MultiDimGrid::FixAxisValue( const wxString& axisName,
                                    const boost::any& value )
{
    int sel = GetAxisIndexByName( axisName );
    if ( sel != wxNOT_FOUND )
    {
        m_axisFixedValues[ sel ] = value;
        m_axisFixedCoords[ sel ] = value;

        RefreshTableData();
    }
}

void MultiDimGrid::RefreshTableData()
{
    // this will call DoRefreshTableData() when the last refresh schedule expires
    GridRefreshTableDataGuard guard( *this );
}

void MultiDimGrid::DoRefreshTableData()
{
    Freeze();
    m_grid->SetTable( m_grid->GetTable(), false );
    m_grid->ForceRefresh();
    Thaw();
}

bool MultiDimGrid::RefreshTableAxis()
{
    // Refresh table data only once
    GridRefreshTableDataGuard guard( *this );
    bool updated = false;
    for( unsigned int i = 0; i < m_dimension; ++i )
        if( DoRefreshTableAxis( i ) )
            updated = true;
    // TODO in the future this method should check for changes in the table data
    // structure (number of axis, types of axis, etc), and refresh accordingly

    if( AutoselectGridAxis() )
        updated = true;

    return updated;
}

bool MultiDimGrid::AutoselectGridAxis()
{
    bool updated = false;
    m_firstAxisChoice->Show( m_dimension > 0 );
    m_secondAxisChoice->Show( m_dimension > 1 );

    if( m_firstGridAxis == wxNOT_FOUND || m_secondGridAxis == wxNOT_FOUND )
    {
        std::pair<int,int> gridSelection = SuggestGridAxisSelection();
        if( gridSelection.first != m_firstGridAxis
         || gridSelection.second != m_secondGridAxis )
        {
            updated = true;
        }
        SetGridAxisSelection( gridSelection.first, gridSelection.second );
    }
    return updated;
}

std::pair<int,int> MultiDimGrid::SuggestGridAxisSelection() const
{
    int newFirst = m_firstGridAxis;
    int newSecond = m_secondGridAxis;
    for( unsigned int i = 0; i < m_dimension; ++i )
    {
        if( m_table->VariesByDimension( i ) )
        {
            if( newFirst == wxNOT_FOUND )
            {
                if( static_cast<int>( i ) != newSecond )
                {
                    newFirst = static_cast<int>( i );
                    if( newSecond != wxNOT_FOUND )
                        break;
                }
            }
            else
            if( newSecond == wxNOT_FOUND && newFirst != static_cast<int>( i ) )
            {
                newSecond = static_cast<int>( i );
                break;
            }
        }
    }
    return std::make_pair( newFirst, newSecond );
}

bool MultiDimGrid::DoRefreshTableAxis( unsigned int n )
{
    // Refresh table data only once
    GridRefreshTableDataGuard guard( *this );
    bool updated = false;
    if( DoRefreshAxisVaries( n ) )
        updated = true;
    if( DoRefreshAxisAdjustment( n ) )
        updated = true;
    return updated;
}

bool MultiDimGrid::DoRefreshAxisVaries( unsigned int axisId )
{
    bool updated = false;

    bool varies = m_table->VariesByDimension( axisId );
    bool canChange = m_table->CanChangeVariationWith( axisId );

    wxCheckBox * box = m_axisVariesCheckboxes[axisId];
    if( box )
    {
        if( varies != box->GetValue() )
        {
            box->SetValue( varies );
            updated = true;
        }

        if( canChange != box->Enable() )
            updated = true;
        box->Show( canChange || !varies );
    }

    wxWindow * win = m_axisChoiceWins[axisId];
    if( win )
        win->Show( varies );

    win = m_axisAdjustWins[axisId];
    if( win )
        win->Show( varies );

    if( updated )
    {
        PopulateGridAxisSelection();
        DoSetGridAxisSelection();

        if( varies )
        {
            AutoselectGridAxis();
        }
    }
    return updated;
}


bool MultiDimGrid::DoRefreshAxisAdjustment( unsigned int n )
{
    if( !m_table->VariesByDimension( n ) )
        return false;
    MultiDimAxisAny & axis = *m_axis[n];
    wxWindow * adjustWin = m_axisAdjustWins[n];

    bool updated = false;
    if( m_table->RefreshAxisAdjustment( axis, n ) )
        updated = true;
    if( axis.RefreshAdjustment( adjustWin, n ) )
        updated = true;

    if( updated )
    {
        if( m_axisChoiceWins[n] )
            axis.UpdateChoiceControl( *m_axisChoiceWins[n] );
        RefreshTableData();
    }
    return updated;
}

bool MultiDimGrid::DoApplyAxisAdjustment( unsigned int n )
{
    if( !m_table->VariesByDimension( n ) )
        return false;
    MultiDimAxisAny & axis = *m_axis[n];
    wxWindow * adjustWin = m_axisAdjustWins[n];

    bool updated = false;

    if( axis.ApplyAdjustment( adjustWin, n ) )
        updated = true;
    if( m_table->ApplyAxisAdjustment( axis, n ) )
        updated = true;

    if( updated )
    {
        if( m_axisChoiceWins[n] )
            axis.UpdateChoiceControl( *m_axisChoiceWins[n] );
        RefreshTableData();
    }
    return updated;
}

void MultiDimGrid::SetXAxisColour( wxColour const & colour )
{
    m_selectedFirstColour = colour;
    m_firstAxisChoice->SetForegroundColour( colour );
    // TODO(ET): Grid label coloring feature is not yet implemented in wxGrid
    // disable it and wait for a new wx version
    // m_grid->SetColLabelColour( colour );

    // Update select axis labels
    DoSetGridAxisSelection();
}

void MultiDimGrid::SetYAxisColour( wxColour const & colour )
{
    m_selectedSecondColour = colour;
    m_secondAxisChoice->SetForegroundColour( colour );
    // TODO(ET): Grid label coloring feature is not yet implemented in wxGrid
    // disable it and wait for a new wx version
    // m_grid->SetRowLabelColour( colour );

    // Update select axis labels
    DoSetGridAxisSelection();
}

wxChoice *
MultiDimGrid::CreateGridAxisSelection( int id,
                                      wxString const & label,
                                      wxColour const & selectedColour )
{
    // wxChoice will grow to its default size if all choice strings are empty,
    // therefore we don't pass the empty string ("") but a space instead (" ")
    wxString onlyEmptyChoice = _T(" ");

    wxChoice * win = new wxChoice( this, id,
        wxDefaultPosition, wxDefaultSize, 1, &onlyEmptyChoice );

    win->SetOwnForegroundColour( selectedColour );

    unsigned int row = id == ID_FIRST_AXIS_CHOICE ? MDGRID_AXIS_X_ROW
                                                  : MDGRID_AXIS_Y_ROW;
    m_axisSizer->Add( new wxStaticText( this, wxID_ANY, label ),
        wxGBPosition( row, MDGRID_LABEL_COL ),
        wxGBSpan( 1, 1 ),
        wxSizerFlags().Align( wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL )
                      .Border( wxRIGHT, 16 ).GetFlags() );

    m_axisSizer->Add( win,
        wxGBPosition( row, MDGRID_CHOICE_COL ),
        wxGBSpan( 1, MDGRID_CHOICE_HSPAN ),
        wxSizerFlags().Align( wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL )
                      .Border( wxALL, 1 ).Expand().GetFlags() );
    return win;
}

void MultiDimGrid::SetAxisLabel(int axisId, wxWindow * newWin)
{
    m_axisLabels[ axisId ] = newWin;
    m_axisSizer->Add( newWin,
        wxGBPosition( MDGridGetAxisLabelRow( axisId ), MDGRID_LABEL_COL ),
        wxGBSpan( 1, 1 ),
        wxSizerFlags().Align( wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL )
                      .Border( wxRIGHT, 16 ).GetFlags() );
}

void MultiDimGrid::SetAxisChoiceControl(int axisId, wxWindow * newWin)
{
    m_axisChoiceWins[ axisId ] = newWin;
    m_axisSizer->Add( newWin,
        wxGBPosition( MDGridGetAxisChoiceRow( axisId ), MDGRID_CHOICE_COL ),
        wxGBSpan( 1, 1 ),
        wxSizerFlags().Align( wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL )
                      .Border( wxALL, 1 ).Expand().GetFlags() );
}

void MultiDimGrid::SetAxisAdjustControl(int axisId, wxWindow * newWin)
{
    m_axisAdjustWins[ axisId ] = newWin;
    if ( newWin )
    {
        m_axisSizer->Add( newWin,
            wxGBPosition( MDGridGetAxisAdjustRow( axisId ), MDGRID_ADJUST_COL ),
            wxGBSpan( 1, MDGRID_ADJUST_HSPAN ),
            wxSizerFlags().Align( wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL )
                          .Border( wxALL, 1 ).GetFlags() );
    }
}

void MultiDimGrid::SetAxisVariesControl( unsigned int axisId )
{
    wxCheckBox * win = new wxCheckBox( this, wxID_ANY, _T("") );
    m_axisSizer->Add( win,
        wxGBPosition( MDGridGetAxisVariesRow( axisId ), MDGRID_VARIES_COL ),
        wxGBSpan( 1, 1 ),
        wxSizerFlags().Align( wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL )
                      .Border( wxALL, 1 ).GetFlags() );
    m_axisVariesCheckboxes[axisId] = win;
}

bool MultiDimGrid::SetGridAxisSelection( int firstAxis, int secondAxis )
{
    wxCHECK_MSG( firstAxis != secondAxis || firstAxis == wxNOT_FOUND,
                false, _T("Select different axes") );
    bool update = ( m_firstGridAxis != firstAxis )
               || ( m_secondGridAxis != secondAxis );
    m_firstGridAxis = firstAxis;
    m_secondGridAxis = secondAxis;
//    if ( update )
    DoSetGridAxisSelection();
    return update;
}

int MultiDimGrid::GetGridAxisSelection( int id )
{
    wxChoice * choice = (id == ID_FIRST_AXIS_CHOICE) ?
                         m_firstAxisChoice : m_secondAxisChoice;
    if( !choice )
    {   // function is called when the control is not yet fully constructed
        return wxNOT_FOUND;
    }
    int oldSel = choice->GetSelection();
    // wxNOT_FOUND - nothing is selected
    // 0 - first empty item (" ") is selected => same as no selection at all
    if( oldSel == wxNOT_FOUND || oldSel == 0 )
        return wxNOT_FOUND;

    return reinterpret_cast<int>( choice->GetClientData( oldSel ) );
}

void MultiDimGrid::DoSetGridAxisSelection( int axisId, int selection )
{
    if( selection == GetGridAxisSelection( axisId ) )
        return;
    wxChoice * choice = (axisId == ID_FIRST_AXIS_CHOICE) ?
                         m_firstAxisChoice : m_secondAxisChoice;

    int oldSelection = GetGridAxisSelection( axisId );

    if( selection != oldSelection )
    {
        for( unsigned int i = 1; i < choice->GetCount(); ++i )
        {
            int cdata = reinterpret_cast<int>( choice->GetClientData( i ) );
            if( cdata == selection )
            {
                choice->SetSelection( i );
                return;
            }
        }
        choice->SetSelection( 0 );
        RefreshTableData();
    }
}

void MultiDimGrid::DoSetGridAxisSelection()
{
    DoSetGridAxisSelection( ID_FIRST_AXIS_CHOICE, m_firstGridAxis );
    DoSetGridAxisSelection( ID_SECOND_AXIS_CHOICE, m_secondGridAxis );

    m_axisFixedCoords = m_axisFixedValues;
    for ( unsigned int i = 0; i < m_dimension; ++i )
    {
        bool selected = (static_cast<int>( i ) == m_firstGridAxis)
                     || (static_cast<int>( i ) == m_secondGridAxis);
        // different colour to the selected axis
        m_axisLabels[ i ]->SetOwnForegroundColour( selected
                ? ( ( static_cast<int>( i ) == m_firstGridAxis )
                                                ? m_selectedFirstColour
                                                : m_selectedSecondColour )
                : GetForegroundColour() );
        // text does not get repainted after font and colour changes, force it
        m_axisLabels[ i ]->Refresh();

        // disable selected window choice control
        wxWindow * choiceWin = m_axisChoiceWins[ i ];
        if ( choiceWin )
        {
            choiceWin->Enable( !selected && m_table->VariesByDimension( i ) );
        }
    }
    RefreshTableData();
}

void MultiDimGrid::PopulateGridAxisSelection()
{
    PopulateGridAxisSelection( ID_FIRST_AXIS_CHOICE );
    PopulateGridAxisSelection( ID_SECOND_AXIS_CHOICE );
    Layout();
}

void MultiDimGrid::PopulateGridAxisSelection( unsigned int id )
{
    wxChoice * choice = (id == ID_FIRST_AXIS_CHOICE) ?
                         m_firstAxisChoice : m_secondAxisChoice;
    if( !choice )
    {   // we are still constructing the MultiDimGrid control
        return;
    }
    choice->Freeze();
    int oldSelection = GetGridAxisSelection( id );
    // set selection to empty line so that it does not change
    choice->SetSelection( 0 );
    int newSelIndex = wxNOT_FOUND;

    {   // remove every item, except the empty one (the first " ")
        for( int i = choice->GetCount() - 1; i >= 1; --i )
        {
            choice->Delete( i );
        }
    }
    {   // repopulate the drop-down list with axis names
        std::size_t const size = m_axis.size();
        for( unsigned int i = 0; i < size; ++i )
        {
            if( m_table->VariesByDimension( i ) )
            {
                choice->Append( m_axis[i]->GetName(),
                                reinterpret_cast<void*>( i ) );
                if( static_cast<int>( i ) == oldSelection )
                {
                    newSelIndex = choice->GetCount() - 1;
                }
            }
        }
    }

    if( newSelIndex != wxNOT_FOUND )
    {
        choice->SetSelection( newSelIndex );
    }

    if( oldSelection != GetGridAxisSelection( id ) )
    {
        DoOnSwitchSelectedAxis( id );
    }
    choice->Thaw();
}

int MultiDimGrid::GetAxisIndexByName( const wxString & axisName )
{
    for (unsigned int i = 0; i < m_dimension; ++i)
        if ( m_axis[i]->GetName() == axisName )
            return i;
    return wxNOT_FOUND;
}

MultiDimAxisAny const & MultiDimGrid::GetAxis( unsigned int n ) const
{
    if ( n >= m_axis.size() )
        throw std::range_error( _T("invalid axis index") );
    return *m_axis[n];
}

MultiDimAxisAny & MultiDimGrid::GetAxis( unsigned int n )
{
    if ( n >= m_axis.size() )
        throw std::range_error( _T("invalid axis index") );
    return *m_axis[n];
}

int MultiDimGrid::GetNumberRows()
{
    if ( m_secondGridAxis == wxNOT_FOUND )
        return 1;
    return m_axis[ m_secondGridAxis ]->GetCardinality();
}

int MultiDimGrid::GetNumberCols()
{
    if ( m_firstGridAxis == wxNOT_FOUND )
        return 1;
    return m_axis[ m_firstGridAxis ]->GetCardinality();
}

bool MultiDimGrid::IsEmptyCell( int row, int col )
{
    return false;
}

void MultiDimGrid::PrepareFixedCoords( int row, int col )
{
    if ( m_firstGridAxis != wxNOT_FOUND )
    {
        m_axisFixedCoords[ m_firstGridAxis ] =
                   m_axis[ m_firstGridAxis ]->GetValue( col );
    }
    else
    {
        wxASSERT_MSG( col == 0, _T("No first grid axis selected") );
    }

    if ( m_secondGridAxis != wxNOT_FOUND )
    {
        m_axisFixedCoords[m_secondGridAxis] =
                    m_axis[m_secondGridAxis]->GetValue( row );
    }
    else
    {
        wxASSERT_MSG( row == 0, _T("No second grid axis selected") );
    }
}

wxString MultiDimGrid::GetValue( int row, int col )
{
    PrepareFixedCoords( row, col );
    boost::any value = m_table->GetAnyValue( m_axisFixedCoords );
    return m_table->ValueToString( value );
}

void MultiDimGrid::SetValue( int row, int col, const wxString& value )
{
    PrepareFixedCoords( row, col );
    m_table->SetAnyValue( m_axisFixedCoords, m_table->StringToValue( value ) );
}

wxString MultiDimGrid::GetRowLabelValue( int row )
{
    if ( m_secondGridAxis != wxNOT_FOUND
        && static_cast<unsigned int>( row )
                < m_axis[ m_secondGridAxis ]->GetCardinality() )
    {
        return m_axis[ m_secondGridAxis ]->GetLabel( row );
    }
    if ( m_firstGridAxis != wxNOT_FOUND )
        return m_axis[ m_firstGridAxis ]->GetName();
    return wxString( _T("") );
}

wxString MultiDimGrid::GetColLabelValue( int col )
{
    if ( m_firstGridAxis != wxNOT_FOUND
        && static_cast<unsigned int>( col )
                < m_axis[ m_firstGridAxis ]->GetCardinality() )
    {
        return m_axis[ m_firstGridAxis ]->GetLabel( col );
    }
    if ( m_secondGridAxis != wxNOT_FOUND )
        return m_axis[ m_secondGridAxis ]->GetName();
    return wxString( _T("") );
}

void MultiDimGrid::OnAxisVariesToggle( wxCommandEvent & event )
{
    // find the checkbox triggered the event
    CheckBoxes::iterator it =
        std::find( m_axisVariesCheckboxes.begin(), m_axisVariesCheckboxes.end(),
                   dynamic_cast<wxCheckBox*>( event.GetEventObject() ) );
    if( it == m_axisVariesCheckboxes.end() )
    {
        wxFAIL_MSG( _T("Unidentified event caught") );
        return;
    }
    std::size_t index = it - m_axisVariesCheckboxes.begin();
    bool varies = m_axisVariesCheckboxes[index]->GetValue();
    if( varies != m_table->VariesByDimension( index ) )
    {
        bool confirmed = true;
        if( !varies )
        {   // we are going to disable that axis - warn user about data loss
            wxString text;

            int answer = wxMessageBox(
                wxString::Format(
                    _("Disabling the axis \"%s\" could cause data loss."),
                    m_axis[index]->GetName().c_str() ),
                m_axis[index]->GetName(),
                wxOK | wxCANCEL | wxICON_EXCLAMATION, this );
            confirmed = ( answer == wxOK );
        }
        if( confirmed )
        {
            m_table->MakeVaryByDimension( index, varies );
            DoRefreshAxisVaries( index );
        }
        else
        {   // restore the varies checkbox value
            m_axisVariesCheckboxes[index]->SetValue(
                                m_table->VariesByDimension( index ) );
        }
    }
}

void MultiDimGrid::OnSwitchSelectedAxis( wxCommandEvent & event )
{
    int id = event.GetId();
    wxASSERT_MSG( id == ID_FIRST_AXIS_CHOICE || id == ID_SECOND_AXIS_CHOICE,
                  _T("Event from unknown control received") );

    DoOnSwitchSelectedAxis( static_cast<unsigned int>( id ) );
}

void MultiDimGrid::DoOnSwitchSelectedAxis( unsigned int axisId )
{
    int newSel = GetGridAxisSelection( axisId );

    if ( axisId == ID_FIRST_AXIS_CHOICE )
    {
        if ( newSel == m_secondGridAxis && newSel != wxNOT_FOUND )
        {
            // means with the X axis we're hitting the Y axis - so switch them
            m_secondGridAxis = m_firstGridAxis;
        }
        m_firstGridAxis = newSel;
    }
    else
    {   // here we have axisId == ID_SECOND_AXIS_CHOICE
        if ( newSel == m_firstGridAxis && newSel != wxNOT_FOUND )
        {
            // means with the Y axis we're hitting the X axis - so switch them
            m_firstGridAxis = m_secondGridAxis;
        }
        m_secondGridAxis = newSel;
    }
    DoSetGridAxisSelection();
}
