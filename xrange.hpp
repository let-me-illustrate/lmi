// Range-checked type.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: xrange.hpp,v 1.4 2005-11-07 01:30:24 chicares Exp $

#ifndef xrange_hpp
#define xrange_hpp

// C arithmetic PODs accept a broad range of values, some of which are not
// appropriate for certain variables. For instance, issue age might be
// constrained as follows:
//   20 <= IssueAge, for all lives
//   IssueAge <= 70, for all rated lives
//   IssueAge <= 80, for all nonrated lives
//
// This template class provides range checking with these behaviors:
//
// Assignment and initialization are permitted only for valid values
// or valid strings; an invalid value triggers an exception.
//
// A string representing a candidate value for assignment can be checked for
// validity without throwing an exception (useful for GUI routines).
//
// The minimum and maximum values can be queried (useful for GUI routines).
//
// The motivation is to create types that are useful for validated input in
// accordance with the principle that invalid input should never be accepted
// and an input class should never be permitted to enter an invalid state.
// We embody the knowledge needed for validating each item in a UDT that
// lets the item validate itself.
//
// As the example above illustrates, range checking can be done only in the
// context of a particular instance-set of all input parameters upon which
// the parameter to be checked depends. This necessitates a tight coupling
// with the class that embodies a set of input parameters.

// Further notes on our particular motivation
//
// There is no unique set {min, max} such that static variables suffice to
// hold the limits. Limits can depend on context, and context varies across
// instances, and across time for a given instance. There can be more than
// one active context if we allow different processes to run at the same time.
//
// Consider fund allocations. Each depends on all other fund allocations, so
// that their total is constrained to be 100%. This demands a function that
// validates each individual allocation change in the scope of a particular
// input-set including the other allocations.
//
// Note that in the case of separate items constrained e.g. to total to 100%,
// validation requires both the input object and the identity of the current
// item. Example: given three inputs {a0, a1, a2} constrained by
//   a0+a1+a2 = 100%,
// determine the range of a0. If a1=10% and a2=20%, then the range of a0 must
// be [70%, 70%]. It is inconvenient to write functions to calculate the sum
// of all elements except a0, except a1, and so on, especially when the
// number of elements is large. It is simpler to calculate the sum of [a0, an)
// and subtract the element in question.
//
// These requirements are sufficient as well for relationships among various
// ages and periods.
//
// Consistency is ensured if each change is validated. Other validators may
// be reset at that time (but need not be called then), or may be reset and
// called when needed for a change to another allocation. Thus, dynamic
// limits should calculated dynamically in the context of the input object,
// not the input class.
//
// For GUI work, presumably a validator class is supplied, and we need to be
// able to create instances of it by supplying the upper and lower limits
// before any change is made.
//
// Example of use
//
// See accompanying file xrange_test.cpp . Template class xrange
// provides a generic UDT with validation and (validated) assignment
// operations. Each particular type needs to implement the following:
//   range_limits()

/*
Do we want xrange to unify semantic and underlying type?
Or should the semantic type do that?
*/

#include "config.hpp"

#include "alert.hpp"
#include "value_cast.hpp"

#include <boost/operators.hpp>

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits/arithmetic_traits.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) /##/
#endif // Defined __BORLANDC__ .

#include <iosfwd>
#include <limits>
#include <ostream>
#include <stdexcept> // range_error
#include <string>
#include <typeinfo>
#include <utility>

/*

Purpose of generic xrange
  range-safe construction and assignment
  access limits
  test candidate value against limits
  hold and access representation as underlying POD type
    later changed to permit UDTs

Requirements for specialized Essence type
  ascertain limits
  access default value

TODO ?? Open questions
  would pure inheritance be a better design?
  where should underlying type be specified?
  should underlying type be a template-parameter of xrange?
  where should *this be held?
  where should *Inputs be held?

If *Inputs is held in the Essence class, then its static member functions
will need to be changed to nonstatic const functions, and the xrange
template will need an object to call them.

A design that puts all responsibility for validation into a GUI doesn't need
to embed *Inputs into each xrange instance. Input items can be changed
only through the GUI, which has *Inputs available. The GUI prevents any
invalid input from reaching the data. The UDTs for data items hold some but
not all of the information necessary for validation. This is true not only
of range types, but also of enum types, some of whose values are not allowed
in every context, e.g. no DBO3 for some products.

callback function?
separate validation in context vs. out of context?
  validator factory for validation in context
there's a census context too: e.g. DOBs not 101 years apart
context also includes database and available product files

parallel structure with lightweight underlying (Essence) types for use
in calculations?

difficulties:
  interdependencies, e.g. issue and retirement age
  forced changes, e.g. an enumerator unavailable in context
    for ranges: force to closer of min and max
    configurable option to throw instead of forcing
    report the problem and the new value when forcing, e.g.
      "Retirement age XX out of bounds--changed to YY."
  different representations of the same datum, e.g. DOB and age
    one data item, and multiple views?
    problem for DOB vs. age when effective date changes:
      age may change, but DOB cannot

*/

// We factor this out and define it in a .cpp file so that we can set a
// breakpoint on it, with debuggers that can't set a breakpoint in a
// template function.
std::string xrange_error_message
    (std::string const& bad_value
    ,std::string const& typeid_name
    ,std::string const& minimum
    ,std::string const& maximum
    );

template<typename Essence, typename Substance>
class xrange_error
    :public std::range_error
{
  public:
    explicit xrange_error
        (Substance const& bad_value
        ,Essence const& = Essence()
        )
        :std::range_error
            (xrange_error_message
                (value_cast<std::string>(bad_value)
                ,typeid(Essence).name()
                ,value_cast<std::string>(Essence::range_limits().first)
                ,value_cast<std::string>(Essence::range_limits().second)
                )
            )
    {}
};

// value_cast<numeric-type>(empty std::string) throws an exception.
// TODO ?? This workaround detects that situation so that corrective
// action can be taken instead of propagating the exception back to
// the main program. It would be better to rewrite the offending code.

template<typename Substance>
bool value_cast_will_succeed(std::string const& s)
{
    if(s.empty())
        {
        return false;
        }
    try
        {
        (void)value_cast<Substance>(s);
        }
    catch(std::exception& x)
        {
        return false;
        }
    return true;
}

template<typename Essence, typename Substance>
class xrange
    :boost::totally_ordered<xrange<Essence,Substance> >
{
public:
    typedef std::pair<Substance,Substance> limits_type;

    xrange();
    xrange(xrange<Essence,Substance> const& z);
    explicit xrange(Substance const& v);
    explicit xrange(std::string const& s);

    xrange& operator=(xrange<Essence,Substance> const& z);
    xrange& operator=(Substance const& v);
    xrange& operator=(std::string const& s);

    // These are needed for non-POD types.
    bool operator==(xrange<Essence,Substance> const& z) const;
    bool operator==(Substance const& v) const;
    bool operator==(std::string const& s) const;

    bool operator<(xrange<Essence,Substance> const&) const;

// TODO ?? Boost coding guidelines 14.3:
// 14.3. Do not define conversion operators, particularly to numeric types.
// Defining operator bool() is always a mistake.
// Write explicit conversion functions instead.
//
// At least for the short-term purpose of porting, we violate this rule.
// Reconsider this later. Even if we don't follow the rule, this should
// be defined outside the class.

    operator Substance const&() const {return representation;}

    std::pair<Substance,Substance> range_limits() const;

    bool is_valid(Substance const& v) const;
    bool is_valid(std::string const& s) const;

    Substance force_valid(Substance const& v) const;
    Substance force_valid(std::string const& s) const;

    Substance value() const;
    std::string str() const;

private:
    void assign_iff_valid(Substance const& v);
    void assign_iff_valid(std::string const& s);

    Substance representation;

    // If Substance is an integer POD type, then it must be signed,
    // so that force_valid() works: else forcing (unsigned)(-1) to
    // [0,100] would yield 100, when 0 is wanted.
// TODO ?? This never worked with bc++5.02, but now that we've allowed
// non-POD types, bc++5.5.1 rejects this; is it correct? If it is
// correct, then is gcc wrong?
#ifndef __BORLANDC__
    BOOST_STATIC_ASSERT
        (  !std::numeric_limits<Substance>::is_integer
        ||  std::numeric_limits<Substance>::is_signed
        );
#endif // not defined __BORLANDC__
};

template<typename Essence, typename Substance>
xrange<Essence,Substance>::xrange()
    :representation(Essence::default_value())
{
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>::xrange
    (xrange<Essence,Substance> const& z
    )
    :representation(z.representation)
{
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>::xrange(Substance const& v)
{
    assign_iff_valid(v);
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>::xrange(std::string const& s)
{
    assign_iff_valid(s);
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>&
xrange<Essence,Substance>::operator=(xrange<Essence,Substance> const& z)
{
    representation = z.representation;
    return *this;
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>&
xrange<Essence,Substance>::operator=(Substance const& v)
{
    assign_iff_valid(v);
    return *this;
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::operator==(xrange<Essence,Substance> const& z) const
{
    return representation == z.representation;
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::operator==(Substance const& v) const
{
    return v == representation;
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::operator==(std::string const& s) const
{
    return s == str();
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::operator<(xrange<Essence,Substance> const& z) const
{
    return representation < z.representation;
}

template<typename Essence, typename Substance>
xrange<Essence,Substance>&
xrange<Essence,Substance>::operator=(std::string const& s)
{
    assign_iff_valid(s);
    return *this;
}

template<typename Essence, typename Substance>
std::pair<Substance,Substance>
xrange<Essence,Substance>::range_limits() const
{
    return Essence::range_limits();
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::is_valid(Substance const& v) const
{
    std::pair<Substance,Substance> limits = range_limits();
    return limits.first <= v && v <= limits.second;
}

template<typename Essence, typename Substance>
bool xrange<Essence,Substance>::is_valid(std::string const& s) const
{
    if(!value_cast_will_succeed<Substance>(s))
        {
        return false;
        }
    return is_valid(value_cast<Substance>(s));
}

template<typename Essence, typename Substance>
Substance xrange<Essence,Substance>::force_valid(Substance const& v) const
{
    std::pair<Substance,Substance> limits = range_limits();
    if(v < limits.first)
        {
        return limits.first;
        }
    else if(limits.second < v)
        {
        return limits.second;
        }
    else
        {
        return v;
        }
}

template<typename Essence, typename Substance>
Substance xrange<Essence,Substance>::force_valid(std::string const& s) const
{
    if(!value_cast_will_succeed<Substance>(s))
        {
        return force_valid(Substance());
        }
    return force_valid(value_cast<Substance>(s));
}

template<typename Essence, typename Substance>
void xrange<Essence,Substance>::assign_iff_valid(Substance const& v)
{
    if(is_valid(v))
        {
        representation = v;
        }
    else
        {
        xrange_error<Essence, Substance> error(v);
        // TODO ?? Instead of the termination semantics of this:
        //   throw xrange_error<Essence, Substance>(v);
        // we want to allow the user to choose resumption semantics,
        // at least until we resolve some logic errors elsewhere.
        hobsons_choice() << error.what() << LMI_FLUSH;
        representation = force_valid(v);
        }
}

template<typename Essence, typename Substance>
void xrange<Essence,Substance>::assign_iff_valid(std::string const& s)
{
    if(value_cast_will_succeed<Substance>(s))
        {
        assign_iff_valid(value_cast<Substance>(s));
        }
    else
        {
        // TODO ?? Instead of the termination semantics of this:
        //   throw xrange_error<Essence, Substance>(s);
        // we want to allow the user to choose resumption semantics,
        // at least until we resolve some logic errors elsewhere.
        hobsons_choice() << "Invalid input: '" << s << "'." << LMI_FLUSH;
        representation = force_valid(s);
        }
}

template<typename Essence, typename Substance>
Substance xrange<Essence,Substance>::value() const
{
    return representation;
}

template<typename Essence, typename Substance>
std::string xrange<Essence,Substance>::str() const
{
    return value_cast<std::string>(representation);
}

template<typename Essence, typename Substance>
std::istream& operator>>
    (std::istream& is
    ,xrange<Essence,Substance>& z
    )
{
    Substance v;
    is >> v;
    z = v;
    return is;
}

template<typename Essence, typename Substance>
std::ostream& operator<<
    (std::ostream& os
    ,xrange<Essence,Substance> const& z
    )
{
    os << z.value();
    return os;
}

#endif // xrange_hpp

