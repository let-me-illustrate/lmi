// Support classes for editing multidimensional data.
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

// $Id: multidimgrid_tools.hpp,v 1.9 2007-03-05 03:50:21 chicares Exp $

#ifndef multidimgrid_tools_hpp
#define multidimgrid_tools_hpp

#include "config.hpp"

#include "multidimgrid_safe.hpp"
#include "value_cast.hpp"

#include <wx/choice.h>
#include <wx/treectrl.h>

#include <algorithm>

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

/// Design notes for AxisMaxBoundAdjuster<Integral>
///
/// Drop down value list for an axis adjustment.
///
/// This control lets adjust the upper bound for a range based integral axis.
/// Taking lower and upper bounds for the right bound of the axis values range
/// it allows user to change it.
///
/// Two parameters used throughout the class methods:
///   - upper_bound - maximal possible value for the highest axis value
///   - lower_bound - minimal possible value for the highest axis value

template<typename Integral>
class AxisMaxBoundAdjuster
  :public AxisMaxBoundAdjusterBase
{
  public:
    AxisMaxBoundAdjuster
        (MultiDimAxisAny&
        ,MultiDimGrid&
        ,Integral lower_bound
        ,Integral upper_bound
        );

// EVGENIY !! Consider the comments for these two functions. Are the
// functions' purposes obvious from their names? If not, then should
// we choose more descriptive names? Either way, we could then remove
// the comments.
    /// Currently chosen maximum axis value
    Integral GetMaxValue() const;
    /// Set the maximum axis value
    void SetMaxValue(Integral max_value);

  private:
    virtual void DoUponChange();

    MultiDimAxisAny& axis_;
    Integral lower_bound_;
    Integral upper_bound_;
};

/// AxisMaxBoundAdjuster implementation

template<typename Integral>
AxisMaxBoundAdjuster<Integral>::AxisMaxBoundAdjuster
    (MultiDimAxisAny& axis
    ,MultiDimGrid& grid
    ,Integral lower_bound
    ,Integral upper_bound
    )
    :AxisMaxBoundAdjusterBase(grid)
    ,axis_(axis)
    ,lower_bound_(lower_bound)
    ,upper_bound_(upper_bound)
{
    if(upper_bound < lower_bound)
        {
        fatal_error()
            << "Invalid bounds ["
            << lower_bound
            << ","
            << upper_bound
            << "]."
            << LMI_FLUSH;
        }
    std::ostringstream oss;
// EVGENIY !! Why use 'LMI_FLUSH' here? A std::stringstream doesn't
// normally need to be flushed, and I don't think you want to report
// the file name and line number.
    oss
        << "Upper bound for '"
        << axis.GetName()
        << "' axis"
        << LMI_FLUSH
        ;
    SetToolTip(oss.str());
    for(Integral i = lower_bound_; i <= upper_bound_; ++i)
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
void AxisMaxBoundAdjuster<Integral>::SetMaxValue(Integral max_value)
{
    if(max_value < lower_bound_ || upper_bound_ < max_value)
        {
        fatal_error()
            << "Maximum value ("
            << max_value
            << ") is outside allowed range ["
            << lower_bound_
            << ", "
            << upper_bound_
            << "]."
            << LMI_FLUSH
            ;
        }
    wxChoice::SetSelection(max_value - lower_bound_);
}

template<typename Integral>
Integral AxisMaxBoundAdjuster<Integral>::GetMaxValue() const
{
    int value = wxChoice::GetSelection();
    if(value == wxNOT_FOUND)
        {value = 0;}
    Integral max_value = lower_bound_ + static_cast<unsigned int>(value);
    if(max_value < lower_bound_ || upper_bound_ < max_value)
        {
        fatal_error()
            << "Maximum value ("
            << max_value
            << ") is outside allowed range ["
            << lower_bound_
            << ", "
            << upper_bound_
            << "]."
            << LMI_FLUSH
            ;
        }
    return max_value;
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
    typedef AxisMaxBoundAdjuster<Integral>                   Adjuster;
    typedef MultiDimIntegralAxis<Integral>                   GrandBaseClass;
    typedef MultiDimAdjustableAxis<Adjuster, GrandBaseClass> BaseClass;

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

    /// If the axis has been adjusted, then refresh value choice control of the axis
    void UpdateChoiceControl(wxWindow& choice_control) const;

  private:
    /// Create the adjustment control
    virtual Adjuster* DoGetAdjustControl(MultiDimGrid&, MultiDimTableAny&);
    /// Applies user changes to this axis, reads adjustment window
    virtual bool DoApplyAdjustment(Adjuster*, unsigned int n);
    /// Sync the corresponding adjustment control with itself
    virtual bool DoRefreshAdjustment(Adjuster*, unsigned int n);

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
AdjustableMaxBoundAxis<Integral>::AdjustableMaxBoundAxis(std::string const& name)
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
        fatal_error()
            << "Bounds are invalid."
            << LMI_FLUSH;
            ;
        }
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
}

template<typename Integral>
void AdjustableMaxBoundAxis<Integral>::UpdateChoiceControl(wxWindow& choice_control) const
{
// EVGENIY !! dynamic_cast followed by static_cast seems like an
// unusual idiom to me. Is there a reason for it that I don't see?
// Why not write it as
//   MultiDimAxisAnyChoice& choice = dynamic_cast<MultiDimAxisAnyChoice&>(choice_control);
// (which would implicitly throw std::bad_cast if the cast fails)
// instead?
    if(NULL == dynamic_cast<MultiDimAxisAnyChoice*>(&choice_control))
        {
        fatal_error() << "Wrong choice-control type." << LMI_FLUSH;
        }
    MultiDimAxisAnyChoice& choice =
        static_cast<MultiDimAxisAnyChoice&>(choice_control);

    Integral min_value = GrandBaseClass::GetMinValue();
    Integral max_value = GrandBaseClass::GetMaxValue();

    unsigned int new_count = static_cast<unsigned int>(max_value - min_value + 1);
    unsigned int common_count = std::min(choice.GetCount(), new_count);

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
        choice.Append(GetLabel(choice.GetCount() + min_value));
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
AdjustableMaxBoundAxis<Integral>::DoGetAdjustControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& table
    )
{
    // called only once
    if(lower_bound_ == upper_bound_)
        {return NULL;}
    return new Adjuster(*this, grid, lower_bound_, upper_bound_);
}

template<typename Integral>
bool AdjustableMaxBoundAxis<Integral>::DoApplyAdjustment
    (Adjuster* adjuster_window
    ,unsigned int n
    )
{
    if(!adjuster_window)
        {return false;}

    Integral new_max_value = adjuster_window->GetMaxValue();
    if(!(lower_bound_ <= new_max_value && new_max_value <= upper_bound_))
        {
        fatal_error()
            << "New maximum value is outside valid range."
            << LMI_FLUSH
            ;
        }
    bool updated = (GrandBaseClass::GetMaxValue() != new_max_value);
    SetMaxValue(new_max_value);
    return updated;
}

template<typename Integral>
bool AdjustableMaxBoundAxis<Integral>::DoRefreshAdjustment
    (Adjuster* adjuster_window
    ,unsigned int n
    )
{
    if(!adjuster_window)
        {return false;}

    Integral max_value = adjuster_window->GetMaxValue();
    bool updated = (GrandBaseClass::GetMaxValue() != max_value);

    adjuster_window->SetMaxValue(GrandBaseClass::GetMaxValue());
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
        ,long style = wxTR_HAS_BUTTONS
        ,wxValidator const& = wxDefaultValidator
        );

    virtual ~AutoResizingTreeCtrl();

  private:
    virtual wxSize DoGetBestSize() const;

    void DoGetBestSizePrivate
        (wxSize&
        ,wxTreeItemId const&
        ,bool is_root = false
        );
};

#endif // multidimgrid_tools_hpp

