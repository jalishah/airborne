

def compose_config_update(id, val):
   return id + '\0' + val.SerializeToString()


def parse_config_update(str):
   split_pos = str.find('\0')
   id = str[0 : split_pos]
   pb = str[split_pos + 1 :]
   val = Value()
   val.ParseFromString(pb)
   return id, val


if __name__ == '__main__':
   from config_pb2 import Value
   val = Value()
   val.dbl_val = 1.0
   print parse_config_update(compose_config_update('123', val))
