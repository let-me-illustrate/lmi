// Manage floating-point environment.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: fenv_lmi.hpp,v 1.5 2005-12-22 13:59:49 chicares Exp $

// Manage the floating-point environment, using C99 7.6 facilities
// where available. Otherwise, use compiler- and platform-specific
// techniques where they are known: only intel hardware is fully
// supported for now.

#ifndef fenv_lmi_hpp
#define fenv_lmi_hpp

#include "config.hpp"

#include "so_attributes.hpp"

// Initialize floating-point environment to reasonable settings.
extern "C" void LMI_SO initialize_fpu();

// Make sure current floating-point environment matches initial
// settings; throw an exception if it doesn't.
extern "C" void LMI_SO validate_fenv();

#endif // fenv_lmi_hpp

