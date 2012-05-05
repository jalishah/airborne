
#
# mtputil.py
# multi-threaded process utility library
# author: Tobias Simon, Ilmenau University of Technology
#


from threading import Thread
from signal import pause


def start_daemon_thread(target):
   thread = Thread(target = target)
   thread.daemon = True
   thread.start()
   return thread


def await_signal():
   pause()

