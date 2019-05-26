# WSL installation.
#
# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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
# http://savannah.nongnu.org/projects/lmi
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
    return
}

# Select a Debian-compatible OS: our build scripts use apt.
$os_list = @(
    @{
        name = 'Ubuntu 18.04'
        filename = 'Ubuntu_18_04.appx'
        uri = 'https://aka.ms/wsl-ubuntu-1804'
    }
    @{
        name = 'Ubuntu 16.04'
        filename = 'Ubuntu_16_04.appx'
        uri = 'https://aka.ms/wsl-ubuntu-1604'
    }
    @{
        name = 'Debian GNU/Linux'
        filename = 'Debian_GNU_Linux.appx'
        uri = 'https://aka.ms/wsl-debian-gnulinux'
    }
)

$options = @()
for ($i=1; $i -le $os_list.Length; $i++) {
    $os_name = $os_list[$i-1].name
    $d = [System.Management.Automation.Host.ChoiceDescription]::new("&$i. $os_name")
    $options += $d
}

$title = 'Please select the OS to install'
$description = "Note: Non-Debian-based OS won't work, as we use apt."

$opt = $host.UI.PromptForChoice($title, $description, $options, 0)
$os = $os_list[$opt]
Write-Output ('"' + $os.name + '" was chosen.')

# Create the download directory.
$dir = 'C:\cache_for_lmi\downloads\'
If (!(Test-Path -Path $dir)) {
    New-Item -ItemType Directory -Force -Path $dir
}

# Download the OS distribution.
$path = 'C:\cache_for_lmi\downloads\' + $os.filename
If (!(Test-Path -Path $path)) {
    Write-Output ('Downloading "' + $os.name + '"...')
    Invoke-WebRequest -Uri $os.uri -OutFile $path -UseBasicParsing
}

# Install the OS.
Write-Output ('Installing "' + $os.name + '"...')
Add-AppxPackage -Path $path

Write-Output 'WSL installation seems to have succeeded'
