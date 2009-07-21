import os,sys

for infile in sys.argv[1:]:
	map = open(infile,"r")
	lines = map.readlines()
	rows = len(lines) - 3
	columns = lines[3].count(',')+1
	print "%d rows x %d columns" % (rows,columns)
