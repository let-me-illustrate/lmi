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

// $Id: multidimgrid_safe.tpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#include "multidimgrid_safe.hpp"

#include <boost/lexical_cast.hpp>

/// MultiDimAxis<E>
/// ---------------

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
/// -------------------

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

#define IMPL_MAKE_ARRAY_n(z, n, unused)                                       \
template <typename Enum>                                                      \
std::vector<std::string> MultiDimEnumAxis<Enum>::MakeArray                    \
    (BOOST_PP_ENUM_PARAMS(n, std::string const& s)                            \
    )                                                                         \
{                                                                             \
    std::string const strings[n] =                                            \
        {                                                                     \
        BOOST_PP_ENUM_PARAMS(n, s)                                            \
        };                                                                    \
    return std::vector<std::string>(strings, strings + n);                    \
}

BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_MAKEARRAY, IMPL_MAKE_ARRAY_n, ~)

#undef IMPL_MAKE_ARRAY_n


/// MultiDimIntegralAxis<Integral>
/// ------------------------------

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
            << "minValue has to be less than or equal to maxValue"
            << LMI_FLUSH
            ;
        }
    if(step < 1)
        {
        fatal_error() << "step has to be at least 1" << LMI_FLUSH;
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
    return boost::lexical_cast<std::string>(min_ + n * step_);
}

template<typename Integral>
Integral MultiDimIntegralAxis<Integral>::DoGetValue(unsigned int n) const
{
    return min_ + n * step_;
}

/// MultiDimTableTypeTraits<ValueType>
/// ----------------------------------

template <typename ValueType>
ValueType MultiDimTableTypeTraits<ValueType>::FromString
    (wxString const& str
    ) const
{
    return str;
}

template <typename ValueType>
wxString MultiDimTableTypeTraits<ValueType>::ToString
    (ValueType const& value
    ) const
{
    return value;
}

/// MultiDimAdjustableAxis<AdjustControl, BaseAxisType>
/// ---------------------------------------------------

template<class AdjustControl, class BaseAxisType>
wxWindow* MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::GetAdjustControl
    (MultiDimGrid& grid
    ,MultiDimTableAny& table
    )
{
    return DoGetAdjustControl(grid, table);
}

template<class AdjustControl, class BaseAxisType>
bool MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::ApplyAdjustment
    (wxWindow* adjustWin
    ,unsigned int n
    )
{
    AxisAdjustControl* win = dynamic_cast<AxisAdjustControl*>(adjustWin);
    if(adjustWin && !win)
        {
        fatal_error()
            << "The axis adjustment control given has incorrect type"
            << LMI_FLUSH
            ;
        }
    return DoApplyAdjustment(win, n);
}

template<class AdjustControl, class BaseAxisType>
bool MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::RefreshAdjustment
    (wxWindow* adjustWin
    ,unsigned int n
    )
{
    AxisAdjustControl* win = dynamic_cast<AxisAdjustControl*>(adjustWin);
    if(adjustWin && !win)
        {
        fatal_error()
            << "The axis adjustment control given has incorrect type"
            << LMI_FLUSH
            ;
        }
    return DoRefreshAdjustment(win, n);
}

template<class AdjustControl, class BaseAxisType>
MultiDimAdjustableAxis<AdjustControl, BaseAxisType>::MultiDimAdjustableAxis
    (std::string const& name
    )
    :BaseAxisType(name)
{
}



/// helper macro used to generate Set/GetValue pseudo-code above
#define MDTABLE_PARAMS_(z, n, unused)                                         \
    BOOST_PP_COMMA_IF(n) boost::any_cast<V##n>(coords[n])                     \

#define MDTABLE_SWITCH_GETAXIS_(z, n, unused)                                 \
    case n: return GetAxis##n();                                              \


/// helper macro used to implement MultiDimTableN and MultiDimGridN classes for
/// given N
#define MDTABLE_IMPLEMENT_FOR_(z, n, unused)                                  \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
unsigned int                                                                  \
MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::GetDimension() const         \
{                                                                             \
    return AxisNumber;                                                        \
}                                                                             \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
MultiDimAxisAny*                                                              \
MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::DoGetAxisAny                 \
    (unsigned int nn                                                          \
    )                                                                         \
{                                                                             \
    switch(nn)                                                                \
        {                                                                     \
        BOOST_PP_REPEAT(n, MDTABLE_SWITCH_GETAXIS_, ~)                        \
        }                                                                     \
    /* will never happen anyway */                                            \
    fatal_error() << "Invalid dimension" << LMI_FLUSH;                        \
    return NULL;                                                                 \
}                                                                             \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
boost::any MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::DoGetValue        \
    (Coords const& coords                                                     \
    ) const                                                                   \
{                                                                             \
    return GetValue(BOOST_PP_REPEAT(n, MDTABLE_PARAMS_, ~));                  \
}                                                                             \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
void MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::DoSetValue              \
    (Coords const& coords                                                     \
    ,boost::any const& value                                                  \
    )                                                                         \
{                                                                             \
    SetValue(BOOST_PP_REPEAT(n, MDTABLE_PARAMS_, ~),                          \
             boost::any_cast<ValueType>(value));                              \
}                                                                             \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
wxString MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::ValueToString       \
    (boost::any const& value                                                  \
    ) const                                                                   \
{                                                                             \
    try                                                                       \
        {                                                                     \
        return converter_.ToString(boost::any_cast<T>(value));                \
        }                                                                     \
    catch(boost::bad_any_cast const&)                                         \
        {                                                                     \
        return "invalid boost::any value type";                               \
        }                                                                     \
}                                                                             \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
boost::any                                                                    \
MultiDimTable##n<T, BOOST_PP_ENUM_PARAMS(n, V)>::StringToValue                \
    (wxString const& str                                                      \
    ) const                                                                   \
{                                                                             \
    return boost::any(static_cast<T>(converter_.FromString(str)));            \
}

/// real code implementing MultiDimGridN classes
BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXIS, MDTABLE_IMPLEMENT_FOR_, ~)

#undef MDTABLE_IMPLEMENT_FOR_
#undef MDTABLE_PARAMS_
#undef MDTABLE_SWITCH_GETAXIS_FOR_

