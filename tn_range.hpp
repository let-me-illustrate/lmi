// Trammeled Numeric range type: class template declaration.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: tn_range.hpp,v 1.8 2006-06-29 19:00:23 wboutin Exp $

#ifndef tn_range_hpp
#define tn_range_hpp

#include "config.hpp"

#include "datum_base.hpp"

#include <boost/operators.hpp>

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) /##/
#endif // Defined __BORLANDC__ .

#include <utility> // std::pair

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
/// TODO ?? Dynamic context not yet implemented.
///
/// The discussion under "Floating-point comparisons" explains why
/// floating-point limits must be adjusted. Derived classes must
/// implement the 'nominal' limit functions; this class implements
/// non-virtual functions that adjust the 'nominal' limits. Rationale:
/// placing that adjustment in this base class ensures that it will be
/// performed as long as class template tn_range calls only the non-
/// virtual limit functions; users can write derived classes without
/// worrying about this.
///
/// This class also provides a default value, which derived classes
/// must implement, and a sanity check that tests the invariant
///   minimum() <= default_value() <= maximum()
///
/// TODO ?? The not-yet-implemented facility to set limits dynamically
/// must ensure that sanity is preserved.
///
/// This class provides distinct maximum() and minimum() functions
/// because that fits the intended use most naturally. For the same
/// reason, class template tn_range provides limits as a std::pair:
/// when used with a GUI, both limits would be needed at the same time.
///
/// The implicitly-defined copy ctor and copy assignment operator do
/// the right thing.

template<typename T>
class trammel_base
{
    template<typename Number, typename Trammel> friend class tn_range;

  public:
    virtual ~trammel_base() {}

  private:
    void check_sanity();
    T maximum();
    T minimum();

    virtual T default_value()   const = 0;
    virtual T nominal_maximum() const = 0;
    virtual T nominal_minimum() const = 0;
};

/// Design notes for class template tn_range.
///
/// Class tn_range encapsulates a numeric value with upper and lower
/// bounds. Attempting to construct, copy, or assign a value outside
/// the bounded range sets the tn_range object's value to the closest
/// bound.
///
/// This class is intended primarily for use with GUI input. Generally,
/// it is desirable not to let input enter an invalid state, so the
/// value-changing semantics are preferable to throwing an exception.
/// To assist clients in validating input, limits can be queried, and
/// candidate values tested for conformity with the allowed range.

/// Implementation notes for class template tn_range.
///
/// It is contemplated that this class template will be instantiated
/// to create numerous types in one translation unit for use in other
/// translation units. Given that usage, it makes sense to instantiate
/// those types explicitly in that one translation unit, in order to
/// avoid bloat. See special note on explicit instantiation below.
///
/// Range limits are held in a class template that is given as a
/// template parameter and must be derived from class trammel_base.
/// This derived 'trammel' class can be reused with different numeric
/// types, for instance to produce distinct integer and floating-point
/// percentage UDTs that are constrained to [0%, 100%].
///
/// The implicitly-defined copy ctor and copy assignment operator do
/// the right thing.

/// Floating-point comparisons: given
///   double const z = 1.07;
/// 2.13.3/1 permits any ordering of {z, 1.07} and doesn't require them
/// to be equal, so a range type restricted to [0.0, 1.07] might, if
/// naively implemented, deem 1.07 to be an invalid value. The
/// implementation prevents this problem by adjusting the maximum_ and
/// minimum_ data members upward and downward respectively by a factor of
/// one plus epsilon.

/// Explicit instantiation
///
/// Specific types require one translation unit (TU) for the
/// instantiation and a header to make them available to other TUs.

template<typename Number, typename Trammel>
class tn_range
    :public datum_base
    ,private boost::equality_comparable<tn_range<Number,Trammel> >
    ,private boost::equality_comparable<tn_range<Number,Trammel>, Number>
    ,private boost::equality_comparable<tn_range<Number,Trammel>, std::string>
{
    BOOST_STATIC_ASSERT(boost::is_arithmetic<Number>::value);

    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((boost::is_base_and_derived
            <trammel_base<Number>
            ,Trammel
            >::value
        ));

  public:
    typedef Number number_type;
    typedef Trammel trammel_type;

    tn_range();
    explicit tn_range(Number);
    explicit tn_range(std::string const&);

    tn_range& operator=(Number);
    tn_range& operator=(std::string const&);

    bool operator==(tn_range<Number,Trammel> const&) const;
    bool operator==(Number) const;
    bool operator==(std::string const&) const;

    bool is_valid(Number) const;
    bool is_valid(std::string const&) const;
    std::pair<Number,Number> limits() const;
    Number trammel(Number) const;

    std::istream& read (std::istream& is);
    std::ostream& write(std::ostream& os) const;

    Number value() const;

// TODO ?? Begin kludge.
//  private:
// End kludge.
    std::string str() const;

    Number maximum_;
    Number minimum_;
    Number value_;
};

#endif // tn_range_hpp

