// A custom input interface.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: custom_io_0.hpp,v 1.1 2005-06-11 15:31:37 chicares Exp $

#ifndef custom_io_0_hpp
#define custom_io_0_hpp

#include "config.hpp"

#include "expimp.hpp"

class IllusInputParms;

bool LMI_EXPIMP DoesSpecialInputFileExist();
bool LMI_EXPIMP GetSpecialAutocloseFlag();
void LMI_EXPIMP SetSpecialInput
    (IllusInputParms& ip
    ,char const*      overridden_filename = 0
    );

#endif  // custom_io_0_hpp

