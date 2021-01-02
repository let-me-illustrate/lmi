// Death benefit option (DBO) rules.
//
// Copyright (C) 2019, 2020, 2021 Gregory W. Chicares.
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

#include "dbo_rules.hpp"

#include "alert.hpp"
#include "mc_enum.hpp"
#include "ssize_lmi.hpp"

#include <array>

namespace
{
/// Permitted transitions among death benefit options.
///
/// Rows: "from" state; columns: "to" state.
///
/// The "zero" state is a special "from"-only state that is not part
/// of the mcenum_dbopt enumeration. It represents an empty state
/// before the policy comes into existence. Thus, the "zero" row
/// specifies the states available upon policy issue. In this example,
/// "ROP" is entirely forbidden.
///
/// It's simplest to view this as a partitioned matrix (below), the
/// "zero" row being a special case. The square submatrix's main
/// diagonal could just as well be all 1's: "ROP" --> "ROP" is a
/// transition from a forbidden state to the same state, so it can
/// never arise anyway. But it's preferable to forbid whatever is
/// never permitted, even if it is impossible.
///
/// This (augmented) matrix implies this graph:
///
///          +---+
///   +----> |MDB| <--+--+
///   |      +---+    ^  ^
///   |        ^      |  |
///   |        |      |  |
/// +---+    +---+ >--+  |
/// | 0 | -> | A |       |
/// +---+    +---+ <--+  |
///   |        ^      ^  |
///   |        |      |  |
///   |      +---+ >--+  |
///   +----> | B |       |
///          +---+ >-----+
///
/// C++ WG21 defect 1270 suggests eliding the outermost superfluous
/// braces, but g++ 8.x doesn't implement that. The innermost sets
/// of superfluous braces are written deliberately so that the
/// initializer cannot match
///   std::array<std::array<bool,5>,4>
/// whose indices look like the correct ones in the declaration
///   bool M1[5][4]
/// but actually need to be specified in a different order.

std::array<std::array<bool,4>,5> M1 =
    // A   B  ROP MDB
    {{{1,  1,  0,  1,}, // "zero" state before issue
    // -------------- // partitioned: square matrix follows...
      {1,  0,  0,  1,}, // A
      {1,  1,  0,  1,}, // B
      {0,  0,  0,  0,}, // ROP
      {0,  0,  0,  1,}, // MDB
     }};
} // Unnamed namespace.

bool dbo_at_issue_is_allowed(mce_dbopt z)
{
    return M1[0][z.ordinal()];
}

bool dbo_transition_is_allowed(mce_dbopt from, mce_dbopt to)
{
    return M1[1 + from.ordinal()][to.ordinal()];
}

// The return value is fairly useless, and may later be removed.

bool dbo_sequence_is_allowed(std::vector<mce_dbopt> v)
{
    if(0 == lmi::ssize(v))
        {
        alarum() << "DBO must not be empty." << LMI_FLUSH;
        return false;
        }
    if(!dbo_at_issue_is_allowed(v[0]))
        {
        alarum() << "Forbidden initial DBO '" << v[0] << "'." << LMI_FLUSH;
        return false;
        }
    for(int j = 1; j < lmi::ssize(v); ++j)
        if(!dbo_transition_is_allowed(v[j - 1], v[j]))
            {
            alarum()
                << "Forbidden DBO change from '" << v[j - 1]
                << "' to '" << v[j]
                << "' after " << j << " years."
                << LMI_FLUSH
                ;
            return false;
            }
    return true;
}
