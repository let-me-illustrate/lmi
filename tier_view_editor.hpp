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

// $Id: tier_view_editor.hpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifndef tier_view_editor_hpp
#define tier_view_editor_hpp

#include "config.hpp"

#include "alert.hpp"
#include "multidimgrid_safe.hpp"
#include "multidimgrid_tools.hpp"

#include <boost/shared_ptr.hpp>

#include <wx/treectrl.h>

#include <string>


typedef std::pair<double, double> double_pair;



/// Stores additional information in a wxTree node

class TierTreeItemData
  :public wxTreeItemData
{
  public:
    TierTreeItemData(std::size_t, std::string const&);

    std::size_t get_id() const;
    std::string const& get_description() const;

  private:
    std::size_t id_;
    std::string description_;
};

inline TierTreeItemData::TierTreeItemData
    (std::size_t id
    ,std::string const& description
    )
    :wxTreeItemData()
    ,id_(id)
    ,description_(description)
{
}
inline std::size_t TierTreeItemData::get_id() const
{
    return id_;
}
inline std::string const& TierTreeItemData::get_description() const
{
    return description_;
}


/// tier_entity_adapter

class tier_entity_adapter
{
  public:
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

    /// get data in pairs (corresponding to a band)
    double_pair get_value(unsigned int band) const;
    /// set data in a pair (corresponding to a band)
    void set_value(unsigned int band, double_pair const& value);

    /// Change number of bands in the underlying stratified_entity object
    void set_bands_count(unsigned int n);
    /// Read the number of bands in the underlying stratified_entity object
    unsigned int get_bands_count() const;

    /// @return true if we don't have no underlying object to manipulate
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
    return limits_ == NULL;
}

inline tier_entity_adapter::tier_entity_adapter()
    :limits_(NULL)
    ,values_(NULL)
{
}

inline tier_entity_adapter::tier_entity_adapter
    (std::vector<double>& limits
    ,std::vector<double>& values
    )
    :limits_(&limits)
    ,values_(&values)
{
    if(limits.size() != values.size())
        fatal_error() << "invalid stratified_entity" << LMI_FLUSH;
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
    return is_void() ? 0 : limits().size();
}


/// Axis representing number of bands in the stratified_entity

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

/// Note.
///
/// MultiDimTable1<double_pair, unsigned int> requires a specification of
/// MultiDimTableTypeTraits to be provided. In case of TierTableAdapter
/// it will never going to be used, therefore specify a dummy implementation
/// with assertion that its never get called.

template <>
class MultiDimTableTypeTraits<double_pair>
{
    void fail() const
    {
        fatal_error() << "Dummy implementation is called" << LMI_FLUSH;
    }

  public:
    /// Convert value respresented by a string into ValueType.
    double_pair FromString(wxString const&) const
    {
        fail();
        return double_pair(0, 0);
    }

    /// Create a string representation of a value
    wxString ToString(double_pair const&) const
    {
        fail();
        return "";
    }
};


/// Table that interfaces between stratified_entity and MultiDimGrid
///
/// It manipulates stratified_entity data using pairs of doubles coresponding to
/// bands.

class TierTableAdapter
  :public MultiDimTable1<double_pair, unsigned int>
{
    typedef MultiDimTable1<double_pair, unsigned int> Base;

  public:
    TierTableAdapter(tier_entity_adapter entity = tier_entity_adapter());
    virtual ~TierTableAdapter();

    /// Getter/Setter for the decorated object
    tier_entity_adapter GetTierEntity() const;
    void                SetTierEntity(tier_entity_adapter const& entity);

    /// Return true if the object data is modified since the last save
    bool IsModified() const;
    void SetModified(bool modified = true);

    void SetBandsCount(unsigned int n);
    unsigned int GetBandsCount() const;

  private:
    friend class TierEditorGrid;

    // MultiDimTableAny method overrides
    virtual bool VariesByDimension(unsigned int n) const;
    virtual bool CanChangeVariationWith(unsigned int n) const;
    virtual void MakeVaryByDimension(unsigned int n, bool varies);
    virtual MultiDimAxis<unsigned int>* GetAxis0();
    virtual bool DoApplyAxisAdjustment(MultiDimAxisAny&, unsigned int n);
    virtual bool DoRefreshAxisAdjustment(MultiDimAxisAny&, unsigned int n);

    virtual double_pair GetValue(unsigned int band) const;
    virtual void        SetValue(unsigned int band, double_pair const& value);

    void EnsureIndexIsZero(unsigned int) const;

    tier_entity_adapter entity_;

    /// Modification flag (dirty flag)
    bool modified_;

    DECLARE_NO_COPY_CLASS(TierTableAdapter)
};

inline TierTableAdapter::TierTableAdapter(tier_entity_adapter entity)
    :entity_()
    ,modified_(false)
{
    SetTierEntity(entity);
}
inline TierTableAdapter::~TierTableAdapter()
{
}

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
    TierEditorGrid
        (wxWindow* parent
        ,boost::shared_ptr<TierTableAdapter> const& table
        ,wxWindowID id = wxID_ANY
        ,wxPoint const& pos = wxDefaultPosition
        ,wxSize const& size = wxDefaultSize
        );
    virtual ~TierEditorGrid();

  private:
    double_pair GetDoublePairValue(int row);

    /// We need to show pairs of doubles, therefore we will override
    /// MultiDimGrid to behave accordingly
    /// Always show data in pairs (two columns in the grid)
    virtual int      GetNumberRows();
    virtual int      GetNumberCols();
    virtual wxString GetValue(int row, int col);
    virtual void     SetValue(int row, int col, wxString const& str);
    virtual wxString GetRowLabelValue(int row);
    virtual wxString GetColLabelValue(int col);

    void CheckRowAndCol(int row, int col) const;
};



#endif // tier_view_editor_hpp

