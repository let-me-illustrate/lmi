// Preload msw dlls to work around an operating-system defect.
//
// Copyright (C) 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: msw_workarounds.cpp,v 1.4 2006-11-02 19:19:07 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "msw_workarounds.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "fenv_lmi.hpp"
#include "handle_exceptions.hpp"

#include <boost/functional.hpp>

#ifdef LMI_MSW
#   include <windows.h>
#endif // LMI_MSW defined.

#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdexcept>

MswDllPreloader::MswDllPreloader()
{
}

MswDllPreloader::~MswDllPreloader()
{
    std::for_each
        (SuccessfullyPreloadedDlls_.begin()
        ,SuccessfullyPreloadedDlls_.end()
        ,boost::bind1st(std::mem_fun(&MswDllPreloader::UnloadOneDll), this)
        );
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
        fatal_error() << "Instantiation failed." << LMI_FLUSH;
        throw std::logic_error("Unreachable"); // Silence compiler warning.
        }
}

void MswDllPreloader::PreloadDesignatedDlls()
{
    std::istringstream iss
        (configurable_settings::instance().libraries_to_preload()
        );
    std::for_each
        (std::istream_iterator<std::string>(iss)
        ,std::istream_iterator<std::string>()
        ,boost::bind1st(std::mem_fun(&MswDllPreloader::PreloadOneDll), this)
        );
    fenv_initialize();
}

void MswDllPreloader::PreloadOneDll(std::string const& dll_name)
{
#ifdef LMI_MSW
    fenv_initialize();

    if(0 == ::LoadLibrary(dll_name.c_str()))
        {
        std::ostringstream oss;
        oss << "Failed to preload '" << dll_name << "'.";
        safely_show_message(oss.str().c_str());
        }
    else
        {
        SuccessfullyPreloadedDlls_.push_front(dll_name);
        if(fenv_is_valid())
            {
            std::ostringstream oss;
            oss
                << "Preloading '"
                << dll_name
                << "' had no effect on the floating-point control word."
                << " You can safely remove it from 'libraries_to_preload'"
                << " in 'configurable_settings.xml'."
                ;
            safely_show_message(oss.str().c_str());
            }
        }
#endif // LMI_MSW defined.
}

void MswDllPreloader::UnloadOneDll(std::string const& dll_name)
{
#ifdef LMI_MSW
    if(0 == ::FreeLibrary(::GetModuleHandle(dll_name.c_str())))
        {
        std::ostringstream oss;
        oss << "Failed to unload '" << dll_name << "'.";
        safely_show_message(oss.str().c_str());
        }
#endif // LMI_MSW defined.
}

