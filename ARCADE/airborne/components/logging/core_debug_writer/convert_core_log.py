#!/usr/bin/env python
#
# core msgpack log unpack utility
#
# unpacks msgpack format to human-readable (space-separated values) text file
# using stdin/stdout
# 
# compression factor is ~2.6


from msgpack import Unpacker
from sys import stdin

unpacker = Unpacker(stdin)

# print header:
header = unpacker.next()
print ' '.join(header)

#print data:
for msg in unpacker:
   print ' '.join(map(str, msg))

