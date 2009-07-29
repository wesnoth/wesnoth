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
import logging

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

	@expose(template="wesstats.templates.addview")
	def addview(self):
		return dict()

	@expose()
	def upload(self, **kw):
		raw_log = pylons.request.body
		wml_tree = helperlib.build_tree(raw_log.split('\n'))

		if not wml_tree.has_key("platform"):
			wml_tree["platform"] = "unknown"

		result_type = "victory"
		if not wml_tree.has_key("victory"):
			result_type = "defeat"
			if not wml_tree.has_key("defeat"):
				result_type = "quit"

		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
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
			int(wml_tree["game"]["start_turn"]),
			int(wml_tree["game"]["time"]),
			result_type,
			int(wml_tree["game"][result_type]["time"]),
			int(wml_tree["game"][result_type].setdefault("gold","0")),
			int(wml_tree["game"][result_type]["end_turn"])) #15 cols
		curs.execute("INSERT INTO GAMES VALUES (DEFAULT,NOW(),%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",params)
		conn.close()
		return dict()
	
	@expose(template="wesstats.templates.deleteview")
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
			view = NotFoundController(url)

		return view, remainder

class NotFoundController(object):
	def __init__(self,url):
		self.url = url
