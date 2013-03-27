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
from wesstats.controllers.killview import KillGraphController
from wesstats.controllers.notfound import NotFoundController

__all__ = ['RootController']

log = logging.getLogger("wesstats")

class RootController(BaseController):
	@expose(template="wesstats.templates.index")
	def index(self,**kw):
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
			log.debug("deprecated log")
			return dict(status="wesnoth client too old")
		wml_tree = helperlib.build_tree(raw_log)

		if isinstance(wml_tree["game"],types.ListType):
			log.debug("multigame log")
			for game in wml_tree["game"]:
				ret = helperlib.upload_log(wml_tree,game)
		else:
			log.debug("singlegame log")	
			ret = helperlib.upload_log(wml_tree,wml_tree["game"])
		status = "uploaded"
		if ret == -1:
			status = "failed"
			log.debug("upload failed")
		else:
			log.debug("upload ok")
		return dict(status=status)
	
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
			view = NotFoundController(url)
		return view, remainder
