// Work around 'wine' defects.
//
// Copyright (C) 2021 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "wine_workarounds.hpp"

#if defined LMI_MSW

#include <windows.h>

bool running_under_wine()
{
    HMODULE module = ::GetModuleHandleA("ntdll.dll");
    return module && GetProcAddress(module, "wine_get_version");
}

#else  // !defined LMI_MSW

bool running_under_wine() {return false;}

#endif // !defined LMI_MSW
