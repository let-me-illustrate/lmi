dnl configure.ac script for lmi, process with autoconf to create configure
dnl
dnl Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Vadim Zeitlin.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License version 2 as
dnl published by the Free Software Foundation.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software Foundation,
dnl Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
dnl
dnl https://savannah.nongnu.org/projects/lmi
dnl email: <gchicares@sbcglobal.net>
dnl snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

dnl Add the given option to CXXFLAGS if it is supported by the compiler.
dnl
dnl Usually used to enable or disable warnings with g++ but is not limited to
dnl warning options.
AC_DEFUN([LMI_CXX_ADD_IF_SUPPORTED],[
    AX_CXX_CHECK_FLAG($1,,,[CXXFLAGS="$CXXFLAGS $1"])
])

dnl Add the given option to both CFLAGS and CXXFLAGS if it supported.
AC_DEFUN([LMI_C_CXX_ADD_IF_SUPPORTED],[
    dnl notice that we assume that if an option is supported by gcc, then it's
    dnl supported by g++ too and skip the second check
    AX_C_CHECK_FLAG($1,,,[CFLAGS="$CFLAGS $1" CXXFLAGS="$CXXFLAGS $1"])
])
