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

// $Id: database_view_editor.cpp,v 1.10.2.1 2007-04-02 11:40:45 etarassov Exp $

#include "database_view_editor.hpp"

#include "alert.hpp"
#include "multidimgrid_safe.tpp"
#include "value_cast.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <wx/msgdlg.h>
#include <wx/utils.h> // wxBusyCursor

#include <exception>
#include <numeric>    // std::accumulate()

DatabaseTableAdapter::DatabaseTableAdapter(TDBValue* db_value)
    :db_value_(db_value)
    ,modified_(false)
{
    indexes_.resize(eda_max);
}

DatabaseTableAdapter::~DatabaseTableAdapter()
{
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
            fatal_error() << "Duration must start at 0." << LMI_FLUSH;
            }
        if(duration_axis.GetMaxValue() < 0)
            {
            fatal_error()
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
        {return false;}

    std::vector<int> const& axis_lengths = db_value_->GetAxisLengths();
    return n < axis_lengths.size() && 1 < axis_lengths[n];
}

/// Require confirmation if reshaping an entity would cause it to have
/// extraordinarily many elements.

bool DatabaseTableAdapter::ConfirmOperation(unsigned int item_count) const
{
    if(item_count < 1000000)
        {return true;}

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
        {return;}

    std::vector<int> axis_lengths = db_value_->GetAxisLengths();

    axis_lengths[n] = varies ? TDBValue::maximum_dimensions()[n] : 1;

    ReshapeTableData(axis_lengths, varies);
}

void DatabaseTableAdapter::SetDurationMaxBound(unsigned int n)
{
    if(IsVoid())
        {return;}

    std::vector<int> axis_lengths = db_value_->GetAxisLengths();

    axis_lengths[eda_duration] = n;

    ReshapeTableData(axis_lengths);
}

void DatabaseTableAdapter::ReshapeTableData
    (std::vector<int>& axis_lengths
    ,bool user_confirm
    )
{
// EVGENIY !! Why use 'std::size_t' here? It can't actually prevent
// std::accumulate() from overflowing MAX_INT, can it?
    std::size_t count = std::accumulate
        (axis_lengths.begin()
        ,axis_lengths.end()
        ,1
        ,std::multiplies<int>()
        );

    if(!user_confirm || ConfirmOperation(count))
        {
// EVGENIY !! I don't actually see an hourglass cursor. Is this an
// object-lifetime problem?
        wxBusyCursor();

        db_value_->Reshape(axis_lengths);
        SetModified();
        }
}

unsigned int DatabaseTableAdapter::GetDurationMaxBound() const
{
    if(IsVoid())
        {return 1;}

    return db_value_->GetAxisLengths()[eda_duration];
}

bool DatabaseTableAdapter::CanChangeVariationWith(unsigned int n) const
{
    if(IsVoid())
        {return false;}

    return n < db_value_->GetAxisLengths().size();
}

void DatabaseTableAdapter::ConvertValue
    (Coords const& coords
    ,std::vector<int>& indexes
    )
{
    indexes[0] = UnwrapAny<enum_gender>(  coords[0]);
    indexes[1] = UnwrapAny<enum_class>(   coords[1]);
    indexes[2] = UnwrapAny<enum_smoking>( coords[2]);
    indexes[3] = UnwrapAny<int>(          coords[3]);
    indexes[4] = UnwrapAny<enum_uw_basis>(coords[4]);
    indexes[5] = UnwrapAny<enum_state>(   coords[5]);
    indexes[6] = UnwrapAny<int>(          coords[6]);
    BOOST_STATIC_ASSERT( eda_max == 7 );
}

unsigned int DatabaseTableAdapter::GetDimension() const
{
    return eda_max;
}

bool DatabaseTableAdapter::IsVoid() const
{
    return db_value_ == NULL;
}

double DatabaseTableAdapter::DoGetValue(Coords const& coords) const
{
    if(IsVoid())
        {
        return 0;
        }

    ConvertValue(coords, indexes_);

    return db_value_->operator[](indexes_);
}

void DatabaseTableAdapter::DoSetValue
    (Coords const& coords
    ,double const& value
    )
{
    if(IsVoid())
        {return;}

    ConvertValue(coords, indexes_);

    db_value_->operator[](indexes_) = value;
    SetModified();
}

MultiDimTableAny::AxesAny DatabaseTableAdapter::DoGetAxesAny()
{
    AxesAny axes(eda_max);
    axes[0] = AxisAny(new DatabaseGenderAxis());
    axes[1] = AxisAny(new DatabaseClassAxis());
    axes[2] = AxisAny(new DatabaseSmokingAxis());
    axes[3] = AxisAny(new DatabaseIssueAgeAxis());
    axes[4] = AxisAny(new DatabaseUwBasisAxis());
    axes[5] = AxisAny(new DatabaseStateAxis());
    axes[6] = AxisAny(new DatabaseDurationAxis());
    BOOST_STATIC_ASSERT( eda_max == 7 );
    return axes;
}
