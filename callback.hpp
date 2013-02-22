// Shared-library callbacks.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifndef callback_hpp
#define callback_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#if !defined __BORLANDC__
#   include <boost/static_assert.hpp>
#   include <boost/type_traits/is_function.hpp>
#   include <boost/type_traits/is_pointer.hpp>
#   include <boost/type_traits/remove_pointer.hpp>
#endif // !defined __BORLANDC__

#include <stdexcept>

/// Design notes for class callback.
///
/// This class encapsulates management of a callback function across a
/// shared-library boundary. The shared library calls the function
/// through a pointer. Another module implements the function. This
/// class helps ensure that the pointer is initialized correctly
/// whenever it's used.
///
/// Motivation.
///
/// Consider an action such as requesting string input. A command-line
/// interface might display a prompt like "Type your name". A GUI
/// might instead pop up a messagebox. A cgi-bin interface would do
/// something else. A shared library that uses this string should be
/// able to call a function like
///   std::string RequestName(std::string prompt);
/// through a pointer defined in the shared library that is resolved
/// at dynamic-link time, relying on the module that provides the
/// particular concrete implementation to set the pointer. This class
/// provides the function pointer, along with a const accessor and a
/// mutator that sets it.
///
/// Public interface.
///
/// operator(): Returns a const copy of the function pointer, throwing
/// an exception if it is null.
///
/// initialize(): Sets the function pointer. Throws an exception if it
/// was not null. Throws an exception if an attempt is made to set it
/// to null (see rationale).
///
/// Implicitly-declared special member functions do the right thing.
///
/// Design alternatives considered; rationale for design choices.
///
/// The function pointer must be set exactly once by invoking
/// initialize(). To permit reinitialization would be trivial, but
/// pointless for the intended use of binding the pointer at dynamic-
/// link time, except in the not-contemplated case of loading and
/// unloading different dynamic libraries.
///
/// Testing that the function pointer is not null on every invocation
/// may be inappropriately expensive for lightweight callbacks that
/// are invoked frequently.
///
/// Compile-time assertions require that the template argument be a
/// pointer to a nonmember function. Probably it would be easy to
/// permit a pointer to member function as well.
///
/// This class is uncopyable and cannot be derived from, not because
/// it would be difficult to allow those things, but because they seem
/// pointless.
///
/// This class does not strive to meet all possible needs for callback
/// functions. An alternative design might use a uniform extern "C"
/// function pointer with a large number of void* arguments that are
/// ignored in most cases; this design prefers type safety.
///
/// Future directions.
///
/// Perhaps this class could be supplanted by boost::function. One of
/// those facilities should be used in 'progress_meter*.?pp' and in
/// 'alert*.?pp'.

template<typename FunctionPointer>
class LMI_SO callback
    :        private lmi::uncopyable <callback<FunctionPointer> >
    ,virtual private obstruct_slicing<callback<FunctionPointer> >
{
#if !defined __BORLANDC__
    BOOST_STATIC_ASSERT(boost::is_pointer<FunctionPointer>::value);
    typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
    BOOST_STATIC_ASSERT(boost::is_function<f_type>::value);
#endif // !defined __BORLANDC__

  public:
    FunctionPointer operator()() const;
    void initialize(FunctionPointer concrete_pointer);

  private:
    static FunctionPointer function_pointer_;
};

template<typename FunctionPointer>
FunctionPointer callback<FunctionPointer>::operator()() const
{
    if(0 == function_pointer_)
        {
        throw std::logic_error("Callback pointer is null.");
        }
    return function_pointer_;
}

template<typename FunctionPointer>
void callback<FunctionPointer>::initialize(FunctionPointer concrete_pointer)
{
    if(0 != function_pointer_)
        {
        throw std::logic_error("Callback pointer already initialized.");
        }
    if(0 == concrete_pointer)
        {
        throw std::logic_error("Attempt to reset callback pointer to null.");
        }
    function_pointer_ = concrete_pointer;
}

#endif // callback_hpp

