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

// $Id: tier_view_editor.cpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#include "tier_view_editor.hpp"

#include "multidimgrid_safe.tpp"

#include <wx/treectrl.h>

#include <string>

void tier_entity_adapter::ensure_not_void() const
{
    if(is_void())
        {
        fatal_error()
            << "stratified_entity being used is void"
            << LMI_FLUSH
            ;
        }
}
void tier_entity_adapter::ensure_valid_band_number(unsigned int band) const
{
    if(band >= limits().size())
        {
        fatal_error()
            << "Invalid band number"
            << LMI_FLUSH
            ;
        }
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
        fatal_error() << "There has to be at least one band" << LMI_FLUSH;

    if(n == limits().size())
        return;

    static double const max_double = std::numeric_limits<double>::max();

    if(limits().empty())
        {
        limits().push_back(max_double);
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
        double add_limit = (size >= 2) ? limits()[size - 2] : 0;
        double add_value = (size >= 2) ? values()[size - 2] : values()[size - 1];
        limits().insert(limits().begin() + size - 1, n - size, add_limit);
        values().insert(values().begin() + size - 1, n - size, add_value);
    }

    if(limits().size() != values().size())
        {
        fatal_error()
            << "Incorect stratified_entity, vectors size does not match"
            << LMI_FLUSH
            ;
        }
}





double_pair TierTableAdapter::GetValue(unsigned int band) const
{
    if(entity_.is_void())
        {
        return double_pair(0,0);
        }

    return entity_.get_value(band);
}

void TierTableAdapter::SetValue(unsigned int band, double_pair const& value)
{
    if(entity_.is_void())
        {
        return;
        }

    entity_.set_value(band, value);
    SetModified();
}

void TierTableAdapter::EnsureIndexIsZero(unsigned int n) const
{
    if(n != 0)
        {
        fatal_error()
            << "TierTableAdapter could only have one axis"
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
            << "Bands axis adjuster has incorrect values"
            << LMI_FLUSH
            ;
        }
    unsigned int max_bound = GetBandsCount();
    updated = max_bound != (ba.GetMaxValue() + 1);
    if(updated)
        SetBandsCount(ba.GetMaxValue() + 1);
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

void TierEditorGrid::CheckRowAndCol(int row, int col) const
{
    if(col != tgc_limit && col != tgc_value)
        {
        fatal_error()
            << "Grid has only two columns: Limit and Value"
            << LMI_FLUSH
            ;
        }
}

wxString TierEditorGrid::GetValue(int row, int col)
{
    CheckRowAndCol(row, col);
    double_pair value = GetDoublePairValue(static_cast<unsigned int>(row));

    wxString str;
    str << (col == tgc_limit ? value.first : value.second);
    return str;
}

void TierEditorGrid::SetValue(int row, int col, wxString const& str)
{
    CheckRowAndCol(row, col);
    double_pair value = GetDoublePairValue(row);

    double as_double;
    if(!str.ToDouble(&as_double))
        as_double = 0;
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
        return "Limit";
    return "Value";
}

double_pair TierEditorGrid::GetDoublePairValue(int row)
{
    // hide first axis from the table
    PrepareFixedCoords(0, row);

    boost::any value = table_->GetAnyValue(axis_fixed_coords_);

    return boost::any_cast<double_pair>(value);
}

