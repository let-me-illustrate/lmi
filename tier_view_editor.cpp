// Stratified charges manager editor classes.
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

#include "tier_view_editor.hpp"

#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "ieee754.hpp"                  // infinity<>()
#include "multidimgrid_safe.tpp"
#include "ssize_lmi.hpp"
#include "stratified_charges.hpp"
#include "value_cast.hpp"

void tier_entity_adapter::ensure_not_void() const
{
    LMI_ASSERT(!is_void());
}

void tier_entity_adapter::ensure_valid_band_number(unsigned int band) const
{
    LMI_ASSERT(band < limits().size());
}

tier_entity_adapter::double_pair
tier_entity_adapter::get_value(unsigned int band) const
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
        {
        alarum() << "There must be at least one band." << LMI_FLUSH;
        }

    if(n == limits().size())
        {
        return;
        }

    if(limits().empty())
        {
        // This is conjectured to be unreachable.
        limits().push_back(infinity<double>());
        values().push_back(0.0);
        }

    unsigned int const size = bourn_cast<unsigned int>(lmi::ssize(limits()));

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
        alarum() << "Inconsistent vector lengths." << LMI_FLUSH;
        }
}

TierTableAdapter::double_pair
TierTableAdapter::DoGetValue(Coords const& coords) const
{
    if(entity_.is_void())
        {
        return double_pair(0,0);
        }

    unsigned int const band = UnwrapAny<unsigned int>(coords[0]);
    return entity_.get_value(band);
}

void TierTableAdapter::DoSetValue(Coords const& coords, double_pair const& value)
{
    if(entity_.is_void())
        {
        return;
        }

    unsigned int const band = UnwrapAny<unsigned int>(coords[0]);
    entity_.set_value(band, value);
    SetModified();
}

void TierTableAdapter::EnsureIndexIsZero(unsigned int n) const
{
    if(n != 0)
        {
        alarum() << "TierTableAdapter must have only one axis." << LMI_FLUSH;
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
    if(ba.GetMinValue() != 0 || ba.GetMaxValue() < ba.GetMinValue())
        {
        alarum() << "Band-axis adjuster has invalid limits." << LMI_FLUSH;
        }
    unsigned int max_bound = GetBandsCount();
    updated = max_bound != (ba.GetMaxValue() + 1);
    if(updated)
        {
        SetBandsCount(ba.GetMaxValue() + 1);
        }
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

MultiDimTableAny::AxesAny TierTableAdapter::DoGetAxesAny()
{
    AxesAny axes(1);
    axes[0] = AxisAnyPtr(new TierBandAxis());
    return axes;
}

// ---------------------------
// TierEditorGrid implementation
// ---------------------------

TierEditorGrid::TierEditorGrid() = default;

TierEditorGrid::~TierEditorGrid() = default;

TierEditorGrid::TierEditorGrid
    (wxWindow* parent
    ,std::shared_ptr<TierTableAdapter> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
{
    // We use default MultiDimGrid constructor and Create() call here, because
    // MultiDimGrid constructor/Create calls some of the functions TierEditorGrid
    // overrides, so the object has to be fully constructed by the time
    // MultiDimGrid::Create() is called.
    Create(parent, table, id, pos, size);
}

bool TierEditorGrid::Create
    (wxWindow* parent
    ,std::shared_ptr<TierTableAdapter> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
{
    return MultiDimGrid::Create(parent, table, id, pos, size);
}

unsigned int TierEditorGrid::DoGetNumberRows() const
{
    return MultiDimGrid::DoGetNumberCols();
}

unsigned int TierEditorGrid::DoGetNumberCols() const
{
    return e_column_max;
}

TierEditorGrid::enum_tier_grid_column TierEditorGrid::EnsureValidColumn
    (int col
    ) const
{
    if(col != e_column_limit && col != e_column_value)
        {
        alarum() << "Grid has only two columns: Limit and Value." << LMI_FLUSH;
        }
    return static_cast<enum_tier_grid_column>(col);
}

std::string TierEditorGrid::DoGetValue
    (unsigned int row
    ,unsigned int col
    ) const
{
    double_pair const value = GetDoublePairValue(row);
    double const dbl_value =
          EnsureValidColumn(col) == e_column_limit
        ? value.first
        : value.second
        ;
    return DoubleToString(dbl_value);
}

void TierEditorGrid::DoSetValue
    (unsigned int row
    ,unsigned int col
    ,std::string const& text
    )
{
    double_pair value = GetDoublePairValue(row);

    double const as_double = StringToDouble(text);
    if(EnsureValidColumn(col) == e_column_limit)
        {
        value.first = as_double;
        }
    else
        {
        value.second = as_double;
        }

    dynamic_cast<TierTableAdapter&>(table()).SetValue
        (PrepareFixedCoords(0, row)
        ,value);
}

std::string TierEditorGrid::DoGetRowLabelValue(unsigned int row) const
{
    return MultiDimGrid::DoGetColLabelValue(row);
}

std::string TierEditorGrid::DoGetColLabelValue(unsigned int col) const
{
    if(EnsureValidColumn(col) == e_column_limit)
        {
        return "Limit";
        }
    return "Value";
}

TierEditorGrid::double_pair
TierEditorGrid::GetDoublePairValue(int band) const
{
    return dynamic_cast<TierTableAdapter const&>(table()).GetValue
        (PrepareFixedCoords(0, band)
        );
}

std::string TierEditorGrid::DoubleToString(double value)
{
    return value_cast<std::string>(value);
}

double TierEditorGrid::StringToDouble(std::string const& text)
{
    return value_cast<double>(text);
}
