
class MAVLinkSource:

   def __init__(self, mavio):
      self.mavio = mavio

   def read_pair(self):
      message = self.mavio.recv_msg()
      if message:
         return message.get_type(), message

