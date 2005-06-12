// Initialize boost filesystem library.
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

// $Id: initialize_filesystem.hpp,v 1.1 2005-06-12 16:58:36 chicares Exp $

#ifndef initialize_filesystem_hpp
#define initialize_filesystem_hpp

#include "config.hpp"

#include <boost/filesystem/path.hpp>

/// Applications that end users would normally run should call this
/// function during initialization--before using this boost library
/// in any other way, to ensure uniform name checking. This enables
/// them to use nonportable paths, as some demand.
///
/// This function is not called in the initialization routine used by
/// all applications, because simple command-line tools should not be
/// forced to depend on this boost library.

inline void initialize_filesystem()
{
    fs::path::default_name_check(fs::native);
}

#endif // initialize_filesystem_hpp

