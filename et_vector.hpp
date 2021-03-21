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

#ifndef et_vector_hpp
#define et_vector_hpp

#if 0
#include "config.hpp"
#endif // 0

#include "et_vector_redirect.hpp"
#include "ssize_lmi.hpp"

#include <sstream>
#include <stdexcept>
#include <vector>

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

/// A (compound) assignment operator.
///
/// It is forbidden to intrude a copy-assignment operator into the
/// std::vector template, but compound assignment operators need not
/// be members.
///
/// Rationale for choosing operator<<=():
///  - std::vector::operator<<=() is hardly ever used;
///  - it's so rare that first-time users will realize they should
///    look for the present documentation;
///  - it's an assignment operator, with very low precedence; and
///  - '<<' is reminiscent of stream inserters, which transfer values
///    from one place to another; but '=' clearly indicates that this
///    isn't a stream operation.

template<typename T, typename U>
inline std::vector<T>& operator<<=(std::vector<T>& t, Expression<U> const& u)
{
    return t = Eval(u);
}

#endif // et_vector_hpp
