#!/bin/sh

cat .git/config | grep git@ > /dev/null
echo -n "detected "
if [ "$?" != "0" ]; then
   echo -n non-
   cp .gitmodules_ro .gitmodules
else
   cp .gitmodules_rw .gitmodules
fi
echo authenticated configuration, applying to submodules

git submodule init
git submodule update
git submodule foreach git checkout master
