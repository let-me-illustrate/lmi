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

// $Id: pedit_tir.hpp,v 1.1.2.3 2006-04-10 20:26:03 etarassov Exp $

#ifndef PEDIT_TIR_HPP_
#define PEDIT_TIR_HPP_

#include "config.hpp"

#include "multidimgrid.hpp"
#include "pedit.hpp"

#include "stratified_charges.hpp"

// some forward declarations
class WXDLLEXPORT wxTreeEvent;

// TODO: where to get that bound of 100 from?
/// Axis representing number of bands in the stratified_entity
class TIRBandAxis : public AdjustableMaxIntegralAxis<unsigned int>
{
    typedef AdjustableMaxIntegralAxis<unsigned int> BaseClass;
    static const
    unsigned int max_bound_band = 99;
public:
    TIRBandAxis()
    : BaseClass( _("# bands"), 0, max_bound_band, 0, max_bound_band )
    {}
};

/// Data type that Tir editor classes will manipulate through its interface
typedef std::pair< double, double > DoublePair;

/**
   Just specify a dummy specialisation of MultiDimTableTypeTraits on DoublePair
   so that the code compiles. This specialisation will never be used at runtime
   anyway.

   @sa MultiDimTableTypeTraits
 */
template <>
class MultiDimTableTypeTraits<DoublePair>
{
public:
    /// Convert value respresented by a string into ValueType.
    DoublePair FromString( const wxString & str ) const
    {   return DoublePair( 0, 0 );     }

    /// Create a string representation of a value
    wxString ToString( DoublePair const & value ) const
    {   return _( "#ERR" );   }
};

/**
   Accessor/wrapper class for stratified_entity objects.

   It is declared as friend to stratified_entity, therefore it can minipulate
   internal values/methods of stratified_entity to modify/save/load TIR data.
 */
class TIRSEntity
{
public:
    TIRSEntity( stratified_entity * pEntity );

    /// Get data in pairs (corresponding to bands)
    DoublePair GetValue( unsigned int band ) const;
    /// Set data in a pair (corresponding to a band)
    void SetValue( unsigned int band, DoublePair const & value );

    /// Change number of bands in the underlying stratified_entity object
    void SetBandsCount( unsigned int n );
    /// Read the number of bands in the underlying stratified_entity object
    unsigned int GetBandsCount() const;

    /// @return true if we don't have no underlying object to manipulate
    bool IsEmpty() const
    {   return m_entity == NULL;    }

private:
    stratified_entity * m_entity;
};

/// Base for the TIR files editor data table
typedef MultiDimTable1< DoublePair,
                        unsigned int >
        TIRPeditTableBase;

/**
   Table that interfaces between stratified_entity and MultiDimGrid

   It manipulates stratified_entity data using pairs of doubles coresponding to
   bands.
   @sa TIRPeditGrid
 */
class TIRPeditTable : public TIRPeditTableBase
{
public:
    TIRPeditTable( TIRSEntity pEntity );

    /// Decorated object accessor
    TIRSEntity GetSEntity() const
    {   return m_entity;    }
    /// Change decorated object
    void SetSEntity( TIRSEntity pEntity );

    virtual ~TIRPeditTable();

    /// We depend on the single axis and vary with it
    virtual bool VariesByDimension( unsigned int WXUNUSED(n) ) const
    {   return !m_entity.IsEmpty();    }

    /// User can't change it
    bool CanChangeVariationWith( unsigned int WXUNUSED(n) ) const
    {   return false;   }
    /// User can't change it
    virtual void MakeVaryByDimension( unsigned int n, bool varies ) {}

    /// Return true if the object data is modified since the last save
    bool IsModified() const { return m_modified; }
    void SetModified( bool modified = true ) { m_modified = modified; }

    /// @sa TIRSEntity::SetBandsCount
    void SetBandsCount( unsigned int n );
    /// @sa TIRSEntity::GetBandsCount
    unsigned int GetBandsCount() const;

protected:
    friend class TIRPeditGrid;
    virtual DoublePair GetValue( unsigned int band ) const;
    virtual void SetValue( unsigned int band, DoublePair const & value );

    MultiDimAxis<unsigned int> * GetAxis0()
    {   return new TIRBandAxis(); }

    bool DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n );
    bool DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n );

private:
    TIRSEntity m_entity;

    /// Modification flag (dirty flag)
    bool m_modified;

    DECLARE_NO_COPY_CLASS( TIRPeditTable )
};

/**
   The only customized part of that grid is its ability to show pairs of doubles
   in two columns in the data grid. It changes default MultiDimGrid
   interface that it uses to retrieve grid strings from underlying table.
 */
class TIRPeditGrid : public MultiDimGrid
{
public:
    TIRPeditGrid( wxWindow *parent,
                  boost::shared_ptr<TIRPeditTable> const & table,
                  wxWindowID id = wxID_ANY,
                  wxPoint const & pos = wxDefaultPosition,
                  wxSize const & size = wxDefaultSize );

protected:
    /**
       We need to show pairs of doubles, therefore we will override
       MultiDimGrid to behave accordingly
     */
    ///@{
    virtual int GetNumberRows();

    /// always show data in pairs (two columns in the grid)
    virtual int GetNumberCols();

    virtual wxString GetValue( int row, int col );

    virtual void SetValue( int row, int col, const wxString& str );

    virtual wxString GetRowLabelValue( int row );

    virtual wxString GetColLabelValue( int col );

private:
    DoublePair GetDoublePairValue( int row );
};

class TIRSCharges
: protected stratified_charges,
  virtual protected obstruct_slicing<stratified_charges>
{
public:
    TIRSCharges()
    : obstruct_slicing<stratified_charges>(), stratified_charges() {}

    /// Read entities (TDBValue) from the file
    void ReadFromFile( std::string const & filename )
    {
        stratified_charges::read( filename );
    }

    void WriteToFile( std::string const & filename )
    {
        stratified_charges::write( filename );
    }

    TIRSEntity GetEntity( e_stratified index )
    {   return TIRSEntity( &stratified_charges::raw_entity( index ) ); }

    stratified_entity const & GetRawEntity( e_stratified index ) const
    {   return stratified_charges::raw_entity( index ); }
};
/**
   MDI Child frame for TIR files editing.
 */
class PeditTIR : public PeditFileFrame,
                 protected TIRSCharges
{
public:
    virtual ~PeditTIR();

    /// Extension of tir files
    static const std::string s_extension; // = "tir"

    /// Register this type of files in PeditFileFrame
    static bool RegisterPeditTIRSubtype()
    {
        PeditFileFrame::RegisterSubtype( PeditTIR::s_extension,
                                         PeditTIR::CreateTIRInstance,
                                         _T("Tiered") );
        return true; // a dummy value
    }

protected:
    /// This class shouldn't be instantiated out of the product editor
    PeditTIR( wxMDIParentFrame *parent, std::string const & filename );

    /// Implement the abstract methods of PeditFileFrame interface
    /// @{
    virtual void DoSave();
    virtual bool DoIsModified() const;
    virtual std::string const & DoGetExtension() const {  return s_extension;  }
    /// @}

private:
    /// MDGrid widget
    TIRPeditGrid * m_grid;
    /// List of TDBValue entities
    AutoSizeTreeCtrl * m_tree;

    /// Data adapter (wrapper around TDBValue)
    boost::shared_ptr<TIRPeditTable> m_table;

    /// Handles selection changes in the entity list
    void OnTreeSelChange( wxTreeEvent & event );

    /// Factory method for the class instantiation
    static PeditFileFrame * CreateTIRInstance( wxMDIParentFrame *parent,
                                               std::string const & filename );

    DECLARE_NO_COPY_CLASS( PeditTIR )
    DECLARE_EVENT_TABLE()
};

#endif /*PEDIT_TIR_HPP_*/
