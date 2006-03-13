#include "pedit_pol.hpp"

#include <memory>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/statbox.h>
#include <wx/scrolwin.h>
#include <wx/msgdlg.h>

// --------------------------
//   PeditPOL implementation
// --------------------------

// initialize static variable
const std::string PeditPOL::s_extension = "pol";
namespace
{   // register in the PeditFileFrame
    static bool dummy = PeditPOL::RegisterPeditPOLSubtype();
}

PeditFileFrame * PeditPOL::CreatePOLInstance( wxMDIParentFrame *parent,
                                              std::string const & filename )
{
    try
    {
        return new PeditPOL( parent, filename );
    }
    catch( std::exception & ex )
    {
        wxMessageBox( wxString::Format( _T("Error reading from %s"), filename.c_str() ),
                      _T("error") );
    }
}

PeditPOL::~PeditPOL() {}

PeditPOL::PeditPOL( wxMDIParentFrame * parent, std::string const & filename )
: PeditFileFrame( parent, filename, wxID_ANY, _T("") ),
  TProductData( filename ), m_modified( false )
{
    wxScrolledWindow * mainPanel = new wxScrolledWindow( this );

    std::auto_ptr<wxBoxSizer> mainSizer( new wxBoxSizer( wxHORIZONTAL ) );

    // Top part of the panel
    wxPanel * leftPanel = new wxPanel( mainPanel, wxID_ANY );
    wxBoxSizer * leftPanelSizer = new wxStaticBoxSizer( new wxStaticBox( leftPanel, wxID_ANY, _T("Parameters") ), wxVERTICAL );
    wxFlexGridSizer * leftSizer = new wxFlexGridSizer( 7, 2, 2, 4 );
    leftPanelSizer->Add( leftSizer, wxSizerFlags().Expand().Proportion(1) );
    leftPanel->SetSizerAndFit( leftPanelSizer );
    leftPanelSizer->SetSizeHints( leftPanel );
    mainSizer->Add( leftPanel, wxSizerFlags().Expand().Proportion(1) );

    // Right part of the panel
    wxPanel * rightPanel = new wxPanel( mainPanel, wxID_ANY );
    wxBoxSizer * rightPanelSizer = new wxStaticBoxSizer( new wxStaticBox( rightPanel, wxID_ANY, _T("Tables") ), wxVERTICAL );
    wxFlexGridSizer * rightSizer = new wxFlexGridSizer( 8, 2, 2, 4 );
    rightPanelSizer->Add( rightSizer, wxSizerFlags().Expand().Proportion(1) );
    rightPanel->SetSizerAndFit( rightPanelSizer );
    rightPanelSizer->SetSizeHints( rightPanel );
    mainSizer->Add( rightPanel, wxSizerFlags().Expand().Proportion(1) );

    long labelStyle = wxALIGN_RIGHT;
    wxSizerFlags labelFlags = wxSizerFlags().Expand().Proportion(0);
    long textStyle = wxTE_LEFT;
    wxSizerFlags textFlags = wxSizerFlags().Expand().Proportion(1);

    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Database"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_paramDatabase = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Tiered"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_paramTiered = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Rounding"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_paramRounding = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Funds"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_paramFunds = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );

    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Table Y"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_oldTableY = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle | wxTE_READONLY ), textFlags );
    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("Premium Tax"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_oldPremiumTax = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle | wxTE_READONLY ), textFlags );
    leftSizer->Add( new wxStaticText( leftPanel, wxID_ANY, _T("83 GAM"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    leftSizer->Add( m_old83GAM = new wxTextCtrl( leftPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle | wxTE_READONLY ), textFlags );

    wxString const deprecatedTooltip = _T("Deprecated -- do not use");
    m_oldTableY->Disable();     m_oldTableY->SetToolTip( deprecatedTooltip );
    m_oldPremiumTax->Disable(); m_oldPremiumTax->SetToolTip( deprecatedTooltip );
    m_old83GAM->Disable();      m_old83GAM->SetToolTip( deprecatedTooltip );

    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("Current COI"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableCurrentCOI = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("Guaranteed COI"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableGuarranteedCOI = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("Waiver of premium"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableWaiverPremium = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("AD&D"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableADD = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("Term rider"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableTermRider = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("CVAT corridor"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableCVAT = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("TAMRA 7 pay"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_tableTAMRA = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );
    rightSizer->Add( new wxStaticText( rightPanel, wxID_ANY, _T("7702 and 7702A q"), wxDefaultPosition, wxDefaultSize, labelStyle ), labelFlags );
    rightSizer->Add( m_table7702Q = new wxTextCtrl( rightPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, textStyle ), textFlags );

    SyncDataToUI();

    mainSizer->SetSizeHints( mainPanel );
    mainPanel->SetSizerAndFit( mainSizer.release() );

    mainPanel->FitInside();

    Layout();
}

void PeditPOL::SyncDataToUI()
{
    m_paramDatabase->SetValue( TProductData::GetDatabaseFilename() );
    m_paramTiered->SetValue( TProductData::GetTierFilename() );
    m_paramRounding->SetValue( TProductData::GetRoundingFilename() );
    m_paramFunds->SetValue( TProductData::GetFundFilename() );
    
    m_oldTableY->SetValue( TProductData::GetTableYFilename() );
    m_oldPremiumTax->SetValue( TProductData::GetPremTaxFilename() );
    m_old83GAM->SetValue( TProductData::GetGam83Filename() );
    
    m_tableCurrentCOI->SetValue( TProductData::GetCurrCOIFilename() );
    m_tableGuarranteedCOI->SetValue( TProductData::GetGuarCOIFilename() );
    m_tableWaiverPremium->SetValue( TProductData::GetWPFilename() );
    m_tableADD->SetValue( TProductData::GetADDFilename() );
    m_tableTermRider->SetValue( _T("?????") ); m_tableTermRider->Disable();
    m_tableCVAT->SetValue( TProductData::GetCorridorFilename() );
    m_tableTAMRA->SetValue( TProductData::GetTAMRA7PayFilename() );
    m_table7702Q->SetValue( TProductData::GetIRC7702Filename() );
}

void PeditPOL::SyncUIToData()
{
    TProductData::DatabaseFilename = m_paramDatabase->GetValue().c_str();
    TProductData::TierFilename = m_paramTiered->GetValue().c_str();
    TProductData::RoundingFilename = m_paramRounding->GetValue().c_str();
    TProductData::FundFilename = m_paramFunds->GetValue().c_str();
        
    TProductData::TableYFilename = m_oldTableY->GetValue().c_str();
    TProductData::PremTaxFilename = m_oldPremiumTax->GetValue().c_str();
    TProductData::Gam83Filename = m_old83GAM->GetValue().c_str();
        
    TProductData::CurrCOIFilename = m_tableCurrentCOI->GetValue().c_str();
    TProductData::GuarCOIFilename = m_tableGuarranteedCOI->GetValue().c_str();
    TProductData::WPFilename = m_tableWaiverPremium->GetValue().c_str();
    TProductData::ADDFilename = m_tableADD->GetValue().c_str();
//        m_tableTermRider->SetValue( _T("?????") ); m_tableTermRider->Disable();
    TProductData::CorridorFilename = m_tableCVAT->GetValue().c_str();
    TProductData::TAMRA7PayFilename = m_tableTAMRA->GetValue().c_str();
    TProductData::IRC7702Filename = m_table7702Q->GetValue().c_str();
}

void PeditPOL::DoSave()
{
    SyncUIToData();
    TProductData::Write( GetFilename() );
    SetModified( false );
}

bool PeditPOL::DoIsModified() const
{
    return m_modified;
}

void PeditPOL::SetModified( bool modified )
{
    m_modified = modified;
}

