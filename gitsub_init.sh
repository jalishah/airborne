#!/bin/sh

cp .gitmodules_ro .gitmodules
cp .gitmodules_rw .gitmodules

git submodule init
git submodule update
git submodule foreach git checkout master
