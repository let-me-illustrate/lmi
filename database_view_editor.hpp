// Database dictionary manager editor classes.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: database_view_editor.hpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifndef database_view_editor_hpp
#define database_view_editor_hpp

#include "config.hpp"

#include "alert.hpp"
#include "ihs_dbvalue.hpp"
#include "multidimgrid_safe.hpp"
#include "multidimgrid_tools.hpp"
#include "xenumtypes.hpp"

#include <boost/shared_ptr.hpp>

#include <wx/treectrl.h>

#include <string>

/// Stores additional information in a wxTree node

class DatabaseTreeItemData
  :public wxTreeItemData
{
  public:
    DatabaseTreeItemData(std::size_t id, std::string const& description);

    std::size_t GetId() const;
    std::string const& GetDescription() const;

  private:
    std::size_t id_;
    std::string description_;
};

inline DatabaseTreeItemData::DatabaseTreeItemData
    (std::size_t id
    ,std::string const& description
    )
    :wxTreeItemData()
    ,id_(id)
    ,description_(description)
{
}

inline std::size_t DatabaseTreeItemData::DatabaseTreeItemData::GetId() const
{
    return id_;
}

inline std::string const& DatabaseTreeItemData::GetDescription() const
{
    return description_;
}


/// Database Axis definitions
/// -------------------------

class DatabaseGenderAxis
  :public MultiDimEnumAxis<enum_gender>
{
  public:
    DatabaseGenderAxis()
        :MultiDimEnumAxis<enum_gender>("Gender", e_gender::all_strings())
    {
    }
};

class DatabaseClassAxis
  :public MultiDimEnumAxis<enum_class>
{
  public:
    DatabaseClassAxis()
        :MultiDimEnumAxis<enum_class>("Class", e_class::all_strings())
    {
    }
};

class DatabaseSmokingAxis
  :public MultiDimEnumAxis<enum_smoking>
{
public:
    DatabaseSmokingAxis()
        :MultiDimEnumAxis<enum_smoking>("Smoking", e_smoking::all_strings())
    {
    }
};

class DatabaseIssueAgeAxis
  :public MultiDimIntAxis
{
public:
    DatabaseIssueAgeAxis()
        :MultiDimIntAxis("Issue Age", 0, 99, 1)
    {
    }
};

class DatabaseUwBasisAxis
  :public MultiDimEnumAxis<enum_uw_basis>
{
public:
    DatabaseUwBasisAxis()
        :MultiDimEnumAxis<enum_uw_basis>("UW Basis", e_uw_basis::all_strings())
    {
    }
};

class DatabaseStateAxis
  :public MultiDimEnumAxis<enum_state>
{
public:
    DatabaseStateAxis()
        :MultiDimEnumAxis<enum_state>("State", e_state::all_strings())
    {
    }
};

class DatabaseDurationAxis
  :public AdjustableMaxBoundAxis<int>
{
    typedef AdjustableMaxBoundAxis<int> BaseClass;
    static const int max_bound_duration = TDBValue::e_max_dim_duration - 1;
  public:
    DatabaseDurationAxis()
        :BaseClass("Duration", 0, max_bound_duration, 1, max_bound_duration)
    {
    }
};

/// Base type for the Database files editing control underlying data table
typedef MultiDimTable7
    <double
    ,enum_gender
    ,enum_class
    ,enum_smoking
    ,int
    ,enum_uw_basis
    ,enum_state
    ,int
    >
DatabaseTableAdapterBase;

/// Database dictionary adapter for TDBValue class
///
/// One could mention Adaptor pattern.
/// It does not really owns the TDBValue instance which is passed to it.
/// The boost::shared_ptr does.
/// Regarding the fact that all the instances of TDBValue are reside
/// in the TDBDictionary object and owned by it, one could pass entity via
/// boost::shared_ptr constructed with deallocator object that does nothing.

class DatabaseTableAdapter
  :public DatabaseTableAdapterBase
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
  public:
    DatabaseTableAdapter(TDBValue* db_value = NULL);

    virtual ~DatabaseTableAdapter();

    /// Decorated object accessors
    TDBValue* GetTDBValue() const;
    void SetTDBValue(TDBValue* db_value);


    /// Return true if the object data is modified since the last save
    bool IsModified() const;
    void SetModified(bool modified = true);

    void SetDurationMaxBound(unsigned int n);
    unsigned int GetDurationMaxBound() const;

  private:
    /// MultiDimTableAny virtuals
    virtual boost::any DoGetValue(Coords const&) const;
    virtual void DoSetValue(Coords const&, boost::any const&);
    virtual bool DoApplyAxisAdjustment(MultiDimAxisAny&, unsigned int);
    virtual bool DoRefreshAxisAdjustment(MultiDimAxisAny&, unsigned int);

    virtual bool VariesByDimension(unsigned int) const;
    virtual void MakeVaryByDimension(unsigned int, bool);
    virtual bool CanChangeVariationWith(unsigned int) const;

    /// Convert from and to double wrapped in boost::any object
    virtual wxString ValueToString(boost::any const&) const;
    virtual boost::any StringToValue(wxString const&) const;

    virtual MultiDimAxis<enum_gender>*   GetAxis0();
    virtual MultiDimAxis<enum_class>*    GetAxis1();
    virtual MultiDimAxis<enum_smoking>*  GetAxis2();
    virtual MultiDimAxis<int>*           GetAxis3();
    virtual MultiDimAxis<enum_uw_basis>* GetAxis4();
    virtual MultiDimAxis<enum_state>*    GetAxis5();
    virtual MultiDimAxis<int>*           GetAxis6();

    // not used but has to be implemented
    virtual double GetValue
        (enum_gender
        ,enum_class
        ,enum_smoking
        ,int
        ,enum_uw_basis
        ,enum_state
        ,int
        ) const;
    virtual void SetValue
        (enum_gender
        ,enum_class
        ,enum_smoking
        ,int
        ,enum_uw_basis
        ,enum_state
        ,int
        ,double const&
        );

    /// Helper, converts array of boost::any into array of ints
    void ConvertValue
        (Coords const&
        ,std::vector<int>&
        ) const;

    bool IsVoid() const;
    bool ConfirmOperation(unsigned int itemCount) const;
    void ReshapeTableData
        (std::vector<int>& axis_lengths
        ,bool user_confirm = false
        );

    /// Pointer to decorated object
    TDBValue* db_value_;

    /// Modification flag (dirty flag)
    bool modified_;

    /// temporary buffer needed by DoGetValue() and DoSetValue() methods
    mutable std::vector<int> indexes_;


    DECLARE_NO_COPY_CLASS(DatabaseTableAdapter)
};

inline TDBValue* DatabaseTableAdapter::GetTDBValue() const
{
    return db_value_;
}

inline void DatabaseTableAdapter::SetTDBValue(TDBValue* db_value)
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

/// Class is the version of MultiDimGrid customized for db4 file data.
///
/// Grid edit the data that depends upon 7 axis described in ihs_dbvalue.hpp

class DatabaseEditorGrid
  :public MultiDimGrid
{
    BOOST_STATIC_ASSERT
        (
           static_cast<int>(TDBValue::e_number_of_axes)
        == static_cast<int>(DatabaseTableAdapter::eda_max)
        );

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

#endif // tier_view_editor_hpp

