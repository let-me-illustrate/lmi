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

#ifndef msw_workarounds_hpp
#define msw_workarounds_hpp

#include "config.hpp"

#if defined LMI_MSW

#include <deque>
#include <string>

/// Design notes for class MswDllPreloader.
///
/// This class preloads msw dlls to work around an operating-system
/// defect: some system libraries needlessly mangle the floating-point
/// control word. For instance, 'DOCPROP2.DLL' is a known offender
/// for msw '2000'.
///
/// It is implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.
///
/// Preloading might have been performed in the ctor; performing it in
/// member PreloadDesignatedDlls() allows the timing to be controlled.
///
/// The dtor unloads preloaded dlls in the reverse of the order in
/// which they were loaded. The data member is of type std::deque
/// in order to let this reversal be expressed more cleanly.

class MswDllPreloader final
{
  public:
    static MswDllPreloader& instance();
    void PreloadDesignatedDlls();

  private:
    MswDllPreloader();
    ~MswDllPreloader();
    MswDllPreloader(MswDllPreloader const&) = delete;
    MswDllPreloader& operator=(MswDllPreloader const&) = delete;

    void PreloadOneDll(std::string const& dll_name);
    void UnloadOneDll (std::string const& dll_name);

    std::deque<std::string> SuccessfullyPreloadedDlls_;
};

#endif // defined LMI_MSW

#endif // msw_workarounds_hpp
