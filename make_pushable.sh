#!/bin/sh

paths=(`git submodule foreach pwd | grep -v 'Entering'`)
configs=()
for i in ${paths[*]}; do
   configs+=("$i/.git/config")
done
configs+=(.gitmodules)

for config in ${configs[*]}; do
   sed -i 's/url = git:\/\/github.com\//url = git@github.com:/g' $config
done
