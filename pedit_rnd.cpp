// Product editor part for rnd file type.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: pedit_rnd.cpp,v 1.1.2.5 2006-03-29 11:02:56 etarassov Exp $

#include "pedit_rnd.hpp"

#include <memory>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/bmpbuttn.h>
#include <wx/tglbtn.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
// needed by GetMinPossibleSize to calculate minimum size based on string length
#include <wx/dcclient.h>

wxSize GetMinPossibleSize( wxWindow * win, unsigned int numLetters = 1 )
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

class RNDStyleButtons : public wxPanel
{
public:
    RNDStyleButtons( wxWindow * parent, rounding_style style = r_indeterminate );
    
    void SetStyle( rounding_style style );
    
    rounding_style GetStyle() const;

    void SetConstraint( rounding_style style );



private:
    rounding_style m_style;
    wxToggleButton * m_btnNotAtAll;
    wxToggleButton * m_btnToNearest;
    wxToggleButton * m_btnUpward;
    wxToggleButton * m_btnDownward;
    
    rounding_style m_originalStyle;

    void OnButtonClick( wxCommandEvent & event );

    DECLARE_EVENT_TABLE()
};

enum
{
    ID_NOTATALL = wxID_HIGHEST,
    ID_TONEAREST,
    ID_UPWARD,
    ID_DOWNWARD
};

BEGIN_EVENT_TABLE(RNDStyleButtons, wxPanel)
    EVT_TOGGLEBUTTON( ID_NOTATALL, RNDStyleButtons::OnButtonClick )
    EVT_TOGGLEBUTTON( ID_TONEAREST, RNDStyleButtons::OnButtonClick )
    EVT_TOGGLEBUTTON( ID_UPWARD, RNDStyleButtons::OnButtonClick )
    EVT_TOGGLEBUTTON( ID_DOWNWARD, RNDStyleButtons::OnButtonClick )
END_EVENT_TABLE()

void RNDStyleButtons::OnButtonClick( wxCommandEvent & event )
{
    int id = event.GetId();
    if( id == ID_NOTATALL )
        SetStyle( r_not_at_all );
    else if( id == ID_TONEAREST )
        SetStyle( r_to_nearest );
    else if( id == ID_UPWARD )
        SetStyle( r_upward );
    else if( id == ID_DOWNWARD )
        SetStyle( r_downward );
}

RNDStyleButtons::RNDStyleButtons( wxWindow * parent, rounding_style style )
: wxPanel( parent, wxID_ANY ), m_style( r_indeterminate ), m_originalStyle( style )
{
    m_btnNotAtAll = new wxToggleButton( this, ID_NOTATALL, _T("@") );
    m_btnNotAtAll->SetToolTip( _("does nothing") );
    m_btnToNearest = new wxToggleButton( this, ID_TONEAREST, _T("x") );
    m_btnToNearest->SetToolTip( _("round to the nearest") );
    m_btnUpward = new wxToggleButton( this, ID_UPWARD, _T("A") );
    m_btnUpward->SetToolTip( _("round upward") );
    m_btnDownward = new wxToggleButton( this, ID_DOWNWARD, _T("V") );
    m_btnDownward->SetToolTip( _("round downward") );

    // lets calculate the minimal width for our toggle buttons
    wxSize btnSize = GetMinPossibleSize( m_btnNotAtAll, 3 );
    btnSize.SetHeight( m_btnNotAtAll->GetSize().GetHeight() );

    m_btnNotAtAll->SetMinSize( btnSize );
    m_btnToNearest->SetMinSize( btnSize );
    m_btnUpward->SetMinSize( btnSize );
    m_btnDownward->SetMinSize( btnSize );

    wxBoxSizer * sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add( m_btnNotAtAll );
    sizer->Add( m_btnToNearest );
    sizer->Add( m_btnUpward );
    sizer->Add( m_btnDownward );

    SetSizerAndFit( sizer );
    sizer->SetSizeHints( this );
    
    SetStyle( style );
}

void RNDStyleButtons::SetStyle( rounding_style style )
{
    m_btnNotAtAll->SetValue( style == r_not_at_all );
    m_btnToNearest->SetValue( style == r_to_nearest );
    m_btnUpward->SetValue( style == r_upward );
    m_btnDownward->SetValue( style == r_downward );
}

rounding_style RNDStyleButtons::GetStyle() const
{
    if( m_btnNotAtAll->GetValue() )
        return r_not_at_all;
    if( m_btnToNearest->GetValue() )
        return r_to_nearest;
    if( m_btnUpward->GetValue() )
        return r_upward;
    if( m_btnDownward->GetValue() )
        return r_downward;
    return r_indeterminate;
}

void RNDStyleButtons::SetConstraint( rounding_style style )
{
    SetStyle( style );
    m_btnNotAtAll->Enable( style == r_not_at_all );
    m_btnToNearest->Enable( style == r_to_nearest );
    m_btnUpward->Enable( style == r_upward );
    m_btnDownward->Enable( style == r_downward );
}

// --------------------------
//   PeditRND implementation
// --------------------------

// initialize static variable
const std::string PeditRND::s_extension = "rnd";
namespace
{   // register in the PeditFileFrame
    static bool dummy = PeditRND::RegisterPeditRNDSubtype();
}

PeditFileFrame * PeditRND::CreateRNDInstance( wxMDIParentFrame *parent,
                                              std::string const & filename )
{
    try
    {
        return new PeditRND( parent, filename );
    }
    catch( std::exception const & ex )
    {
        wxMessageBox( wxString::Format( _("Error [%s] while reading from .rnd file [%s]"),
                                        ex.what(), filename.c_str() ),
                      _("Error") );
        throw;
    }
}

PeditRND::~PeditRND() {}

enum
{
    PeditRND_SIZER_VGAP = 4,
    PeditRND_SIZER_HGAP = 8
};

PeditRND::PeditRND( wxMDIParentFrame *parent, std::string const & filename )
: PeditFileFrame( parent, filename, wxID_ANY, _T("") ),
  StreamableRoundingRules( filename ), m_modified( false )
{
    wxPanel * main = new wxPanel( this );
    // wxFlexGridSizer(int rows, int cols, int vgap, int hgap)
    std::auto_ptr<wxFlexGridSizer> sizer(
        new wxFlexGridSizer( 13, 9, PeditRND_SIZER_HGAP, PeditRND_SIZER_VGAP ) );

    sizer->AddGrowableCol( 0, 1 );
    sizer->AddGrowableCol( 4, 1 );
    sizer->AddGrowableCol( 8, 1 );

    wxSizerFlags flagsHeader = wxSizerFlags().Expand().Center()
            .Proportion( 0 ).Border( wxLEFT|wxRIGHT, PeditRND_SIZER_HGAP );

    // top labels
    sizer->AddStretchSpacer( 1 );
    sizer->AddSpacer( PeditRND_SIZER_HGAP );
    sizer->Add( new wxStaticText( main, wxID_ANY, _("Direction") ), flagsHeader );
    sizer->Add( new wxStaticText( main, wxID_ANY, _("Decimals") ), flagsHeader );
    sizer->AddStretchSpacer( 1 );
    sizer->AddSpacer( PeditRND_SIZER_HGAP );
    sizer->Add( new wxStaticText( main, wxID_ANY, _("Direction") ), flagsHeader );
    sizer->Add( new wxStaticText( main, wxID_ANY, _("Decimals") ), flagsHeader );
    sizer->AddStretchSpacer( 1 );

    // creates controls and read the data
    DoOnAllFields( true, main, sizer.get() );

    sizer->SetSizeHints( main );
    main->SetSizerAndFit( sizer.release() );

    Layout();
}

// if populate is true, then Data -> UI
// otherwise       UI -> Data
void PeditRND::DoOnAllFields( bool populate, wxWindow * parent, wxSizer * sizer )
{
    struct Field
    {
        wxString label;
        RNDStyleButtons *& btns;
        wxSpinCtrl *& spin;
        round_to<double> & rounder;
    };
    static unsigned int const FieldCount = 14;
    Field fields[FieldCount] = {
    { gettext_noop("Special Amount"),     m_btnsSpecAmount,   m_spinSpecAmount,   rounding_rules::round_specamt_ },
    { gettext_noop("Death benefit"),      m_btnsDeathBenefit, m_spinDeathBenefit, rounding_rules::round_death_benefit_ },
    { gettext_noop("NAAR"),               m_btnsNAAR,         m_spinNAAR,         rounding_rules::round_naar_ },
    { gettext_noop("COI rate"),           m_btnsCOIRate,      m_spinCOIRate,      rounding_rules::round_coi_rate_ },
    { gettext_noop("Mortality charge"),   m_btnsMortality,    m_spinMortality,    rounding_rules::round_coi_charge_ },
    { gettext_noop("Gross premium"),      m_btnsGrossPremium, m_spinGrossPremium, rounding_rules::round_gross_premium_ },
    { gettext_noop("Net premium"),        m_btnsNetPremium,   m_spinNetPremium,   rounding_rules::round_net_premium_ },
    { gettext_noop("Interest rate"),      m_btnsIntRate,      m_spinIntRate,      rounding_rules::round_interest_rate_ },
    { gettext_noop("Intereset credit"),   m_btnsIntCredit,    m_spinIntCredit,    rounding_rules::round_interest_credit_ },
    { gettext_noop("Withdrawal"),         m_btnsWithdrawal,   m_spinWithdrawal,   rounding_rules::round_withdrawal_ },
    { gettext_noop("Loan"),               m_btnsLoan,         m_spinLoan,         rounding_rules::round_loan_ },
    { gettext_noop("Non-7702 corridor"),  m_btns7702,         m_spin7702,         rounding_rules::round_corridor_factor_ },
    { gettext_noop("Surrender charge"),   m_btnsSurrCharge,   m_spinSurrCharge,   rounding_rules::round_surrender_charge_ },
    { gettext_noop("IRR"),                m_btnsIRR,          m_spinIRR,          rounding_rules::round_irr_ } };

    if( populate )
    {
        // create UI and load data into it
        wxSizerFlags flagsLabel = wxSizerFlags().Expand().Right()
                .Proportion( 0 ).Border( wxLEFT|wxRIGHT, PeditRND_SIZER_HGAP );
        wxSizerFlags flagsButtons = wxSizerFlags().Expand().Center()
                .Proportion( 0 ).Border( wxLEFT|wxRIGHT, PeditRND_SIZER_HGAP );
        wxSizerFlags flagsSpin = wxSizerFlags().Expand().Center()
                .Proportion( 0 ).Border( wxLEFT|wxRIGHT, PeditRND_SIZER_HGAP );

        // create two columns of controls, the left one contains 12 controls
        // the right one -- only 2. Maybe make 7 controls in both?
        static unsigned int const VertCount = 12;
        static unsigned int const TotalCount = 2 * VertCount;
        for( unsigned int i = 0; i < TotalCount; ++i )
        {
            sizer->AddStretchSpacer( 1 );
            unsigned int id = ( i % 2 == 0 ) ? (i / 2) : ( VertCount + i / 2 );
            if( id < FieldCount )
            {
                Field field = fields[id];
                sizer->Add( new wxStaticText( parent, wxID_ANY, _(field.label) ), flagsLabel );
                sizer->Add( field.btns = new RNDStyleButtons( parent ), flagsButtons );
                sizer->Add( field.spin = new wxSpinCtrl( parent ), flagsSpin );

                wxSize spinSize = GetMinPossibleSize( field.spin, 6 );
                spinSize.SetHeight( field.spin->GetSize().GetHeight() );
                field.spin->SetMinSize( spinSize );

                field.spin->SetValue( field.rounder.decimals() );
                field.btns->SetStyle( field.rounder.style() );
                // the last one, "IRR" must be rounded down
                if( id == FieldCount - 1 )
                {
                    field.btns->SetConstraint( r_downward );
                }
            }
            else
            {
                sizer->AddSpacer( PeditRND_SIZER_HGAP );
                sizer->AddSpacer( PeditRND_SIZER_HGAP );
                sizer->AddSpacer( PeditRND_SIZER_HGAP );
            }
            if( i % 2 != 0 )
            {
                sizer->AddStretchSpacer( 1 );
            }
        }
    }
    else // populate
    {
        // load data from UI
        for( unsigned int i = 0; i < FieldCount; ++i )
        {
            Field field = fields[i];
            field.rounder = round_to<double>( field.spin->GetValue(), field.btns->GetStyle() );
        }
    }
}

void PeditRND::SyncUIToData()
{
    DoOnAllFields( false );
}

void PeditRND::DoSave()
{
    SyncUIToData();
    StreamableRoundingRules::Write( GetFilename() );
    SetModified( false );
}

bool PeditRND::DoIsModified() const
{
    return m_modified;
}
