// Database dictionary manager editor classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifndef database_view_editor_hpp
#define database_view_editor_hpp

#include "config.hpp"

// EVGENIY !! These two headers seem to declare base classes that are
// required for various derived classes declared here. What's the
// difference between these two headers--is there an easy way to know
// when to include one versus the other? Do they both have standalone
// uses, or must both always be included together?
#include "multidimgrid_safe.hpp"
#include "multidimgrid_tools.hpp"

// EVGENIY !! I suspect that we can avoid including "dbvalue.hpp"
// here by reworking or moving code for which a forward declaration
// doesn't work today.

#include "dbvalue.hpp"

#include <boost/shared_ptr.hpp>

#include <string>

/// Database dictionary adapter for database_entity class
///
/// One could mention Adaptor pattern.
/// It does not really owns the database_entity instance which is passed to it.
/// The boost::shared_ptr does.
/// Regarding the fact that all the instances of database_entity are reside
/// in the DBDictionary object and owned by it, one could pass entity via
/// boost::shared_ptr constructed with deallocator object that does nothing.

class DatabaseTableAdapter
  :public MultiDimTable<double, DatabaseTableAdapter>
{
    enum enum_database_axis
        {eda_gender = 0
        ,eda_class
        ,eda_smoking
        ,eda_issue_age
        ,eda_uw_basis
        ,eda_state
        ,eda_duration
        ,eda_max
        };
    BOOST_STATIC_ASSERT
        (
           static_cast<int>(DatabaseTableAdapter::eda_max)
        == static_cast<int>(e_number_of_axes)
        );
  public:
    DatabaseTableAdapter(database_entity* db_value = NULL);

    virtual ~DatabaseTableAdapter();

    /// Decorated object accessors
    database_entity* GetTDBValue() const;
    void SetTDBValue(database_entity* db_value);

    /// Return true if the object data is modified since the last save
    bool IsModified() const;
    void SetModified(bool modified = true);

    void SetDurationMaxBound(unsigned int n);
    unsigned int GetDurationMaxBound() const;

    // MultiDimGridN contract.
    double DoGetValue(Coords const&) const;
    void DoSetValue(Coords const&, double const&);

  private:
    /// MultiDimTableAny required implementation.
    virtual bool VariesByDimension(unsigned int) const;
    virtual void MakeVaryByDimension(unsigned int, bool);
    virtual bool CanChangeVariationWith(unsigned int) const;
    AxesAny DoGetAxesAny();
    virtual unsigned int DoGetDimension() const;

    /// MultiDimTableAny overrides.
    virtual bool DoApplyAxisAdjustment(MultiDimAxisAny&, unsigned int);
    virtual bool DoRefreshAxisAdjustment(MultiDimAxisAny&, unsigned int);

    /// Helper, converts array of boost::any into array of ints
    static void ConvertValue(Coords const&, std::vector<int>&);

    bool IsVoid() const;
    bool ConfirmOperation(int itemCount) const;
    void ReshapeTableData
        (std::vector<int>& axis_lengths
        ,bool user_confirm = false
        );

    /// Pointer to decorated object
    database_entity* db_value_;

    /// Modification flag (dirty flag)
    bool modified_;

    /// temporary buffer needed by DoGetValue() and DoSetValue() methods
    mutable std::vector<int> indexes_;

    DECLARE_NO_COPY_CLASS(DatabaseTableAdapter)
};

inline database_entity* DatabaseTableAdapter::GetTDBValue() const
{
    return db_value_;
}

inline void DatabaseTableAdapter::SetTDBValue(database_entity* db_value)
{
    db_value_ = db_value;
}

inline bool DatabaseTableAdapter::IsModified() const
{
    return modified_;
}

inline void DatabaseTableAdapter::SetModified(bool modified)
{
    modified_ = modified;
}

/// Class is the version of MultiDimGrid customized for '.database' data.
///
/// Grid edit the data that depends upon 7 axis described in 'dbvalue.hpp'

class DatabaseEditorGrid
  :public MultiDimGrid
{
  public:
    DatabaseEditorGrid
        (wxWindow*
        ,boost::shared_ptr<DatabaseTableAdapter> const&
        ,wxWindowID = wxID_ANY
        ,wxPoint const& = wxDefaultPosition
        ,wxSize const& = wxDefaultSize
        );
};

inline DatabaseEditorGrid::DatabaseEditorGrid
    (wxWindow* parent
    ,boost::shared_ptr<DatabaseTableAdapter> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
    : MultiDimGrid(parent, table, id, pos, size)
{
}

#endif // database_view_editor_hpp

