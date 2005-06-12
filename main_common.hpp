// Startup code common to all interfaces.
//
// Copyright (C) 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: main_common.hpp,v 1.2 2005-06-12 16:58:36 chicares Exp $

#ifndef main_common_hpp
#define main_common_hpp

#include "config.hpp"

// Included so that try_main() definitions can use EXIT_SUCCESS, EXIT_FAILURE.
#include <cstdlib>

void initialize_application();
int try_main(int argc, char* argv[]);

#endif // main_common_hpp

