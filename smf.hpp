// Special member functions (SMFs) and move semantics.
//
// Copyright (C) 2022 Gregory W. Chicares.
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

#ifndef smf_hpp
#define smf_hpp

#include "config.hpp"

#include <type_traits>

namespace smf_mechanics
{
enum provenience
    {extraordinary
    ,default_constructed
    ,copy_constructed
    ,move_constructed
    ,copy_assigned
    ,move_assigned
    };

/// Detect another class's move and copy operations.
///
/// Add a data member of this class to the class to be instrumented
/// (or, less plainly, inherit from it as a mixin).

class sensor
{
  public:
    sensor()                         : p_ {default_constructed} {}
    ~sensor() = default;
    sensor(sensor const&)            : p_ {copy_constructed} {}
    sensor(sensor&&)                 : p_ {move_constructed} {}
    sensor& operator=(sensor const&) {p_ = copy_assigned; return *this;}
    sensor& operator=(sensor&&)      {p_ = move_assigned; return *this;}

    provenience p() const {return p_;}

  private:
    provenience p_ {extraordinary};
};

/// Induce ambiguity between a class's copy and move SMFs.
///
/// If class T has both a copy and a move ctor, both of which can be
/// considered in overload resolution, then instantiating this:
///   T t {ambiguator<T>{}};
/// would be an error because neither ctor is better than the other.
/// However, detecting that ambiguity in this way:
///   !std::is_constructible_v<T, ambiguator<T>>
/// is not an error. Similarly, this:
///   !std::is_assignable_v   <T, ambiguator<T>>
/// detects equiplausible assignment without inducing an error.
///
/// Those non-erroneous expressions happen to do the right thing even
/// if class T is an aggregate, for which instantiating this:
///   T t {ambiguator<T>{}};
/// would be an error for a different reason. See:
///   https://lists.nongnu.org/archive/html/lmi/2022-08/msg00005.html

template<typename T>
struct ambiguator
{
    operator T const&();
    operator T&&();
};

template<typename T> concept equiplausibly_constructible =
    !std::is_constructible_v<T,ambiguator<T>>;

template<typename T> concept equiplausibly_assignable =
    !std::is_assignable_v<T,ambiguator<T>>;
} // namespace smf_mechanics

template<typename T> concept well_move_constructible =
       std::is_move_constructible_v<T>
    && smf_mechanics::equiplausibly_constructible<T>
    ;

template<typename T> concept well_move_assignable =
       std::is_move_assignable_v<T>
    && smf_mechanics::equiplausibly_assignable<T>
    ;

#endif // smf_hpp
