
# arbiter protcol driver


from arbiter_pb2 import Request, Reply
from dispatcher import StateMachineError
from core_interface import CoreError


class ArbiterProtocolDriver:
   """arbiter protocol driver: responsible for delegating incoming commands"""

   def __init__(self, socket, handler):
      """socket: a zmq socket, handler: a handler object for requests"""
      self._socket = socket
      self._handler = handler

   def run(self):
      """ receives, parses and executes arbiter command requests using the submited handler object"""
      while True:
         try:
            req = Request()
            req.ParseFromString(self._socket.recv())
            rep_data = Reply()
            self._handler.handle(req)
            self._send_ok()
         except ValueError, e: # may be raised by handler methods on invalid parameter values
            self._send_err(-1, str(e))
         except StateMachineError: # raised by the state machine in case of an invalid transition
            self._send_err(-2, 'command rejected by state machine')
         except CoreError, e:
            self._send_err(-3, 'core error')

   def _send_rep(self, reply):
      """serialize and send message via _socket"""
      self._socket.send(reply.SerializeToString())
   
   def _send_ok(self):
      """reply with OK message"""
      rep = Reply()
      rep.status = 0
      self._send_rep(rep)

   def _send_err(self, code, msg):
      """reply with error code and message"""
      rep = Reply()
      rep.status = code
      rep.message = msg
      self._send_rep(rep)

