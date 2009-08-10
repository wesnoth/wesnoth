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
import evaluators
import helperlib

from tg import expose
from wesstats.lib.base import BaseController

__all__ = ['KillGraphController']

log = logging.getLogger("wesstats")

class KillGraphController(BaseController):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.killview")
	def default(self,**kw):
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		curs.execute("SELECT DISTINCT scenario_name,map_id FROM `KILLMAP` GROUP BY map_id")
		maps = curs.fetchall()
		conn.close()
		return dict(maps=maps)
