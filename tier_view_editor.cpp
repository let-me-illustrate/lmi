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

// $Id: tier_view_editor.cpp,v 1.9.2.9 2007-03-27 09:11:53 etarassov Exp $

#include "tier_view_editor.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "multidimgrid_safe.tpp"
#include "stratified_charges.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"

#include <wx/grid.h>
#include <wx/treectrl.h>

namespace
{

/// Cell 'read-only' attribute provider. Merely an adapter to query the grid.

class TierCellAttrProvider
    :public wxGridCellAttrProvider
{
  public:
    TierCellAttrProvider(TierEditorGrid& grid) :grid_(grid) {}
    virtual ~TierCellAttrProvider() {}

  private:
    virtual wxGridCellAttr* GetAttr
        (int row
        ,int col
        ,wxGridCellAttr::wxAttrKind kind
        ) const;

  private:
    TierEditorGrid& grid_;
};

wxGridCellAttr* TierCellAttrProvider::GetAttr
    (int row
    ,int col
    ,wxGridCellAttr::wxAttrKind kind
    ) const
{
    // Only return cell attributes.
    if(kind == wxGridCellAttr::Any)
        {
        // Return read-only for the last row of the second column.
        // Simplify it by returning read-only for the rightmous-bottom cell.
        if(grid_.IsReadOnlyCell(row, col))
            {
            // Ownership is transfered to the calling code. wx convention.
            wxGridCellAttr* readonly_attr = new(wx) wxGridCellAttr;
            readonly_attr->SetReadOnly();
            return readonly_attr;
            }
        }
    return wxGridCellAttrProvider::GetAttr(row, col, kind);
}

} // unnamed namespace

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

double_pair TierTableAdapter::DoGetValue(Coords const& coords) const
{
    if(entity_.is_void())
        {return double_pair(0,0);}

    unsigned int band = UnwrapAny<unsigned int>(coords[0]);
    return entity_.get_value(band);
}

void TierTableAdapter::DoSetValue(Coords const& coords, double_pair const& value)
{
    if(entity_.is_void())
        {return;}

    unsigned int band = UnwrapAny<unsigned int>(coords[0]);
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

MultiDimTableAny::AxesAny TierTableAdapter::DoGetAxesAny()
{
    AxesAny axes(1);
    axes[0] = AxisAny(new TierBandAxis());
    return axes;
}

// ---------------------------
// TierEditorGrid implementation
// ---------------------------
TierEditorGrid::TierEditorGrid
    (wxWindow* parent
    ,boost::shared_ptr<TierTableAdapter> const& table
    ,wxWindowID id
    ,wxPoint const& pos
    ,wxSize const& size
    )
    :MultiDimGrid(parent, table, id, pos, size)
{
    SetAttrProvider(new(wx) TierCellAttrProvider(*this));
}

TierEditorGrid::~TierEditorGrid()
{
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
    (unsigned int col
    ) const
{
    if(col != e_column_limit && col != e_column_value)
        {
        fatal_error()
            << "Grid has only two columns: Limit and Value."
            << LMI_FLUSH
            ;
        }
    return static_cast<enum_tier_grid_column>(col);
}

std::string TierEditorGrid::DoGetValue
    (unsigned int row
    ,unsigned int col
    ) const
{
    double_pair const value = GetDoublePairValue(row);
    double const dbl_value = (EnsureValidColumn(col) == e_column_limit)
        ? value.first
        : value.second;
    return DoubleToString(dbl_value);
}

void TierEditorGrid::DoSetValue
    (unsigned int row
    ,unsigned int col
    ,std::string const& text)
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
        {return "Limit";}
    return "Value";
}

double_pair TierEditorGrid::GetDoublePairValue(unsigned int band) const
{
    // hide first axis from the table
    Coords& coords = PrepareFixedCoords(0, band);
    return dynamic_cast<TierTableAdapter const&>(table()).GetValue(coords);
}

std::string TierEditorGrid::DoubleToString(double value)
{
    if(.999 * stratified_entity::limit_maximum < value)
        return "MAXIMUM";
    return value_cast<std::string>(value);
}

double TierEditorGrid::StringToDouble(std::string const& text)
{
    if(0 == text.compare(0, 3, "MAX"))
        return stratified_entity::limit_maximum;
    return value_cast<double>(text);
}

bool TierEditorGrid::IsReadOnlyCell(int row, int col) const
{
    // EVGENIY !! Ensure the band axis is always selected. Part of the product
    // editor feedback to do list.
    return row == static_cast<int>(DoGetNumberRows()) - 1
        && col == e_column_limit
        ;
}

