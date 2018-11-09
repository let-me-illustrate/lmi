README file for building lmi with MSVS
======================================

0. Requirements
---------------

- Libraries in the submodules, i.e. clone the repository using

    $ git clone --recurse-submodules -b tt https://github.com/vadz/lmi.git

- Compiler: MSVS 2017, which supports C++17, previous versions do _not_ work.

- Bakefile 0.x from https://github.com/vadz/xmlwrapp/ is required to generate
  the project files for xmlwrapp.

- libxml2 currently must be installed system-wide, i.e. its headers and
  libraries must be found by MSVS.


1. Building
-----------

Start by doing

    $ cd 3rdparty/xmlwrapp
    $ bkl -t vs2017 platform/Win32/xmlwrapp.bkl

Then open `lmi.sln` solution and build it.


2. Running
----------

Currently you need to copy `data` directory from the official lmi installation
and use `--data_path` option when running lmi or the GUI tests, e.g.

    $ ./Release/lmi_wx.exe --data_path='v:/src/tt/MassMutual/data'
