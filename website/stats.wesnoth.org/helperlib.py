import MySQLdb
import types
import time
import logging
import datetime
import re
import hashlib
import base64
import os.path

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

def is_valid_date(date):
	date = date.split('/')
	if len(date) != 3:
		return False
	for i in date:
		if not i.isdigit():
			return False
		if int(i) < 1:
			return False
	return True

def is_valid_level(lev):
	return lev.isdigit()	

def scaled_query(curs,tbl,query,threshold,evaluator):
	s_time = time.time()
	#list of all the sample sizes
	_query = "SELECT TABLE_NAME FROM information_schema.tables WHERE TABLE_NAME REGEXP '^%sSMPL%s'" % (configuration.DB_TABLE_PREFIX,tbl)
	log.debug(_query)
	curs.execute(_query)
	results = curs.fetchall()
	sizes = []
	log.debug(sizes)
	for result in results:
		sizes.append(int(result[0][len(configuration.DB_TABLE_PREFIX+"SMPL"+tbl):]))
	sizes.sort()
	#print sizes
	#try query on all the sample sizes in increasing order until we get one that meets the threshold
	for size in sizes:
		tblname = "%sSMPL%s%d" % (configuration.DB_TABLE_PREFIX,tbl,size)
		nquery = query.replace(tbl,tblname)
		log.debug("new query: "+nquery)
		curs.execute(nquery)
		results = curs.fetchall()
		length = evaluator(results)
		if length > threshold:
			log.debug("query took " + str(time.time()-s_time) + " seconds")
			return results
	log.debug("too few results from sample tables, using entire table")
	log.debug(query)
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

def rangeconstruct(filters,colname,start,end):
	if len(filters) != 0:
		filters += " AND "
	else:
		filters += " WHERE "
	filters += "%s BETWEEN '%s' AND '%s'" % (colname,start,end)
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

def get_map_dimensions(filename):
	map = open(filename,"r")
	rows = map.readlines()
	map.close()
	width = len( rows[3].split(',') )+1
	height = len(rows) - 3
	return (height,width)

def save_map(map):
	#decode the map data to a standard map definition
	map = map.replace(";","\n")

	#save a copy of the map if we haven't seen it yet
	map_id = hashlib.sha256()
	map_id.update(map)
	id_digest = base64.urlsafe_b64encode(map_id.digest())
	map_filename = configuration.MAP_DIR + id_digest
	if not os.path.exists(map_filename):
		map_file = open(map_filename,"w")
		map_file.writelines(map)
		map_file.close()
	return id_digest

def upload_log(wml_tree,game):
	log_type = "singleplayer" #possible values are singleplayer,multiplayer,ai
	if game["campaign"] == "multiplayer":
		log_type = "multiplayer"
	if game.has_key("upload_log") and log_type == "multiplayer":
		if game["upload_log"].has_key("ai_log"):
			log_type = "ai"
	log.debug("received log_type: "+log_type)

	map = game["map_data"]
	id_digest = save_map(game["map_data"])

	conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_WRITE_USERNAME,configuration.DB_WRITE_PASSWORD,configuration.DB_NAME,use_unicode=True)
	curs = conn.cursor()

	if log_type == "multiplayer":
		params = (
			wml_tree["id"],
			wml_tree["serial"],
			wml_tree["version"],
			game["campaign"],
			game["scenario"],
			wml_tree.setdefault("platform","unknown")) #6 cols
		curs.execute("INSERT INTO GAMES_MP VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s)",params)
	elif log_type == "singleplayer":
		result_type = "victory"
		if not game.has_key("victory"):
			result_type = "defeat"
			if not game.has_key("defeat"):
				result_type = "quit"
				if not game.has_key("quit"):
					log.debug("broken log, malformed, no game end result")
					conn.close()
					return -1
		params = (
			wml_tree["id"],
			wml_tree["serial"],
			wml_tree.setdefault("platform","unknown"),
			wml_tree["version"],
			game["campaign"],
			game["difficulty"],
			int(game["gold"]),
			int(game["num_turns"]),
			game["scenario"],
			int(game.setdefault("start_turn",0)),
			int(game["time"]),
			result_type,
			int(game[result_type]["time"]),
			int(game[result_type].setdefault("gold","0")),
			int(game[result_type]["end_turn"])) #15 cols
		curs.execute("INSERT INTO GAMES_SP VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",params)
	elif log_type == "ai":
		ai_log = game["upload_log"]["ai_log"]
		winner = "faction1"
		units_winner = 0
		units_loser = 0
		gold_winner = 0
		gold_loser = 0
		if ai_log["result"] == "victory":
			if ai_log["winner"] == "1":
				units_winner = ai_log["end_units1"]
				units_loser = ai_log["end_units2"]
				gold_winner = ai_log["end_units1"]
				gold_loser = ai_log["end_units2"]
			else:
				units_winner = ai_log["end_units2"]
				units_loser = ai_log["end_units1"]
				gold_winner = ai_log["end_units2"]
				gold_loser = ai_log["end_units1"]
				winner = "faction2"
		params = (
			wml_tree["id"], #user_id
			wml_tree["serial"],
			wml_tree.setdefault("platform","unknown"),
			wml_tree["version"],
			game["scenario"],
			ai_log["result"],
			ai_log["end_turn"],
			ai_log["faction1"],
			ai_log["faction2"],
			winner,
			ai_log.setdefault("ai_id1",""),
			ai_log.setdefault("ai_id2",""),
			"",
			"",
			ai_log.setdefault("ai_label",""),
			int(units_winner),
			int(units_loser),
			int(gold_winner),
			int(gold_loser)) #19 cols
		curs.execute("INSERT INTO GAMES_AI VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",params)

	kill_events = game.setdefault("kill_event",[])
	for kill in kill_events:
		if isinstance(kill,types.StringType):
			continue
		killed_unit = kill["results"]["unit_hit"]
		
		killed_lvl = kill["attack"]["defender_lvl"]
		killer_lvl = kill["attack"]["attacker_lvl"]
		killed_id = kill["attack"]["defender_type"]
		killer_id = kill["attack"]["attacker_type"]
		killed_position = [ kill["attack"]["destination"]["x"], kill["attack"]["destination"]["y"] ]
		
		if killed_unit == "attacker":
			killer_lvl = kill["attack"]["defender_lvl"]
			killed_lvl = kill["attack"]["attacker_lvl"]
			killer_id = kill["attack"]["defender_type"]
			killed_id = kill["attack"]["attacker_type"]
			killed_position = [ kill["attack"]["source"]["x"], kill["attack"]["source"]["y"] ]
			
		params = (
			game["scenario"],	
			id_digest,
			kill["attack"]["turn"],
			killed_id,
			killed_lvl,
			killer_id,
			killer_lvl,
			killed_position[0]+","+killed_position[1],
			log_type )
		curs.execute("INSERT INTO KILLMAP VALUES (%s,%s,LAST_INSERT_ID(),%s,%s,%s,%s,%s,%s,%s)",params)
	conn.commit()
	conn.close()
	return 1
