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

#ifndef uncopyable_lmi_hpp
#define uncopyable_lmi_hpp

#include "config.hpp"

#include <type_traits>

/// Forbid compiler to generate copy and assignment functions.
///
/// This implementation is an original work. The idea of a mixin with
/// private copy and assignment members is very old and of uncertain
/// provenance. The idea of making that mixin a template seems to have
/// been suggested first by Cacciola:
///   http://lists.boost.org/Archives/boost/2001/09/16912.php
///   http://lists.boost.org/Archives/boost/2001/09/17385.php
///
/// This class is often seen in a non-template guise, but consider:
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
//    class Uncopyable /* non-template implementation */;
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
///   http://www.parashift.com/c++-faq-lite/multiple-inheritance.html#faq-25.9
/// Adding a new class later, e.g.:
///   class E : private Uncopyable, public D {};
/// would require changing D's inheritance to virtual, yet D and E are
/// likely to be declared in different source files.
///
/// The present class does requires a template parameter (which is
/// reasonably constrained to name the class rendered uncopyable):
///
///   class B0 : private uncopyable<B0> {};
///   class B1 : private uncopyable<B1> {};
///   class D  : private uncopyable<D>, public B0, public B1 {};
///
/// but its clarity and efficiency make it the least objectionable
/// option.

namespace lmi
{
template<typename T>
class uncopyable
{
  protected:
    uncopyable() = default;
    ~uncopyable()
        {
        static_assert(std::is_base_of<uncopyable<T>,T>::value, "");
        }

  private:
    uncopyable(uncopyable const&);
    uncopyable& operator=(uncopyable const&);
};
} // namespace lmi

// If lmi provided unit tests that deliberately fail to compile, then
// this could be used:
//
// #include "uncopyable_lmi.hpp"
//
// class X : private lmi::uncopyable<X> {};
//
// int main()
// {
//     X x;
//     X y(x); // Error: cannot copy.
//     x = y;  // Error: cannot assign.
// }

#endif // uncopyable_lmi_hpp

