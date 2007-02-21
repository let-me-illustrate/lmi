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

// $Id: database_view_editor.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#include "database_view_editor.hpp"

#include "multidimgrid_safe.tpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/timer.h>
#include <wx/utils.h>

#include <numeric>

DatabaseTableAdapter::DatabaseTableAdapter(TDBValue* db_value)
    :db_value_(db_value)
    ,modified_(false)
{
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
        DatabaseDurationAxis& duration_axis
            = static_cast<DatabaseDurationAxis&>(axis);
        if(duration_axis.GetMinValue() != 0)
            fatal_error() << "Duration has to start at 0" << LMI_FLUSH;
        if(duration_axis.GetMaxValue() < 0)
            {
            fatal_error()
                << "Duration has to have at least one value"
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
        DatabaseDurationAxis& duration_axis
            = static_cast<DatabaseDurationAxis&>(axis);
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
        return false;

    std::vector<int> const& axis_lengths = db_value_->GetAxisLengths();
    return n < axis_lengths.size() && axis_lengths[n] > 1;
}

bool DatabaseTableAdapter::ConfirmOperation(unsigned int item_count) const
{
    // allow up to 1M elements
    if(item_count < 1000000)
        return true;

    // ask user about having more than a million elements
    wxString message
        = "The resulting entity will have more than 1 million entries."
          "Are you sure you want to continue?"
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
        return;

    std::vector<int> axis_lengths = db_value_->GetAxisLengths();

    axis_lengths[n] = varies ? TDBValue::maximum_dimensions()[n] : 1;

    ReshapeTableData(axis_lengths, varies);
}

void DatabaseTableAdapter::SetDurationMaxBound(unsigned int n)
{
    if(IsVoid())
        return;

    std::vector<int> axis_lengths = db_value_->GetAxisLengths();

    axis_lengths[eda_duration] = n;

    ReshapeTableData(axis_lengths);
}

void DatabaseTableAdapter::ReshapeTableData
    (std::vector<int>& axis_lengths
    ,bool user_confirm
    )
{
    std::size_t count = std::accumulate
        (axis_lengths.begin()
        ,axis_lengths.end()
        ,1
        ,std::multiplies<int>()
        );

    // should potentially heavy operation be performed?
    if(!user_confirm || ConfirmOperation(count))
        {
        wxBusyCursor();

        db_value_->Reshape(axis_lengths);
        SetModified();
        }
}


unsigned int DatabaseTableAdapter::GetDurationMaxBound() const
{
    if(IsVoid())
        return 1;

    return db_value_->GetAxisLengths()[eda_duration];
}

bool DatabaseTableAdapter::CanChangeVariationWith(unsigned int n) const
{
    if(IsVoid())
        return false;
    return n < db_value_->GetAxisLengths().size();
}

wxString DatabaseTableAdapter::ValueToString(boost::any const& value) const
{
    try
        {
        return wxString::Format("%f", boost::any_cast<double>(value));
        }
    catch(boost::bad_any_cast const&)
        {
        return "#ERR";
        }
}

boost::any DatabaseTableAdapter::StringToValue(wxString const& value) const
{
    double res;
    if(!value.ToDouble(&res))
        res = 0;
    return boost::any(res);
}

void DatabaseTableAdapter::ConvertValue
    (Coords const& coords
    ,std::vector<int>& indexes
    ) const
{
    #define CAST_BOOST_ANY_TO_INT_(z, n, unused) \
    indexes[n] = boost::any_cast<BOOST_PP_CAT(AxisValueType,n)>(coords[n]);

    BOOST_STATIC_ASSERT(DatabaseTableAdapter::eda_max == 7);
    BOOST_PP_REPEAT_FROM_TO(0, 7, CAST_BOOST_ANY_TO_INT_, ~)

    #undef CAST_BOOST_ANY_TO_INT_
}

bool DatabaseTableAdapter::IsVoid() const
{
    return db_value_ == NULL;
}

boost::any DatabaseTableAdapter::DoGetValue(Coords const& coords) const
{
    if(IsVoid())
        return boost::any(static_cast<double>(0));

    indexes_.clear();
    indexes_.resize(coords.size());
    ConvertValue(coords, indexes_);

    double val = db_value_->operator[](indexes_);
    return boost::any(val);
}

void DatabaseTableAdapter::DoSetValue
    (Coords const& coords
    ,boost::any const& value
    )
{
    if(IsVoid())
        return;

    indexes_.clear();
    indexes_.resize(coords.size());
    ConvertValue(coords, indexes_);

    double double_value = boost::any_cast<double>(value);
    db_value_->operator[](indexes_) = double_value;
    SetModified();
}

/// The two methods below are not used but still have to be implemented.
/// The reason for that wierdness is the fact that we are showing
/// pairs of doubles as two columns of doubles and not as a single column
/// (which is the default implementation). Therefore we trick MDGrid and
/// MDGridTable and interface them in such a way, that these two
/// pure virtual methods are not used at all, hense an implementation still
/// has to provided (in this case an empty one, since functions are not used).
double DatabaseTableAdapter::GetValue
    (enum_gender
    ,enum_class
    ,enum_smoking
    ,int
    ,enum_uw_basis
    ,enum_state
    ,int
    ) const
{
    return 0;
}
void DatabaseTableAdapter::SetValue
    (enum_gender
    ,enum_class
    ,enum_smoking
    ,int
    ,enum_uw_basis
    ,enum_state
    ,int
    ,double const&
    )
{
}

MultiDimAxis<enum_gender>* DatabaseTableAdapter::GetAxis0()
{
    return new DatabaseGenderAxis();
}

MultiDimAxis<enum_class>* DatabaseTableAdapter::GetAxis1()
{
    return new DatabaseClassAxis();
}

MultiDimAxis<enum_smoking>* DatabaseTableAdapter::GetAxis2()
{
    return new DatabaseSmokingAxis();
}

MultiDimAxis<int>* DatabaseTableAdapter::GetAxis3()
{
    return new DatabaseIssueAgeAxis();
}

MultiDimAxis<enum_uw_basis>* DatabaseTableAdapter::GetAxis4()
{
    return new DatabaseUwBasisAxis();
}

MultiDimAxis<enum_state>* DatabaseTableAdapter::GetAxis5()
{
    return new DatabaseStateAxis();
}

MultiDimAxis<int>* DatabaseTableAdapter::GetAxis6()
{
    return new DatabaseDurationAxis();
}

