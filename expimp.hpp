// Export-import macros for msw dlls.
//
// Copyright (C) 1998, 2001, 2005 Gregory W. Chicares.
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

// $Id: expimp.hpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifndef expimp_hpp
#define expimp_hpp

#include "config.hpp"

#ifdef LMI_MSW
#   ifdef LMI_BUILD_DLL
#       define LMI_EXPIMP __declspec(dllexport)
#   elif defined(LMI_USE_DLL)
#       define LMI_EXPIMP __declspec(dllimport)
#   else // neither build nor use dll
#       define LMI_EXPIMP
#   endif // neither build nor use dll
#else // LMI_MSW
#   define LMI_EXPIMP
#endif // LMI_MSW

#endif // expimp_hpp

