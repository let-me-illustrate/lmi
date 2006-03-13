// Product editor part for db4 file type.
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

// $Id: pedit_db4.hpp,v 1.1.2.1 2006-03-13 19:56:52 etarassov Exp $

#ifndef PEDIT_DB4_HPP_
#define PEDIT_DB4_HPP_

#include "config.hpp"
#include "pedit.hpp"
#include "ihs_dbvalue.hpp"
#include "ihs_dbdict.hpp"
#include "xenumtypes.hpp"
#include "multidimgrid.hpp"

#include <wx/arrstr.h>
#include <wx/stattext.h>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>


// some forward declarations
class WXDLLEXPORT wxTreeEvent;

namespace pedit
{
    /// convert vector of strings into wx array of strings
    static
    wxArrayString MakeArray( std::vector<std::string> const & names )
    {
        wxArrayString arr;
        arr.Alloc( names.size() );
        for( std::size_t i = 0; i < names.size(); ++i )
            arr.Add( names[i].c_str() );
        return arr;
    }
}

/// DB4 Gender Axis
class DB4GenderAxis : public MultiDimEnumAxis<enum_gender>
{
public:
    DB4GenderAxis()
    : MultiDimEnumAxis<enum_gender>( _T("Gender"),
            pedit::MakeArray( e_gender::all_strings() ) ) {}
};

/// DB4 Class Axis
class DB4ClassAxis : public MultiDimEnumAxis<enum_class>
{
public:
    DB4ClassAxis()
    : MultiDimEnumAxis<enum_class>( _T("Class"),
            pedit::MakeArray( e_class::all_strings() ) ) {}
};

/// DB4 Smoking Axis
class DB4SmokingAxis : public MultiDimEnumAxis<enum_smoking>
{
public:
    DB4SmokingAxis()
    : MultiDimEnumAxis<enum_smoking>( _T("Smoking"),
            pedit::MakeArray( e_smoking::all_strings() ) ) {}
};

/// DB4 Issue Age axis
class DB4IssueAgeAxis : public MultiDimIntAxis
{
public:
    DB4IssueAgeAxis()
    : MultiDimIntAxis( _T("Issue Age"), 0, 99, 1 ) {}
};

/// DB4 UW Basis axis
class DB4UwBasisAxis : public MultiDimEnumAxis<enum_uw_basis>
{
public:
    DB4UwBasisAxis()
    : MultiDimEnumAxis<enum_uw_basis>( _T("UW Basis"),
            pedit::MakeArray( e_uw_basis::all_strings() ) ) {}
};

/// State axis
class DB4StateAxis : public MultiDimEnumAxis<enum_state>
{
public:
    DB4StateAxis()
    : MultiDimEnumAxis<enum_state>( _T("State"),
            pedit::MakeArray( e_state::all_strings() ) ) {}
};

/// DB4 Duration axis
class DB4DurationAxis : public AdjustableMaxIntegralAxis<int>
{
    typedef AdjustableMaxIntegralAxis<int> BaseClass;
    static const
    int max_bound_duration = TDBValue::e_max_dim_duration - 1;
public:
    DB4DurationAxis()
    : BaseClass( _T("Duration"), 0, max_bound_duration, 1, max_bound_duration )
    {}
};

/// Base type for the DB4 files editing control underlying data table
typedef MultiDimTable7< double,
                        enum_gender,
                        enum_class,
                        enum_smoking,
                        int,
                        enum_uw_basis,
                        enum_state,
                        int>
        DB4PeditTableBase;

/**
   DB4 wrapper for TDBValue class
   
   One could mention Adaptor pattern.
   It does not really owns the TDBValue instance which is passed to it.
   The boost::shared_ptr does.
   Regarding the fact that all the instances of TDBValue are reside
   in the TDBDictionary object and owned by it, one could pass entity via
   boost::shared_ptr constructed with deallocator object that does nothing.
 */
class DB4PeditTable : public DB4PeditTableBase
{
public:
    /// Shared pointer to the real data object (TDBValue)
    typedef TDBValue * PtrTDBValue;

    // Ctors
    DB4PeditTable();
    DB4PeditTable( PtrTDBValue pValue );

    /// Decorated object accessor
    PtrTDBValue GetTable() const;

    /// Change decorated object
    void SetTable( PtrTDBValue pValue );

    /// Virtual dtor
    virtual ~DB4PeditTable();

    /// Implement MultiDimTableAny interface
    virtual bool VariesByDimension( unsigned int n ) const;
    virtual void MakeVaryByDimension( unsigned int n, bool varies );

    bool CanChangeVariationWith( unsigned int n ) const;

    /// Convert from and to double wrapped in boost::any object
    virtual wxString ValueToString( const boost::any & value ) const;
    virtual boost::any StringToValue( const wxString & value ) const;

    /// Return true if the object data is modified since the last save
    bool IsModified() const { return m_modified; }
    void SetModified( bool modified = true ) { m_modified = modified; }

    void SetDurationMaxBound( unsigned int n );
    unsigned int GetDurationMaxBound() const;

protected:
    /// Implement MultiDimTableAny interface
    virtual boost::any DoGetValue(const Coords& coords) const;
    virtual void DoSetValue(const Coords& coords, const boost::any& value);

    // not used but has to be implemented
    virtual
    double GetValue( enum_gender eg, enum_class ec, enum_smoking es, int i,
                     enum_uw_basis eub, enum_state est, int d ) const
    {   return 0; }
    // not used but has to be implemented
    virtual
    void SetValue( enum_gender eg, enum_class ec, enum_smoking es, int i,
                   enum_uw_basis eub, enum_state est, int d,
                   double const & v )
    {}

    /// Create axis of the table
    /// @{
    MultiDimAxis<enum_gender> * GetAxis0()
    {   return new DB4GenderAxis(); }
    MultiDimAxis<enum_class> * GetAxis1()
    {   return new DB4ClassAxis();  }
    MultiDimAxis<enum_smoking> * GetAxis2()
    {   return new DB4SmokingAxis();    }
    MultiDimAxis<int> * GetAxis3()
    {   return new DB4IssueAgeAxis();   }
    MultiDimAxis<enum_uw_basis> * GetAxis4()
    {   return new DB4UwBasisAxis();    }
    MultiDimAxis<enum_state> * GetAxis5()
    {   return new DB4StateAxis();  }
    MultiDimAxis<int> * GetAxis6()
    {   return new DB4DurationAxis();   }
    /// @}

    bool DoApplyAxisAdjustment( MultiDimAxisAny & axis, unsigned int n );
    bool DoRefreshAxisAdjustment( MultiDimAxisAny & axis, unsigned int n );

private:
    /// Pointer to decorated object
    PtrTDBValue m_value;

    /// Modification flag (dirty flag)
    bool m_modified;

    /// temporary buffer needed by DoGetValue() and DoSetValue() methods
    mutable std::vector<int> m_indexes;

    /// Helper, converts array of boost::any into array of ints
    void DoConvertValue( Coords const & coords,
                         std::vector<int> & indexes) const;

    bool ConfirmOperation( unsigned int itemCount ) const;

    DECLARE_NO_COPY_CLASS( DB4PeditTable )
};

/**
   Class is the version of MultiDimGrid customized for db4 file data.
   
   Grid edit the data that depends upon 7 axis described in ihs_dbvalue.hpp
 */
class DB4PeditGrid : public MultiDimGrid
{
public:
    BOOST_STATIC_ASSERT( TDBValue::e_number_of_axes == 7 );
    DB4PeditGrid( wxWindow *parent,
                  boost::shared_ptr<DB4PeditTable> const & table,
                  wxWindowID id = wxID_ANY,
                  wxPoint const & pos = wxDefaultPosition,
                  wxSize const & size = wxDefaultSize )
    : MultiDimGrid( parent,
                       table,
                       id,
                       pos,
                       size )
    {}
};

/**
   Class opens the access to the internal TDBDictionary methods and variables.

   It is declared as a friend of TDBDictionary class and allows us to use
   Read/Write methods as long as to overcome the singleton nature
   of the TDBDictionary class.
 */
class PeditDB4DBDocument
{
protected:
    /// Read entities (TDBValue) from the file
    void ReadFromDBDictionary( std::string const & filename, dict_map & dict )
    {
        DBDictionary & instance = DBDictionary::instance();

        // double-swap workaround for the singleton constraint
        dict.swap( instance.GetDictionary() );
        try
        {
            std::string oldCacheFilename = DBDictionary::CachedFilename;
            instance.Init( filename );
            DBDictionary::CachedFilename = oldCacheFilename;
        }
        catch(...)
        {
            dict.swap( instance.GetDictionary() );
            throw;
        }
        dict.swap( instance.GetDictionary() );
    }
    /// Write entities (TDBValue) to the file
    void WriteToDBDictionary( std::string const & filename, dict_map & dict )
    {
        DBDictionary & instance = DBDictionary::instance();

        // double-swap workaround for the singleton constraint
        dict.swap( instance.GetDictionary() );
        try
        {
            instance.WriteDB( filename );
        }
        catch(...)
        {
            dict.swap( instance.GetDictionary() );
            throw;
        }
        dict.swap( instance.GetDictionary() );
    }
};

/**
   MDI Child frame for DB4 files editing.
 */
class PeditDB4 : public PeditFileFrame,
                 private PeditDB4DBDocument // hack to be able to load/save DBDictionary in files
{
public:
    virtual ~PeditDB4();

    /// Extension of db4 files
    static const std::string s_extension; // = "db4"

    /// Register this type of files in PeditFileFrame
    static bool RegisterPeditDB4Subtype()
    {
        PeditFileFrame::RegisterSubtype( PeditDB4::s_extension,
                                         PeditDB4::CreateDB4Instance,
                                         _T("Database") );
        return true; // a dummy value
    }

protected:
    /// This class shouldn't be instantiated out of the product editor
    PeditDB4( wxMDIParentFrame *parent, std::string const & filename );

    /// Implement the abstract methods of PeditFileFrame interface
    virtual void DoSave();
    virtual bool DoIsModified() const;
    virtual std::string const & DoGetExtension() const {  return s_extension;  }

private:
    /// MDGrid widget
    DB4PeditGrid * m_grid;
    /// List of TDBValue entities
    AutoSizeTreeCtrl * m_tree;
    /// The TDBDictionary storage containing all the entities
    dict_map m_dict;
    /// Data adapter (wrapper around TDBValue)
    boost::shared_ptr<DB4PeditTable> m_table;

    /// Handles selection changes in the entity list
    void OnTreeSelChange( wxTreeEvent & event );

    /// Factory method for the class instantiation
    static PeditFileFrame * CreateDB4Instance( wxMDIParentFrame *parent,
                                               std::string const & filename );

    DECLARE_NO_COPY_CLASS( PeditDB4 )
    DECLARE_EVENT_TABLE()
};

#endif /*PEDIT_DB4_HPP_*/
