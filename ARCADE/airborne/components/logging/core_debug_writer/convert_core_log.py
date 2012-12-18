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
try:
   header = unpacker.next()
   print ' '.join(header)
except:
   # if we don't have a header, don't care..
   pass

#print data:
for msg in unpacker:
   try:
      print ' '.join(map(str, msg))
   except:
      # corrupt data (maybe last line incomplete), don't care...
      pass

