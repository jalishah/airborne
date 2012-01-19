#!/bin/sh

rw=`cat .git/config | grep git@`
if ["$rw" -eq ""]; then
   echo t
   cp .gitmodules_ro .gitmodules
else
   echo u
   cp .gitmodules_rw .gitmodules
fi


#git submodule init
#git submodule update
#git submodule foreach git checkout master
