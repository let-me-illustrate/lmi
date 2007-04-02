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

// $Id: multidimgrid_safe.hpp,v 1.7.4.2 2007-04-02 11:40:45 etarassov Exp $

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

// TODO ?? EVGENIY !! Section 8.2 of the boost coding standards says
// "Protected data members are forbidden". Is there a really good
// reason to violate that standard here?
  protected:
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

/// Notes for MultiDimTableN <Type, Derived, ConversionPolicy>
///
/// Provides type-safety for multi-dimensional table values.
///
/// Template parameters are:
///   - T                Type of the table values.
///   - Derived          Type of the deriving class.
///   - ConversionPolicy A class providing two const member (or static) methods
///       ValueToString and StringToValue which will be used to convert between
///       std::string and T. The default is to use ValueCastConversion.
///
/// DoGetValue and DoSetValue: These two methods has to be provided in
/// the derived class. These methods are not virtual in MultiDimGridN.
///
/// Example of usage:
/// class MyTable
///     : MultiDimTableN<int, MyTable>
/// {
///   public:
///     /// MultiDimTableN contract:
///     int GetTypedValue(Coords const&) const;
///     void SetTypedValue(Coords const&, int const&);
/// };

template
    <typename T
    ,typename Derived
    ,typename ConversionPolicy = ValueCastConversion<T>
    >
class MultiDimTableN
    :public MultiDimTableAny
    ,private ConversionPolicy
{
  public:
    typedef T ValueType;

    /// Statically casts '*this' to Derived and calls DoGet/SetValue.
    T GetValue(Coords const&) const;
    void SetValue(Coords const&, T const&);

  protected:
    /// MultiDimTableN contract:
    /// The Derived class has to implement these two methods.
    /// Note that these methods are not virtual.
    T    DoGetValue(Coords const&) const;
    void DoSetValue(Coords const&, T const&);

    /// Helper methods for unwrapping/wrapping a value from/into boost::any.
    template<typename A>
    static A UnwrapAny(boost::any const&);
    template<typename A>
    static boost::any WrapAny(A const&);

  private:
    /// MultiDimTableAny overrides.
    virtual boost::any DoGetValueAny(Coords const&) const;
    virtual void       DoSetValueAny(Coords const&, boost::any const&);
    virtual boost::any  StringToValue(std::string const&) const;
    virtual std::string ValueToString(boost::any const&) const;
};

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

