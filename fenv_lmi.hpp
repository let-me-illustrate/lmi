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

// $Id: fenv_lmi.hpp,v 1.6 2005-12-27 15:36:50 chicares Exp $

// Manage the floating-point environment, using C99 7.6 facilities
// where available. Otherwise, use compiler- and platform-specific
// techniques where they are known: only intel hardware is fully
// supported for now.

#ifndef fenv_lmi_hpp
#define fenv_lmi_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#ifdef LMI_X86
#   include "fenv_lmi_x86.hpp"
#else  // Unknown compiler or platform.
#   error Unknown compiler or platform. Please contribute an implementation.
#endif // Unknown compiler or platform.

#if defined __STDC_IEC_559__ || defined __MINGW32__
// In case the C++ compiler supports C99 7.6 facilities, assume that
// it defines __STDC_IEC_559__ (except that MinGW supports some such
// facilities but defines no such macro), and puts prototypes in
// <fenv.h> but not in namespace std.
#   include <fenv.h>
#   if defined __GNUC__ && LMI_GCC_VERSION <= 40100
// As of 2005-04-08, the gcc manual here:
//   http://gcc.gnu.org/onlinedocs/gcc/Floating-point-implementation.html
// which "corresponds to GCC version 4.1.0" says "This pragma is not
// implemented".
#   else  // Pragma STDC FENV_ACCESS implemented.
#       pragma STDC FENV_ACCESS ON
#   endif // Pragma STDC FENV_ACCESS implemented.
#endif // defined __STDC_IEC_559__ || defined __MINGW32__

// Initialize floating-point environment to lmi default settings.
void LMI_SO initialize_fpu();

// Make sure current floating-point environment matches initial
// settings; display a message and return 'false' if it doesn't.
bool LMI_SO validate_fenv();

#endif // fenv_lmi_hpp

