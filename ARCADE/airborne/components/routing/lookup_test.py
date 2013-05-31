
def get_key_from_value(my_dict, v):
	keys = my_dict.keys()
	key_list = []
	for i in keys:
		if v in my_dict[i]:
			key_list.append(i)
	if key_list is True:
		return key_list
	else:
		return 0;
		

 
# dictionary of chemical symbols
symbol_dic = {1: [3,4], 7: [5,6], 9: [1,3,4,5,6]}
print get_key_from_value(symbol_dic, 2)
