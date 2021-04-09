// Reformed std::size() returning a signed integer.
//
// Copyright (C) 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef ssize_lmi_hpp
#define ssize_lmi_hpp

#include "config.hpp"

#include "bourn_cast.hpp"

#include <cstring>                      // strlen()

/// Reformed std::size() [C++17 27.8/2-3] returning a signed integer.
///
/// Precondition: Cardinality of array or container does not exceed
///   numeric_limits<std::make_signed_t<std::size_t>>::max()
/// . Throws, or fails to compile, if violated.
///
/// Type lmi::ssize_t is simply 'int'. An earlier version had used
///   using ssize_t = std::make_signed_t<std::size_t>;
/// but that is 'long int' for LP64:
///   https://lists.nongnu.org/archive/html/lmi/2018-11/msg00006.html
/// whereas 'int' is preferred as explained below.
///
/// Motivation: to avoid error-prone mixing of signed and unsigned.
///
///   https://github.com/ericniebler/stl2/issues/182
/// "The fact that .size() returns an unsigned integer and iterators'
/// difference type is required to be signed pretty much guarantees
/// that there will be signed/unsigned mismatches in simple and sane
/// uses of the STL. It's a mess. We should use signed integers
/// everywhere that bit-twiddling isn't required."
///
/// Also see:
///   http://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf
/// and the panel discussion here:
///   https://isocpp.org/blog/2013/09/goingnative-2013-videos-now-available
/// relevant portions of which are transcribed below.
///
/// 12:55 Stroustrup: "Use int until you have a reason not to. Don't use
/// unsigned unless you are fiddling with bit patterns. And never mix
/// signed and unsigned."
/// 42:55 Stroustrup: "Whenever you mix signed and unsigned numbers you
/// get trouble. The rules are just very surprising, and they turn up in
/// code in the strangest places. They correlate very strongly with bugs.
/// Now, when people use unsigned numbers, they usually have a reason,
/// and the reason will be something like, well, 'it can't be negative',
/// or 'I need an extra bit'. If you need an extra bit, I am very
/// reluctant to believe you that you really need it, and I don't think
/// that's a good reason. When you think you can't have negative numbers,
/// you will have somebody who initialized your unsigned with minus two,
/// and think they get minus two--and things like that. It is just highly
/// error prone. I think one of the sad things about the standard library
/// is that the indices are unsigned, whereas array indices are signed,
/// and you're sort of doomed to have confusion and problems with that.
/// There are far too many integer types. There are far too lenient rules
/// for mixing them together, and it's a major bug source--which is why
/// I'm saying: stay as simple as you can, use integers till you really,
/// really need something else."
/// Sutter: "It's unfortunately a mistake in the STL, in the standard
/// library, that we use unsigned indices."
/// Carruth: "How frequently have you written arithmetic and wanted ...
/// 2^32 modular behavior? That's really unusual. Why would you select a
/// type which gives you that?"
/// 1:02:50 Question: "but all of the ordinals in the STL, vector::size()
/// and all that kind of stuff, they're unsigned and so it's..."
/// Sutter: "They're wrong."
/// Question: "...it's gonna pollute, right?"
/// Carruth: "We're sorry."
/// Sutter: "As Scott would say, 'we were young'."

namespace lmi
{
using ssize_t = int;

template<typename Container>
constexpr ssize_t ssize(Container const& c)
{
    return bourn_cast<ssize_t>(c.size());
}

template<typename T, auto n>
constexpr ssize_t ssize(T const(&)[n])
{
    return {n};
}

inline ssize_t sstrlen(char const* s)
{
    return bourn_cast<ssize_t>(std::strlen(s));
}
} // namespace lmi

#endif // ssize_lmi_hpp
