// PETE with std::vector: casual unit test--cf. 'et_vector_0_test.cpp'.
//
// Copyright (C) 2008, 2010, 2016, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "PETE/PETE.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <vector>

// Include "et_vector_operators.hpp" last because it's generated
// automatically and doesn't include all the headers it needs.

#include "et_vector_operators.hpp"

/// Create vector-reference leaves.

template<class T>
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

template<class T>
struct LeafFunctor<Scalar<T>, SizeLeaf>
{
    typedef bool Type_t;
    static bool apply(Scalar<T> const&, SizeLeaf const&)
    {
        return true; // Scalars conform to any vector's length.
    }
};

template<class T>
struct LeafFunctor<std::vector<T>, SizeLeaf>
{
    typedef bool Type_t;
    static bool apply(std::vector<T> const& v, SizeLeaf const& s)
    {
        return s(v.size());
    }
};

template<class T>
struct LeafFunctor<std::vector<T>, EvalLeaf1>
{
    typedef T Type_t;
    static Type_t apply(std::vector<T> const& vec, EvalLeaf1 const& f)
    {
        return vec[f.val1()];
    }
};

/// All PETE assignment operators call evaluate().

template<class T, class Op, class U>
inline void evaluate(std::vector<T>& t, Op const& op, Expression<U> const& u)
{
    if(!forEach(u, SizeLeaf(t.size()), AndCombine()))
        {
        throw std::runtime_error("Error: LHS and RHS don't conform.");
        }

    for(int i = 0; i < t.size(); ++i)
        {
        op(t[i], forEach(u, EvalLeaf1(i), OpCombine()));
        }
}

template <typename T>
void show_vector(std::vector<T> const& v)
{
    std::ostream_iterator<double> osi(std::cout, " ");
    std::copy(v.begin(), v.end(), osi);
    std::cout << std::endl;
}

int main()
{
    std::vector<double> v0;

    v0.push_back(0.0);
    v0.push_back(1.1);
    v0.push_back(2.2);

    v0 *= v0;

    show_vector(v0);

    // Test peteCast().
    std::vector<int> v1(v0.size());
    peteCast(int(0), v0);
    assign(v1, peteCast(int(0), v0));
    show_vector(v1);

    // Test std::unary_function.
    assign(v0, apply_unary(std::negate<double>(), v0));
    show_vector(v0);

    // Test std::binary_function.
    assign(v0, apply_binary(std::multiplies<double>(), -1.0, v0));
    show_vector(v0);
    assign(v0, sqrt(v0));
    show_vector(v0);
    assign(v0, apply_binary(std::multiplies<double>(), v0, -1.0));
    show_vector(v0);
    assign(v0, apply_binary(std::multiplies<double>(), v0, v0));
    show_vector(v0);
    assign(v0, apply_binary(std::plus<double>(), v0, 100.0));
    assign(v0, apply_binary(std::plus<double>(), 10000.0, v0));
    show_vector(v0);

    // Test Min() and Max().
    std::vector<double> v2 = {1.2, 2.3, 3.4, 7.7};
    std::vector<double> v3 = {1.9, 2.9, 3.9, 0.0};
    std::vector<double> v4(v2.size());
    assign(v4, Max(v2, v3));
    show_vector(v4);
    assign(v4, Min(v2, v3));
    show_vector(v4);

    std::cout << "Completed." << std::endl;
}
