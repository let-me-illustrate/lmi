// Dereference a non-null pointer, optionally downcasting it.
//
// Copyright (C) 2007, 2008 Gregory W. Chicares.
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

// $Id: safely_dereference_as.hpp,v 1.4 2008-04-11 14:33:29 chicares Exp $

#ifndef safely_dereference_as_hpp
#define safely_dereference_as_hpp

#include "config.hpp"

#include "rtti_lmi.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits.hpp>
#else  // Defined __BORLANDC__ .
#   define BOOST_STATIC_ASSERT(deliberately_ignored) class IgNoRe
#endif // Defined __BORLANDC__ .

#include <sstream>
#include <stdexcept>

/// Dereference a non-null pointer, optionally downcasting it.
///
/// Throws informative exceptions if the pointer is null, or if
/// downcasting fails.
///
/// Motivation: Some libraries provide accessors that return pointers,
/// which may be null, to a base class, e.g.
///   wxWindow* wxView::GetFrame(); // NULL if no frame exists.
/// through which an application may need to call derived-class member
/// functions. Undefined behavior occurs if the pointer is null or the
/// pointee's dynamic type is inappropriate, and it is all too easy to
/// operate on the pointer without establishing those preconditions.
/// That problem can be avoided by using this function template to
/// turn the pointer into a reference, and then operating only on the
/// reference.
///
/// Alternatives considered: A macro might have been used to report
/// __LINE__ and __FILE__. We prefer to avoid macros in general. This
/// function template is intended to guard against logic errors, which
/// should be rare. It is easy enough to add a breakpoint, e.g.
///   asm("int $3"); // x86 specific.
/// if the cause of one of those rare errors is not immediately clear.
///
/// The static assertion might have been omitted. That would have
/// permitted checked upcasts and crosscasts; however, those usages
/// seemed likelier to arise from a mistake than from deliberate
/// intention, and the static assertion brings real benefits. With
/// that assertion, static_cast could almost be substituted for
/// dynamic_cast, except that there is no generally-accepted way to
/// assert the absence of virtual derivation as 5.2.9/5 requires.
///
/// Future directions: Enhancements to be made as needs arise include:
///   - Add const analogs.
///   - Optimize for T==U if profiling indicates a worthwhile benefit.

template<typename T, typename U>
T& safely_dereference_as(U* u)
{
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT
        ((
            boost::is_same            <U,T>::value
        ||  boost::is_base_and_derived<U,T>::value
        ));
    if(!u)
        {
        std::ostringstream oss;
        oss
            << "Cannot dereference null pointer of type '"
            << lmi::TypeInfo(typeid(u))
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    T* t = dynamic_cast<T*>(u);
    if(!t)
        {
        std::ostringstream oss;
        oss
            << "Cannot cast pointer of type '"
            << lmi::TypeInfo(typeid(u))
            << "' to type '"
            << lmi::TypeInfo(typeid(T*))
            << "'."
            ;
        throw std::runtime_error(oss.str());
        }
    return *t;
}

#endif // safely_dereference_as_hpp

