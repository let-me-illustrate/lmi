#include "multidimgrid.hpp"

#include <wx/sizer.h>
#include <wx/choice.h>
#include <wx/datetime.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/validate.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include <wx/app.h>

#include <map>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <math.h> // for log10

// needed by MultiDimIntAxisAdjuster to calculate its contols size
#include <wx/dcclient.h>

// to avoid using long ids (such as boost::filesystem::path)
using namespace boost;

// Option Type axis values enum
enum OptionType
{
    Option_Put,
    Option_Call,
    Option_Max
};

// Option Exercise axis values enum
enum OptionExercise
{
    Exercise_European,
    Exercise_American,
    Exercise_Max
};

// Option exercise axis
class ExerciseAxis : public MultiDimEnumAxis<OptionExercise>
{
public:
    ExerciseAxis() : MultiDimEnumAxis<OptionExercise>("Exercise type",
                                      MakeArray("European", "American"))
    {
    }
};

// Option type axis
class TypeAxis : public MultiDimEnumAxis<OptionType>
{
public:
    TypeAxis() : MultiDimEnumAxis<OptionType>("Option type",
                                              MakeArray("Put", "Call"))
    {
    }
};


// forward MultiDimAdjustableIntAxis class declaration
class MultiDimAdjustableIntAxis;

// adjustment control used by MultiDimAdjustableIntAxis class to adjust its values
class MultiDimIntAxisAdjuster : public wxPanel
{
public:
    MultiDimIntAxisAdjuster( MultiDimAdjustableIntAxis & axis, MultiDimGridAny & grid );
private:
    MultiDimAdjustableIntAxis & axis;
    MultiDimGridAny & grid;

    wxTextCtrl * m_minValue;
    wxTextCtrl * m_maxValue;
    wxButton * m_button;

    // event handler function called when min/max value is changed
    void OnRangeChange( wxCommandEvent & event );
    // event handler function called when 'apply' button is pushed
    void OnConfirm( wxCommandEvent & event );

    friend class MultiDimAdjustableIntAxis;
    // function called by OnCofirm. applies adjustment values
    void DoOnConfirm();

    // helper - calculate minimal suitable size for a control with some text
    // needed to adjust min/max/apply controls size
    wxSize GetMinSizeForTextControl( wxWindow * win, int numLetters );

    DECLARE_NO_COPY_CLASS( MultiDimIntAxisAdjuster )
    DECLARE_EVENT_TABLE()
};

// --------------------------
// MultiDimAdjustableIntAxis
// --------------------------
// Adjustable axis - the only difference with MultiDimIntAxis is that
// it provides adjustment control and allows narrowing of value range
// It stores m_minOriginal and m_maxOriginal values to allow the 'resetting'
// of adjustments
class MultiDimAdjustableIntAxis
    : public MultiDimAdjustableAxisAny< MultiDimIntAxisAdjuster >
{
    typedef MultiDimAdjustableAxisAny< MultiDimIntAxisAdjuster > BaseClass;
public:
    typedef int ValueType;

    // see MultiDimIntAxis::MultiDimIntAxis constructor for details
    MultiDimAdjustableIntAxis(
        const wxString& name,
        int minValue,
        int maxValue,
        int step = 1
    )
    : BaseClass(name), m_minOriginal(minValue), m_maxOriginal(maxValue),
    m_min(minValue), m_max(maxValue), m_step(step)
    {
        wxASSERT_MSG( minValue <= maxValue,
                      _T("minValue have to less or equal to maxValue") );
        wxASSERT_MSG( step >= 1,
                      _T("step has to be at least 1") );
    }

    // the same as MultiDimIntAxis::GetCardinality
    virtual unsigned int GetCardinality() const
    {
        return ( m_max - m_min ) / m_step + 1;
    }

    // the same as MultiDimIntAxis::GetCardinality
    virtual wxString GetLabel(unsigned int n) const
    {
        return wxString::Format( _T( "%d" ), m_min + n * m_step );
    }

    // the same as MultiDimIntAxis::GetCardinality
    virtual boost::any GetValue(unsigned int n) const
    {
        return boost::any( static_cast<int>( m_min + n * m_step ) );
    }

protected:
    // create the adjustment control
    MultiDimIntAxisAdjuster * DoGetAdjustControl( MultiDimGridAny *grid ) const
    {
        wxASSERT( grid != NULL );
        return new MultiDimIntAxisAdjuster(
            *const_cast< MultiDimAdjustableIntAxis* >(this),
            *grid
        );
    }

    // asks adjustment window to reapply its filters
    virtual void DoApplyAdjustment( MultiDimIntAxisAdjuster * win )
    {
        if( win != NULL )
            win->DoOnConfirm();
    }

    // just restore the original values
    virtual void DoResetAdjustment( MultiDimIntAxisAdjuster * win )
    {
        m_min = m_minOriginal;
        m_max = m_maxOriginal;
    }

private:
    friend class MultiDimIntAxisAdjuster;

    // original range boundaries
    int m_minOriginal;
    int m_maxOriginal;
    // working boundaries - could be altered by adjustment control
    int m_min;
    int m_max;
    
    int m_step;


    void DoApplyAdjustment( int minValue, int maxValue )
    {
        // minValue should not be greater than m_maxOriginal
        minValue = wxMin( minValue, m_maxOriginal );
        // snap minValue to the step-grid
        minValue = m_minOriginal
                 + (int)(( minValue - m_minOriginal + m_step - 1 ) / m_step) * m_step;
        // minValue should not be less than m_minOriginal
        minValue = wxMax( minValue, m_minOriginal );

        // maxValue should not be greater than m_maxOriginal, nor less than minValue
        maxValue = wxMax( wxMin( maxValue, m_maxOriginal ), minValue );

        m_min = minValue;
        m_max = maxValue;
    }
};

// --------------------------
// StrikeAxis implementation
// --------------------------
// this should be double but for now it's not supported...
class StrikeAxis : public MultiDimAdjustableIntAxis
{
public:
    StrikeAxis() : MultiDimAdjustableIntAxis("Strike", 10, 100, 5)
    {
    }
};

// ----------------------------------
// MultiDimIntAxisAdjuster implementation
// ----------------------------------
BEGIN_EVENT_TABLE( MultiDimIntAxisAdjuster, wxPanel )
    EVT_TEXT(wxID_ANY, MultiDimIntAxisAdjuster::OnRangeChange)
    EVT_BUTTON(wxID_ANY, MultiDimIntAxisAdjuster::OnConfirm)
END_EVENT_TABLE()

wxSize MultiDimIntAxisAdjuster::GetMinSizeForTextControl( wxWindow * win, int numLetters )
{
    wxCoord h, w;
    {
        wxClientDC dc( win );
        dc.SetFont( win->GetFont() );
        // taking the widest letter size
        dc.GetTextExtent( _T("W"), &w, &h );
    }
    wxSize size( w * numLetters, h );
    size += win->GetSize() - win->GetClientSize();
    return size;
}

MultiDimIntAxisAdjuster::MultiDimIntAxisAdjuster(MultiDimAdjustableIntAxis & aaxis,
                                                 MultiDimGridAny & agrid )
    : wxPanel(&agrid, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL),
      axis( aaxis ),
      grid( agrid  ),
      m_button( NULL )
{
    wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );

    m_minValue = new wxTextCtrl( this, wxID_ANY,
            _T(""), wxDefaultPosition, wxDefaultSize, 0,
            wxTextValidator( wxFILTER_NUMERIC ) );
    m_minValue->SetToolTip( _T("Lower bound for shown values") );

    m_maxValue = new wxTextCtrl( this, wxID_ANY,
            _T(""), wxDefaultPosition, wxDefaultSize, 0,
            wxTextValidator( wxFILTER_NUMERIC ) );
    m_maxValue->SetToolTip( _T("Upper bound for shown values") );

    m_button = new wxButton(this, wxID_ANY, _T("&Apply"));
    m_button->Enable( false );

    sizer->Add( m_minValue, wxSizerFlags().Expand().Border(wxLEFT, 4) );
    sizer->Add( m_maxValue, wxSizerFlags().Expand().Border(wxLEFT, 4) );
    sizer->Add( m_button, wxSizerFlags().Expand().Border(wxLEFT, 3) );

    // maximum possible number of digits for a number in between axis.m_min and axis.m_max
    unsigned int numDigits = 0;
    {
        double maxValue = wxMax( axis.m_min > 0 ? axis.m_min : -axis.m_min,
                                 axis.m_max > 0 ? axis.m_max : -axis.m_max );
        numDigits = (int)log10( wxMax( 1, maxValue ) );
    }

    m_minValue->SetMinSize( GetMinSizeForTextControl( m_minValue, numDigits) );
    m_maxValue->SetMinSize( GetMinSizeForTextControl( m_maxValue, numDigits) );

    SetSizer( sizer );
    sizer->SetSizeHints( this );
    Layout();
}


void MultiDimIntAxisAdjuster::OnRangeChange( wxCommandEvent & event )
{
    if( m_button )
        m_button->Enable( m_minValue->Validate() && m_maxValue->Validate() );
}

void MultiDimIntAxisAdjuster::OnConfirm( wxCommandEvent & event )
{
    return DoOnConfirm();
}

void MultiDimIntAxisAdjuster::DoOnConfirm( )
{
    long minVal, maxVal;
    wxString minStr = m_minValue->GetValue();
    wxString maxStr = m_maxValue->GetValue();
    if( ( !minStr.empty() && !minStr.ToLong(&minVal) )
        || ( !maxStr.empty() && !maxStr.ToLong(&maxVal) ) )
    {
        wxMessageBox( _T("Invalid input - enter numbers only"),
                      _T("Invalid input"), wxOK | wxICON_ERROR, this );
    }
    int minAdjusted = !minStr.empty() ? minVal
                                      : axis.m_minOriginal;
    int maxAdjusted = !maxStr.empty() ? maxVal
                                      : axis.m_maxOriginal;

    m_button->Enable( false );

    axis.DoApplyAdjustment( minAdjusted, maxAdjusted );

    // if entered boundary value was modified by the control - then show it
    if( axis.m_min < minAdjusted || minAdjusted < axis.m_minOriginal )
    {
        m_minValue->SetValue( wxString::Format( _T("%d"), axis.m_min ) );
    }
    // if entered boundary value was modified by the control - then show it
    if( axis.m_max > maxAdjusted || maxAdjusted > axis.m_maxOriginal )
    {
        m_maxValue->SetValue( wxString::Format( _T("%d"), axis.m_max ) );
    }
    grid.UpdateGridAxis( axis.GetName() );
}


// we handle only maturities 12 months ahead
class MaturityAxis : public MultiDimAxisAny
{
public:
    typedef wxDateTime ValueType;

    MaturityAxis() : MultiDimAxisAny("Maturity")
    {
    }

    virtual unsigned int GetCardinality() const { return 12; }
    virtual wxString GetLabel(unsigned int n) const
    {
        return wxDateTime::GetMonthName(static_cast<wxDateTime::Month>(n));
    }
    virtual boost::any GetValue(unsigned int n) const
    {
        // we only interested in the month name
        return boost::any(wxDateTime(
                static_cast<wxDateTime::wxDateTime_t>(1),
                static_cast<wxDateTime::Month>(n),
                1, 0, 0, 0, 0
            ));
    }
};

typedef MultiDimGrid4<unsigned int,
                      TypeAxis,
                      ExerciseAxis,
                      StrikeAxis,
                      MaturityAxis> OptionGrid;

// this is a trivial sparse table implementation
class OptionTable : public MultiDimTable4<unsigned int,
                                          OptionType,
                                          OptionExercise,
                                          int,
                                          wxDateTime>
{
public:
    virtual unsigned int
    GetValue(OptionType type,
             OptionExercise exercise,
             int strike,
             wxDateTime maturity) const
    {
        const Values::const_iterator
            i = m_values.find(make_tuple(type, exercise, strike, maturity));
        return i == m_values.end() ? 0 : i->second;
    }

    virtual void
    SetValue(OptionType type,
             OptionExercise exercise,
             int strike,
             wxDateTime maturity,
             const unsigned int& value)
    {
        m_values[make_tuple(type, exercise, strike, maturity)] = value;
    }

private:
    typedef std::map< tuple<OptionType, OptionExercise, int, wxDateTime>,
                      unsigned int > Values;

    Values m_values;
};



class TestApp : public wxApp
{
public:
  virtual bool OnInit();
};

IMPLEMENT_APP(TestApp)

bool TestApp::OnInit()
{
    wxFrame * frame = new wxFrame(
        NULL, wxID_ANY, _T("MultiDimGridAny Test App"),
        wxPoint( 200, 200 ), wxSize( 600, 400) );

    shared_ptr<OptionTable> table(new OptionTable);

    wxBoxSizer * sizer = new wxBoxSizer( wxVERTICAL );

    OptionGrid * grid = new OptionGrid(frame, table);
    sizer->Add( grid, wxSizerFlags().Proportion(1).Expand() );

    frame->SetSizerAndFit( sizer );
    sizer->SetSizeHints( frame );
    frame->Layout();

    frame->Show(true);
    SetTopWindow(frame);
    frame->Layout();

    frame->SetSize( wxSize( 600, 400) );

//    Uncomment these lines if you want to get highlighting of axis selection
//    grid->SetXAxisColour( wxColour(0, 0, 100) );
//    grid->SetYAxisColour( wxColour(0, 100, 0) );
    return true;
}

