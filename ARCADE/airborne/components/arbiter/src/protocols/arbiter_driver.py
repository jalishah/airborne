
# arbiter protcol driver


from arbiter_pb2 import Request, Reply
from flight_manager import StateMachineError
from core_interface import CoreError


class ArbiterProtocolDriver:
   """arbiter protocol driver: responsible for delegating incoming commands"""

   def __init__(self, socket, handler):
      """socket: a zmq socket, handler: a handler object for requests"""
      self._socket = socket
      self._handler = handler

   def handle(self):
      """ receives, parses and executes arbiter command requests using the submited handler object"""
      req = Request()
      req.ParseFromString(self._socket.recv())
      rep_data = Reply()
      self._handler.handle(req)
      self.send_ok()

   def _send_rep(self, reply):
      """serialize and send message via _socket"""
      self._socket.send(reply.SerializeToString())
   
   def send_ok(self):
      """reply with OK message"""
      rep = Reply()
      rep.status = 0
      self._send_rep(rep)

   def send_err(self, code, msg):
      """reply with error code and message"""
      rep = Reply()
      rep.status = code
      rep.message = msg
      self._send_rep(rep)

