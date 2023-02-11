// Convenient base classes that use CRTP.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2022, 2023 Gregory W. Chicares.
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

/// Root class for a polymorphic hierarchy
///
/// A polymorphic hierarchy's root class generally has a virtual dtor.
/// Declaring a dtor inhibits implicit declaration of certain special
/// member functions (move functions are non-declared; copy functions
/// are declared in C++20, but that behavior is deprecated), so the
/// root class must declare them explicitly if they are wanted (as is
/// most often the case). Deriving from class polymorphic_base and
/// relying on the Rule of Zero is preferable to copying and pasting
/// these six declarations into every class.
///
/// CRTP rationale: same as class uncopyable.

template<typename T>
class polymorphic_base
{
  protected:
    polymorphic_base()                                   = default;
    virtual ~polymorphic_base()                          = default;
    polymorphic_base(polymorphic_base const&)            = default;
    polymorphic_base(polymorphic_base&&)                 = default;
    polymorphic_base& operator=(polymorphic_base const&) = default;
    polymorphic_base& operator=(polymorphic_base&&)      = default;
};

// This gcc warning appears to be invalid: class abstract_base has
// (public) implicitly-declared special member functions.
#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#endif // defined __GNUC__

/// Root class for an abstract-xor-final hierarchy
///
/// "Make every class in a polymorphic hierarchy abstract or final" is
/// a reasonable guideline, which this class is intended to support.
/// To be abstract, it must have at least one pure virtual function.
/// It is often recommended to make the dtor pure, especially if no
/// other function is an obvious candidate. Instead, this class uses
/// concrete_if_not_pure() for that purpose. Rationale:
///   - First and foremost, if the dtor is the only pure function,
///     then derived classes are concrete by default. Making non-leaf
///     derived classes abstract by repeatedly declaring each one's
///     dtor pure requires extra work, which is too easily overlooked;
///     and neither gcc-11 nor clang-13 offers a warning option that
///     identifies overlooked cases. But using concrete_if_not_pure()
///     instead of a pure dtor makes derived classes both polymorphic
///     and abstract by default.
///   - Second, because a pure-specifier cannot be combined with a
///     default definition on the same line, a pure dtor requires an
///     out-of-line definition--omission of which is an error that
///     compilers diagnose, so this poses no danger, but it does
///     increase verbosity in each non-leaf class.
/// This approach does require defining concrete_if_not_pure() in each
/// leaf class, but compilers enforce constructibility for classes
/// that are actually instantiated, so there's no risk if this is
/// overlooked. Adding that one line is a reasonable tradeoff for
/// eliding several lines of boilerplate thanks to the Rule of Zero.
///
/// Unfortunately, the "final" keyword must be typed manually in order
/// to complete the "abstract or final" paradigm. Code such as
///   static_assert(std::is_final_v<T>);
/// could be pasted into the concrete_if_not_pure() function body to
/// ensure a compiler warning if desired.
///
/// CRTP rationale: same as class uncopyable.

template<typename T>
class abstract_base : private polymorphic_base<T>
{
  private:
    virtual void concrete_if_not_pure() = 0;
};

#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__

} // namespace lmi

#endif // crtp_base_hpp
