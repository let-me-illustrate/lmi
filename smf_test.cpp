// Special member functions (SMFs) and move semantics--unit test.
//
// Copyright (C) 2022, 2023 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "smf.hpp"

#include "test_tools.hpp"

#include <type_traits>
#include <utility>                      // move()

class smf_test
{
  public:
    static void test()
        {
        test_classes();
        }

  private:
    static void test_classes();
};

struct can_move
{
};

can_move moveable_instance;

struct no_can_move
{
    no_can_move()                              = default;
    ~no_can_move()                             = default;
    no_can_move(no_can_move const&)            = default;
    no_can_move(no_can_move&&)                 = delete;
    no_can_move& operator=(no_can_move const&) = default;
    no_can_move& operator=(no_can_move&&)      = delete;
};

// Classes for unit testing.
//
// See the smf_test::test_classes() documentation for a key to
// class names.

/// This struct is an aggregate.
///
/// It is important to test an aggregate because this:
///   A1111 a1111 {smf_mechanics::ambiguator<A1111>{}};
/// is not ambiguous; instead, it has too many initializers. See:
///   https://lists.nongnu.org/archive/html/lmi/2022-08/msg00005.html

struct A1111
{
    smf_mechanics::sensor s;
};
static_assert(std::is_aggregate_v<A1111>);

struct C0000
{
    C0000()                        = default;
    ~C0000()                       = default;
    C0000(C0000 const&)            = default;
    C0000(C0000&&)                 = default;
    C0000& operator=(C0000 const&) = default;
    C0000& operator=(C0000&&)      = default;
    smf_mechanics::sensor s;
};

struct C0101
{
    C0101()                        = default;
    ~C0101()                       = default;
    C0101(C0101 const&)            = default;
//  C0101(C0101&&)                 // not declared
    C0101& operator=(C0101 const&) = default;
//  C0101& operator=(C0101&&)      // not declared
    smf_mechanics::sensor s;
};

struct C0202
{
    C0202()                        = default;
    ~C0202()                       = default;
    C0202(C0202 const&)            = default;
    C0202& operator=(C0202 const&) = default;
    smf_mechanics::sensor s;
  protected:
    C0202(C0202&&)                 = default;
    C0202& operator=(C0202&&)      = default;
};

struct C0303
{
    C0303()                        = default;
    ~C0303()                       = default;
    C0303(C0303 const&)            = default;
    C0303(C0303&&)                 = delete;
    C0303& operator=(C0303 const&) = default;
    C0303& operator=(C0303&&)      = delete;
    smf_mechanics::sensor s;
};

#if defined LMI_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdefaulted-function-deleted"
#endif // defined LMI_CLANG
struct C0404 : public no_can_move
{
    C0404()                        = default;
    ~C0404()                       = default;
    C0404(C0404 const&)            = default;
    C0404(C0404&&)                 = default; // implicitly deleted
    C0404& operator=(C0404 const&) = default;
    C0404& operator=(C0404&&)      = default; // implicitly deleted
    smf_mechanics::sensor s;
};

struct C0505
{
    C0505()                        = default;
    ~C0505()                       = default;
    C0505(C0505 const&)            = default;
    C0505(C0505&&)                 = default;
    C0505& operator=(C0505 const&) = default; // implicitly deleted
    C0505& operator=(C0505&&)      = default; // implicitly deleted
    smf_mechanics::sensor s;
    // reference or const member:
    //  - allows copy and move construction
    //  - inhibits copy and move assignment
    can_move& cm_ {moveable_instance};
    int const i_  {};
};
#if defined LMI_CLANG
#   pragma clang diagnostic pop
#endif // defined LMI_CLANG

struct C3030
{
    C3030()                        = default;
    ~C3030()                       = default;
    C3030(C3030 const&)            = delete;
    C3030(C3030&&)                 = default;
    C3030& operator=(C3030 const&) = delete;
    C3030& operator=(C3030&&)      = default;
    smf_mechanics::sensor s;
};

template<typename T>
constexpr bool was_move_constructed_as_expected(T const& t)
{
    return
        well_move_constructible<T>
        ? smf_mechanics::move_constructed == t.s.p()
        : smf_mechanics::copy_constructed == t.s.p()
        ;
}

template<typename T>
constexpr bool was_move_assigned_as_expected(T const& t)
{
    return
        well_move_assignable<T>
        ? smf_mechanics::move_assigned == t.s.p()
        : smf_mechanics::copy_assigned == t.s.p()
        ;
}

/// Statically tested properties, encoded as bits in an 'int'.
///
/// There is no great advantage to representing this as a std::bitset.
///
/// See the smf_test::test_classes() documentation for a key to
/// the two-letter codes (such as "Cc") used in comments.

template<typename T>
constexpr int complexion()
{
    using namespace smf_mechanics;
    return
          128 * std::is_copy_constructible_v<T> // Cc
        +  64 * std::is_move_constructible_v<T> // Mc
        +  32 * equiplausibly_constructible <T> // Ec
        +  16 * well_move_constructible     <T> // Wc
        +   8 * std::is_copy_assignable_v   <T> // Ca
        +   4 * std::is_move_assignable_v   <T> // Ma
        +   2 * equiplausibly_assignable    <T> // Ea
        +   1 * well_move_assignable        <T> // Wa
        ;
}

/// Test various properties of unit-testing classes.
///
/// Class names are generally one capital letter and four digits,
/// signifying {cp_ctor, mv_ctor, cp_assign, mv_assign}:
///   0 = user declared as defaulted
///   1 = not user declared
///   2 = explicitly defaulted, but inaccessible
///   3 = explicitly deleted
///   4 = implicitly deleted [due to base]
///   5 = unassignable [due to reference or const member]
///
/// Statically tested properties:
///   Cc: is_copy_constructible
///   Mc: is_move_constructible
///   Ec: equiplausibly_constructible
///   Wc: well_move_constructible
///   Ca: is_copy_assignable
///   Ma: is_move_assignable
///   Ea: equiplausibly_assignable
///   Wa: well_move_assignable
/// Compile-time properties:
///   e: move construct compiles without error
///   f: move assign compiles without error
///   g: move construct has move semantics
///   h: move assign has move semantics
///
/// True by definition:
///   Wc ≡ Mc ∧ Ec
///   Wa ≡ Ma ∧ Ea
/// Hypotheses:
///   e ?≡ Mc
///   f ?≡ Ma
///   Ec ?≡ Ea [need example to falsify this]
///   g ?≡ Wc
///   h ?≡ Wa

void smf_test::test_classes()
{
    using namespace smf_mechanics;

    // Tabulate and assert statically tested properties.
    //
    // It might also be interesting to explore such properties as:
    //   std::copy_constructible
    //   std::move_constructible
    //   std::copyable
    //   std::movable
    //
    //              ╭────────Cc: is_copy_constructible
    //              │╭───────Mc: is_move_constructible
    //              ││╭──────Ec: equiplausibly_constructible
    //              │││╭─────Wc: move construct has move semantics
    //              ││││╭────Ca: is_copy_assignable
    //              │││││╭───Ma: is_move_assignable
    //              ││││││╭──Ea: equiplausibly_assignable
    //              │││││││╭─Wa: move assign has move semantics
    //              ││││││││
    //              CMEWCMEW Copy, Move, Equiplausibly, Well
    //              ccccaaaa Constructible, Assignable
    //              --------
    static_assert(0b11111111 == complexion<A1111>());
    static_assert(0b11111111 == complexion<C0000>());
    static_assert(0b11001100 == complexion<C0101>());
    static_assert(0b10101010 == complexion<C0202>());
    static_assert(0b10101010 == complexion<C0303>());
    static_assert(0b11001100 == complexion<C0404>());
    static_assert(0b11110010 == complexion<C0505>());
    static_assert(0b01110111 == complexion<C3030>());

    // Instantiate unit-testing classes, and attempt to move them.
    //
    // Copying could of course be tested as well as moving.

    A1111 t_A1111 {};
    C0000 t_C0000 {};
    C0101 t_C0101 {};
    C0202 t_C0202 {};
    C0303 t_C0303 {};
    C0404 t_C0404 {};
    C0505 t_C0505 {};
    C3030 t_C3030 {};

    // Test hypothesis: e ?≡ Mc
    A1111 u_A1111 {std::move(t_A1111)};
    C0000 u_C0000 {std::move(t_C0000)};
    C0101 u_C0101 {std::move(t_C0101)};
//  C0202 u_C0202 {std::move(t_C0202)}; // !e: protected
//  C0303 u_C0303 {std::move(t_C0303)}; // !e: deleted
    C0404 u_C0404 {std::move(t_C0404)};
    C0505 u_C0505 {std::move(t_C0505)};
    C3030 u_C3030 {std::move(t_C3030)};

    // Test hypothesis: g ?≡ Wc
    LMI_TEST(was_move_constructed_as_expected(u_A1111));
    LMI_TEST(was_move_constructed_as_expected(u_C0000));
    LMI_TEST(was_move_constructed_as_expected(u_C0101));
//  LMI_TEST(was_move_constructed_as_expected(u_C0202));
//  LMI_TEST(was_move_constructed_as_expected(u_C0303));
    LMI_TEST(was_move_constructed_as_expected(u_C0404));
    LMI_TEST(was_move_constructed_as_expected(u_C0505));
    LMI_TEST(was_move_constructed_as_expected(u_C3030));

    // Test hypothesis: f ?≡ Ma
    A1111 v_A1111; v_A1111 = std::move(t_A1111);
    C0000 v_C0000; v_C0000 = std::move(t_C0000);
    C0101 v_C0101; v_C0101 = std::move(t_C0101);
//  C0202 v_C0202; v_C0202 = std::move(t_C0202); // !f: protected
//  C0303 v_C0303; v_C0303 = std::move(t_C0303); // !f: deleted
    C0404 v_C0404; v_C0404 = std::move(t_C0404);
//  C0505 v_C0505; v_C0505 = std::move(t_C0505); // !f: implicitly deleted
    C3030 v_C3030; v_C3030 = std::move(t_C3030);

    // Test hypothesis: h ?≡ Wa
    LMI_TEST(was_move_assigned_as_expected(v_A1111));
    LMI_TEST(was_move_assigned_as_expected(v_C0000));
    LMI_TEST(was_move_assigned_as_expected(v_C0101));
//  LMI_TEST(was_move_assigned_as_expected(v_C0202));
//  LMI_TEST(was_move_assigned_as_expected(v_C0303));
    LMI_TEST(was_move_assigned_as_expected(v_C0404));
//  LMI_TEST(was_move_assigned_as_expected(v_C0505));
    LMI_TEST(was_move_assigned_as_expected(v_C3030));

    stifle_unused_warning(t_C0202);
    stifle_unused_warning(t_C0303);

    stifle_unused_warning(u_A1111);
    stifle_unused_warning(u_C0000);
    stifle_unused_warning(u_C0101);
//  stifle_unused_warning(u_C0202);
//  stifle_unused_warning(u_C0303);
    stifle_unused_warning(u_C0404);
    stifle_unused_warning(u_C0505);
    stifle_unused_warning(u_C3030);
}

int test_main(int, char*[])
{
    smf_test::test();

    return EXIT_SUCCESS;
}
