import MySQLdb
import types
import time
import logging
import datetime
import re

import configuration

log = logging.getLogger("wesstats")

#return the intersection of the items in the lists
def intersect(*lists):
	return list(reduce(set.intersection, (set(l) for l in lists)))

def get_date_range(startdate,enddate):
	startdate = startdate.split('/')
	enddate = enddate.split('/')
	print startdate
	startdate = datetime.date(int(startdate[0]),int(startdate[1]),int(startdate[2]))
	enddate = datetime.date(int(enddate[0]),int(enddate[1]),int(enddate[2]))
	delta = abs( (enddate-startdate).days )
	return delta

def isvalid(date):
	date = date.split('/')
	if len(date) != 3:
		return False
	for i in date:
		if not i.isdigit():
			return False
		if int(i) < 1:
			return False
	return True

def scaled_query(curs,query,threshold,evaluator):
	s_time = time.time()
	#list of all the sample sizes
	curs.execute("SELECT TABLE_NAME FROM information_schema.tables WHERE `TABLE_NAME` REGEXP '^"+configuration.DB_TABLE_PREFIX+"SMPL'")
	results = curs.fetchall()
	sizes = []
	for result in results:
		sizes.append(int(result[0][len(configuration.DB_TABLE_PREFIX+"SMPL"):]))
	sizes.sort()
	#print sizes
	#try query on all the sample sizes in increasing order until we get one that meets the threshold
	for size in sizes:
		tblname = configuration.DB_TABLE_PREFIX+"SMPL"+str(size)
		nquery = query.replace("GAMES",tblname)
		curs.execute(nquery)
		results = curs.fetchall()
		length = evaluator(results)
		if length > threshold:
			log.debug("query took " + str(time.time()-s_time) + " seconds")
			return results
	log.debug("too few results from sample tables, using entire table")
	curs.execute(query)
	log.debug("query took " + str(time.time()-s_time) + " seconds")
	return curs.fetchall()

def fconstruct(filters,colname,list):
	if list[0] == "all":
		return filters
	newfilter = colname+" IN ("
	for i in range(0,len(list)):
		newfilter += "'" + list[i] + "'"
		if i != len(list) - 1:
			newfilter += ','
	newfilter += ')'
	
	return fconstruct_helper(filters,newfilter)

def fconstruct_helper(filters,newfilter):
	if len(filters) != 0:
		filters += " AND "
	else:
		filters += " WHERE "
	filters += newfilter
	return filters

def dateconstruct(filters,start_date,end_date):
	if len(filters) != 0:
		filters += " AND "
	else:
		filters += " WHERE "
	filters += "timestamp BETWEEN '"+start_date+"' AND '"+end_date+"'"
	return filters

def listfix(l):
	if not isinstance(l,types.ListType):
		return [l]
	return l

tag = re.compile('^\[.*\]')
endtag = re.compile('^\[/.*\]')
wmlvar = re.compile('.*=.*')

def build_tree(lines):
        vars = dict()
        i = 0
        while i < len(lines):
                l = lines[i].strip()
                if tag.match(l) and not endtag.match(l):
                        #start of a new wml tag
                        tagname = l[1:len(l)-1]
			if not vars.has_key(tagname):
				vars[tagname] = build_tree(lines[i+1:])
			else:
				if not isinstance(vars[tagname],types.ListType):
					vars[tagname] = [ vars[tagname] ]
				vars[tagname].append(build_tree(lines[i+1:]))
                        #go past the end of the tag we just processed
                        while lines[i].strip() != ("[/%s]" % (tagname)):
                                i += 1
                elif endtag.match(l):
                        return vars
                elif wmlvar.match(l):
                        #variable within tag
                        lvalue = l[0:l.index("=")]
                        rvalue = l[l.index("=")+2:-1]
                        vars[lvalue] = rvalue
                i += 1
        return vars

