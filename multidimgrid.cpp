// implementation of multidimensional data editor control
#include "multidimgrid.hpp"
#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/window.h>
#include <wx/colour.h>

// ----------------------------------------------------------
// MultiDimAxisAnyChoice
// ----------------------------------------------------------
// Helper widget class used to allow the user to select
// a value for the axis (see MultiDimAxisAny::GetChoiceControl())
// ----------------------------------------------------------
class MultiDimAxisAnyChoice : public wxChoice
{
protected:
    friend class MultiDimAxisAny;
    MultiDimAxisAnyChoice( MultiDimAxisAny const & axis, MultiDimGridAny & grid );

    // selection change hook
    void OnSelectionChange( wxCommandEvent & event );

    // the axis that this object controls
    MultiDimAxisAny const & m_axis;
    // grid object to be notified of any axis value changes
    MultiDimGridAny & m_grid;

private:
    DECLARE_NO_COPY_CLASS( MultiDimAxisAnyChoice )
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------
// MultiDimAxisAnyChoice implementation
// ----------------------------------------------------------

BEGIN_EVENT_TABLE(MultiDimAxisAnyChoice, wxChoice)
    EVT_CHOICE(wxID_ANY, MultiDimAxisAnyChoice::OnSelectionChange)
END_EVENT_TABLE()


MultiDimAxisAnyChoice::MultiDimAxisAnyChoice
(
    MultiDimAxisAny const & axis,
    MultiDimGridAny & grid
)
: wxChoice(), m_axis(axis), m_grid(grid)
{
    wxArrayString labels;
    unsigned int numAxis = axis.GetCardinality();
    labels.Alloc( numAxis );
    for ( unsigned int i = 0; i < numAxis; ++i )
    {
        labels.Add( axis.GetLabel( i ) );
    }
    wxChoice::Create( &grid, wxID_ANY, wxDefaultPosition, wxDefaultSize, labels );

    if ( numAxis > 0 )
    {
        SetSelection( 0 );
        m_grid.FixAxisValue( m_axis.GetName(),  m_axis.GetValue( 0 ) );
    }
}

void MultiDimAxisAnyChoice::OnSelectionChange(wxCommandEvent & WXUNUSED(event))
{
    unsigned int sel = GetSelection();

    wxASSERT( sel >= 0 && sel < m_axis.GetCardinality() );

    m_grid.FixAxisValue( m_axis.GetName(),  m_axis.GetValue( sel ) );
}


// -------------------------------------------------------
// Implementation of the class MultiDimAxisAny
// -------------------------------------------------------
wxWindow * MultiDimAxisAny::GetChoiceControl(MultiDimGridAny * grid) const
{
    return new MultiDimAxisAnyChoice(*this, *grid);
}

// -------------------------------------------------------
// Class MultiDimGridGrid
// -------------------------------------------------------
// wxGrid customised to the needs of MultiDimGridAny.
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


// ---------------------------------------
// Implementation of class MultiDimGridAny
// ---------------------------------------

// Two values to distinguish between X axis and Y axis
enum {
    ID_FIRST_AXIS_CHOICE = wxID_HIGHEST + 1,
    ID_SECOND_AXIS_CHOICE
};

BEGIN_EVENT_TABLE(MultiDimGridAny, wxControl)
    EVT_CHOICE  (ID_FIRST_AXIS_CHOICE,  MultiDimGridAny::OnAxisChoiceSwitch)
    EVT_CHOICE  (ID_SECOND_AXIS_CHOICE, MultiDimGridAny::OnAxisChoiceSwitch)
END_EVENT_TABLE()

// Used to indicate that an axis (X or Y) does not have a selection
enum {
    wxAXIS_NOT_FOUND = -1
};

void MultiDimGridAny::Init()
{
    m_firstGridAxis = wxAXIS_NOT_FOUND;
    m_secondGridAxis = wxAXIS_NOT_FOUND;
}

bool
MultiDimGridAny::Create(wxWindow *parent,
                        boost::shared_ptr<MultiDimTableAny> const & atable,
                        Axis const & aaxis,
                        wxWindowID id,
                        wxPoint const & pos,
                        wxSize const & size)
{
    m_table = atable;
    m_axis = aaxis;
    m_dimension = m_table->GetDimension();

    // wxVSCROLL does not appear to influent the wxPanel behaviour under msw
    // leaving it only for other platforms
    long style = wxWANTS_CHARS | wxVSCROLL;
    // setting self as a wxPanel
    wxPanel::Create( parent, id, pos, size, style );

    // msw: without the following we don't get any scrollbars at all
    // we only want the vertical scrollbar enabled
    SetScrollbars( 0, 20, 0, 50 );

    // setting default colour to both X and Y axis
    m_selectedFirstColour = GetForegroundColour();
    m_selectedSecondColour = GetForegroundColour();

    wxASSERT_MSG( atable->GetDimension() == aaxis.size(),
            _T("Table size and number of axis do not match") );

    wxStaticBoxSizer * stBoxSizer = new wxStaticBoxSizer(
            wxHORIZONTAL, this, _T("Axis") );

    m_axisSizer = new wxFlexGridSizer( 2, m_dimension + 3 );
    stBoxSizer->Add( m_axisSizer,
        wxSizerFlags().Expand().Proportion( 1 ).Border(wxRIGHT | wxLEFT, 4) );

    m_firstAxisChoice = CreateAxisSelection( ID_FIRST_AXIS_CHOICE,
                                        _T("X axis"), m_selectedFirstColour );
    m_secondAxisChoice = CreateAxisSelection( ID_SECOND_AXIS_CHOICE,
                                        _T("Y axis"), m_selectedSecondColour );

    // space between X and Y axis selection controls and axis controls
    m_axisSizer->AddSpacer( 16 ); m_axisSizer->AddSpacer( 16 );

    // initializing fixed values with empty ones
    m_axisFixedValues.resize( m_dimension, boost::any() );
    m_axisFixedCoords.resize( m_dimension, boost::any() );

    // Data table grid component
    m_grid = new MultiDimGridGrid( this, wxID_ANY, wxDefaultPosition );
    m_grid->SetTable( this, false );

    // main sizer contains axis controls in the left part and the grid in the right
    m_gridSizer = new wxBoxSizer( wxHORIZONTAL );
    m_gridSizer->Add( stBoxSizer, wxSizerFlags().Expand().Border( wxLEFT | wxRIGHT, 4 ) );
    m_gridSizer->Add( m_grid, wxSizerFlags().Proportion(1)
                                    .Right().Expand().Border(wxALL, 1) );

    SetSizer( m_gridSizer );
    m_gridSizer->SetSizeHints( this );

    // adding axis choice control to the widget
    m_axisLabels.resize( m_dimension, NULL );
    m_axisChoiceWins.resize( m_dimension, NULL );
    m_axisAdjustWins.resize( m_dimension, NULL );

    for ( unsigned int i = 0; i < m_dimension; ++i )
    {
        SetAxisLabel( i, new wxStaticText( this, wxID_ANY, m_axis[i]->GetName() ) );
        SetAxisChoiceControl( i, m_axis[i]->GetChoiceControl(this) );
        SetAxisAdjustControl( i, m_axis[i]->GetAdjustControl(this) );
    }

    // selecting appropriate axis
    if ( m_dimension >= 2 )
        SetAxisSelection( m_dimension - 2, m_dimension - 1 );
    else if ( m_dimension == 1 )
        SetAxisSelection( m_dimension - 1, wxAXIS_NOT_FOUND );
    else
        SetAxisSelection( wxAXIS_NOT_FOUND, wxAXIS_NOT_FOUND );

    Layout();

    return true;
}

wxChoice *
MultiDimGridAny::CreateAxisSelection(int id,
                                     wxString const & label,
                                     wxColour const & selectedColour )
{
    wxArrayString axisChoices;
    axisChoices.Alloc( m_dimension + 1 );
    // adding an empty selection at position 0 ( == wxAXIS_NOT_FOUND + 1 )
    axisChoices.Add( _T("") );
    for ( unsigned int i = 0; i < m_dimension; ++i )
    {
        // adding i-th axis at position (i + 1)
        axisChoices.Add( m_axis[i]->GetName() );
    }

    wxChoice * win = new wxChoice( this, id,
        wxDefaultPosition, wxDefaultSize, axisChoices );
    win->SetOwnForegroundColour( selectedColour );

    m_axisSizer->Add( new wxStaticText( this, wxID_ANY, label ),
                wxSizerFlags().Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL)
                .Border(wxRIGHT, 16) );

    m_axisSizer->Add( win, wxSizerFlags().Border(wxALL, 1).Expand()
                            .Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL) );
    // reserving one row for adjustment controls
    m_axisSizer->AddSpacer( 1 );
    m_axisSizer->AddSpacer( 1 );
    return win;
}

void MultiDimGridAny::SetAxisLabel(int axisId, wxWindow * newWin)
{
    m_axisLabels[ axisId ] = newWin;
    m_axisSizer->Add( newWin, wxSizerFlags().Border(wxRIGHT, 16)
                            .Align(wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL) );
}

void MultiDimGridAny::SetAxisChoiceControl(int axisId, wxWindow * newWin)
{
    m_axisChoiceWins[ axisId ] = newWin;
    m_axisSizer->Add( newWin, wxSizerFlags().Border(wxALL, 1)
                .Expand().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL) );
}

void MultiDimGridAny::SetAxisAdjustControl(int axisId, wxWindow * newWin)
{
    m_axisAdjustWins[ axisId ] = newWin;
    if ( newWin )
        newWin->Show( false );
}

void MultiDimGridAny::OnAxisChoiceSwitch( wxCommandEvent & event )
{
    wxObject * obj = event.GetEventObject();
    wxChoice * choice = dynamic_cast< wxChoice* >( obj );
    if ( !choice )
    {
        event.Skip( true );
        return;
    }
    int newSel = choice->GetSelection() - 1;
    int id = event.GetId();
    wxASSERT( id == ID_FIRST_AXIS_CHOICE || id == ID_SECOND_AXIS_CHOICE );
    if ( id == ID_FIRST_AXIS_CHOICE )
    {
        if ( newSel == m_secondGridAxis && newSel != wxAXIS_NOT_FOUND )
        {
            // means with the X axis we're hitting the Y axis - so switch them
            m_secondGridAxis = m_firstGridAxis;
        }
        m_firstGridAxis = newSel;
    }
    else
    {
        if ( newSel == m_firstGridAxis && newSel != wxAXIS_NOT_FOUND )
        {
            // means with the Y axis we're hitting the X axis - so switch them
            m_firstGridAxis = m_secondGridAxis;
        }
        m_secondGridAxis = newSel;
    }
    DoSetAxisSelection();
}

const MultiDimAxisAny& MultiDimGridAny::GetAxis( unsigned int n ) const
{
    if ( n >= m_axis.size() )
    {
        throw std::range_error( _T("invalid axis index") );
    }
    return *m_axis[n];
}

int MultiDimGridAny::GetAxisIndexByName( const wxString & axisName )
{
    for (unsigned int i = 0; i < m_dimension; ++i)
        if ( m_axis[i]->GetName() == axisName )
            return i;
    return wxAXIS_NOT_FOUND;
}


void MultiDimGridAny::FixAxisValue(const wxString& axisName, const boost::any& value)
{
    int sel = GetAxisIndexByName( axisName );
    if ( sel == wxAXIS_NOT_FOUND )
        return;

    m_axisFixedValues[ sel ] = value;
    m_axisFixedCoords[ sel ] = value;
    RefreshTableData();
}

void MultiDimGridAny::UpdateGridAxis( const wxString& axisName )
{
    int sel = GetAxisIndexByName( axisName );
    if ( sel == wxAXIS_NOT_FOUND )
        return;

    if ( sel == m_firstGridAxis || sel == m_secondGridAxis )
        RefreshTableData();
}

void MultiDimGridAny::RefreshTableData()
{
    m_grid->SetTable( m_grid->GetTable(), false );
    m_grid->ForceRefresh();
}

void MultiDimGridAny::SetXAxisColour( wxColour const & colour )
{
    m_selectedFirstColour = colour;
    m_firstAxisChoice->SetForegroundColour( colour );
    // TODO(ET): Grid label coloring feature is not yet implemented in wxGrid
    // disable it and wait for a new wx version
    // m_grid->SetColLabelColour( colour );
    DoSetAxisSelection();
}
void MultiDimGridAny::SetYAxisColour( wxColour const & colour )
{
    m_selectedSecondColour = colour;
    m_secondAxisChoice->SetForegroundColour( colour );
    // TODO(ET): Grid label coloring feature is not yet implemented in wxGrid
    // disable it and wait for a new wx version
    // m_grid->SetRowLabelColour( colour );
    DoSetAxisSelection();
}


bool MultiDimGridAny::SetAxisSelection( int firstAxis, int secondAxis )
{
    wxCHECK_MSG( firstAxis != secondAxis || firstAxis == wxAXIS_NOT_FOUND,
                false, _T("Select different axes") );
    bool update = ( m_firstGridAxis != firstAxis )
                || ( m_secondGridAxis != secondAxis );
    m_firstGridAxis = firstAxis;
    m_secondGridAxis = secondAxis;
    if ( update )
        DoSetAxisSelection();
    return true;
}

int MultiDimGridAny::GetNumberRows()
{
    if ( m_secondGridAxis == wxAXIS_NOT_FOUND )
        return 1;
    return m_axis[ m_secondGridAxis ]->GetCardinality();
}

int MultiDimGridAny::GetNumberCols()
{
    if ( m_firstGridAxis == wxAXIS_NOT_FOUND )
        return 1;
    return m_axis[ m_firstGridAxis ]->GetCardinality();
}

bool MultiDimGridAny::IsEmptyCell( int row, int col )
{
    return false;
}

void MultiDimGridAny::PrepareFixedCoords( int row, int col )
{
    if ( m_firstGridAxis != wxAXIS_NOT_FOUND )
    {
        m_axisFixedCoords[ m_firstGridAxis ] =
                    m_axis[ m_firstGridAxis ]->GetValue( col );
    }
    else
    {
        wxASSERT( col == 0 );
    }

    if ( m_secondGridAxis != wxAXIS_NOT_FOUND )
    {
        m_axisFixedCoords[m_secondGridAxis] =
                    m_axis[m_secondGridAxis]->GetValue( row );
    }
    else
    {
        wxASSERT( row == 0 );
    }
}

wxString MultiDimGridAny::GetValue( int row, int col )
{
    PrepareFixedCoords( row, col );
    boost::any value = m_table->GetAnyValue( m_axisFixedCoords );
    return m_table->ValueToString( value );
}

void MultiDimGridAny::SetValue( int row, int col, const wxString& value )
{
    PrepareFixedCoords( row, col );
    m_table->SetAnyValue( m_axisFixedCoords, m_table->StringToValue( value ) );
}

wxString MultiDimGridAny::GetRowLabelValue( int row )
{
    if ( m_secondGridAxis != wxAXIS_NOT_FOUND
        && static_cast<unsigned int>( row )
                < m_axis[ m_secondGridAxis ]->GetCardinality() )
    {
        return m_axis[ m_secondGridAxis ]->GetLabel( row );
    }
    if ( m_firstGridAxis != wxAXIS_NOT_FOUND )
        return m_axis[ m_firstGridAxis ]->GetName();
    return wxString( _T("") );
}

wxString MultiDimGridAny::GetColLabelValue( int col )
{
    if ( m_firstGridAxis != wxAXIS_NOT_FOUND
        && static_cast<unsigned int>( col )
                < m_axis[ m_firstGridAxis ]->GetCardinality() )
    {
        return m_axis[ m_firstGridAxis ]->GetLabel( col );
    }
    if ( m_secondGridAxis != wxAXIS_NOT_FOUND )
        return m_axis[ m_secondGridAxis ]->GetName();
    return wxString( _T("") );
}



void MultiDimGridAny::DoSetAxisSelection()
{
    if ( m_firstAxisChoice->GetSelection() != m_firstGridAxis + 1 )
    {
        m_firstAxisChoice->SetSelection( m_firstGridAxis + 1 );
    }
    if ( m_secondAxisChoice->GetSelection() != m_secondGridAxis + 1 )
    {
        m_secondAxisChoice->SetSelection( m_secondGridAxis + 1 );
    }
    m_axisFixedCoords = m_axisFixedValues;
    for ( unsigned int i = 0; i < m_dimension; ++i )
    {
        bool selected = ((int)i == m_firstGridAxis) || ((int)i == m_secondGridAxis);
        {
            // different colour to the selected axis
            m_axisLabels[ i ]->SetOwnForegroundColour( selected
                    ? ( ( (int)i == m_firstGridAxis )
                                ? m_selectedFirstColour
                                : m_selectedSecondColour )
                    : GetForegroundColour() );
            // text does not get repainted after font and colour changes, force it
            m_axisLabels[ i ]->Refresh();
        }
        {
            // disable selected window choice control
            wxWindow * choiceWin = m_axisChoiceWins[ i ];
            if ( choiceWin )
            {
                choiceWin->Enable( !selected );
            }
            wxWindow * adjustWin = m_axisAdjustWins[ i ];
            if ( adjustWin )
            {
                adjustWin->Show( selected );
            }
        }
        {
            // reset or apply any adjustments for the axis
            wxWindow * adjWin = m_axisAdjustWins[ i ];
            if ( adjWin )
            {
                if ( selected )
                    m_axis[ i ]->ApplyAdjustment( adjWin );
                else
                    m_axis[ i ]->ResetAdjustment( adjWin );
            }
        }
    }
    ShowAxisAdjustControls();
    RefreshTableData();
    // TODO(ET): does not seem to work - layout does not seem to be adjusted
    Layout();
}

void MultiDimGridAny::ShowAxisAdjustControls()
{
    // we check if the controls are already created and the sizer has enough slots
    // 8 stands here for the number of slots occupied by X and Y selection labels
    // and controls. If there is less than 8 slots occupied in the sizer, then
    // we are in the middle of the control creation.
    if ( m_axisSizer->GetChildren().GetCount() < 8 )
        return;

    const unsigned int X_axis_adjWindow_slot = 3;
    const unsigned int Y_axis_adjWindow_slot = 7;

    int axisId;

    // first we remove any adjustment window for the Y axis (in the last slot)
    // then for the X slot -- at the 3rd slot
    unsigned int position = Y_axis_adjWindow_slot;
    for ( unsigned int cycle = 0; cycle < 2; ++cycle )
    {
        if ( m_axisSizer->GetChildren().GetCount() > position )
        {
            if ( m_axisSizer->GetItem( position )->IsWindow() )
                m_axisSizer->Detach( position );
            else
                m_axisSizer->Remove( position );
        }
        position = X_axis_adjWindow_slot;
    }

    axisId = m_firstGridAxis;
    // now we insert the adjustment window for the axis X and then for Y
    position = X_axis_adjWindow_slot;
    for ( unsigned int cycle = 0; cycle < 2; ++cycle )
    {
        wxWindow * win = ( axisId != wxAXIS_NOT_FOUND )
                                ? m_axisAdjustWins[ axisId ] : NULL;

        if ( win )
        {
            m_axisSizer->Insert( position, win, wxSizerFlags()
                            .Border( wxALL, 1 )
                            .Align( wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL ) );
        }
        else
        {
            m_axisSizer->InsertSpacer( position, 0 );
        }
        axisId = m_secondGridAxis;
        position = Y_axis_adjWindow_slot;
    }
    m_axisSizer->Layout();
}

