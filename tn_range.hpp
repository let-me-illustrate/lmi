// Trammeled Numeric range type: class template declaration.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef tn_range_hpp
#define tn_range_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include <string>
#include <type_traits>
#include <typeinfo>

/// Design notes for class template trammel_base.
///
/// Abstract class trammel_base encapsulates range limits. Classes
/// derived from it are intended to be used only with class template
/// tn_range, to which it grants friendship; having only private
/// members, it cannot be used otherwise.
///
/// If range limits were always integral, then they might have been
/// implemented as non-type template parameters, but floating-point
/// limits are important and often must assume non-integral values, as
/// for interest rates.
///
/// If range limits were always constant, then they might have been
/// implemented as static data or function members. However, limits
/// often must vary depending on dynamic context, so virtual behavior
/// is required.
///
/// Floating-point limits must be adjusted. Given
///   double const z = 1.07;
/// 2.13.3/1 permits any ordering of {z, 1.07} and doesn't require
/// that they be equal, so a range type restricted to [0.0, 1.07]
/// might, if naively implemented, deem 1.07 to be an invalid value.
/// The present implementation prevents that problem by adjusting the
/// nominal minimum and maximum downward and upward, respectively, by
/// a factor of one plus epsilon.
///
/// Private pure virtuals provide the nominal limits and a default
/// value. Public nonvirtuals forward the default value and adjust the
/// nominal limits. Performing that adjustment in this class ensures
/// that it always takes place, the unadjusted limits being externally
/// inaccessible.
///
/// This class also provides a function asserting the postcondition
///   nominal_minimum() <= default_value() <= nominal_maximum()
/// which should be called in the most-derived object's ctor. This is
/// the classic postconstructor problem:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/80ab79d85b150e17
/// which has no tidy general solution. If a maintainer later adds
/// another ctor to class template tn_range, or uses a class derived
/// from class template trammel_base in an unanticipated way, then the
/// postcondition could be violated, but only if the separate mistake
/// of writing limits that are actually inconsistent is also made.

template<typename T>
class trammel_base
{
    friend class tn_range_test;

  public:
    virtual ~trammel_base() = default;

    void assert_sanity   () const;
    T minimum_minimorum  () const;
    T default_initializer() const;
    T maximum_maximorum  () const;

  protected:
    trammel_base() = default;
    trammel_base(trammel_base const&) = default;
    trammel_base& operator=(trammel_base const&) = default;

  private:
    virtual T nominal_minimum() const = 0;
    virtual T default_value  () const = 0;
    virtual T nominal_maximum() const = 0;
};

/// Design notes for class template tn_range_base.
///
/// This class's raison d'être is to permit its member functions to be
/// called across a shared-library boundary, and through a base-class
/// pointer without knowing the template arguments of a concrete
/// instance of derived class tn_range_base.
///
/// Member functions.
///
/// operator==(): Forwards to equal_to(). This string comparison is
/// the only equality operator needed by the MVC framework.
///
/// diagnose_invalidity(): Convert a string argument to a number, and
/// verify that the converted number lies within the permitted range;
/// return a std::string that either explains why such conversion or
/// verification failed, or is empty if both succeeded.
///
/// enforce_circumscription(): Constrain a derived class's value to
/// its range limits.
///
/// equal_to(): Compare a string representation of a number to a
/// derived class's value.
///
/// universal_minimum(), universal_maximum(): Return a dynamic limit
/// as type double, not as the actual type used in a derived class.
/// The motivation is to allow a GUI application to set the limits of,
/// e.g., a spin control, through a pointer to this abstract class.
/// Range-limited controls in wx generally have limits of type int;
/// type double is used here for greater generality.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO tn_range_base
    :public datum_base
{
  public:
    bool operator==(std::string const& s) const {return equal_to(s);}

    virtual std::string diagnose_invalidity(std::string const&) const = 0;
    virtual void enforce_circumscription() = 0;
    virtual bool equal_to(std::string const&) const = 0;
    virtual std::string str() const = 0;
    virtual double universal_minimum() const = 0;
    virtual double universal_maximum() const = 0;
    virtual std::type_info const& value_type() const = 0;
};

/// Design notes for class template tn_range.
///
/// Class tn_range encapsulates a numeric value with upper and lower
/// bounds. Attempting to construct, copy, or assign a value outside
/// the bounded range sets the tn_range object's value to the closest
/// bound. Limits may be altered dynamically subject to
///   nominal_minimum() <= minimum() <= maximum() <= nominal_maximum()
/// where all four quantities are adjusted as in class trammel_base.
///
/// This class is intended primarily for use with GUI input. To assist
/// clients in validating input, limits can be queried, and candidate
/// values tested for conformity with the allowed range. Generally,
/// it is desirable not to let input enter an invalid state, so the
/// value-changing semantics are preferable to throwing an exception.
/// Accordingly, the invariant
///   minimum() <= value() <= maximum()
/// is maintained as a postcondition by all member functions save only
/// those that set the minimum or maximum explicitly: those member
/// functions forbear to modify the value in order to respect the MVC
/// Model's separation of operations that mutate the value (performed
/// in MvcModel::Transmogrify()) from those that do not (performed in
/// MvcModel::Harmonize()).
///
/// As an alternative considered but rejected, the minimum() and
/// maximum() mutators might have been made private. It would suffice
/// to offer only minimum_and_maximum(), which is more robust, as its
/// documentation explains. Consider, though:
///
///    // Minimum and maximum ages must not be permitted to cross.
///    age_minimum.minimum_and_maximum
///        (age_minimum.trammel().minimum_minimorum()
///        ,age_maximum.value()
///        );
///    age_maximum.minimum_and_maximum
///        (age_minimum.value()
///        ,age_maximum.trammel().maximum_maximorum()
///        );
///
/// Requiring defaults to be spelled out makes the code less clear and
/// novel mistakes more likely, particularly because the defaults bear
/// no actual information. This straightforward implementation:
///
///    // Minimum and maximum ages must not be permitted to cross.
///    age_minimum.maximum(age_maximum.value());
///    age_maximum.minimum(age_minimum.value());
///
/// is far clearer, and, under the assumption that the a priori
/// minimum minimorum and maximum maximorum are always suitable, not
/// vulnerable to the problem that minimum_and_maximum() is designed
/// to solve. OTOH, this example:
///
///    // This thing has a minimum.
///    age.minimum(age_minimum.value());
///    // Oh, and it has a maximum, too.
///    age.maximum(age_maximum.value());
///    // Hope the minimum and maximum don't cross!
///
/// is far better written as:
///
///    // This thing has a limited range.
///    age.minimum_and_maximum(age_minimum.value(), age_maximum.value());
///
/// Implementation notes for class template tn_range.
///
/// It is contemplated that this class template will be instantiated
/// to create numerous types in a single translation unit for use in
/// other translation units. Given that usage, it makes sense to
/// instantiate those types explicitly in that single translation
/// unit, in order to avoid bloat.
///
/// Range limits are held in a template class that is specified as a
/// template argument; the implementation enforces the requirement
/// that it be derived from class template trammel_base. Different
/// 'trammel' template classes can be instantiated from the same class
/// template for different numeric types--for instance, to create
/// distinct integer and floating-point percentage UDTs that are
/// constrained to an equivalent [0%, 100%] range.
///
/// The implicitly-defined copy ctor and copy assignment operator do
/// the right thing.

template<typename Number, typename Trammel>
class tn_range final
    :public tn_range_base
{
    static_assert(std::is_base_of_v<trammel_base<Number>,Trammel>);

    friend class tn_range_test;

    typedef Number number_type;
    typedef Trammel trammel_type;

  public:
    tn_range();
    explicit tn_range(Number);
    explicit tn_range(std::string const&);

    tn_range& operator=(Number);
    tn_range& operator=(std::string const&);

    void minimum(Number);
    void maximum(Number);
    void minimum_and_maximum(Number, Number);

    Number minimum() const;
    Number maximum() const;

    bool operator==(tn_range<Number,Trammel> const&) const;
    bool operator==(Number) const;
    bool operator==(std::string const&) const;
    bool operator!=(tn_range<Number,Trammel> const&) const;
    bool operator!=(Number) const;
    bool operator!=(std::string const&) const;
    bool operator<(tn_range<Number,Trammel> const&) const;

    Trammel const& trammel() const;
    Number value() const;

  private:
    Number curb(Number) const;
    std::string format_limits_for_error_message() const;
    bool is_valid(Number) const;

    void concrete_if_not_pure() override {}

    // datum_base required implementation.
    std::istream& read (std::istream&) override;
    std::ostream& write(std::ostream&) const override;

    // tn_range_base required implementation.
    std::string diagnose_invalidity(std::string const&) const override;
    void enforce_circumscription() override;
    bool equal_to(std::string const&) const override;
    std::string str() const override;
    double universal_minimum() const override;
    double universal_maximum() const override;
    std::type_info const& value_type() const override;

    Trammel trammel_;

    Number minimum_;
    Number maximum_;
    Number value_;
};

// Define these few equality operators inline.
//
// To avoid bloat, great pains are taken to instantiate tn_range types
// explicitly in a single translation unit. The same could be done for
// these operators; however, explicit instantiations of four operators
// would take four times as many statements as one class needs, with
// little incremental effect on bloat.

template<typename Number, typename Trammel>
bool operator==(Number n, tn_range<Number,Trammel> const& z)
{
    return z.operator==(n);
}

template<typename Number, typename Trammel>
bool operator==(std::string const& s, tn_range<Number,Trammel> const& z)
{
    return z.operator==(s);
}

template<typename Number, typename Trammel>
bool operator!=(Number n, tn_range<Number,Trammel> const& z)
{
    return !z.operator==(n);
}

template<typename Number, typename Trammel>
bool operator!=(std::string const& s, tn_range<Number,Trammel> const& z)
{
    return !z.operator==(s);
}

#endif // tn_range_hpp
