# $Id$
"""
   Copyright (C) 2009 by Gregory Shikhman <cornmander@cornmander.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
"""
# -*- coding: utf-8 -*-

import MySQLdb
import gzip
import StringIO
import logging
import hashlib
import base64
import os.path
import types

import configuration
import helperlib

from tg import expose
import pylons

from wesstats.lib.base import BaseController
from wesstats.controllers.error import ErrorController
from wesstats.controllers.pieview import PieGraphController
from wesstats.controllers.barview import BarGraphController
from wesstats.controllers.lineview import LineGraphController
from wesstats.controllers.killview import KillGraphController

__all__ = ['RootController']

log = logging.getLogger("wesstats")

class RootController(BaseController):
	@expose(template="wesstats.templates.index")
	def index(self):
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		curs.execute("SELECT title,url,type FROM _wsviews")
		views = curs.fetchall()
		conn.close()
		return dict(views=views)

	#@expose(template="wesstats.templates.addview")
	def addview(self):
		return dict()

	@expose("wesstats.templates.upload")
	def upload(self, **kw):
		raw_log = pylons.request.body
		if len(raw_log) == 0:
			log.debug("uploader pageview")
			return dict(status="this is not the page you're looking for.")
			
		#check for gzip compression
		if raw_log[0] == '\037' and raw_log[1] == '\213':
			#try to decompress log
			raw_log = gzip.GzipFile("","r",9,StringIO.StringIO(raw_log))
			raw_log = raw_log.readlines()		
		else:
			#log is uncompressed, probably a v1 log
			#split v1 log into an array instead of a blob
			raw_log = raw_log.split('\n')
		wml_tree = helperlib.build_tree(raw_log)
		
		log_type = "singleplayer" #possible values are singleplayer,multiplayer,ai
		if wml_tree["game"]["campaign"] == "multiplayer":
			log_type = "multiplayer"
		if wml_tree["game"].has_key("upload_log"):
			if wml_tree["game"]["upload_log"].has_key("ai_log"):
				log_type = "ai"
	
		map = wml_tree["game"]["map_data"]
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

		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_WRITE_USERNAME,configuration.DB_WRITE_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
	
		if log_type == "multiplayer":
			params = (
				wml_tree["id"],
				wml_tree["serial"],
				wml_tree["version"],
				wml_tree["game"]["campaign"],
				wml_tree["game"]["scenario"],
				wml_tree.setdefault("platform","unknown")) #6 cols
			curs.execute("INSERT INTO GAMES_MP VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s)",params)
		elif log_type == "singleplayer":
			result_type = "victory"
			if not wml_tree.has_key("victory"):
				result_type = "defeat"
				if not wml_tree.has_key("defeat"):
					result_type = "quit"
			params = (
				wml_tree["id"],
				wml_tree["serial"],
				wml_tree.setdefault("platform","unknown"),
				wml_tree["version"],
				wml_tree["game"]["campaign"],
				wml_tree["game"]["difficulty"],
				int(wml_tree["game"]["gold"]),
				int(wml_tree["game"]["num_turns"]),
				wml_tree["game"]["scenario"],
				int(wml_tree["game"].setdefault("start_turn",0)),
				int(wml_tree["game"]["time"]),
				result_type,
				int(wml_tree["game"][result_type]["time"]),
				int(wml_tree["game"][result_type].setdefault("gold","0")),
				int(wml_tree["game"][result_type]["end_turn"])) #15 cols
			curs.execute("INSERT INTO GAMES_SP VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",params)
		elif log_type == "ai":
			ai_log = wml_tree["game"]["upload_log"]["ai_log"]
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
				wml_tree["game"]["scenario"],
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

		kill_events = wml_tree["game"].setdefault("kill_event",[])
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
				wml_tree["game"]["scenario"],	
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
		log.debug("upload OK")
		return dict(status="uploaded")
	
	#@expose(template="wesstats.templates.deleteview")
	def deleteview(self,**kw):
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		#check for a deletion request
		request = None
		for key in kw:
			if kw[key] == "Delete":
				request = key
				break
		if request != None:
			log.info("view deletion request for page "+key)
			curs.execute("DELETE FROM _wsviews WHERE url = %s", (request,))
		curs.execute("SELECT title,url FROM _wsviews")
		views = curs.fetchall()
		conn.close()
		return dict(views=views)
	
	@expose()
	def lookup(self,url,*remainder):
		#check if view exists
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT url FROM _wsviews WHERE url = %s", (url,))
		results = curs.fetchall()
		exists = len(results) == 1

		type = None
		if exists:
			#get the graph type
			curs.execute("SELECT type FROM _wsviews WHERE url = %s", (url,))
			results = curs.fetchall()
			type = results[0][0]
		conn.close()
		
		view = None
		if type == "bar":
			view = BarGraphController(url)
		elif type == "pie":
			view = PieGraphController(url)
		elif type == "line":
			view = LineGraphController(url)
		elif type == "kill":
			view = KillGraphController(url)
		else:
			return dict()
		return view, remainder
