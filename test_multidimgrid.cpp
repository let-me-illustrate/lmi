// Test case for multidimentional grid editor gui component.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: test_multidimgrid.cpp,v 1.1.2.5 2006-04-10 20:26:03 etarassov Exp $

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
#include <bitset>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/preprocessor/repetition.hpp>

#include <cmath>     // std::log10()

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
    ExerciseAxis()
        : MultiDimEnumAxis<OptionExercise>("Exercise type",
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
    MultiDimIntAxisAdjuster( MultiDimAdjustableIntAxis & axis,
                             MultiDimGrid & grid );
    int GetMinValue() const;
    int GetMaxValue() const;
    void SetMinValue( int minValue );
    void SetMaxValue( int maxValue );

private:
    MultiDimAdjustableIntAxis & m_axis;

    wxTextCtrl * m_minValue;
    wxTextCtrl * m_maxValue;
    wxButton * m_button;

    /// number of digits needed to represent a value from the range [m_minValue, m_maxValue]
    unsigned int m_numDigits;

    // event handler function called when min/max value is changed
    void OnRangeChange( wxCommandEvent & event );
    // event handler function called when 'apply' button is pushed
    void OnConfirm( wxCommandEvent & event );

    friend class MultiDimAdjustableIntAxis;

    // function that really validates the input
    bool DoValidateInput() const;
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
class MultiDimAdjustableIntAxis : public MultiDimAdjustableAxis<
                                                MultiDimIntAxisAdjuster,
                                                MultiDimIntAxis >
{
public:
    typedef MultiDimAdjustableAxis< MultiDimIntAxisAdjuster, MultiDimIntAxis >
            BaseClass;

    using MultiDimIntAxis::SetValues;
    using MultiDimIntAxis::GetMinValue;
    using MultiDimIntAxis::GetMaxValue;
    using BaseClass::GetAdjustControl;
    using BaseClass::ApplyAdjustment;
    using BaseClass::RefreshAdjustment;

    // see MultiDimIntAxis::MultiDimIntAxis constructor for details
    MultiDimAdjustableIntAxis( const wxString& name,
                               int minValue,
                               int maxValue,
                               int step )
    : BaseClass( name )
    {
        SetValues( minValue, maxValue, step );
    }

    MultiDimAdjustableIntAxis( const wxString& name )
    : BaseClass( name )
    {
        SetValues( 0, 100, 1 );
    }

protected:
    // create the adjustment control
    MultiDimIntAxisAdjuster * DoGetAdjustControl( MultiDimGrid & grid,
                                                  MultiDimTableAny & table )
    {
        return new MultiDimIntAxisAdjuster( *this, grid );
    }

    // queries adjustment window for a new range information
    virtual bool DoApplyAdjustment( MultiDimIntAxisAdjuster * adjustWin,
                                    unsigned int n )
    {
        if( adjustWin != NULL )
        {
            bool updated = adjustWin->GetMinValue() == GetMinValue();
            updated = updated || adjustWin->GetMaxValue() == GetMaxValue();
            SetValues( adjustWin->GetMinValue(), adjustWin->GetMaxValue(),
                       GetStep() );
            return updated;
        }
        return false;
    }

    // refreshes the adjustment window with the correct range information
    virtual bool DoRefreshAdjustment( MultiDimIntAxisAdjuster * adjustWin,
                                      unsigned int n )
    {
        if( adjustWin != NULL )
        {
            bool updated = adjustWin->GetMinValue() == GetMinValue();
            updated = updated || adjustWin->GetMaxValue() == GetMaxValue();
            adjustWin->SetMinValue( GetMinValue() );
            adjustWin->SetMaxValue( GetMaxValue() );
            return updated;
        }
        return false;
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

// --------------------------------------
// MultiDimIntAxisAdjuster implementation
// --------------------------------------
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

MultiDimIntAxisAdjuster::MultiDimIntAxisAdjuster( MultiDimAdjustableIntAxis & axis,
                                                  MultiDimGrid & grid )
    : wxPanel(&grid, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL),
      m_axis( axis ), m_button( NULL ), m_numDigits( 3 )
{
    wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );

    m_minValue = new wxTextCtrl( this, wxID_ANY,
            _T(""), wxDefaultPosition, wxDefaultSize, 0,
            wxTextValidator( wxFILTER_NUMERIC ) );
    m_minValue->SetToolTip( _("Lower bound for shown values") );

    m_maxValue = new wxTextCtrl( this, wxID_ANY,
            _T(""), wxDefaultPosition, wxDefaultSize, 0,
            wxTextValidator( wxFILTER_NUMERIC ) );
    m_maxValue->SetToolTip( _("Upper bound for shown values") );

    m_button = new wxButton(this, wxID_ANY, _("&Apply"));
    m_button->Enable( false );

    sizer->Add( m_minValue, wxSizerFlags().Expand().Border(wxLEFT, 4) );
    sizer->Add( m_maxValue, wxSizerFlags().Expand().Border(wxLEFT, 4) );
    sizer->Add( m_button, wxSizerFlags().Expand().Border(wxLEFT, 3) );

    SetMinValue( 0 );
    SetMaxValue( 0 );

    SetSizer( sizer );
    sizer->SetSizeHints( this );
    Layout();
}


void MultiDimIntAxisAdjuster::OnRangeChange( wxCommandEvent & event )
{
    if( m_button )
        m_button->Enable( DoValidateInput() );
}

void MultiDimIntAxisAdjuster::OnConfirm( wxCommandEvent & event )
{
    return DoOnConfirm();
}

int MultiDimIntAxisAdjuster::GetMinValue( ) const
{
    long minVal;
    return m_minValue->GetValue().ToLong(&minVal) ? minVal : 0;
}

int MultiDimIntAxisAdjuster::GetMaxValue( ) const
{
    long maxVal;
    return m_maxValue->GetValue().ToLong(&maxVal) ? maxVal : 0;
}

void MultiDimIntAxisAdjuster::SetMinValue( int minValue )
{
    m_minValue->SetValue( wxString::Format( _T("%d"), minValue ) );

    // maximum possible number of digits for a number in between axis.m_min and axis.m_max
    unsigned int numDigits = static_cast< unsigned int>( std::log10(
                                wxMax( 1, wxMax( minValue, -minValue ) ) ) );
    m_numDigits = wxMax( m_numDigits, numDigits );

    m_minValue->SetMinSize( GetMinSizeForTextControl( m_minValue, m_numDigits) );
}

void MultiDimIntAxisAdjuster::SetMaxValue( int maxValue )
{
    m_maxValue->SetValue( wxString::Format( _T("%d"), maxValue ) );

    // maximum possible number of digits for a number in between axis.m_min and axis.m_max
    unsigned int numDigits = static_cast< unsigned int>( std::log10(
                                wxMax( 1, wxMax( maxValue, -maxValue ) ) ) );
    m_numDigits = wxMax( m_numDigits, numDigits );

    m_maxValue->SetMinSize( GetMinSizeForTextControl( m_maxValue, m_numDigits) );
}

bool MultiDimIntAxisAdjuster::DoValidateInput() const
{
    if( !m_minValue->Validate() || !m_maxValue->Validate() )
        return false;
    long minVal, maxVal;
    if( !m_minValue->GetValue().ToLong( &minVal )
     || !m_maxValue->GetValue().ToLong( &maxVal ) )
        return false;
    return minVal <= maxVal;
}

void MultiDimIntAxisAdjuster::DoOnConfirm( )
{
    if( !DoValidateInput() )
    {
        wxMessageBox( _("Invalid input - enter numbers only"),
                      _("Invalid input"), wxOK | wxICON_ERROR, this );
        return;
    }
    MultiDimGrid * grid = dynamic_cast<MultiDimGrid *>( GetParent() );

    if( grid )
    {
        grid->ApplyAxisAdjustment( m_axis.GetName() );

        m_button->Enable( false );
    }
}

// we handle only maturities 12 months ahead
class MaturityAxis : public MultiDimAxis<wxDateTime>
{
public:
    typedef wxDateTime ValueType;

    MaturityAxis() : MultiDimAxis<wxDateTime>("Maturity")
    {
    }

    virtual unsigned int GetCardinality() const { return 12; }
    virtual wxString GetLabel(unsigned int n) const
    {
        return wxDateTime::GetMonthName(static_cast<wxDateTime::Month>(n));
    }
    virtual wxDateTime DoGetValue(unsigned int n) const
    {
        // we only interested in the month name
        return wxDateTime(
                static_cast<wxDateTime::wxDateTime_t>(1),
                static_cast<wxDateTime::Month>(n),
                1, 0, 0, 0, 0
            );
    }
};

// Template used by OptionTableDataComparator to inline a set of key comparisons
template< int N, int n, typename Tuple, typename Varies >
struct OptionTableDataComparatorHelper;

// Specialisation for the last element in the keys (end of the comparison list)
template< int N, typename Tuple, typename Varies >
struct OptionTableDataComparatorHelper<N, N, Tuple, Varies>
{
    inline static
    bool compare( Tuple const & k1, Tuple const & k2, Varies const & varies )
    {   return false;   }
};

// Compare nth key element and recursivly call for the next element
template< int N, int n, typename Tuple, typename Varies >
struct OptionTableDataComparatorHelper
{
    inline static
    bool compare( Tuple const & k1, Tuple const & k2, Varies const & varies )
    {
        if( varies.test( n ) )
        {
            if( k1.get<n>() < k2.get<n>() ) return true;
            if( k2.get<n>() < k1.get<n>() ) return false;
        }
        return OptionTableDataComparatorHelper< N, n+1, Tuple, Varies >
                                            ::compare( k1, k2, varies );
    }
};

// Template for comparing tuples of length N.
// Uses OptionTableDataComparatorHelper to generate the code
template< int N, typename Tuple >
class OptionTableDataComparator
{
public:
    typedef std::bitset<N> Varies;
    explicit OptionTableDataComparator( Varies & varies )
    : m_varies( varies ) {}
    OptionTableDataComparator( OptionTableDataComparator const & comp )
    : m_varies( comp.m_varies ) {}

    OptionTableDataComparator & operator = ( OptionTableDataComparator const & comp )
    {  return *this; }

    bool operator() ( Tuple const & k1, Tuple const & k2 ) const
    {
        return OptionTableDataComparatorHelper< N, 0, Tuple, Varies >
                                                ::compare( k1, k2, m_varies );
    }
private:
    Varies const & m_varies;
};


// Trivial sparse table implementation
// The reason to use multimap (instead of usual map container) is only
// not to immediatly discard data when user disables
// of the axis (means to disable and to enable an axis, discards no data)
class OptionTable : public MultiDimTable4< unsigned int,
                                           OptionType,
                                           OptionExercise,
                                           int,
                                           wxDateTime >
{
public:
    OptionTable();
    virtual unsigned int
    GetValue( OptionType type,
              OptionExercise exercise,
              int strike,
              wxDateTime maturity ) const;

    virtual void
    SetValue( OptionType type,
              OptionExercise exercise,
              int strike,
              wxDateTime maturity,
              const unsigned int& value );

    virtual bool VariesByDimension( unsigned int n ) const;
    virtual void MakeVaryByDimension( unsigned int n, bool val );
    virtual bool CanChangeVariationWith( unsigned int n ) const;

    // keep the same empty implementation of these methods
//    bool DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
//    bool DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )

protected:
    virtual MultiDimAxis<OptionType> * GetAxis0()
    {   return new TypeAxis();  }

    virtual MultiDimAxis<OptionExercise> * GetAxis1()
    {   return new ExerciseAxis();  }

    virtual MultiDimAxis<int> * GetAxis2()
    {   return new StrikeAxis();  }

    virtual MultiDimAxis<wxDateTime> * GetAxis3()
    {   return new MaturityAxis();  }

private:
    // structure that hold data variation along different axis
    // for every axis Varies[n] indicates whether data varies with that nth axis
    typedef std::bitset<4> Varies;

    typedef tuple<OptionType, OptionExercise, int, wxDateTime> ValueKey;

    typedef OptionTableDataComparator< 4, ValueKey > Comparator;
    typedef std::multimap< ValueKey, unsigned int, Comparator > Values;

    // bitset holding data variation along dimentions
    Varies m_varies;

    // the actual data
    Values m_values;
};

OptionTable::OptionTable()
: m_varies(), m_values( Comparator(m_varies) )
{
}

unsigned int OptionTable::GetValue( OptionType type,
                                    OptionExercise exercise,
                                    int strike,
                                    wxDateTime maturity ) const
{
    ValueKey tuple = make_tuple(type, exercise, strike, maturity);
    const Values::const_iterator lower = m_values.lower_bound( tuple );
    const Values::const_iterator upper = m_values.upper_bound( tuple );

    // disambiguate between [lower, upper) found elements
    // take the one with the lowest key (in dictionary order)
    unsigned int result = 0;
    bool first = true;
    for( Values::const_iterator cit = lower; cit != upper; ++cit )
    {
        if( first )
        {
            tuple = cit->first;
            first = false;
            result = cit->second;
        }
        else if( cit->first < tuple )
        {
            tuple = cit->first;
            result = cit->second;
        }
    }
    return result;
}

void OptionTable::SetValue( OptionType type,
                            OptionExercise exercise,
                            int strike,
                            wxDateTime maturity,
                            const unsigned int& value )
{
    // just erase old matching values and replace it with one entered value
    ValueKey tuple = make_tuple(type, exercise, strike, maturity);
    m_values.erase( m_values.lower_bound( tuple ), m_values.upper_bound( tuple ) );
    m_values.insert( std::make_pair( tuple, value ) );
}

bool OptionTable::VariesByDimension( unsigned int n ) const
{
    wxASSERT_MSG( n < m_varies.size(), _T("incorrect dimension") );
    return m_varies.test(n);
}

void OptionTable::MakeVaryByDimension( unsigned int n, bool val )
{
    wxASSERT_MSG( n < m_varies.size(), _T("incorrect dimension") );

    m_varies.set( n, val );
}

bool OptionTable::CanChangeVariationWith( unsigned int n ) const
{
    return (n != 0);
}

class TestApp : public wxApp
{
public:
  virtual bool OnInit();
};

IMPLEMENT_APP(TestApp)

bool TestApp::OnInit()
{
    wxFrame * frame = new wxFrame(
        NULL, wxID_ANY, _("MultiDimGrid Test App"),
        wxPoint( 200, 200 ), wxSize( 600, 400) );

    shared_ptr<OptionTable> table(new OptionTable);
    table->MakeVaryByDimension( 0, true );
    table->MakeVaryByDimension( 2, true );

    wxBoxSizer * sizer = new wxBoxSizer( wxVERTICAL );

    MultiDimGrid * grid = new MultiDimGrid(frame, table);
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
