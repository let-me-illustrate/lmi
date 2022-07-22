// Convenient base classes that use CRTP.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2022 Gregory W. Chicares.
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

#ifndef crtp_base_hpp
#define crtp_base_hpp

#include "config.hpp"

#include <type_traits>

namespace lmi
{

/// Forbid compiler to generate copy and assignment functions.
///
/// Historical note. A mixin class like this was widely used prior to
/// C++11, when the best way to prevent copying was a trick: declare
/// the copy members private and don't implement them (and encapsulate
/// that trick in a base class). Since C++11, no trickery is required:
/// unwanted special members can be declared as deleted. While this
/// class itself should no longer be used, it is preserved because its
/// refinements to the less sophisticated base class typically used
/// before C++11 demonstrate a useful technique that guides the design
/// of other classes. Accordingly, this modernized implementation
/// sheds its historical roots and explicitly deletes unwanted special
/// members, including move as well as copy members.
///
/// This implementation is an original work. The idea of a mixin with
/// private copy and assignment members is very old and of uncertain
/// provenance. The idea of using CRTP for that mixin seems to have
/// been suggested first by Cacciola:
///   http://lists.boost.org/Archives/boost/2001/09/16912.php
///   http://lists.boost.org/Archives/boost/2001/09/17385.php
///
/// This class was often seen in a non-CRTP guise, but consider:
///
///   class B0 : private Uncopyable {};
///   class B1 : private Uncopyable {};
///   class D  : public B0, public B1 {};
///
/// The derived class has two distinct copies of Uncopyable, to which
/// the empty base class optimization cannot be applied:
///   http://groups.google.com/group/comp.lang.c++.moderated/msg/6cc884d20b336d08
/// Rewriting that example to use virtual inheritance:
///
///   class B0 : virtual private Uncopyable {};
///   class B1 : virtual private Uncopyable {};
///   class D  : public B0, public B1 {};
///
/// is likely to introduce its own efficiency issues:
///   http://lists.boost.org/Archives/boost/2001/09/17391.php
///
/// Even where such inefficiencies don't matter, the template version
/// is preferable for its clarity. Consider:
///
///   class Uncopyable /* non-template implementation */;
///   class B0 : private Uncopyable {};
///   class B1 : private Uncopyable {};
///   class D  : private Uncopyable, public B0, public B1 {};
///
/// Deriving class D explicitly from Uncopyable serves two purposes:
/// it prevents the compiler from implicitly defining a copy ctor or
/// a copy assignment operator; and it also documents that the lack of
/// explicit declarations for those special member functions is not an
/// oversight. As self-enforcing documentation, it belongs in class D.
/// However, this example is likely to elicit an ambiguity warning--a
/// Bad Thing when one strives to write code that compiles without any
/// warnings.
///
/// Such warnings may be avoided by virtual inheritance, as in the
/// second example above--though that's inefficient, as already noted.
/// It's also not ideal for documenting the derived class, because
/// 'virtual' belongs in the base classes:
///   https://isocpp.org/wiki/faq/multiple-inheritance#virtual-inheritance-where
/// Adding a new class later, e.g.:
///   class E : private Uncopyable, public D {};
/// would require changing D's inheritance to virtual, yet D and E are
/// likely to be declared in different source files.
///
/// The present class uses CRTP, making its use slightly more verbose:
///
///   class B0 : private uncopyable<B0> {};
///   class B1 : private uncopyable<B1> {};
///   class D  : private uncopyable<D>, public B0, public B1 {};
///
/// but clarity and efficiency are more important. Asserted
/// precondition: the template parameter names the derived class.

template<typename T>
class uncopyable
{
  protected:
    uncopyable() = default;
    uncopyable(uncopyable const&)            = delete;
    uncopyable(uncopyable&&)                 = delete;
    uncopyable& operator=(uncopyable const&) = delete;
    uncopyable& operator=(uncopyable&&)      = delete;
    ~uncopyable()
        {
        static_assert(std::is_base_of_v<uncopyable<T>,T>);
        }
};

} // namespace lmi

#endif // crtp_base_hpp
