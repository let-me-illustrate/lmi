// Database dictionary manager editor classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile_wx.hpp"

#include "database_view_editor.hpp"

#include "alert.hpp"
#include "mc_enum_types_aux.hpp"
#include "multidimgrid_safe.tpp"
#include "value_cast.hpp"

#include <wx/msgdlg.h>
#include <wx/utils.h>                   // wxBusyCursor

#include <exception>
#include <numeric>                      // accumulate()

/// Database Axis definitions
/// -------------------------

class DatabaseGenderAxis
  :public MultiDimEnumAxis<mcenum_gender>
{
  public:
    DatabaseGenderAxis()
        :MultiDimEnumAxis<mcenum_gender>("Gender", all_strings_gender())
    {}
};

class DatabaseClassAxis
  :public MultiDimEnumAxis<mcenum_class>
{
  public:
    DatabaseClassAxis()
        :MultiDimEnumAxis<mcenum_class>("Class", all_strings_class())
    {}
};

class DatabaseSmokingAxis
  :public MultiDimEnumAxis<mcenum_smoking>
{
  public:
    DatabaseSmokingAxis()
        :MultiDimEnumAxis<mcenum_smoking>("Smoking", all_strings_smoking())
    {}
};

class DatabaseIssueAgeAxis
  :public MultiDimIntAxis
{
  public:
    DatabaseIssueAgeAxis()
        :MultiDimIntAxis("Issue Age", 0, 99, 1)
    {}
};

class DatabaseUwBasisAxis
  :public MultiDimEnumAxis<mcenum_uw_basis>
{
  public:
    DatabaseUwBasisAxis()
        :MultiDimEnumAxis<mcenum_uw_basis>("UW Basis", all_strings_uw_basis())
    {}
};

class DatabaseStateAxis
  :public MultiDimEnumAxis<mcenum_state>
{
  public:
    DatabaseStateAxis()
        :MultiDimEnumAxis<mcenum_state>("State", all_strings_state())
    {}
};

class DatabaseDurationAxis
  :public AdjustableMaxBoundAxis<int>
{
    typedef AdjustableMaxBoundAxis<int> BaseClass;
    static const int max_bound_duration = e_max_dim_duration - 1;

  public:
    DatabaseDurationAxis()
        :BaseClass("Duration", 0, max_bound_duration, 1, max_bound_duration)
    {}
};

DatabaseTableAdapter::DatabaseTableAdapter(database_entity* db_value)
    :db_value_ {db_value}
    ,modified_ {false}
{
    indexes_.resize(eda_max);
}

bool DatabaseTableAdapter::DoApplyAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    bool updated = false;
    // Duration axis
    if(n == eda_duration)
        {
        DatabaseDurationAxis& duration_axis =
            static_cast<DatabaseDurationAxis&>(axis);
        if(duration_axis.GetMinValue() != 0)
            {
            alarum() << "Duration must start at 0." << LMI_FLUSH;
            }
        if(duration_axis.GetMaxValue() < 0)
            {
            alarum()
                << "Duration must have at least one value."
                << LMI_FLUSH
                ;
            }

        // we are operating with [,) - exclusive upper bound, but
        // duration axis has [,] - inclusive upper bound, therefore +1
        int const max_bound = GetDurationMaxBound();
        int const new_max_bound = duration_axis.GetMaxValue() + 1;

        updated = max_bound != new_max_bound;
        SetDurationMaxBound(new_max_bound);
        }
    return updated;
}

bool DatabaseTableAdapter::DoRefreshAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    bool updated = false;
    // Duration axis
    if(n == eda_duration)
        {
        DatabaseDurationAxis& duration_axis =
            static_cast<DatabaseDurationAxis&>(axis);
        int max_bound = GetDurationMaxBound();
        updated = max_bound != (duration_axis.GetMaxValue() + 1);
        duration_axis.SetMaxValue(GetDurationMaxBound() - 1);
        }
    return updated;
}

bool DatabaseTableAdapter::VariesByDimension(unsigned int n) const
{
    EnsureValidDimensionIndex(n);

    if(IsVoid())
        {
        return false;
        }

    std::vector<int> const& axis_lengths = db_value_->axis_lengths();
    return n < axis_lengths.size() && 1 < axis_lengths[n];
}

/// Require confirmation if reshaping an entity would cause it to have
/// extraordinarily many elements.

bool DatabaseTableAdapter::ConfirmOperation(int item_count) const
{
    if(item_count < 1000000)
        {
        return true;
        }

    std::string const message =
        "The resulting entity will have more than one million elements."
        " Continue anyway?"
        ;
    return wxYES == wxMessageBox
        (message
        ,"Memory Consumption"
        ,wxYES_NO | wxICON_QUESTION
        );
}

void DatabaseTableAdapter::MakeVaryByDimension(unsigned int n, bool varies)
{
    EnsureValidDimensionIndex(n);

    if(IsVoid())
        {
        return;
        }

    std::vector<int> axis_lengths = db_value_->axis_lengths();

    axis_lengths[n] = varies ? maximum_database_dimensions()[n] : 1;

    ReshapeTableData(axis_lengths, varies);
}

void DatabaseTableAdapter::SetDurationMaxBound(unsigned int n)
{
    if(IsVoid())
        {
        return;
        }

    std::vector<int> axis_lengths = db_value_->axis_lengths();

    axis_lengths[eda_duration] = n;

    ReshapeTableData(axis_lengths);
}

void DatabaseTableAdapter::ReshapeTableData
    (std::vector<int>& axis_lengths
    ,bool user_confirm
    )
{
    int count = std::accumulate
        (axis_lengths.begin()
        ,axis_lengths.end()
        ,1
        ,std::multiplies<int>()
        );

    if(!user_confirm || ConfirmOperation(count))
        {
        wxBusyCursor reverie;

        db_value_->reshape(axis_lengths);
        SetModified();
        }
}

unsigned int DatabaseTableAdapter::GetDurationMaxBound() const
{
    if(IsVoid())
        {
        return 1U;
        }

    return db_value_->axis_lengths()[eda_duration];
}

bool DatabaseTableAdapter::CanChangeVariationWith(unsigned int n) const
{
    if(IsVoid())
        {
        return false;
        }

    return n < db_value_->axis_lengths().size();
}

void DatabaseTableAdapter::ConvertValue
    (Coords const& coords
    ,std::vector<int>& indexes
    )
{
    indexes[eda_gender]    = UnwrapAny<mcenum_gender>  (coords[eda_gender]);
    indexes[eda_class]     = UnwrapAny<mcenum_class>   (coords[eda_class]);
    indexes[eda_smoking]   = UnwrapAny<mcenum_smoking> (coords[eda_smoking]);
    indexes[eda_issue_age] = UnwrapAny<int>            (coords[eda_issue_age]);
    indexes[eda_uw_basis]  = UnwrapAny<mcenum_uw_basis>(coords[eda_uw_basis]);
    indexes[eda_state]     = UnwrapAny<mcenum_state>   (coords[eda_state]);
    indexes[eda_duration]  = UnwrapAny<int>            (coords[eda_duration]);
    // If the following assert fails, then it probably means that
    // the number of axes has been changed and the change should
    // be reflected in the code above.
    static_assert(eda_max == 7);
}

unsigned int DatabaseTableAdapter::DoGetDimension() const
{
    return eda_max;
}

bool DatabaseTableAdapter::IsVoid() const
{
    return db_value_ == nullptr;
}

double DatabaseTableAdapter::DoGetValue(Coords const& coords) const
{
    if(IsVoid())
        {
        return 0;
        }

    ConvertValue(coords, indexes_);

    return (*db_value_)[indexes_];
}

void DatabaseTableAdapter::DoSetValue
    (Coords const& coords
    ,double const& value
    )
{
    if(IsVoid())
        {
        return;
        }

    ConvertValue(coords, indexes_);

    (*db_value_)[indexes_] = value;
    SetModified();
}

MultiDimTableAny::AxesAny DatabaseTableAdapter::DoGetAxesAny()
{
    AxesAny axes(eda_max);
    axes[eda_gender]    = AxisAnyPtr(new DatabaseGenderAxis());
    axes[eda_class]     = AxisAnyPtr(new DatabaseClassAxis());
    axes[eda_smoking]   = AxisAnyPtr(new DatabaseSmokingAxis());
    axes[eda_issue_age] = AxisAnyPtr(new DatabaseIssueAgeAxis());
    axes[eda_uw_basis]  = AxisAnyPtr(new DatabaseUwBasisAxis());
    axes[eda_state]     = AxisAnyPtr(new DatabaseStateAxis());
    axes[eda_duration]  = AxisAnyPtr(new DatabaseDurationAxis());
    // If the following assert fails, then it probably means that
    // the number of axes has been changed and the change should
    // be reflected in the code above.
    static_assert(eda_max == 7);
    return axes;
}
