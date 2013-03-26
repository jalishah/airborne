

def find_key(dic, val):
    """return the key of dictionary dic given the value"""
    return [k for k, v in symbol_dic.iteritems() if v == val][0]

def find_value(dic, key):
    """return the value of dictionary dic given the key"""
    return dic[key]


class Lookup(dict):
    """
    a dictionary which can lookup value by key, or keys by value
    """
    def __init__(self, items=[]):
        """items can be a list of pair_lists or a dictionary"""
        dict.__init__(self, items)
    def get_key(self, value):
        """find the key(s) as a list given a value"""
        return [item[0] for item in self.items() if item[1] == value]
    def get_value(self, key):
        """find the value given a key"""
        return self[key]


# test it out
if __name__ == '__main__':
    
    # dictionary of chemical symbols
    rout = {1: (86,), 2: (86,), 3: (86,), 4: (0,), 5: (86,78,56), 86: (0,), 22: (0,), 37: (0,),81:()}
    look = Lookup(rout)
    links = look.get_value(2)
    k = 24	
    if k not in links:
	rout[2] = rout[2] + (k,)
    print rout


