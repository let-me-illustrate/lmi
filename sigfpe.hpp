// Life insurance illustrations.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: sigfpe.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef sigfpe_hpp
#define sigfpe_hpp

#include "config.hpp"

#include "expimp.hpp"

void LMI_EXPIMP floating_point_error_handler(int);

// TODO ?? Make the unit test a separate module.
void LMI_EXPIMP fpe_handler_test();

#endif // sigfpe_hpp

