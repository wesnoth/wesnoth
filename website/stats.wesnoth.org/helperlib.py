import MySQLdb
import types
import configuration
import evaluators
import time
import logging

log = logging.getLogger("wesstats")

#return the intersection of the items in the lists
def intersect(*lists):
	return list(reduce(set.intersection, (set(l) for l in lists)))

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

