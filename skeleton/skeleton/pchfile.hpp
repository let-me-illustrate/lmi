// Precompiled header file.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: pchfile.hpp,v 1.1.1.1 2004-05-15 19:59:12 chicares Exp $

// Always include this header first in every '.cpp' file, before
// anything else except comments and whitespace. Never include it in
// any header file. Include any headers to be precompiled here.

#ifndef pchfile_hpp
#define pchfile_hpp

#include "config.hpp"

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH
#   include <wxprec.h>
#endif

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#endif // pchfile_hpp

