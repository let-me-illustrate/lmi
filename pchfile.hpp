// Precompiled header file.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

// Always include this header first in every '.cpp' file, before
// anything else except comments and whitespace. Never include it in
// any header file. Include any headers to be precompiled here.

#ifndef pchfile_hpp
#define pchfile_hpp

#include "config.hpp"

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH
// This is merely a casual guess, not supported by any measurement of
// its effect on performance. The optimal set of headers to precompile
// probably varies by compiler. Including wx headers might impair
// performance for non-wx targets.
#   include <wx/wxprec.h>
#endif

#endif // pchfile_hpp

