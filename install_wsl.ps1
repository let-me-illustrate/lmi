# WSL installation.
#
# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# If you have UnauthorizedAccess error, run this command in PowerShell:
# Set-ExecutionPolicy Unrestricted
#
# Return back to the Restricted policy after installing:
# Set-ExecutionPolicy Restricted
#
# More info: https:/go.microsoft.com/fwlink/?LinkID=135170

$wsl_registry = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Lxss'
if ((Test-Path -Path $wsl_registry) -And `
    (Get-ChildItem $wsl_registry).Length -gt 0) {
    Write-Output 'WSL OS already installed:'
    Write-Output (Get-ChildItem $wsl_registry | ForEach-Object {Get-ItemProperty $_.PSPath}).DistributionName
    Write-Output 'Note: Only "Ubuntu 18.04" is currently supported.'
    return
}

# Create the download directory.
$dir = 'C:\srv\cache_for_lmi\downloads\'
If (!(Test-Path -Path $dir)) {
    New-Item -ItemType Directory -Force -Path $dir
}

# Download the OS distribution.
$path = 'C:\srv\cache_for_lmi\downloads\Ubuntu_18_04.appx'
If (!(Test-Path -Path $path)) {
    Write-Output ('Downloading "Ubuntu 18.04"...')
    $os_uri = 'https://aka.ms/wsl-ubuntu-1804'
    Invoke-WebRequest -Uri $os_uri -OutFile $path -UseBasicParsing
}

# Install the OS.
Write-Output ('Installing "Ubuntu 18.04"...')
Add-AppxPackage -Path $path

Write-Output 'WSL installation seems to have succeeded'
