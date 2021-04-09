// Stratified charges manager editor classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef tier_view_editor_hpp
#define tier_view_editor_hpp

#include "config.hpp"

// Both these headers define base classes that are derived from here.
#include "multidimgrid_safe.hpp"
#include "multidimgrid_tools.hpp"

#include "alert.hpp"
#include "bourn_cast.hpp"
#include "ssize_lmi.hpp"

#include <wx/version.h>                 // Mark this file as wx dependent.

#include <memory>                       // shared_ptr
#include <string>
#include <utility>                      // pair
#include <vector>

/// Notes on TierEditorGrid and TierTableAdapter.
///
/// The data being manipulated is a set of pairs of doubles. Because of that
/// from the point of view of TierTableAdapter the problem is one dimensional.
/// But from the user's point of view it is two dimensional problem with the
/// second dimension being restrained to the set [0, 1] - first and second
/// component of every pair of doubles.
///
/// That's why TierTableAdapter manipulates a one dimensional set of values
/// while TierEditorGrid presents it to the user as a two dimensional set
/// of doubles. For that to work TierEditorGrid overrides default MultiDimGrid
/// behaviour and translates two dimensions into one dimension x [0,1].
/// As a consequence the default implementation of
/// TierEditorGrid::ValueToString and TierEditorGrid::String2Value
/// forces us to define conversions between double_pair and std::string.
/// See below.

/// tier_entity_adapter

class tier_entity_adapter
{
  public:
    typedef std::pair<double,double> double_pair;

    /// We can't store a pointer/reference to stratified_entity because
    /// stratified_entity has private interface for accessing
    /// its limits_ and values_ data members. Therefore we are storing
    /// references directly to its inner data members.
//    tier_entity_adapter(stratified_entity& entity);
    tier_entity_adapter();
    tier_entity_adapter
        (std::vector<double>& limits
        ,std::vector<double>& values
        );

    /// Access data in pairs (corresponding to a band).
    double_pair get_value(unsigned int band) const;
    void set_value(unsigned int band, double_pair const& value);

    /// Access the underlying stratified_entity object number of bands.
    unsigned int get_bands_count() const;
    void set_bands_count(unsigned int n);

    /// Return true if there is no underlying object to manipulate.
    bool is_void() const;

    std::vector<double>&       limits();
    std::vector<double>&       values();
    std::vector<double> const& limits() const;
    std::vector<double> const& values() const;

  private:
    void ensure_not_void() const;
    void ensure_valid_band_number(unsigned int band) const;

    /// stratified_entity is a closed class with no setters for its data
    /// therefore this substitute
    /// stratified_entity* entity_;
    std::vector<double>* limits_;
    std::vector<double>* values_;
};

inline bool tier_entity_adapter::is_void() const
{
    return limits_ == nullptr;
}

inline tier_entity_adapter::tier_entity_adapter()
    :limits_ {nullptr}
    ,values_ {nullptr}
{
}

inline tier_entity_adapter::tier_entity_adapter
    (std::vector<double>& limits
    ,std::vector<double>& values
    )
    :limits_ {&limits}
    ,values_ {&values}
{
    if(limits.size() != values.size())
        {
        alarum() << "Inconsistent vector lengths." << LMI_FLUSH;
        }
}

inline std::vector<double>&       tier_entity_adapter::limits()
{
    return *limits_;
}
inline std::vector<double>&       tier_entity_adapter::values()
{
    return *values_;
}
inline std::vector<double> const& tier_entity_adapter::limits() const
{
    return *limits_;
}
inline std::vector<double> const& tier_entity_adapter::values() const
{
    return *values_;
}

inline unsigned int tier_entity_adapter::get_bands_count() const
{
    return is_void() ? 0 : bourn_cast<unsigned int>(lmi::ssize(limits()));
}

/// Axis representing number of bands in the stratified_entity
///
/// EVGENIY !! Is this an implementation detail that could be moved to
/// the implementation file? If not, then would it make sense to move
/// it into its own header? (Probably the same comment applies
/// elsewhere.)

class TierBandAxis
  :public AdjustableMaxBoundAxis<unsigned int>
{
  public:
    TierBandAxis();

  private:
    typedef AdjustableMaxBoundAxis<unsigned int> Base;
    // TODO ?? where to get that bound of 100 from?
    static unsigned int const max_bound_band = 99;
};

inline TierBandAxis::TierBandAxis()
    :Base("# bands", 0, max_bound_band, 0, max_bound_band)
{
}

/// Note: MultiDimTable<double_pair, unsigned int> requires a conversion
/// between double_pair and std::string (via value_cast). Because of a twist
/// in TierTableAdapter (see the general note above) it will never be used.
/// Therefore specify a dummy conversion and add an extra-assertion to make
/// sure it never gets called.

struct FakeConversion
{
// TODO ?? EVGENIY !! Is an actual implementation needed?
    void fail() const
    {
        alarum() << "Dummy implementation called." << LMI_FLUSH;
    }
  public:
    tier_entity_adapter::double_pair StringToValue(std::string const&) const
    {
        fail();
        throw "Unreachable--silences a compiler diagnostic.";
    }
    std::string ValueToString(tier_entity_adapter::double_pair const&) const
    {
        fail();
        throw "Unreachable--silences a compiler diagnostic.";
    }
};

/// Table that interfaces between stratified_entity and MultiDimGrid
///
/// It manipulates stratified_entity data using pairs of doubles coresponding to
/// bands.

class TierTableAdapter
  :public MultiDimTable<tier_entity_adapter::double_pair, TierTableAdapter, FakeConversion>
{
    friend class TierEditorGrid;

  public:
    typedef tier_entity_adapter::double_pair double_pair;

    TierTableAdapter(tier_entity_adapter entity = tier_entity_adapter());
    ~TierTableAdapter() override;

    /// Getter/Setter for the decorated object
    tier_entity_adapter GetTierEntity() const;
    void                SetTierEntity(tier_entity_adapter const& entity);

    /// Return true if the object data is modified since the last save
    bool IsModified() const;
    void SetModified(bool modified = true);

    void SetBandsCount(unsigned int n);
    unsigned int GetBandsCount() const;

    double_pair DoGetValue(Coords const&) const;
    void        DoSetValue(Coords const&, double_pair const&);

  private:
    // MultiDimTableAny required implementation.
    bool CanChangeVariationWith(unsigned int n) const override;
    unsigned int DoGetDimension() const override {return 1;}
    void MakeVaryByDimension(unsigned int n, bool varies) override;
    AxesAny DoGetAxesAny() override;
    bool VariesByDimension(unsigned int n) const override;

    // MultiDimTableAny overrides.
    bool DoApplyAxisAdjustment(MultiDimAxisAny&, unsigned int n) override;
    bool DoRefreshAxisAdjustment(MultiDimAxisAny&, unsigned int n) override;

    void EnsureIndexIsZero(unsigned int) const;

    tier_entity_adapter entity_;

    /// Modification flag (dirty flag)
    bool modified_;

    DECLARE_NO_COPY_CLASS(TierTableAdapter)
};

inline TierTableAdapter::TierTableAdapter(tier_entity_adapter entity)
    :entity_   {}
    ,modified_ {false}
{
    SetTierEntity(entity);
}
inline TierTableAdapter::~TierTableAdapter() = default;

inline tier_entity_adapter TierTableAdapter::GetTierEntity() const
{
    return entity_;
}

/// Change decorated object
inline void TierTableAdapter::SetTierEntity(tier_entity_adapter const& entity)
{
    entity_ = entity;
    if(entity_.get_bands_count() == 0)
        {
        if(!entity.is_void())
            {
            SetBandsCount(1);
            }
        }
}

inline void TierTableAdapter::SetBandsCount(unsigned int n)
{
    unsigned int old_n = entity_.get_bands_count();
    entity_.set_bands_count(n);
    SetModified(old_n != n);
}

inline unsigned int TierTableAdapter::GetBandsCount() const
{
    // if we have a null entity, then we need to show at least one empty row,
    // thus 1 if is_empty()
    return entity_.is_void() ? 1 : entity_.get_bands_count();
}

/// We depend on the single axis and vary with it
inline bool TierTableAdapter::VariesByDimension(unsigned int) const
{
    return !entity_.is_void();
}

/// User can't change it
inline bool TierTableAdapter::CanChangeVariationWith(unsigned int) const
{
    return false;
}

/// User can't change it
inline void TierTableAdapter::MakeVaryByDimension(unsigned int, bool)
{
}

inline bool TierTableAdapter::IsModified() const
{
    return modified_;
}

inline void TierTableAdapter::SetModified(bool modified)
{
    modified_ = modified;
}

/// The only customized part of that grid is its ability to show pairs of doubles
/// in two columns in the data grid. It changes default MultiDimGrid
/// interface that it uses to retrieve grid strings from underlying table.

class TierEditorGrid
    :public MultiDimGrid
{
  public:
    typedef tier_entity_adapter::double_pair double_pair;

    /// Default constructor, use Create() to really create the control.
    TierEditorGrid();
    ~TierEditorGrid() override;

    TierEditorGrid
        (wxWindow* parent
        ,std::shared_ptr<TierTableAdapter> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );

    bool Create
        (wxWindow* parent
        ,std::shared_ptr<TierTableAdapter> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );

  protected:
    /// Override class MultiDimGrid to show pairs of doubles as two
    /// columns in the grid.

    // MultiDimGrid overrides.
    unsigned int DoGetNumberCols() const override;
    unsigned int DoGetNumberRows() const override;
    std::string DoGetValue(unsigned int row, unsigned int col) const override;
    void        DoSetValue
        (unsigned int row
        ,unsigned int col
        ,std::string const&
        ) override;
    std::string DoGetColLabelValue(unsigned int col) const override;
    std::string DoGetRowLabelValue(unsigned int row) const override;

  private:
    enum enum_tier_grid_column
        {e_column_limit = 0
        ,e_column_value
        ,e_column_max
        };
    double_pair GetDoublePairValue(int row) const;

    enum_tier_grid_column EnsureValidColumn(int col) const;

    static std::string DoubleToString(double);
    static double      StringToDouble(std::string const&);
};

#endif // tier_view_editor_hpp
