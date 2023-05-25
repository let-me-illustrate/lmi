#!/bin/sh
set -e

# To be made into options if necessary.
arch=x64
config=Debug

srcdir=$(realpath .)
builddir=output/$arch/$config
distdir=../lmi-dist-$(date --utc +'%Y%m%dT%H%M')

mkdir -p $distdir
distdir=$(realpath $distdir)

cd $builddir
for f in *.dll; do
    cp $f $(basename $f .dll).pdb $distdir
done
for f in lmi_*.exe; do
    cp $f $(basename $f .exe).pdb $distdir
done

cd $srcdir
git archive HEAD | tar -C $distdir -xf-
