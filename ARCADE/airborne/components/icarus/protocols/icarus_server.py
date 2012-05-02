

# ICARUS server
###############
#
# purpose: responsible for receiving, delegating and replying incoming commands
#
# Author: Tobias Simon, Ilmenau University of Technology


from icarus_pb2 import Request, Reply, OK, E_SYN, E_SEM


class ICARUS_Exception(Exception):

   def __init__(self, code, msg):
      self.code = code
      self.msg = msg


class ICARUS_Server:

   '''
   ICARUS server
   responsible for receiving, delegating and replying incoming commands
   '''

   def __init__(self, socket, delegate):
      '''
      socket: a zmq socket
      delegate: object providing handle(request) routine, raising ICARUS_Exception
      '''
      self._socket = socket
      self._delegate = delegate


   def run(self):
      '''
      receives, parses and executes commands using the submited delegate in a loop
      '''
      while True:
         # receive message via SCL:
         req = Request()
         try:
            data = self._socket.recv()
         except:
            # it would not make sense to send an error message here,
            # as something seems to be wrong with the socket
            print 'could not read SCL message'
            continue
         # parse message into protobuf structure:
         try:
            req.ParseFromString(data)
         except:
            # syntactic error in ParseFromString
            self.send_err(E_SYN, 'could not parse protobuf payload')
            continue
         # handle parsed protobuf message and send reply:
         try:
            self._delegate.handle(req)
            self.send_ok()
         except ICARUS_Exception, ex:
            # semantic error:
            self.send_err(E_SEM, ex.msg)


   def send_err(self, code, msg):
      '''
      reply with error code and message
      '''
      rep = Reply()
      rep.status = code
      rep.message = msg
      self._send_rep(rep)


   def send_ok(self):
      '''
      reply with OK message
      '''
      rep = Reply()
      rep.status = OK
      self._send_rep(rep)


   def _send_rep(self, rep):
      '''
      serialize and send message via _socket
      '''
      self._socket.send(rep.SerializeToString())

