// Obstruct slicing.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: obstruct_slicing.hpp,v 1.2 2005-11-15 19:35:37 chicares Exp $

#ifndef obstruct_slicing_hpp
#define obstruct_slicing_hpp

#include "config.hpp"

/// Is it desirable to "prevent derivation" from a class? Some say
/// the lack of a virtual destructor is a warning that any competent
/// C++ programmer would consider thoughtfully; others, that feasible
/// safeguards ought to be used where slicing seems to be a danger.
///
/// The purpose of such a safeguard is only to prevent slicing. It may
/// be useful, for instance, for proving that it's safe to revise a
/// legacy class that appears not to need a virtual dtor, but has one
/// anyway, perhaps because of an obsolete "make all dtors virtual"
/// guideline. After such a proof, it could be removed if wanted, but
/// the accompanying unit test shows little runtime cost. It is vain
/// to think that this safeguard would make virtual function calls
/// faster: its purpose is safety, not efficiency.
///
/// Three techniques are discussed here, and one is exposed for use.
/// None literally prevents derivation, but all create obstacles to
/// instantiating a derived class.
///
/// 1. Probably the simplest technique is private virtual derivation
/// from a utility class with a protected constructor:
///
/// class no_derive{ protected: no_derive0() {} };
/// class A0 :private virtual no_derive0 {};
/// class A1 :public A0 {};
///
/// That's too simplistic. The utility class should have a protected
/// copy ctor as well, of course. Worse, two classes in the same
/// inheritance hierarchy might derive from the same utility class,
/// defeating its purpose:
///
/// class A1 :public A0, private virtual no_derive0 {};
///
/// That may seem like fraud, but could happen accidentally.
///
/// 2. The problems noted for the first technique may be solved thus:
///
/// template<typename T>
/// class no_derive1
/// {
///   protected:
///     no_derive1() {}
///     no_derive1(no_derive1 const&) {}
/// };
///
/// class B0: private virtual no_derive1<B0> {};
///
/// class B1: public B0, private virtual no_derive1<B1> {};
///
/// Specializing the utility class on the class to be protected makes
/// it difficult to subvert the mechanism accidentally, because that
/// would require code like
///
/// class B1: public B0, private virtual no_derive1<B0> {};
///
/// which is less likely to go unnoticed. And this technique permits
/// protecting classes that are related by inheritance.
///
/// 3. It seems impossible to subvert the technique described here:
///   http://www.research.att.com/~bs/bs_faq2.html#no-derivation
/// but that is inconveniently verbose. It cannot be simplified by
/// making the utility class a template. No single macro would work,
/// and a pair of macros would be ugly.
///
/// The second technique seems terse and effective enough to be
/// useful, and is therefore exposed here.

template<typename T>
class obstruct_slicing
{
  protected:
    obstruct_slicing() {}
    obstruct_slicing(obstruct_slicing const&) {}
};

#endif // obstruct_slicing_hpp

