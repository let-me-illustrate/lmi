// A control for editing multidimensional data and supporting classes.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef multidimgrid_safe_hpp
#define multidimgrid_safe_hpp

/// Typesafe version of MultiDimGrid classes. It requires to specify the value
/// and axis types during compile-time: this is more restrictive
/// but is much safer so, whenever possible, these FooN (where N = 0, 1, ...)
/// classes should be used.

#include "config.hpp"

#include "multidimgrid_any.hpp"

#include <any>
#include <type_traits>

/// MultiDim* type-safe classes
/// ---------------------------
///
/// Type-safe classes.
///
/// These classes are provided to allow the type-safe use/checks when the axis
/// and table types are known at compile time.
///
/// Almost every untyped virtual function is reimplemented in those classes to
/// redirect the work to its Do* twin virtual function that takes correctly typed
/// parameters.
/// You should not use untyped function versions (such as MultiDimAxis::GetValue(),
/// MultiDimTable::SetValue()) - use typed versions instead
/// (MultiDimAxis::DoGetValue(), MultiDimTable::DoSetValue()).

/// Design notes for MultiDimAxis<E>
///
/// This is the typed version of MultiDimAxisAny.
///
/// Use it as the base class for any specific custom axis with known value type.
/// See also MultiDimEnumAxis MultiDimIntAxis
///
/// GetValue(n): Redirect the function to the typed function DoGetValue()
/// This function should not be used at all, use DoGetValue() instead

template<typename E>
class MultiDimAxis
    :public MultiDimAxisAny
{
  public:
    /// Type of values for this axis
    typedef E ValueType;

    MultiDimAxis(std::string const& name);

    std::any GetValue(unsigned int n) const override;

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

template<typename Enum>
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

    /// Base class virtuals
    unsigned int GetCardinality() const override;
    std::string  GetLabel(unsigned int n) const override;
    Enum         DoGetValue(unsigned int n) const override;

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
    unsigned int GetCardinality() const override;
    /// Override MultiDimAxisAny::GetLabel()
    std::string GetLabel(unsigned int n) const override;
    /// Override MultiDimAxis::DoGetValue()
    Integral DoGetValue(unsigned int n) const override;

  private:
    Integral min_;
    Integral max_;
    Integral step_;
};

typedef MultiDimIntegralAxis<int> MultiDimIntAxis;
typedef MultiDimIntegralAxis<unsigned int> MultiDimUIntAxis;

/// Default conversion policy for type-safe MultiDimGrid.
///
/// Internally it uses value_cast.

template<typename T>
struct ValueCastConversion
{
    std::string ValueToString(T const&) const;
    T StringToValue(std::string const&) const;
};

/// Notes for MultiDimTable <Type, Derived, ConversionPolicy>
///
/// Provides type-safety for multi-dimensional table values.
///
/// Template parameters are:
///   - T                Type of the table values.
///   - Derived          Type of the deriving class.
///   - ConversionPolicy A class providing two const member (or static) functions
///       ValueToString and StringToValue which will be used to convert between
///       std::string and T. The default is to use ValueCastConversion.
///
/// DoGetValue and DoSetValue: These two functions has to be provided in
/// the derived class. These functions are not virtual in MultiDimGridN.
///
/// Example of usage:
/// class MyTable
///     : MultiDimTable<int,MyTable>
/// {
///   public:
///     /// MultiDimTable contract:
///     int GetTypedValue(Coords const&) const;
///     void SetTypedValue(Coords const&, int const&);
/// };

template
    <typename T
    ,typename Derived
    ,typename ConversionPolicy = ValueCastConversion<T>
    >
class MultiDimTable
    :public MultiDimTableAny
    ,private ConversionPolicy
{
  public:
    typedef T ValueType;

    /// Statically casts '*this' to Derived and calls DoGet/SetValue.
    T GetValue(Coords const&) const;
    void SetValue(Coords const&, T const&);

  protected:
    /// MultiDimTable contract:
    /// The Derived class has to implement these two functions.
    /// Note that these functions are not virtual.
    T    DoGetValue(Coords const&) const;
    void DoSetValue(Coords const&, T const&);

    /// Helper functions for unwrapping/wrapping a value from/into std::any.
    template<typename A>
    static A UnwrapAny(std::any const&);
    template<typename A>
    static std::any WrapAny(A const&);

  private:
    /// MultiDimTableAny overrides.
    std::any    DoGetValueAny(Coords const&) const override;
    void        DoSetValueAny(Coords const&, std::any const&) override;
    std::any    StringToValue(std::string const&) const override;
    std::string ValueToString(std::any const&) const override;
};

/// Design notes for MultiDimAdjustableAxis<AdjustControl,BaseAxisType>
///
/// Adjustable Axis base class.
///
/// Use this class as the base for you adjustable axis. This class defines
/// type-safe functions to implement.
///
/// This template uses BaseAxisType as its base class. The only constraint
/// is that this base class has to have a constructor taking axis name as
/// the only parameter.
///
/// Template parameters:
///   - AdjustControl type of the adjustment control you will use for the axis
///   - BaseAxisType base class to use for the axis
///
/// CreateAdjustControl(grid, table): Redirects to type-safe function
/// DoCreateAdjustControl().
/// Do not override this function, override DoCreateAdjustControl instead.

template<typename AdjustControl, typename BaseAxisType = MultiDimAxisAny>
class MultiDimAdjustableAxis
    :public BaseAxisType
{
    static_assert(std::is_base_of_v<MultiDimAxisAny,BaseAxisType>);

  protected:
    typedef AdjustControl AxisAdjustControl;

    /// Default constructor.
    /// This ctor is protected because this class has to be derived from.
    MultiDimAdjustableAxis(std::string const& name);

    /// Type-safe versions of the corresponding functions.
    virtual AxisAdjustControl* DoCreateAdjustControl
        (MultiDimGrid& grid
        ,MultiDimTableAny& table
        ) = 0;
    virtual bool DoApplyAdjustment
        (AxisAdjustControl&
        ,unsigned int axis_id
        ) = 0;
    virtual bool DoRefreshAdjustment
        (AxisAdjustControl&
        ,unsigned int axis_id
        ) = 0;

  private:
    /// MultiDimAxisAny overrides.
    wxWindow* CreateAdjustControl(MultiDimGrid&, MultiDimTableAny&) override;
    bool ApplyAdjustment(wxWindow&, unsigned int axis_id) override;
    bool RefreshAdjustment(wxWindow&, unsigned int axis_id) override;
};

#endif // multidimgrid_safe_hpp
