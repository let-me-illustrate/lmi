// Stratified charges manager editor classes.
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

// $Id: tier_view_editor.cpp,v 1.9.2.2 2007-03-16 13:46:50 etarassov Exp $

#include "tier_view_editor.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "multidimgrid_safe.tpp"
#include "stratified_charges.hpp"
#include "value_cast.hpp"

#include <wx/treectrl.h>

void tier_entity_adapter::ensure_not_void() const
{
    LMI_ASSERT(!is_void());
}

void tier_entity_adapter::ensure_valid_band_number(unsigned int band) const
{
    LMI_ASSERT(band < limits().size());
}

double_pair tier_entity_adapter::get_value(unsigned int band) const
{
    ensure_not_void();
    ensure_valid_band_number(band);

    return double_pair(limits()[band], values()[band]);
}

void tier_entity_adapter::set_value
    (unsigned int band
    ,double_pair const& value
    )
{
    ensure_not_void();
    ensure_valid_band_number(band);

    limits()[band] = value.first;
    values()[band] = value.second;
}

void tier_entity_adapter::set_bands_count(unsigned int n)
{
    ensure_not_void();

    if(n == 0)
        {fatal_error() << "There must be at least one band." << LMI_FLUSH;}

    if(n == limits().size())
        {return;}

    if(limits().empty())
        {
        limits().push_back(stratified_entity::limit_maximum);
        values().push_back(0);
        }

    unsigned int const size = limits().size();

    if(n < size)
        {
        limits().erase(limits().begin() + n - 1, limits().begin() + size - 1);
        values().erase(values().begin() + n - 1, values().begin() + size - 1);
        }
    else
        {
        double add_limit = 2 <= size ? limits()[size - 2] : 0;
        double add_value = 2 <= size ? values()[size - 2] : values()[size - 1];
        limits().insert(limits().begin() + size - 1, n - size, add_limit);
        values().insert(values().begin() + size - 1, n - size, add_value);
        }

    if(limits().size() != values().size())
        {
        fatal_error()
            << "Inconsistent vector lengths."
            << LMI_FLUSH
            ;
        }
}

double_pair TierTableAdapter::GetValue(unsigned int band) const
{
    if(entity_.is_void())
        {return double_pair(0,0);}

    return entity_.get_value(band);
}

void TierTableAdapter::SetValue(unsigned int band, double_pair const& value)
{
    if(entity_.is_void())
        {return;}

    entity_.set_value(band, value);
    SetModified();
}

void TierTableAdapter::EnsureIndexIsZero(unsigned int n) const
{
    if(n != 0)
        {
        fatal_error()
            << "TierTableAdapter must have only one axis."
            << LMI_FLUSH
            ;
        }
}

bool TierTableAdapter::DoApplyAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    EnsureIndexIsZero(n);

    bool updated = false;

    TierBandAxis& ba = static_cast<TierBandAxis&>(axis);
    if(ba.GetMinValue() != 0 || ba.GetMaxValue() < 0)
        {
        fatal_error()
            << "Band-axis adjuster has invalid limits."
            << LMI_FLUSH
            ;
        }
    unsigned int max_bound = GetBandsCount();
    updated = max_bound != (ba.GetMaxValue() + 1);
    if(updated)
        {SetBandsCount(ba.GetMaxValue() + 1);}
    return updated;
}

bool TierTableAdapter::DoRefreshAxisAdjustment
    (MultiDimAxisAny& axis
    ,unsigned int n
    )
{
    EnsureIndexIsZero(n);

    TierBandAxis& ba = static_cast<TierBandAxis&>(axis);

    bool updated = GetBandsCount() != (ba.GetMaxValue() + 1);
    ba.SetMaxValue(GetBandsCount() - 1);
    return updated;
}

MultiDimAxis<unsigned int>* TierTableAdapter::GetAxis0()
{
    return new TierBandAxis();
}

// ---------------------------
// TierEditorGrid implementation
// ---------------------------
enum e_tier_grid_columns
    {tgc_limit = 0
    ,tgc_value
    ,tgc_max
    };

TierEditorGrid::TierEditorGrid
    (wxWindow* parent
    ,boost::shared_ptr<TierTableAdapter> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
    :MultiDimGrid(parent, table, id, pos, size)
{
}

TierEditorGrid::~TierEditorGrid()
{
}

int TierEditorGrid::GetNumberRows()
{
    return MultiDimGrid::GetNumberCols();
}

int TierEditorGrid::GetNumberCols()
{
    return tgc_max;
}

// EVGENIY !! Isn't the 'row' parameter unused? It appears that
// this function checks only the 'col', not 'row'. Is there any
// constraint on 'row' that should be enforced here? It looks like
// this function is called as a precondition test for most member
// functions, but not for GetDoublePairValue(), which takes only a
// 'row' argument; but if we add a test for 'row' here, then that
// function should probably call this one, too. Wouldn't it be
// better to find some other way to write this, as suggested under
// GetValue() below?

void TierEditorGrid::CheckRowAndCol(int row, int col) const
{
    if(col != tgc_limit && col != tgc_value)
        {
        fatal_error()
            << "Grid has only two columns: Limit and Value."
            << LMI_FLUSH
            ;
        }
}

// EVGENIY !! Consider the conditional operator in this function.
// Here's how I read it:
//
//   switch(col)
//     case tgc_limit: /* use value.first  */ ; break;
//     case tgc_value: /* use value.second */ ; break;
//     case tgc_max:   /* assume that this is impossible */ goto tgc_value;
//     default:        /* assume that this is impossible */ goto tgc_value;
//
// And there are other places where the code assumes that only the
// first two enumerator values are possible. That's in effect
// asserted by CheckRowAndCol(), but I didn't perceive that at first.
// Is there a way to write this more clearly? Should a UDT be used
// instead of int? Should the base class test these arguments against
// the maximum? Do any other ideas occur to you?

wxString TierEditorGrid::GetValue(int row, int col)
{
    CheckRowAndCol(row, col);
    double_pair value = GetDoublePairValue(static_cast<unsigned int>(row));
    return value_cast<std::string>(col == tgc_limit ? value.first : value.second);
}

void TierEditorGrid::SetValue(int row, int col, wxString const& str)
{
    CheckRowAndCol(row, col);
    double_pair value = GetDoublePairValue(row);

    double as_double;
    if(!str.ToDouble(&as_double))
        {
        as_double = 0;
        }
    if(col == tgc_limit)
        {
        value.first = as_double;
        }
    else
        {
        value.second = as_double;
        }

    table_->SetAnyValue(axis_fixed_coords_, boost::any(value));
}

wxString TierEditorGrid::GetRowLabelValue(int row)
{
    CheckRowAndCol(row, 1);
    return MultiDimGrid::GetColLabelValue(row);
}

wxString TierEditorGrid::GetColLabelValue(int col)
{
    CheckRowAndCol(1, col);
    if(col == tgc_limit)
        {return "Limit";}
    return "Value";
}

double_pair TierEditorGrid::GetDoublePairValue(int row)
{
    // hide first axis from the table
    PrepareFixedCoords(0, row);

    boost::any value = table_->GetAnyValue(axis_fixed_coords_);

    return boost::any_cast<double_pair>(value);
}

