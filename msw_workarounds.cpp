// Preload msw dlls to work around an operating-system defect.
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "msw_workarounds.hpp"

#if defined LMI_MSW

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "fenv_lmi.hpp"
#include "handle_exceptions.hpp"        // report_exception()

#include <windows.h>

#include <iostream>                     // cout, endl
#include <iterator>                     // istream_iterator
#include <sstream>

MswDllPreloader::MswDllPreloader()
{
}

MswDllPreloader::~MswDllPreloader()
{
    for(auto const& i : SuccessfullyPreloadedDlls_)
        {
        UnloadOneDll(i);
        }
}

MswDllPreloader& MswDllPreloader::instance()
{
    try
        {
        static MswDllPreloader z;
        return z;
        }
    catch(...)
        {
        report_exception();
        alarum() << "Instantiation failed." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void MswDllPreloader::PreloadDesignatedDlls()
{
    configurable_settings const& c = configurable_settings::instance();
    std::istringstream iss(c.libraries_to_preload());
    std::istream_iterator<std::string> i(iss);
    std::istream_iterator<std::string> const eos;
    for(; eos != i; ++i)
        {
        PreloadOneDll(*i);
        }
    fenv_initialize();
}

void MswDllPreloader::PreloadOneDll(std::string const& dll_name)
{
    std::cout << "Preloading '" << dll_name << "'." << std::endl;

    fenv_initialize();

    if(nullptr == ::LoadLibraryA(dll_name.c_str()))
        {
        warning() << "Failed to preload '" << dll_name << "'." << LMI_FLUSH;
        }
    else
        {
        SuccessfullyPreloadedDlls_.push_front(dll_name);
        if(fenv_is_valid())
            {
            warning()
                << "Preloading '"
                << dll_name
                << "' had no effect on the floating-point control word."
                << " You can safely remove it from 'libraries_to_preload'"
                << " in 'configurable_settings.xml'."
                << LMI_FLUSH
                ;
            }
        }
}

void MswDllPreloader::UnloadOneDll(std::string const& dll_name)
{
    std::cout << "Unloading '" << dll_name << "'." << std::endl;

    if(0 == ::FreeLibrary(::GetModuleHandleA(dll_name.c_str())))
        {
        warning() << "Failed to unload '" << dll_name << "'." << LMI_FLUSH;
        }
}

#endif // defined LMI_MSW
