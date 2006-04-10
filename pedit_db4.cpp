// Product editor part for db4 file type.
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

// $Id: pedit_db4.cpp,v 1.1.2.4 2006-04-10 20:26:03 etarassov Exp $

#include "pedit_db4.hpp"
#include "dbnames.hpp"

#include <numeric>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/utils.h>
#include <wx/timer.h>

DB4PeditTable::DB4PeditTable()
: m_value(), m_modified( false )
{}

DB4PeditTable::DB4PeditTable( PtrTDBValue pValue )
: m_value( pValue ), m_modified( false )
{}

DB4PeditTable::PtrTDBValue DB4PeditTable::GetTable() const
{
    return m_value;
}

void DB4PeditTable::SetTable( PtrTDBValue pValue )
{
    m_value = pValue;
}

DB4PeditTable::~DB4PeditTable()
{}

bool DB4PeditTable::DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
{
    bool updated = false;
    if( n == 6 ) // Duration axis
    {
        DB4DurationAxis * da = static_cast<DB4DurationAxis*>( &axis );
        wxASSERT_MSG( da->GetMinValue() == 0, _T("Duration has to start by 0") );
        wxASSERT_MSG( da->GetMaxValue() >= 0, _T("Duration has to have at least one value") );
        int maxBound = GetDurationMaxBound();
        updated = maxBound != (da->GetMaxValue() + 1);
        SetDurationMaxBound( da->GetMaxValue() + 1 );
    }
    if( n == 3 ) // Issue Age axis
    {}
    return updated;
}

bool DB4PeditTable::DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n )
{
    bool updated = false;
    if( n == 6 ) // Duration axis
    {
        DB4DurationAxis * da = static_cast<DB4DurationAxis*>( &axis );
        int maxBound = GetDurationMaxBound();
        updated = maxBound != (da->GetMaxValue() + 1);
        da->SetMaxValue( GetDurationMaxBound() - 1 );
    }
    if( n == 3 ) // Issue Age axis
    {}
    return updated;
}

bool DB4PeditTable::VariesByDimension( unsigned int n ) const
{
    wxASSERT_MSG( n < GetDimension(), _T("Invalid dimension index specified") );
    if( m_value == NULL )
        return false;
    std::vector<int> const & axis_lengths = m_value->GetAxisLengths();
    return n < axis_lengths.size() && axis_lengths[n] > 1;
}

bool DB4PeditTable::ConfirmOperation( unsigned int itemCount ) const
{
    return itemCount < 1000000 // its not that bad to have 1M elements
        || wxYES == // user is aware of the memory consumption
            wxMessageBox( wxString(_("The resulting entity will have more than 1 million entries. Are you sure you want to continue?")),
                          _("Memory Consumption"),
                          wxYES_NO | wxICON_QUESTION );
}

void DB4PeditTable::MakeVaryByDimension( unsigned int n, bool varies )
{
    wxASSERT_MSG( n < GetDimension(), _T("Invalid dimension index specified") );
    if( m_value == NULL )
        return;
    std::vector<int> axis_lengths = m_value->GetAxisLengths();
    if( n >= axis_lengths.size() )
        return;
    axis_lengths[n] = varies ? TDBValue::maximum_dimensions()[n] : 1;
    std::size_t count = std::accumulate( axis_lengths.begin(),
                                         axis_lengths.end(),
                                         1, std::multiplies<int>() );
    // are we really want to perform the operation?
    if( !varies || ConfirmOperation( count ) )
    {
        wxStartTimer();
        wxBeginBusyCursor();

        m_value->Reshape( axis_lengths );
        SetModified();

        wxEndBusyCursor();
        // beep if the delay is more than one second
        if( wxGetElapsedTime() > 1000 )
            wxBell();
    }
}

void DB4PeditTable::SetDurationMaxBound( unsigned int n )
{
    if( m_value == NULL )
        return;
    std::vector<int> axis_lengths = m_value->GetAxisLengths();
    axis_lengths[6] = n;
    std::size_t count = std::accumulate( axis_lengths.begin(),
                                         axis_lengths.end(),
                                         1, std::multiplies<int>() );
    if( ConfirmOperation( count ) )
    {
        wxStartTimer();
        wxBeginBusyCursor();

        m_value->Reshape( axis_lengths );
        SetModified();

        wxEndBusyCursor();
        // beep if the delay is more than one second
        if( wxGetElapsedTime() > 1000 )
            wxBell();
    }
}

unsigned int DB4PeditTable::GetDurationMaxBound() const
{
    if( m_value == NULL )
        return 1;
    return m_value->GetAxisLengths()[6];
}

bool DB4PeditTable::CanChangeVariationWith( unsigned int n ) const
{
    if( m_value == NULL )
        return false;
    return n < m_value->GetAxisLengths().size();
}

wxString DB4PeditTable::ValueToString(const boost::any & value) const
{
    try {
        return wxString::Format( _T("%f"),
                                 boost::any_cast<double>( value ) );
    }
    catch(...)
    {
        return _T("#ERR");
    }
}

boost::any DB4PeditTable::StringToValue(const wxString & value) const
{
    double res;
    if( !value.ToDouble( &res ) )
        res = 0.;
    return boost::any( res );
}

void DB4PeditTable::DoConvertValue(Coords const & coords, std::vector<int> & indexes) const
{
    indexes[0] = boost::any_cast<enum_gender>( coords[0] );
    indexes[1] = boost::any_cast<enum_class>( coords[1] );
    indexes[2] = boost::any_cast<enum_smoking>( coords[2] );
    indexes[3] = boost::any_cast<int>( coords[3] );
    indexes[4] = boost::any_cast<enum_uw_basis>( coords[4] );
    indexes[5] = boost::any_cast<enum_state>( coords[5] );
    indexes[6] = boost::any_cast<int>( coords[6] );
}

boost::any DB4PeditTable::DoGetValue(Coords const & coords) const
{
    if( m_value == NULL )
        return boost::any( static_cast<double>( 0 ) );

    m_indexes.clear();
    m_indexes.resize( coords.size(), 0 );
    DoConvertValue( coords, m_indexes );

    double val = (*m_value)[ m_indexes ];
    return boost::any( val );
}

void DB4PeditTable::DoSetValue(const Coords& coords, const boost::any& value)
{
    if( m_value == NULL )
        return;

    m_indexes.clear();
    m_indexes.resize( coords.size(), 0 );
    DoConvertValue( coords, m_indexes );

    double val = boost::any_cast<double>( value );
    (*m_value)[ m_indexes ] = val;
    SetModified();
}

// --------------------------
//   PeditDB4 implementation
// --------------------------
BEGIN_EVENT_TABLE(PeditDB4, PeditFileFrame)
    EVT_TREE_SEL_CHANGED  ( wxID_ANY, PeditDB4::OnTreeSelChange )
END_EVENT_TABLE()

// initialize static variable
const std::string PeditDB4::s_extension = "db4";
namespace
{   // register in the PeditFileFrame
    static bool dummy = PeditDB4::RegisterPeditDB4Subtype();
}

PeditFileFrame * PeditDB4::CreateDB4Instance( wxMDIParentFrame *parent,
                                              std::string const & filename )
{
    return new PeditDB4( parent, filename );
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

PeditDB4::PeditDB4( wxMDIParentFrame *parent, std::string const & filename )
: PeditFileFrame( parent, filename, wxID_ANY, _T("") )
{
    try
    {
        ReadFromDBDictionary( filename, m_dict );
    }
    catch( std::exception const & ex )
    {
        wxMessageBox( wxString::Format( _("Error [%s] while reading from .db4 file [%s]"),
                                        ex.what(), filename.c_str() ),
                      _("Error") );
        throw;
    }

    m_table.reset( new DB4PeditTable() );

    std::auto_ptr<wxBoxSizer> sizer( new wxBoxSizer( wxHORIZONTAL ) );

    {
        m_tree = new AutoSizeTreeCtrl(
            this, wxID_ANY,
            wxDefaultPosition, wxDefaultSize,
            wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT );

        std::vector<db_names> const& names = GetDBNames();
        std::map<DatabaseNames, wxTreeItemId> treeIds;

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

    m_grid = new DB4PeditGrid( this, m_table );

    sizer->Add( m_grid, wxSizerFlags().Proportion(1).Expand() );

    sizer->SetSizeHints( this );
    SetSizerAndFit( sizer.release() );

    Layout();
}

PeditDB4::~PeditDB4()
{}

void PeditDB4::OnTreeSelChange( wxTreeEvent & event )
{
    MyTreeItemData * data = dynamic_cast<MyTreeItemData *>(
                                m_tree->GetItemData( event.GetItem() ) );
    if( data )
    {
        std::size_t index = data->GetId();

        m_table->SetTable( &(m_dict[index]) );

        bool isTopic = m_tree->GetChildrenCount( event.GetItem() );
        {
            wxString statusText = isTopic ? _("Topic: ") : _("Item: ");
            statusText += data->GetDescription();
            SetStatusText( statusText );
        }
        m_grid->Enable( !isTopic );

        m_grid->RefreshTableFull();
    }
}

void PeditDB4::DoSave()
{
    WriteToDBDictionary( GetFilename(), m_dict );
    m_table->SetModified( false );
}

bool PeditDB4::DoIsModified() const
{
    return m_table->IsModified();
}
