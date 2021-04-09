// Support classes for editing multidimensional data.
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

#ifndef multidimgrid_tools_hpp
#define multidimgrid_tools_hpp

#include "config.hpp"

#include "multidimgrid_safe.hpp"

#include "alert.hpp"
#include "value_cast.hpp"

#include <wx/choice.h>
#include <wx/treectrl.h>

#include <algorithm>
#include <type_traits>

/// Design notes for AxisMaxBoundAdjusterBase
///
/// Non-template base for MaxValueAdjuster template-class
///
/// wxWidgets won't accept a template class with an event_table, because
/// the corresponding macros does not accept templates, only plain classes.
///
/// This helper registers handler for wxChoice selection change events and
/// defines virtual function DoUponChange() that serves as real handler.

class AxisMaxBoundAdjusterBase
  :public wxChoice
{
  public:
    AxisMaxBoundAdjusterBase(MultiDimGrid&);

  protected:
    /// Apply adjustment value.
    virtual void DoUponChange() = 0;

    /// Gets the parent and cast it to the MultiDimGrid type
    MultiDimGrid&       GetGrid();
    MultiDimGrid const& GetGrid() const;

  private:
    void UponChange(wxCommandEvent&);

    DECLARE_NO_COPY_CLASS(AxisMaxBoundAdjusterBase)
    DECLARE_EVENT_TABLE()
};

/// Design notes for AxisMaxBoundAdjuster
///
/// The class is an adjustment window. It allows user to change a maximum value
/// for axis, where the axis is a range based integral axis.
///
/// It presents a drop down choice list with possible maximum value.
/// The allowed maximum axis values are specified with two parameters:
/// maximum_lower_bound and maximum_upper_bound.
///
/// Consider an int axis with values from [-10, N], where N is from [100, 200].
/// AxisMaxBoundAdjuster then presents a choice list 100..200 to the user.
/// Once user selects a number (115 for example), this number becomes maximum
/// for the axis values (the axis value domain becomes [-10, 115]).
///
/// Two parameters used throughout the class functions:
///   - maximum_upper_bound - upper bound for the maximum axis value
///   - maximum_lower_bound - lower bound for the maximum axis value

template<typename Integral>
class AxisMaxBoundAdjuster
    :public AxisMaxBoundAdjusterBase
{
    static_assert(std::is_integral_v<Integral>);

  public:
    AxisMaxBoundAdjuster
        (MultiDimAxisAny&
        ,MultiDimGrid&
        ,Integral maximum_lower_bound
        ,Integral maximum_upper_bound
        );

    Integral GetMaximumAxisValue() const;
    void     SetMaximumAxisValue(Integral max_value);

  private:
    AxisMaxBoundAdjuster(AxisMaxBoundAdjuster const&) = delete;
    AxisMaxBoundAdjuster& operator=(AxisMaxBoundAdjuster const&) = delete;

    void DoUponChange() override;
    void EnsureValidMaximumAxisValue(Integral const&) const;

    MultiDimAxisAny& axis_;
    Integral maximum_lower_bound_;
    Integral maximum_upper_bound_;
};

/// AxisMaxBoundAdjuster implementation

template<typename Integral>
AxisMaxBoundAdjuster<Integral>::AxisMaxBoundAdjuster
    (MultiDimAxisAny& axis
    ,MultiDimGrid& grid
    ,Integral maximum_lower_bound
    ,Integral maximum_upper_bound
    )
    :AxisMaxBoundAdjusterBase(grid)
    ,axis_(axis)
    ,maximum_lower_bound_{maximum_lower_bound}
    ,maximum_upper_bound_{maximum_upper_bound}
{
    if(maximum_upper_bound < maximum_lower_bound)
        {
        alarum()
            << "Invalid bounds ["
            << maximum_lower_bound
            << ","
            << maximum_upper_bound
            << "]."
            << LMI_FLUSH
            ;
        }
    std::ostringstream oss;
    oss
        << "Upper bound for '"
        << axis.GetName()
        << "' axis"
        ;
    SetToolTip(oss.str());
    for(Integral i = maximum_lower_bound_; i <= maximum_upper_bound_; ++i)
        {
        wxChoice::Append(value_cast<std::string>(i + 1));
        }
}

template<typename Integral>
void AxisMaxBoundAdjuster<Integral>::DoUponChange()
{
    GetGrid().ApplyAxisAdjustment(axis_.GetName());
}

template<typename Integral>
void AxisMaxBoundAdjuster<Integral>::SetMaximumAxisValue(Integral max_value)
{
    EnsureValidMaximumAxisValue(max_value);
    wxChoice::SetSelection(max_value - maximum_lower_bound_);
}

template<typename Integral>
Integral AxisMaxBoundAdjuster<Integral>::GetMaximumAxisValue() const
{
    int value = wxChoice::GetSelection();
    if(value == wxNOT_FOUND)
        {
        value = 0;
        }

    Integral const max_value =
        maximum_lower_bound_ + static_cast<unsigned int>(value);

    EnsureValidMaximumAxisValue(max_value);

    return max_value;
}

template<typename Integral>
void AxisMaxBoundAdjuster<Integral>::EnsureValidMaximumAxisValue
    (Integral const& max_value) const
{
    if(max_value < maximum_lower_bound_ || maximum_upper_bound_ < max_value)
        {
        alarum()
            << "Maximum value ("
            << max_value
            << ") is outside allowed range ["
            << maximum_lower_bound_
            << ", "
            << maximum_upper_bound_
            << "]."
            << LMI_FLUSH
            ;
        }
}

/// Design notes for AdjustableMaxBoundAxis<Integral>
///
/// Adjustable axis - the only difference with AdjustableMaxIntegralAxis is
/// that it provides adjustment control and allows narrowing of value range,
/// precisely it allows the user to control the maximum value the axis values
/// could change.
///
/// Common parameters:
///   - min_value   - current minimal axis value
///   - max_value   - current maximal axis value
///   - lower_bound - the lowest possible value for the maximal axis value
///   - upper_bound - the highest possible value for the maximal axis value
///
/// EVGENIY !! It appears that this is instantiated only with
/// Integral types of 'int' and 'unsigned int'. Why is 'unsigned int'
/// useful--can't it always be 'int', in which case we could write
/// this class and, I think, its base classes as non-template classes?
/// Here and elsewhere (many places in the product editor), I'd prefer
/// that we generally avoid 'unsigned' in the interface as Lakos's
/// book recommends [his arguments are summarized here:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/849321a0199501e5
/// ; see also lmi coding standard section 16.13].

template<typename Integral>
class AdjustableMaxBoundAxis
  :public MultiDimAdjustableAxis
    <AxisMaxBoundAdjuster<Integral>
    ,MultiDimIntegralAxis<Integral>
    >
{
    typedef AxisMaxBoundAdjuster<Integral>                  Adjuster;
    typedef MultiDimIntegralAxis<Integral>                  GrandBaseClass;
    typedef MultiDimAdjustableAxis<Adjuster,GrandBaseClass> BaseClass;

  public:
    AdjustableMaxBoundAxis
        (std::string const& name
        ,Integral min_value
        ,Integral max_value
        ,Integral lower_bound
        ,Integral upper_bound
        );

    /// Use SetValue to set the corresponding values
    AdjustableMaxBoundAxis(std::string const& name);

    /// Change current maximal value
    void SetMaxValue(Integral max_value);

    /// Change lower/upper bounds
    void SetBounds(Integral lower_bound, Integral upper_bound);

    Integral GetLowerBound() const;

    Integral GetUpperBound() const;

    /// If the axis has been adjusted, then refresh value choice control
    void UpdateChoiceControl(MultiDimAxisAnyChoice& choice) const override;

  private:
    /// Create the adjustment control
    Adjuster* DoCreateAdjustControl(MultiDimGrid&, MultiDimTableAny&) override;
    /// Applies user changes to this axis, reads adjustment window
    bool DoApplyAdjustment(Adjuster&, unsigned int axis_id) override;
    /// Sync the corresponding adjustment control with itself
    bool DoRefreshAdjustment(Adjuster&, unsigned int axis_id) override;

    Integral lower_bound_;
    Integral upper_bound_;
};

// ----------------------------------------
// AdjustableMaxIntegralAxis implementation
// ----------------------------------------
template<typename Integral>
AdjustableMaxBoundAxis<Integral>::AdjustableMaxBoundAxis
    (std::string const& name
    ,Integral min_value
    ,Integral max_value
    ,Integral lower_bound
    ,Integral upper_bound
    )
    :BaseClass(name)
{
    GrandBaseClass::SetValues(min_value, max_value, 1);
    SetBounds(lower_bound, upper_bound);
}

template<typename Integral>
AdjustableMaxBoundAxis<Integral>::AdjustableMaxBoundAxis
    (std::string const& name
    )
    :BaseClass(name)
{
    SetBounds(0, 0);
    GrandBaseClass::SetValues(0, 0, 1);
}

template<typename Integral>
Integral AdjustableMaxBoundAxis<Integral>::GetLowerBound() const
{
    return lower_bound_;
}

template<typename Integral>
Integral AdjustableMaxBoundAxis<Integral>::GetUpperBound() const
{
    return upper_bound_;
}

template<typename Integral>
void AdjustableMaxBoundAxis<Integral>::SetMaxValue(Integral max_value)
{
    GrandBaseClass::SetValues
        (GrandBaseClass::GetMinValue()
        ,max_value
        ,GrandBaseClass::GetStep()
        );
}

template<typename Integral>
void AdjustableMaxBoundAxis<Integral>::SetBounds(Integral lower_bound, Integral upper_bound)
{
    if(lower_bound < 0 || upper_bound < lower_bound)
        {
        alarum() << "Bounds are invalid." << LMI_FLUSH;
        }
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
}

template<typename Integral>
void AdjustableMaxBoundAxis<Integral>::UpdateChoiceControl
    (MultiDimAxisAnyChoice& choice_window
    ) const
{
    MultiDimAxisAnyChoice& choice =
        dynamic_cast<MultiDimAxisAnyChoice&>(choice_window);

    Integral const min_value = GrandBaseClass::GetMinValue();
    Integral const max_value = GrandBaseClass::GetMaxValue();

    unsigned int const new_count =
        static_cast<unsigned int>(max_value - min_value + 1);
    unsigned int const common_count =
        std::min(choice.GetCount(), new_count);

    int selection = choice.GetSelection();
    if(selection != wxNOT_FOUND && static_cast<int>(common_count) <= selection)
        {
        selection = wxNOT_FOUND;
        }
    while(common_count < choice.GetCount())
        {
        choice.Delete(choice.GetCount() - 1);
        }

    while(choice.GetCount() < new_count)
        {
        choice.Append(this->GetLabel(choice.GetCount() + min_value));
        }

    if(selection == wxNOT_FOUND)
        {
        // selection was changed
        if(!choice.IsEmpty())
            {
            choice.SetSelection(choice.GetCount() - 1);
            }
        choice.SelectionChanged();
        }
}

template<typename Integral>
typename AdjustableMaxBoundAxis<Integral>::Adjuster*
AdjustableMaxBoundAxis<Integral>::DoCreateAdjustControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& // unused: table
    )
{
    // called only once
    if(lower_bound_ == upper_bound_)
        {
        return nullptr;
        }
    return new Adjuster(*this, grid, lower_bound_, upper_bound_);
}

template<typename Integral>
bool AdjustableMaxBoundAxis<Integral>::DoApplyAdjustment
    (Adjuster& adjust_window
    ,unsigned int // unused: axis_id
    )
{
    Integral const new_max_value = adjust_window.GetMaximumAxisValue();
    if(!(lower_bound_ <= new_max_value && new_max_value <= upper_bound_))
        {
        alarum() << "New maximum value is outside valid range." << LMI_FLUSH;
        }
    bool const updated = (GrandBaseClass::GetMaxValue() != new_max_value);
    SetMaxValue(new_max_value);
    return updated;
}

template<typename Integral>
bool AdjustableMaxBoundAxis<Integral>::DoRefreshAdjustment
    (Adjuster& adjust_window
    ,unsigned int // unused: axis_id
    )
{
    Integral const max_value = adjust_window.GetMaximumAxisValue();
    bool const updated = (GrandBaseClass::GetMaxValue() != max_value);

    adjust_window.SetMaximumAxisValue(GrandBaseClass::GetMaxValue());
    return updated;
}

/// Custom variation of wxTreeCtrl
///
/// It automatically resizes itself to fit every item, so that no horizontal
/// scrolling bar is needed.

class AutoResizingTreeCtrl
    :public wxTreeCtrl
{
  public:
    AutoResizingTreeCtrl
        (wxWindow*
        ,wxWindowID
        ,wxPoint const& = wxDefaultPosition
        ,wxSize const& = wxDefaultSize
        ,long int style = wxTR_HAS_BUTTONS
        ,wxValidator const& = wxDefaultValidator
        );

    ~AutoResizingTreeCtrl() override;

  private:
    wxSize DoGetBestSize() const override;

    void DoGetBestSizePrivate
        (wxSize&
        ,wxTreeItemId const&
        ,bool is_root = false
        );
};

#endif // multidimgrid_tools_hpp
