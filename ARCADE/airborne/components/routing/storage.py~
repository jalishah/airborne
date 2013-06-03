from time import sleep
from lookup import *
import yaml

yaml_conf = {}

with open('config.yaml') as f:
	yaml_conf = yaml.load(f)

# in dictionary (rout) the keys are the sender ids
rout = {};

unique_ids_list = []  # to avoid repeatition of brodcast for batman org msg

my_id = yaml_conf['header']['my_id']

# To check weather the key already exist in the routing table
def chk_rout_key( key ):
	if rout.has_key(key) == False:   # check if the key already exist
		set_new_route_key(key)
	return

# update rout add the value to routing table
def set_new_route_key( key ):
	global rout	
	rout[key] = []
	return	

def add_new_rout_values( key, value):
	global rout
	look = Lookup(rout)
	links = look.get_value(key)
	if value not in links:
		rout[key].append(value)
	return

def find_rout(value):
	global rout
	keys = get_key_from_value(rout,value)
	print keys
	
	if not keys:
		return 0


	else:
		if my_id in keys:
			return value
		else: 
			return keys[0]
	
	
def get_rout (): 
	print "generate routing table: " , rout
	return



def empty_uniq_id_list():  
	global unique_ids_list
	while 1:				
		sleep(120)
		del unique_ids_list[:]
		print  unique_ids_list


# remove the msg unique id's from the list 

