def get_key_from_value(my_dict, v):
	keys = my_dict.keys()
	key_list = []
	for i in keys:
		if v in my_dict[i]:
			key_list.append(i)
	return key_list


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
