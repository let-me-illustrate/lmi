@REM Cygwin installation.
@REM
@REM Copyright (C) 2010, 2011, 2012, 2013 Gregory W. Chicares.
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
@REM http://savannah.nongnu.org/projects/lmi
@REM email: <gchicares@sbcglobal.net>
@REM snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

@REM $Id$

@ECHO OFF

@REM Install Cygwin. See file 'INSTALL'.

IF EXIST C:\cygwin\NUL     GOTO FoundOldInstallation
IF EXIST C:\cygwin-lmi\NUL GOTO FoundOldInstallation

C:
cd C:\cache_for_lmi
START "Installing Cygwin" /WAIT setup ^
  --quiet-mode ^
  --site ftp://mirror.mcs.anl.gov/pub/cygwin/ ^
  --root C:/cygwin-lmi --packages ^
  cvs,doxygen,gdb,libtool,make,openssh,patch,rsync,subversion,unzip,wget,zsh
cd C:\cygwin-lmi\etc
echo # >> fstab
echo C:/opt/lmi/MinGW-20090203 /MinGW_        lmi_specific binary,user 0 0 >> fstab
echo C:/opt/lmi                /opt/lmi       lmi_specific binary,user 0 0 >> fstab
echo C:/lmi                    /lmi           lmi_specific binary,user 0 0 >> fstab
echo C:/cache_for_lmi          /cache_for_lmi lmi_specific binary,user 0 0 >> fstab
echo Cygwin installation seems to have succeeded
GOTO End

:FoundOldInstallation
echo Cygwin appears already to have been installed
GOTO End

:End
cd C:\cache_for_lmi

