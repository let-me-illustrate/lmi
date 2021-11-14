// Stream and stream buffer that discard output--unit test.
//
// Copyright (C) 2021 Gregory W. Chicares.
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

#include "null_stream.hpp"

#include "miscellany.hpp"               // ios_out_app_binary()
#include "test_tools.hpp"
#include "timer.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

/// Reference to a static null stream--not for general use.
///
/// This was originally a public facility, intended to be used as a
/// default ostream& argument:
///   void foo(std::ostream& os = null_stream());
/// for functions that never change the streambuf. Now it is
/// sequestered in this unit-test TU, where it serves only to show
/// what could go wrong if it were used elsewhere. The essential
/// problem is that the static object is in effect a global variable.
/// Replacing its streambuf by calling rdbuf(another_streambuf)
/// therefore has a global effect that is probably unwanted.
/// Prefer to create a local object instead, e.g.:
///   std::ostream local_os(&null_streambuf());
///   local_os.setstate(std::ios::badbit);
///   local_os << "written to oblivion";
///   local_os.rdbuf(std::cout.rdbuf); // effect is only local
///   local_os << "written to stdout";

std::ostream& null_stream()
{
    static std::ostream z(&null_streambuf());
    z.setstate(std::ios::badbit);
    return z;
}

void emit_text_to_stream(std::ostream& os)
{
    for(int i = 0; i < 10; ++i)
        {
        os
            << "The Beaver had counted with scrupulous care,\n"
            << "  Attending to every word:\n"
            << "But it fairly lost heart, and outgrabe in despair,\n"
            << "  When the third repetition occurred.\n"
            << std::flush
            ;
        os
            << std::setprecision(21)
            << std::setw(12) << 3.14159
            << std::fixed
            << std::hex
            << std::hexfloat
            << std::setw(12) << 3.14159
            << std::endl
            ;
        }
}

/// Test writing to '/dev/null'.
///
/// Not every OS has '/dev/null', but for msw, using 'NUL' doesn't
/// make this any faster.

void mete_dev_null()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ofstream ofs("/dev/null", ios_out_app_binary());
        emit_text_to_stream(ofs);
        }
}

// Test writing to null stream derived from Dietmar Kühl's work.

void mete_kuehl()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ostream os(&null_streambuf());
        emit_text_to_stream(os);
        }
}

// Test writing to null stream with 'badbit' set.

void mete_badbit_kuehl()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ostream os(&null_streambuf());
        os.setstate(std::ios_base::badbit);
        emit_text_to_stream(os);
        }
}

// Test writing to static null stream.

void mete_kuehl_static()
{
    static std::ostream z(&null_streambuf());
    z.setstate(std::ios::badbit);
    for(int i = 0; i < 1e4; ++i)
        {
        emit_text_to_stream(z);
        }
}

// Test writing to local std::ostream(nullptr).

void mete_local_ostream_0()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ostream os(nullptr);
        emit_text_to_stream(os);
        }
}

// Test writing to "global" std::ostream(nullptr).

void mete_global_ostream_0()
{
    std::ostream os(nullptr); // as if global
    for(int i = 0; i < 1e4; ++i)
        {
        emit_text_to_stream(os);
        }
}

/// Test writing to an unopened ofstream.

void mete_unopened_fstream()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ofstream ofs;
        emit_text_to_stream(ofs);
        }
}

/// Test writing to an ofstream with 'badbit' set.

void mete_badbit_fstream()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ofstream ofs;
        ofs.setstate(std::ios_base::badbit);
        emit_text_to_stream(ofs);
        }
}

/// Test writing to an ofstream with 'badbit' set.

void mete_badbit_sstream()
{
    for(int i = 0; i < 1e4; ++i)
        {
        std::ostringstream oss;
        oss.setstate(std::ios_base::badbit);
        emit_text_to_stream(oss);
        }
}

void test_fundamentals()
{
    // This shouldn't appear anywhere. Of course, there's no way to
    // verify its absence everywhere.
    std::ostream& os0 = null_stream();
    os0
        << "'But oh, beamish nephew, beware of the day,\n"
        << "  If your Snark be a Boojum! For then\n"
        << "You will softly and suddenly vanish away,\n"
        << "  and never be met with again!'\n"
        << std::flush
        ;

    // This alternative explicitly constructs a std::ostream each time
    // it's used, which is costlier but avoids the peril shown below.
    std::ostream os1(&null_streambuf());
    os1
        << "But if ever I meet with a Boojum, that day,\n"
        << "  In a moment (of this I am sure),\n"
        << "I shall softly and suddenly vanish away--\n"
        << "  And the notion I cannot endure!\n"
        << std::flush
        ;

    // LMI_TEST_[UN]EQUAL macros insert arguments into output stream
    // upon failure, so cast pointer arguments to void* where default
    // insertion as char* could have astonishing effects.
    void* p = static_cast<void*>(&null_streambuf());

    // Demonstrate a peril.
    {
    LMI_TEST_EQUAL(p, static_cast<void*>(os0.rdbuf()));
    LMI_TEST_EQUAL(p, static_cast<void*>(os1.rdbuf()));
    // First stream: substitute a non-discarding streambuf.
    std::ostringstream oss0;
    os0.rdbuf(oss0.rdbuf());
    os0 << "This text is not to be discarded." << std::endl;
    // Second stream: likewise.
    std::ostringstream oss1;
    os1.rdbuf(oss1.rdbuf());
    os1 << "Neither is this." << std::endl;
    LMI_TEST_UNEQUAL(p, static_cast<void*>(os0.rdbuf()));
    LMI_TEST_UNEQUAL(p, static_cast<void*>(os1.rdbuf()));
    }

    // Now rdbuf() would return a dangling pointer for each stream.
    LMI_TEST_UNEQUAL(p, static_cast<void*>(os0.rdbuf()));
    LMI_TEST_UNEQUAL(p, static_cast<void*>(os1.rdbuf()));

    // This would be okay:
    std::ostream new_os1(&null_streambuf());
    LMI_TEST_EQUAL(p, static_cast<void*>(new_os1.rdbuf()));
    // But this would not...
    std::ostream& new_os0 = null_stream();
    // ...as demonstrated here:
    LMI_TEST_UNEQUAL(p, static_cast<void*>(new_os0.rdbuf()));

    // This would segfault:
//  os0 << "But while he was seeking with thimbles and care" << std::endl;
    // As would this:
//  os1 << "A Bandersnatch swiftly drew nigh" << std::endl;
    // These do not segfault, but remembering to reset the
    // streambuf manually requires too much smiles and soap...
    os0.rdbuf(&null_streambuf());
    os0 << "Segfault avoided with thimbles and care." << std::endl;
    os1.rdbuf(&null_streambuf());
    os1 << "Segfault avoided with forks and hope." << std::endl;
    // ...so prefer to create a local std::ostream instead.
}

void assay_speed()
{
    std::cout
        << "\n  Speed tests..."
        << "\n  /dev/null        : " << TimeAnAliquot(mete_dev_null)
        << "\n  Kühl             : " << TimeAnAliquot(mete_kuehl)
        << "\n  Kühl, badbit     : " << TimeAnAliquot(mete_badbit_kuehl)
        << "\n  Kühl, static     : " << TimeAnAliquot(mete_kuehl_static)
        << "\n  global ostream(0): " << TimeAnAliquot(mete_global_ostream_0)
        << "\n  local ostream(0) : " << TimeAnAliquot(mete_local_ostream_0)
        << "\n  unopened fstream : " << TimeAnAliquot(mete_unopened_fstream)
        << "\n  fstream, badbit  : " << TimeAnAliquot(mete_badbit_fstream)
        << "\n  sstream, badbit  : " << TimeAnAliquot(mete_badbit_sstream)
        << std::endl
        ;
}

int test_main(int, char*[])
{
    test_fundamentals();
    assay_speed();

    return 0;
}
