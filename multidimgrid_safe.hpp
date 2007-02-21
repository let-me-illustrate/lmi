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

// $Id: multidimgrid_safe.hpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifndef multidimgrid_safe_hpp
#define multidimgrid_safe_hpp

/// Typesafe version of MultiDimGrid classes. It requires to specify the value
/// and axis types during compile-time: this is more restrictive
/// but is much safer so, whenever possible, these FooN (where N = 0, 1, ...)
/// classes should be used.

#include "config.hpp"

#include "multidimgrid_any.hpp"

#include <boost/any.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <wx/string.h>
#include <wx/window.h>

/// This constant specifies the maximal number of arguments for the MakeArray
/// method to accept.
///
/// Note that this doesn't affect at all the max number of elements in the
/// array passed to MultiDimEnumAxis ctor which is unlimited.
#define MAX_MULTIDIMGRID_MAKEARRAY 10

/// This constant specifies the number of type safe MultiDimGridN classes
/// declared in this header. It may be set in the compiler options to any
/// values but the compilation may be slower if it is very large.
///
/// Note that this doesn't affect at all the number of axis in MultiDimGrid
/// which is unlimited.
#define MAX_MULTIDIMGRID_AXIS 10

/// MultiDim* type-safe classes
/// ---------------------------
///
/// Type-safe classes.
///
/// These classes are provided to allow the type-safe use/checks when the axis
/// and table types are known at compile time.
///
/// Almost every untyped virtual method is reimplemented in those classes to
/// redirect the work to its Do* twin virtual method that takes correctly typed
/// parameters.
/// You should not use untyped method versions (such as MultiDimAxis::GetValue(),
/// MultiDimTableN::SetValue()) - use typed versions instead
/// (MultiDimAxis::DoGetValue(), MultiDimTableN::DoSetValue()).


/// Design notes for MultiDimAxis<E>
///
/// This is the typed version of MultiDimAxisAny.
///
/// Use it as the base class for any specific custom axis with known value type.
/// See also MultiDimEnumAxis MultiDimIntAxis
///
/// GetValue(n): Redirect the method to the typed method DoGetValue()
/// This method should not be used at all, use DoGetValue() instead

template<typename E>
class MultiDimAxis
    :public MultiDimAxisAny
{
  public:
    /// Type of values for this axis
    typedef E ValueType;

    /// @param axis name
    MultiDimAxis(std::string const& name);

    virtual boost::any GetValue(unsigned int n) const;

  protected:
    /// Implement this function returning values of the type ValueType
    virtual ValueType DoGetValue(unsigned int n) const = 0;
};

/// Design notes for MultiDimEnumAxis<E>
///
/// Represents an axis whose values is a set of strings.
///
/// The string values typically correspond to an enum internally, hence the
/// name of the class.
///
/// Template parameter E is the enum type which corresponds to the values of
/// this axis.
///
/// MultiDimEnumAxis(name, values): Creates the axis with the given set of
/// possible values.
///   - name the name of the axis, for GetName() implementation
///   - values all possible values for this axis
///
/// MakeArray(s1, ..., sN): Helper for passing the values to constructor
/// argument.
/// This function takes N strings and returns an array with N elements.
/// Note that in reality there is not a single function but a family of
/// overloaded functions taking up to MAX_MULTIDIMGRID_MAKEARRAY
/// parameters.
///   - s1 the label for the first value
///   - ...
///   - sN the label for the last value

template <typename Enum>
class MultiDimEnumAxis
    :public MultiDimAxis<Enum>
{
    typedef MultiDimAxis<Enum> BaseClass;
  public:
    typedef typename BaseClass::ValueType ValueType;
    MultiDimEnumAxis
        (std::string const& name
        ,std::vector<std::string> const& values
        );

    #define MAKE_ARRAY_n(z, n, unused)                  \
    static std::vector<std::string> MakeArray           \
        (BOOST_PP_ENUM_PARAMS(n, std::string const& s)  \
        );

    /// c++ standard does not allow empty static arrays therefore we should
    /// exclude MakeArray_0 case, starting from 1
    BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_MAKEARRAY, MAKE_ARRAY_n, ~)

    #undef MAKE_ARRAY_n

    /// Base class virtuals
    virtual unsigned int GetCardinality() const;
    virtual std::string  GetLabel(unsigned int n) const;
    virtual Enum         DoGetValue(unsigned int n) const;

  private:
    std::vector<std::string> values_;
};

/// Design notes for MultiDimIntegralAxis<Integral>
///
/// Represents an axis whose values is a range of integeres.
///
/// MultiDimIntegralAxis(name, minValue, maxValue, step): Creates the axis
/// for the range minValue .. maxValue. minValue must be strictly inferior
/// to maxValue, the ctor does not reorder them.
///   - name the name of the axis, for GetName() implementation
///   - minValue the minimal axis value (inclusive), e.g. 0
///   - maxValue the maximal axis value (inclusive), e.g. 100
///   - step only values offset from minValue by a multiple of step are
///     valid values; step must be strictly positive

template<typename Integral>
class MultiDimIntegralAxis
    :public MultiDimAxis<Integral>
{
  public:
    MultiDimIntegralAxis
        (std::string const& name
        ,Integral minValue
        ,Integral maxValue
        ,Integral step
        );

    MultiDimIntegralAxis(std::string const& name);
    /// Return the range lower bound (including)
    Integral GetMinValue() const;
    /// Return the range upper bound (including)
    Integral GetMaxValue() const;
    /// Return the step
    Integral GetStep() const;
    /// Modifier for GetMinValue(), GetMaxValue() and GetStep()
    void SetValues(Integral minValue, Integral maxValue, Integral step);
    /// Override MultiDimAxisAny::GetCardinality()
    virtual unsigned int GetCardinality() const;
    /// Override MultiDimAxisAny::GetLabel()
    virtual std::string GetLabel(unsigned int n) const;
    /// Override MultiDimAxis::DoGetValue()
    virtual Integral DoGetValue(unsigned int n) const;

  protected:
    Integral min_;
    Integral max_;
    Integral step_;
};

typedef MultiDimIntegralAxis<int> MultiDimIntAxis;
typedef MultiDimIntegralAxis<unsigned int> MultiDimUIntAxis;

/// Design notes for MultiDimTableTypeTraits<ValueType>
///
/// Conversion helper for MultiDimTableN template classes.
///
/// Implements convertion between ValueType and wxString.
/// To support a new data type in the MultiDimTable one should specialise
/// this template for the desired type.
/// See int template specialisation for an example.

/// One could use boost::lexical to implement a generic conversion
/// using lexical casts (lexical cast - cast that uses standard iostream
/// facilities as a conversion black box).

template <typename ValueType>
class MultiDimTableTypeTraits
{
  public:
    /// Convert value respresented by a string into ValueType.
    ValueType FromString(wxString const& str) const;
    /// Create a string representation of a value
    wxString ToString(ValueType const& value) const;
};

/// Helper macro implementing MultiDimTableTypeTraits for a given integral type

#define MDTABLE_TTRAITS_INTEGRAL(ValueType              \
    ,FromMethod                                         \
    ,FromType                                           \
    ,ErrFromValue                                       \
    )                                                   \
template <>                                             \
class MultiDimTableTypeTraits<ValueType>                \
{                                                       \
  public:                                               \
    ValueType FromString(wxString const& str) const     \
    {                                                   \
        FromType value;                                 \
        if(str.FromMethod(&value))                      \
            {                                           \
            return value;                               \
            }                                           \
        return ErrFromValue;                            \
    }                                                   \
    wxString ToString(ValueType value) const            \
    {                                                   \
        wxString res;                                   \
        res << value;                                   \
        return res;                                     \
    }                                                   \
}

/// sepcialisations of the MultiDimTableTypeTraits for some common types
MDTABLE_TTRAITS_INTEGRAL(int, ToLong, long, -1);
MDTABLE_TTRAITS_INTEGRAL(unsigned int, ToULong, unsigned long, 0);
MDTABLE_TTRAITS_INTEGRAL(long, ToLong, long, -1);
MDTABLE_TTRAITS_INTEGRAL(unsigned long, ToULong, unsigned long, 0);
MDTABLE_TTRAITS_INTEGRAL(double, ToDouble, double, -1);

/// Design notes for MultiDimTableN<T, V1, ..., VN>
///
/// Type-safe N-dimensional table.
///
/// There is no class MultiDimTableN in reality, only classes MultiDimTable0,
/// MultiDimTable1, ... and so on up to MAX_MULTIDIMGRID_AXIS which is
/// sufficiently large by default but may be predefined to be even larger if
/// this is not enough.
///
/// Template parameters are:
///   - T the type of the grid values
///   - VN the type of the values of the N-th axis
///
/// AxisNumber: Number of axis in the table
///
/// GetValue(a1, ..., an): This function which must be overridden to provide
/// read access to the table values.
///
/// SetValue(a1, ..., an, value): This function which must be overridden
/// to provide write access to the table values.
///
/// GetAxisM(): family of functions for every M in 0..(N-1)
///
/// DoGetValue(coords): Implement base class pure virtual in terms
/// of public GetValue()
///
/// DoSetValue(coords, value): Implement base class pure virtual in terms
/// of public SetValue()
///
/// converter_: internal instance of the converter object
///
/// ValueToString(value) and StringToValue(str):
/// String vs ValueType convertion helpers.
/// Delegates the convertion between type ValueType and wxString
/// to template class MultiDimTableTypeTraits<ValueType>
/// To support new type one should instantiate the MultiDimTableTypeTraits
/// for the type and implement the converting methods.

#define MDTABLE_DECLARE_GETAXIS_(z, n, unused)                                \
    virtual MultiDimAxis<V##n>* GetAxis##n() = 0;                             \

#define MDTABLE_AXISVALUETYPE_TYPEDEF_(z, n, unused)                          \
    typedef V##n AxisValueType##n;                                            \

/// helper macro used to declare MultiDimTableN and MultiDimGridN classes for
/// given N
#define MDTABLE_DECLARE_FOR_(z, n, unused)                                    \
                                                                              \
template <typename T, BOOST_PP_ENUM_PARAMS(n, typename V)>                    \
class MultiDimTable##n                                                        \
    :public MultiDimTableAny                                                  \
{                                                                             \
  public:                                                                     \
    typedef T ValueType;                                                      \
    static unsigned int const AxisNumber = n;                                 \
    BOOST_PP_REPEAT(n, MDTABLE_AXISVALUETYPE_TYPEDEF_, ~)                     \
                                                                              \
    virtual T GetValue(BOOST_PP_ENUM_BINARY_PARAMS(n, V, v)) const = 0;       \
    virtual void SetValue                                                     \
        (BOOST_PP_ENUM_BINARY_PARAMS(n, V, v)                                 \
        ,T const& value                                                       \
        ) = 0;                                                                \
                                                                              \
    virtual unsigned int GetDimension() const;                                \
                                                                              \
    BOOST_PP_REPEAT(n, MDTABLE_DECLARE_GETAXIS_, ~)                           \
                                                                              \
    virtual MultiDimAxisAny* DoGetAxisAny(unsigned int nn);                   \
                                                                              \
  protected:                                                                  \
    virtual boost::any DoGetValue(Coords const& coords) const;                \
                                                                              \
    virtual void DoSetValue(Coords const& coords, boost::any const& value);   \
private:                                                                      \
    MultiDimTableTypeTraits<T> converter_;                                    \
    virtual wxString ValueToString(boost::any const& value) const;            \
    virtual boost::any StringToValue(wxString const& str) const;              \
};

/// real code declaring MultiDimGridN classes
BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXIS, MDTABLE_DECLARE_FOR_, ~)

/// Design notes for MultiDimTableAxisValueType<Table, M>
///
/// Helper class to easily retrieve type information
/// from a MultiDimTableN class.
///
/// It retrieves the type of the Nth axis ot the table.
///
/// It is specialized for every N in 1..MAX_MULTIDIMGRID_AXIS.

template<class Table, int M>
struct MultiDimTableAxisValueType;

#define MDTABLE_AXISVALUETYPE_FOR_(z, n, unused)                            \
                                                                            \
template<class Table>                                                       \
struct MultiDimTableAxisValueType<Table, n>                                 \
{                                                                           \
    static unsigned int const N = n;                                        \
    typedef typename Table::AxisValueType##n Type;                          \
};

BOOST_PP_REPEAT_FROM_TO(1, MAX_MULTIDIMGRID_AXIS, MDTABLE_AXISVALUETYPE_FOR_, ~)

#undef MDTABLE_AXISVALUETYPE_FOR_
#undef MDTABLE_DECLARE_GETAXIS_FOR_
#undef MDTABLE_AXISVALUETYPE_TYPEDEF_
#undef MDTABLE_DECLARE_FOR_
#undef MDTABLE_TTRAITS_INTEGRAL


/// Design notes for MultiDimAdjustableAxis<AdjustControl, BaseAxisType>
///
/// Adjustable Axis base class.
///
/// Use this class as the base for you adjustable axis. This class defines
/// type-safe methods to implement.
///
/// This template uses BaseAxisType as its base class. The only constraint
/// is that this base class has to have a constructor taking axis name as
/// the only parameter.
///
/// Template parameters:
///   - AdjustControl type of the adjustment control you will use for the axis
///   - BaseAxisType base class to use for the axis
///
/// GetAdjustControl(grid, table): Redirects to type-safe method
/// DoGetAdjustControl().
/// Do not override this method, override DoGetAdjustControl instead.

template<class AdjustControl, class BaseAxisType = MultiDimAxisAny>
class MultiDimAdjustableAxis
    :public BaseAxisType
{
    BOOST_STATIC_ASSERT(
        (  boost::is_base_of<MultiDimAxisAny, BaseAxisType>::value
        || boost::is_same<MultiDimAxisAny, BaseAxisType>::value
        ));
  public:
    typedef AdjustControl AxisAdjustControl;

    virtual wxWindow* GetAdjustControl
        (MultiDimGrid& grid
        ,MultiDimTableAny& table
        );
    /// Redirects to the type-safe DoApplyAdjustment()
    virtual bool ApplyAdjustment
        (wxWindow* adjustWin
        ,unsigned int n
        );
    /// Redirects to the type-safe DoRefreshAdjustment()
    virtual bool RefreshAdjustment
        (wxWindow* adjustWin
        ,unsigned int n
        );

  protected:
    /// Default constructor.
    /// This ctor is protected because this class has to be derived from.
    MultiDimAdjustableAxis(std::string const& name);

    /// Type-safe method to override in the derived user class.
    virtual AxisAdjustControl* DoGetAdjustControl
        (MultiDimGrid& grid
        ,MultiDimTableAny& table
        ) = 0;
    /// Type-safe method to override in the derived user class.
    virtual bool DoApplyAdjustment
        (AxisAdjustControl* adjustWin
        ,unsigned int n
        ) = 0;
    /// Type-safe method to override in the derived user class.
    virtual bool DoRefreshAdjustment
        (AxisAdjustControl* adjustWin
        ,unsigned int n
        ) = 0;
};


#endif // multidimgrid_safe_hpp

