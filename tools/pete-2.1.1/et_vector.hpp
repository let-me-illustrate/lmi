// PETE with std::vector.
//
// Copyright (C) 1998, 1999, 2000, 2002  Los Alamos National Laboratory,
// Copyright (C) 1998, 1999, 2000, 2002  CodeSourcery, LLC
// Copyright (C) 2008, 2021 Gregory W. Chicares.
//
// Portions of this file were adapted from FreePOOMA.
// Do not confuse this with any original version available from LANL.
//
// FreePOOMA is free software; you can redistribute it and/or modify it
// under the terms of the Expat license.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the Expat
// license for more details.
//
// You should have received a copy of the Expat license along with
// FreePOOMA; see the file LICENSE.
//
// Modified extensively by Gregory W. Chicares in 2008 and later years.
// GWC modifications are licensed as follows:
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

// This header would seem to belong in lmi's main source directory,
// but instead it resides in a subdirectory due to the weird structure
// of PETE. Study these old discussions:
//    https://lists.nongnu.org/archive/html/lmi/2008-09/msg00021.html
//    https://lists.nongnu.org/archive/html/lmi/2008-11/msg00008.html
// before attempting to "fix" this.

#ifndef et_vector_hpp
#define et_vector_hpp

#if 0
#include "config.hpp"
#endif // 0

#include "ssize_lmi.hpp"

#include "PETE/PETE.h"

#include <sstream>
#include <stdexcept>
#include <vector>

// Include "et_vector_operators.hpp" last because it's generated
// automatically and doesn't include all the headers it needs.

// gcc's '-Weffc++' flags user-defined boolean AND and OR operators
// because they cannot implement short-circuit evaluation. Although
// Meyers's "Never overload &&, ||, or ," is a generally sound rule
// for scalar classes, it doesn't make sense for containers, where
// short-circuit evaluation is not possible anyway. Presumably this
// warning would be issued for std::valarray::operator||() and &&(),
// except that they're standard. Similarly, it would issued here for
// PETE's || and && operators, except for the pragma.

#if defined __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Weffc++"
#endif // defined __GNUC__
#include "et_vector_operators.hpp"
#if defined __GNUC__
#   pragma GCC diagnostic pop
#endif // defined __GNUC__

/// Create vector-reference leaves.

template<typename T>
struct CreateLeaf<std::vector<T>>
{
    typedef Reference<std::vector<T>> Leaf_t;
    static Leaf_t make(std::vector<T> const& v) {return Leaf_t(v);}
};

/// Compare vector size with a stored value.

class SizeLeaf
{
  public:
    SizeLeaf(int s) : length_(s) {}
    SizeLeaf(SizeLeaf const& model) : length_(model.length_) {}
    bool operator()(int s) const {return length_ == s;}

  private:
    int length_;
};

template<typename T>
struct LeafFunctor<Scalar<T>, SizeLeaf>
{
    typedef bool Type_t;
    static bool apply(Scalar<T> const&, SizeLeaf const&)
    {
        return true; // Scalars conform to any vector's length.
    }
};

template<typename T>
struct LeafFunctor<std::vector<T>, SizeLeaf>
{
    typedef bool Type_t;
    static bool apply(std::vector<T> const& v, SizeLeaf const& s)
    {
        return s(lmi::ssize(v));
    }
};

template<typename T>
struct LeafFunctor<std::vector<T>, EvalLeaf1>
{
    typedef T Type_t;
    static Type_t apply(std::vector<T> const& vec, EvalLeaf1 const& f)
    {
        return vec[f.val1()];
    }
};

/// Return vector length (zero for scalars).

struct LengthLeaf {};

template<typename T>
struct LeafFunctor<std::vector<T>, LengthLeaf>
{
    typedef int Type_t;
    static Type_t apply(std::vector<T> const& v, LengthLeaf const&)
        {return lmi::ssize(v);}
};

template<typename T>
struct LeafFunctor<T, LengthLeaf>
{
    typedef int Type_t;
    static Type_t apply(T const&, LengthLeaf const&)
        {return 0;}
};

struct MaxCombine
{
    PETE_EMPTY_CONSTRUCTORS(MaxCombine)
};

template<typename Op>
struct Combine2<int, int, Op, MaxCombine>
{
    typedef int Type_t;
    static Type_t combine(int a, int b, MaxCombine)
    {
        if(a < b) return b; else return a;
    }
};

/// Like APL's monadic 'rho': return argument's length.

template<typename T>
inline int Rho(Expression<T> const& t)
{
    return forEach(t, LengthLeaf(), MaxCombine());
}

/// All PETE assignment operators call evaluate().

template<typename T, typename Op, typename U>
inline void evaluate(std::vector<T>& t, Op const& op, Expression<U> const& u)
{
    if(!forEach(u, SizeLeaf(lmi::ssize(t)), AndCombine()))
        {
        std::ostringstream oss;
        oss
            << "Nonconformable lengths: "
            << lmi::ssize(t) << " lhs vs. "
            << Rho(u) << " rhs."
            ;
        throw std::runtime_error(oss.str());
        }

    for(int i = 0; i < lmi::ssize(t); ++i)
        {
        op(t[i], forEach(u, EvalLeaf1(i), OpCombine()));
        }
}

template<typename U>
inline auto Eval(Expression<U> const& u)
{
    int const n {Rho(u)};
    using Deduced = decltype(forEach(u, EvalLeaf1(0), OpCombine()));
    std::vector<Deduced> z;
    z.reserve(n);
    for(int i = 0; i < n; ++i)
        {
        z.push_back(forEach(u, EvalLeaf1(i), OpCombine()));
        }
    return z;
}

#endif // et_vector_hpp
