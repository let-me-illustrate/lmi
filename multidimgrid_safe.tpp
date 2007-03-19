// A control for editing multidimensional data and supporting classes.
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

// $Id: multidimgrid_safe.tpp,v 1.7.2.4 2007-03-19 22:35:27 etarassov Exp $

#include "multidimgrid_safe.hpp"

#include "alert.hpp"
#include "value_cast.hpp"

/// MultiDimAxis<E>

template<typename E>
MultiDimAxis<E>::MultiDimAxis(std::string const& name)
    :MultiDimAxisAny(name)
{
}

template<typename E>
boost::any MultiDimAxis<E>::GetValue(unsigned int n) const
{
    return boost::any(static_cast<ValueType>(DoGetValue(n)));
}

/// MultiDimEnumAxis<E>

template <typename E>
MultiDimEnumAxis<E>::MultiDimEnumAxis
    (std::string const& name
    ,std::vector<std::string> const& values
    )
    :MultiDimAxis<E>(name)
    ,values_(values)
{
}

template <typename E>
unsigned int MultiDimEnumAxis<E>::GetCardinality() const
{
    return values_.size();
}

template <typename E>
std::string MultiDimEnumAxis<E>::GetLabel(unsigned int n) const
{
    return values_[n];
}

template <typename Enum>
Enum MultiDimEnumAxis<Enum>::DoGetValue
    (unsigned int n
    ) const
{
    return static_cast<Enum>(n);
}

/// MultiDimIntegralAxis<Integral>

template<typename Integral>
MultiDimIntegralAxis<Integral>::MultiDimIntegralAxis
    (std::string const& name
    ,Integral minValue
    ,Integral maxValue
    ,Integral step
    )
    :MultiDimAxis<Integral>(name)
{
    SetValues(minValue, maxValue, step);
}

template<typename Integral>
MultiDimIntegralAxis<Integral>::MultiDimIntegralAxis(std::string const& name)
    :MultiDimAxis<Integral>(name)
{
    SetValues(0, 100, 1);
}

template<typename Integral>
Integral MultiDimIntegralAxis<Integral>::GetMinValue() const
{
    return min_;
}

template<typename Integral>
Integral MultiDimIntegralAxis<Integral>::GetMaxValue() const
{
    return max_;
}

template<typename Integral>
Integral MultiDimIntegralAxis<Integral>::GetStep() const
{
    return step_;
}

template<typename Integral>
void MultiDimIntegralAxis<Integral>::SetValues
    (Integral min_value
    ,Integral max_value
    ,Integral step
    )
{
    if(!(min_value <= max_value))
        {
        fatal_error()
            << "Minimum value exceeds maximum value."
            << LMI_FLUSH
            ;
        }
    if(step < 1)
        {
        fatal_error() << "Step must be at least 1." << LMI_FLUSH;
        }

    min_ = min_value;
    max_ = max_value;
    step_ = step;
}

template<typename Integral>
unsigned int MultiDimIntegralAxis<Integral>::GetCardinality() const
{
    return (max_ - min_ + step_) / step_;
}

template<typename Integral>
std::string MultiDimIntegralAxis<Integral>::GetLabel(unsigned int n) const
{
    return value_cast<std::string>(min_ + n * step_);
}

template<typename Integral>
Integral MultiDimIntegralAxis<Integral>::DoGetValue(unsigned int n) const
{
    return min_ + n * step_;
}

/// ValueCastConversion implementation

template<typename T>
std::string ValueCastConversion<T>::ValueToString(T const& t) const
{
    return value_cast<std::string>(t);
}

template<typename T>
T ValueCastConversion<T>::StringToValue(std::string const& text) const
{
    return value_cast<T>(text);
}

/// MultiDimAdjustableAxis<AdjustControl, BaseAxisType>

template<class AdjustControl, class BaseAxisType>
wxWindow* MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::CreateAdjustControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& table
    )
{
    return DoCreateAdjustControl(grid, table);
}

template<class AdjustControl, class BaseAxisType>
bool MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::ApplyAdjustment
    (wxWindow& adjust_window
    ,unsigned int axis_id
    )
{
    return DoApplyAdjustment
        (dynamic_cast<AxisAdjustControl&>(adjust_window)
        ,axis_id
        );
}

template<class AdjustControl, class BaseAxisType>
bool MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::RefreshAdjustment
    (wxWindow& adjust_window
    ,unsigned int axis_id
    )
{
    return DoRefreshAdjustment
        (dynamic_cast<AxisAdjustControl&>(adjust_window)
        ,axis_id
        );
}

template<class AdjustControl, class BaseAxisType>
MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::MultiDimAdjustableAxis
    (std::string const& name
    )
    :BaseAxisType(name)
{
}

// MultiDimTableN

template <typename T, typename Derived, typename C>
T MultiDimTableN<T, Derived, C>::GetValue(Coords const& coords) const
{
    return static_cast<Derived const&>(*this).DoGetValue(coords);
}
template <typename T, typename Derived, typename C>
void MultiDimTableN<T, Derived, C>::SetValue(Coords const& coords, T const& t)
{
    return static_cast<Derived&>(*this).DoSetValue(coords, t);
}

template <typename T, typename D, typename C>
boost::any MultiDimTableN<T, D, C>::DoGetValueAny
    (Coords const& coords
    ) const
{
    return WrapAny<T>(GetValue(coords));
}

template <typename T, typename D, typename C>
void MultiDimTableN<T, D, C>::DoSetValueAny
    (Coords const& coords
    ,boost::any const& value
    )
{
    return SetValue(coords, UnwrapAny<T>(value));
}

template <typename T, typename D, typename C>
boost::any MultiDimTableN<T, D, C>::StringToValue
    (std::string const& text
    ) const
{
    return WrapAny<T>(static_cast<C const&>(*this).StringToValue(text));
}

template <typename T, typename D, typename C>
std::string MultiDimTableN<T, D, C>::ValueToString
    (boost::any const& value
    ) const
{
    return static_cast<C const&>(*this).ValueToString(UnwrapAny<T>(value));
}

template<typename T, typename D, typename C>
template<typename A>
A MultiDimTableN<T, D, C>::UnwrapAny(boost::any const& any)
{
    try
        {
        return boost::any_cast<A>(any);
        }
    catch(boost::bad_any_cast const& e)
        {
        fatal_error()
            << "Type mismatch : "
            << e.what()
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

template<typename T, typename D, typename C>
template<typename A>
boost::any MultiDimTableN<T, D, C>::WrapAny(A const& a)
{
    return boost::any(a);
}

