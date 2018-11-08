README file for building lmi with MSVS
======================================

0. Requirements
---------------

Only MSVS 2017 has support for C++17, previous versions do _not_ work.

Make sure to clone the repository with submodules, i.e.

    $ git clone --recurse-submodules -b tt https://github.com/vadz/lmi.git


1. Building
-----------

Just building the entire solution should work.


2. Running
----------

Currently you need to copy `data` directory from the official lmi installation
and use `--data_path` option when running lmi or the GUI tests, e.g.

    $ ./Release/lmi_wx.exe --data_path='v:/src/tt/MassMutual/data'
