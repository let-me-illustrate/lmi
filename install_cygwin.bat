@REM Cygwin installation.
@REM
@REM Copyright (C) 2010, 2011, 2012 Gregory W. Chicares.
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

@REM Install both Cygwin-1.5 and Cygwin-1.7 . See file 'INSTALL'.
@REM
@REM Motivation: in case any problem is encountered with the current version,
@REM it is convenient to have the older version to fall back on.
@REM
@REM Cygwin-1.5's installer appears to be somewhat anomalous. Even when told
@REM to install to a nondefault directory, it creates 'C:\Cygwin' and puts
@REM some files and directories there. Deleting the unwanted directory and
@REM running the installer again decreases the anomaly; repeating those steps
@REM one more time produces an installation that seems to be valid. For the
@REM versions of msw that have been tested, three iterations are necessary, so
@REM for conservatism five are allowed. Postinstall script 'autoconf2.5.sh'
@REM fails because '/usr/share/info/autoconf2.61.info.gz' doesn't exist, but
@REM it's acceptable if only that 'info' page is missing.
@REM
@REM For the 'rebaseall' commands, see:
@REM   http://cygwin.com/ml/cygwin/2012-01/msg00269.html
@REM   http://cygwin.com/ml/cygwin/2005-07/msg00825.html

IF EXIST C:\cygwin\NUL     GOTO FoundOldInstallation
IF EXIST C:\cygwin-1_5\NUL GOTO FoundOldInstallation
IF EXIST C:\cygwin-1_7\NUL GOTO FoundOldInstallation

C:
cd C:\cache_for_lmi

FOR /L %%i IN (1,1,5) DO (
  CALL :TryToInstall_1_5
  IF NOT EXIST C:\cygwin\NUL (
    echo Cygwin-1.5 installation seems to have succeeded on iteration %%i
    ver | findstr /L "6.1."
    IF ERRORLEVEL 1 GOTO Got_1_5
    cd C:\cygwin-1_5\bin
    C:\cygwin-1_5\bin\ash rebaseall
    echo Cygwin-1.5 rebased
    GOTO Got_1_5
  )
  echo Anomalous 'C:\cygwin' directory present after iteration %%i
  rmdir /S /Q C:\cygwin
)
echo Cygwin-1.5 installation has failed
GOTO End

:TryToInstall_1_5
cd C:\cache_for_lmi
START "Installing Cygwin-1.5" /WAIT setup-legacy ^
  --quiet-mode --no-shortcuts ^
  --site ftp://mirror.mcs.anl.gov/pub/cygwin/ ^
  --root C:/cygwin-1_5 --packages ^
  cvs,doxygen,gdb,libtool,make,openssh,patch,rsync,subversion,wget,zsh
GOTO:EOF

:Got_1_5
cd C:\cache_for_lmi
START "Installing Cygwin-1.7" /WAIT setup ^
  --quiet-mode ^
  --site ftp://mirror.mcs.anl.gov/pub/cygwin/ ^
  --root C:/cygwin-1_7 --packages ^
  cvs,doxygen,gdb,libtool,make,openssh,patch,rsync,subversion,wget,zsh
cd C:\cygwin-1_7\etc
echo # >> fstab
echo C:/opt/lmi/MinGW-20090203 /MinGW_        lmi_specific binary,user 0 0 >> fstab
echo C:/opt/lmi                /opt/lmi       lmi_specific binary,user 0 0 >> fstab
echo C:/lmi                    /lmi           lmi_specific binary,user 0 0 >> fstab
echo C:/cache_for_lmi          /cache_for_lmi lmi_specific binary,user 0 0 >> fstab
echo Cygwin-1.7 installation seems to have succeeded
@REM The '_autorebase' facility has made it unnecessary to invoke any
@REM explicit rebase command for non-legacy versions. See:
@REM   http://cygwin.com/ml/cygwin/2012-03/msg00700.html
@REM ver | findstr /L "6.1."
@REM IF ERRORLEVEL 1 GOTO End
@REM cd C:\cygwin-1_7\bin
@REM C:\cygwin-1_7\bin\dash -l -i -c "rebaseall"
@REM echo Cygwin-1.7 rebased
GOTO End

:FoundOldInstallation
echo Cygwin appears already to have been installed
GOTO End

:End
cd C:\cache_for_lmi

