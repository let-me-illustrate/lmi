// Exception-throwing macro for lightweight assertions.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef assert_lmi_hpp
#define assert_lmi_hpp

#include "config.hpp"

#include <cstring>
#include <sstream>
#include <stdexcept>

/// This replacement for the standard 'assert' macro throws an
/// exception on failure instead of writing to stdout and calling
/// std::abort(), and does not depend on whether NDEBUG is defined.
///
/// Rationale: An exception can be caught and handled upstream; that
/// is preferable to abnormal termination. Dependence on NDEBUG leads
/// all too easily to ODR violations and bifurcation of behavior; this
/// macro is appropriate only for lightweight assertions that should
/// be left in released code.
///
/// The logic could more plainly be expressed as
///   if(!(condition)) {throw...}
/// but is instead written as
///   if(condition) {} else {throw...}
/// which, by avoiding an extra level of parentheses, helps compilers
/// give better diagnostics--see:
///   https://lists.nongnu.org/archive/html/lmi/2019-06/msg00010.html
///
/// The last line eats a semicolon written after the macro invocation.
///
/// For a more-elaborate runtime-error facility, see 'alert*.?pp'.

#define LMI_ASSERT(condition)                                   \
    do                                                          \
        {                                                       \
        if(condition)                                           \
            {                                                   \
            }                                                   \
        else                                                    \
            {                                                   \
            std::ostringstream assert_message;                  \
            assert_message                                      \
                << "Assertion '" << (#condition) << "' failed." \
                << "\n[" << 1 + std::strrchr("/" __FILE__, '/') \
                << " : " << __LINE__ << "]\n"                   \
                ;                                               \
            throw std::runtime_error(assert_message.str());     \
            }                                                   \
        }                                                       \
    while(0)

/// This LMI_ASSERT variant displays an extra message on failure.
///
/// The 'message' parameter is deliberately not token-pasted, so
/// that it can include streaming operators, e.g.:
///   "value is " << value

#define LMI_ASSERT_WITH_MSG(condition,message)                  \
    do                                                          \
        {                                                       \
        if(condition)                                           \
            {                                                   \
            }                                                   \
        else                                                    \
            {                                                   \
            std::ostringstream assert_message;                  \
            assert_message                                      \
                << "Assertion '" << (#condition) << "' failed"  \
                << "\n(" << message << ")."                     \
                << "\n[" << 1 + std::strrchr("/" __FILE__, '/') \
                << " : " << __LINE__ << "]\n"                   \
                ;                                               \
            throw std::runtime_error(assert_message.str());     \
            }                                                   \
        }                                                       \
    while(0)

/// This LMI_ASSERT variant displays both its parameters if unequal.

#define LMI_ASSERT_EQUAL(observed,expected)                         \
    LMI_ASSERT_WITH_MSG                                             \
        ((observed) == (expected)                                   \
        ,"expected " << (expected) << " vs observed " << (observed) \
        )

#endif // assert_lmi_hpp
