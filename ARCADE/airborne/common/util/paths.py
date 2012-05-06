

from os import makedirs, getenv, sep
import errno


def user_data_dir():
   '''
   $HOME/.ARCADE is used for storing device-specific files:
   - configuration files
   - logfiles
   - calibration data
   '''
   path = getenv('HOME') + sep + '.ARCADE'
   try:
      makedirs(path)
   except OSError, e:
      if e.errno != errno.EEXIST:
         raise
   return path

