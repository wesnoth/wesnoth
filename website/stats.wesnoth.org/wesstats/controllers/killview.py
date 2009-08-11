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
		
		cur_map = kw.setdefault("map","6f50ba078308f4ed29b8b79a0727fd9b")
		#check for input sanity, we will be fetching map tiles based on this name
		if not ( cur_map.isalnum() and len(cur_map) == 32):
			cur_map = ""
		
		m_dimensions = ()
		if cur_map != "":
			m_dimensions = helperlib.get_map_dimensions(configuration.MAP_DIR+cur_map)
	
		#compute kill frequency per hex
		curs.execute("SELECT position,COUNT(position) FROM `KILLMAP` WHERE map_id = %s GROUP BY position ORDER BY COUNT(position) DESC LIMIT 0,100", (cur_map,))
		hexdata = curs.fetchall()
		conn.close()
		#first item is 'hottest' -> red (255,0,0), last item is 'coldest' -> blue (0,0,255), linearly interpolate hotness of all values inbetween
		grid_colors = ""
		max = hexdata[0][1]
		min = hexdata[99][1]
		for hex in hexdata:
			v = hex[1]
			red_val = 255.0*float(v-min)/float(max-min)	
			blue_val = 255.0 * (1 - (float(v-min)/float(max-min)))
			hex_color = "%.2x00%.2x" % (int(red_val),int(blue_val))
			#construct the javascript dicitonary that will store hex->color mappings
			grid_colors += '"%s":"%s",' % (hex[0],hex_color)
		grid_colors = grid_colors[:-1]
		return dict(maps=maps,cur_map=cur_map,dimensions=m_dimensions,grid_colors=grid_colors)
