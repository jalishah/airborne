#!/bin/sh

file_in=~/core_debug.msgpack
file_gen=~/.ARCADE/core_debug.msgpack
file_out=~/core_debug.txt

svctrl --start core_debug_writer
$MOBICOM_PATH/build/ARCADE/airborne/components/core/core $file_in
svctrl --stop core_debug_writer
convert_core_log < $file_gen > $file_out
