// Product editor part for tir file type.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: pedit_tir.cpp,v 1.1.2.5 2006-04-10 20:26:03 etarassov Exp $

#include "pedit_tir.hpp"
#include "stratified_charges.xpp"

#include <limits>
#include <boost/shared_ptr.hpp>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/timer.h>

// Some features not implemented in stratified_charges.*pp files that we need
struct tir_names
{
    enum e_stratified    Idx;
    e_stratified    ParentIdx;
    char const*     ShortName;
    char const*     LongName;
};

std::vector<tir_names> const & GetTIRNames()
{
    static int const n = e_stratified_last;

    static tir_names const static_TIRNames[n]
        = { LMI_TIERED_CHARGE_NAMES };

    static std::vector<tir_names> const v( static_TIRNames + 0, (static_TIRNames + n) );

    return v;
}

// -------------------------
// TIRSEntity implementation
// -------------------------
// TIRSEntity object could have NULL underlying stratified_entity. Therefore
// at any time (when internal data is accessed) there is wxASSERT for
// m_entity not to be NULL
// That behaviour is intentional - it allows the data-table to point nowhere.
// When we point to a node entity, there is no corresponding stratified_entity
// and we should not construct an empty one. Therefore we assure that when
// such a node is selected (therefore NULL entity passed to the table)
// the MDGrid object is disabled and does not query data-table.
TIRSEntity::TIRSEntity( stratified_entity * pEntity )
: m_entity( pEntity )
{}

DoublePair TIRSEntity::GetValue( unsigned int band ) const
{
    // do wxASSERT and vector::operator[] instead of vector::at() to perform
    // faster at runtime. Band being out of range is internal library error and
    // should be check for only at debug
    wxASSERT_MSG( m_entity, _T("This TIRSEntity has null entity") ); // only for debug information if NULL
    wxASSERT_MSG( band < m_entity->limits_.size() && band < m_entity->values_.size(),
                  _T("Incorect band number") );
    return DoublePair( m_entity->limits_[band], m_entity->values_[band] );
}

void TIRSEntity::SetValue( unsigned int band, DoublePair const & value )
{
    wxASSERT_MSG( m_entity, _T("This TIRSEntity has null entity") ); // only for debug information if NULL
    wxASSERT_MSG( band < m_entity->limits_.size() && band < m_entity->values_.size(),
                  _T("Incorect band number") );

    m_entity->limits_[band] = value.first;
    m_entity->values_[band] = value.second;
}

void TIRSEntity::SetBandsCount( unsigned int n )
{
    wxASSERT_MSG( m_entity, _T("This TIRSEntity has null entity") ); // only for debug information, if NULL
    wxASSERT_MSG( m_entity->limits_.size() == m_entity->values_.size(),
                  _T("Incorect data inside the entity, size does not match") );
    wxASSERT_MSG( !(m_entity->limits_.empty()) && n > 0,
                  _T("There should be at least one band") );

    if( n == m_entity->limits_.size() )
        return;

    static double const max_double = std::numeric_limits<double>::max();

    unsigned int size = m_entity->limits_.size();

    if( n < size )
    {
        m_entity->limits_.erase( m_entity->limits_.begin() + n - 1,
                                 m_entity->limits_.begin() + size - 1 );
        m_entity->values_.erase( m_entity->values_.begin() + n - 1,
                                 m_entity->values_.begin() + size - 1 );
    }
    else
    {
        double addLimit = (size >= 2) ? m_entity->limits_[size - 2] : 0;
        double addValue = (size >= 2) ? m_entity->values_[size - 2] : max_double;
        m_entity->limits_.insert( m_entity->limits_.begin() + size - 1, n - size, addLimit );
        m_entity->values_.insert( m_entity->values_.begin() + size - 1, n - size, addValue );
    }
}

unsigned int TIRSEntity::GetBandsCount() const
{
    return m_entity ? m_entity->limits_.size() : 1;
}

// ----------------------------
// TIRPeditTable implementation
// ----------------------------
TIRPeditTable::TIRPeditTable( TIRSEntity pEntity = 0 )
: m_entity( pEntity )
{}

TIRPeditTable::~TIRPeditTable()
{}

DoublePair TIRPeditTable::GetValue( unsigned int band ) const
{
    if( m_entity.IsEmpty() )
        return DoublePair(0,0);
    wxASSERT_MSG( band < m_entity.GetBandsCount(), _T("Incorrect band index") );
    return m_entity.GetValue( band );
}

void TIRPeditTable::SetValue( unsigned int band, DoublePair const & value )
{
    if( m_entity.IsEmpty() )
        return;
    wxASSERT_MSG( band < m_entity.GetBandsCount(), _T("Incorrect band index") );
    m_entity.SetValue( band, value );
}

void TIRPeditTable::SetBandsCount( unsigned int n )
{
    m_entity.SetBandsCount( n );
}

unsigned int TIRPeditTable::GetBandsCount() const
{
    return m_entity.GetBandsCount();
}

bool TIRPeditTable::DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
{
    bool updated = false;
    wxASSERT_MSG( n == 0, _T("TIRPeditTable has only one axis") );

    TIRBandAxis * ba = static_cast<TIRBandAxis*>( &axis );
    wxASSERT_MSG( ba->GetMinValue() == 0 && ba->GetMaxValue() >= 0,
                  _T("Bands axis adjuster hasincorrect values") );
    unsigned int maxBound = GetBandsCount();
    updated = ( maxBound != ( ba->GetMaxValue() + 1 ) );
    if( updated )
        SetBandsCount( ba->GetMaxValue() + 1 );
    return updated;
}

bool TIRPeditTable::DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
{
    bool updated = false;
    wxASSERT_MSG( n == 0, _T("Table has only one axis") );

    TIRBandAxis * ba = static_cast<TIRBandAxis*>( &axis );
    unsigned int maxBound = GetBandsCount();
    updated = maxBound != (ba->GetMaxValue() + 1);
    ba->SetMaxValue( GetBandsCount() - 1 );
    return updated;
}

void TIRPeditTable::SetSEntity( TIRSEntity pEntity )
{
    m_entity = pEntity;
}

// ---------------------------
// TIRPeditGrid implementation
// ---------------------------

enum TirColumns {
  TIRColumn_Limit = 0,
  TIRColumn_Value,
  TIRColumn_Max
};

TIRPeditGrid::TIRPeditGrid( wxWindow *parent,
                            boost::shared_ptr<TIRPeditTable> const & table,
                            wxWindowID id,
                            wxPoint const & pos,
                            wxSize const & size )
: MultiDimGrid( parent,
                   table,
                   id,
                   pos,
                   size )
{}

int TIRPeditGrid::GetNumberRows()
{
    return MultiDimGrid::GetNumberCols();
}

int TIRPeditGrid::GetNumberCols()
{
    return TIRColumn_Max;
}

wxString TIRPeditGrid::GetValue( int row, int col )
{
    wxASSERT_MSG( col == TIRColumn_Limit || col == TIRColumn_Value,
                  _T("Grid has only two columns: Limit and Value") );
    DoublePair dPair = GetDoublePairValue( static_cast<unsigned int>( row ) );

    wxString str;
    str << (col == TIRColumn_Limit ? dPair.first : dPair.second);
    return str;
}

void TIRPeditGrid::SetValue( int row, int col, const wxString& str )
{
    wxASSERT_MSG( col == TIRColumn_Limit || col == TIRColumn_Value,
                  _T("Grid has only two columns: Limit and Value") );
    DoublePair dPair = GetDoublePairValue( row );

    double value;
    if( !str.ToDouble( &value ) )
        value = 0;
    if( col == TIRColumn_Limit )
        dPair.first = value;
    else
        dPair.second = value;

    m_table->SetAnyValue( m_axisFixedCoords, boost::any( dPair ) );
}

wxString TIRPeditGrid::GetRowLabelValue( int row )
{
    return MultiDimGrid::GetColLabelValue( row );
}

wxString TIRPeditGrid::GetColLabelValue( int col )
{
    wxASSERT_MSG( col == TIRColumn_Limit || col == TIRColumn_Value,
                  _T("Grid has only two columns: Limit and Value") );
    if( col == TIRColumn_Limit )
        return wxString(_("Limit"));
    return wxString(_("Value"));
}

DoublePair TIRPeditGrid::GetDoublePairValue( int row )
{
    // we hide first axis from the table -> 0
    PrepareFixedCoords( 0, row );

    boost::any value = m_table->GetAnyValue( m_axisFixedCoords );

    return boost::any_cast<DoublePair>( value );
}

// --------------------------
//   PeditTIR implementation
// --------------------------
BEGIN_EVENT_TABLE(PeditTIR, PeditFileFrame)
    EVT_TREE_SEL_CHANGED  ( wxID_ANY, PeditTIR::OnTreeSelChange )
END_EVENT_TABLE()

// initialize static variable
const std::string PeditTIR::s_extension = "tir";
namespace
{   // register in the PeditFileFrame
    static bool dummy = PeditTIR::RegisterPeditTIRSubtype();
}

PeditFileFrame * PeditTIR::CreateTIRInstance( wxMDIParentFrame *parent,
                                              std::string const & filename )
{
    return new PeditTIR( parent, filename );
}

// Helper class to store needed information in the entity tree (left part)
class MyTreeItemData : public wxTreeItemData
{
public:
    MyTreeItemData( std::size_t id, std::string const & description )
    : wxTreeItemData(), m_id( id ), m_description( description ) {}

    std::size_t GetId() const { return m_id; }
    std::string const & GetDescription() const { return m_description; }
private:
    std::size_t m_id;
    std::string m_description;
};

PeditTIR::PeditTIR( wxMDIParentFrame *parent, std::string const & filename )
: PeditFileFrame( parent, filename, wxID_ANY, _T("") )
{
   try
   {
        TIRSCharges::ReadFromFile( filename );
    }
    catch( std::exception const & ex )
    {
        wxMessageBox( wxString::Format( _T("Error [%s] while reading from .tir file [%s]"),
                                        ex.what(), filename.c_str() ),
                      _("Error") );
        throw;
    }

    m_table.reset( new TIRPeditTable() );

    std::auto_ptr<wxBoxSizer> sizer( new wxBoxSizer( wxHORIZONTAL ) );

    {
        m_tree = new AutoSizeTreeCtrl( this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT );

        std::vector<tir_names> const& names = GetTIRNames();
        std::map<e_stratified, wxTreeItemId> treeIds;

        for( std::size_t i = 0; i < names.size(); ++i )
        {
            if( names[i].Idx == names[i].ParentIdx )
            {
                wxTreeItemId id =
                    m_tree->AddRoot( _T("*") );
                treeIds[ names[i].Idx ] = id;
            }
            else
            {
                wxTreeItemId parent = treeIds[ names[i].ParentIdx ];
                wxTreeItemId id =
                    m_tree->AppendItem( parent, names[i].ShortName, -1, -1,
                                        new MyTreeItemData( i, names[i].LongName ) );
                treeIds[ names[i].Idx ] = id;
            }
        }

        sizer->Add( m_tree, wxSizerFlags().Proportion(0).Expand() );
    }

    m_grid = new TIRPeditGrid( this, m_table );

    sizer->Add( m_grid, wxSizerFlags().Proportion(1).Expand() );

    sizer->SetSizeHints( this );
    SetSizerAndFit( sizer.release() );

    Layout();
}

PeditTIR::~PeditTIR()
{}

void PeditTIR::OnTreeSelChange( wxTreeEvent & event )
{
    MyTreeItemData * data = dynamic_cast<MyTreeItemData *>(
                                m_tree->GetItemData( event.GetItem() ) );
    if( data )
    {
        std::size_t index = data->GetId();

        std::vector<tir_names> const& names = GetTIRNames();

        bool isTopic = m_tree->GetChildrenCount( event.GetItem() );
        {
            wxString statusText = isTopic ? _("Topic: ") : _("Item: ");
            statusText += data->GetDescription();
            SetStatusText( statusText );
        }

        if( !isTopic )
        {
            m_table->SetSEntity( TIRSCharges::GetEntity( names[index].Idx ) );
        }
        else // !isTopic
        {
            m_table->SetSEntity( TIRSEntity(NULL) );
        }

        m_grid->Enable( !isTopic );

        m_grid->RefreshTableFull();
    }
}

// Implement the abstract methods of PeditFileFrame interface
void PeditTIR::DoSave()
{
    try
    {
        TIRSCharges::WriteToFile( GetFilename() );
        m_table->SetModified( false );
    }
    catch( std::exception const & ex )
    {
        wxMessageBox( wxString::Format( _T("Writing into .tir file [%s]"),
                                        GetFilename().c_str(),
                                        ex.what() ),
                      _T("Error") );
        throw;
    }
}

bool PeditTIR::DoIsModified() const
{
    return m_table->IsModified();
}
