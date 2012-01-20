#!/bin/sh

cat .git/config | grep git@ > /dev/null
if [ "$?" != "0" ]; then
   cp .gitmodules_ro .gitmodules
else
   echo rw
   cp .gitmodules_rw .gitmodules
fi

git submodule init
git submodule update
git submodule foreach git checkout master
