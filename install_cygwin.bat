@REM Cygwin installation.
@REM
@REM Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
@REM
@REM This program is free software; you can redistribute it and/or modify
@REM it under the terms of the GNU General Public License version 2 as
@REM published by the Free Software Foundation.
@REM
@REM This program is distributed in the hope that it will be useful,
@REM but WITHOUT ANY WARRANTY; without even the implied warranty of
@REM MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@REM GNU General Public License for more details.
@REM
@REM You should have received a copy of the GNU General Public License
@REM along with this program; if not, write to the Free Software Foundation,
@REM Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
@REM
@REM https://savannah.nongnu.org/projects/lmi
@REM email: <gchicares@sbcglobal.net>
@REM snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

@ECHO OFF

@REM Install Cygwin. See file 'INSTALL'.

IF EXIST C:\cygwin\NUL       GOTO FoundOldInstallation
IF EXIST C:\cygwin64_lmi\NUL GOTO FoundOldInstallation

C:
cd C:\srv\cache_for_lmi
START "Installing Cygwin" /WAIT setup-x86_64 ^
  --wait --quiet-mode ^
  --site http://mirrors.kernel.org/sourceware/cygwin/ ^
  --root C:/cygwin64_lmi --packages ^
   "autoconf,automake,bc,bsdtar,curl,dos2unix,doxygen,gdb,git,libtool,make,openssh,patch,pkg-config,rsync,unzip,vim,wget,zip,zsh"
cd C:\cygwin64_lmi\etc
echo # >> fstab
echo C:/opt/lmi           /opt/lmi           lmi_specific binary,user 0 0 >> fstab
echo C:/srv/cache_for_lmi /srv/cache_for_lmi lmi_specific binary,user 0 0 >> fstab
echo Cygwin installation seems to have succeeded
GOTO End

:FoundOldInstallation
echo Cygwin appears already to have been installed
GOTO End

:End
cd C:\srv\cache_for_lmi
