// Exception-throwing macro for lightweight assertions.
//
// Copyright (C) 2006, 2007, 2008, 2009 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: assert_lmi.hpp,v 1.4 2008-12-27 02:56:36 chicares Exp $

#ifndef assert_lmi_hpp
#define assert_lmi_hpp

#include "config.hpp"

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
/// The last line eats a semicolon written after the macro invocation.
///
/// For a more-elaborate runtime-error facility, see 'alert*.?pp'.

#define LMI_ASSERT(condition)                               \
    if(!(condition))                                        \
        {                                                   \
        std::ostringstream oss;                             \
        oss                                                 \
            << "Assertion '" << (#condition) << "' failed." \
            << "\n[file " << __FILE__                       \
            << ", line " << __LINE__ << "]\n"               \
            ;                                               \
        throw std::runtime_error(oss.str());                \
        }                                                   \
    do {} while(0)

#endif // assert_lmi_hpp

