import yaml

with open('config.yaml') as f:
	doc = yaml.load(f)
	print doc['prefix']
