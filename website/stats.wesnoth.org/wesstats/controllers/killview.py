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
	
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform,tbl FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		log.debug("kill map request, here is SQL data for this view:")
		log.debug(view_data)
		games_tbl = view_data[7]
		
		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		#@TODO: right now I overloaded the filters for the KILLMAPS table into y_xform, there needs to be a better solution for this in the future
		available_filters_map = view_data[6].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT "+filter+" FROM "+games_tbl)
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		for filter in available_filters_map:
			curs.execute("SELECT DISTINCT "+filter+" FROM KILLMAP WHERE game_type='"+view_data[2]+"'")
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		
		#print fdata
		filters = ""
		filters_map = ""
		used_filters = helperlib.intersect(kw.keys(),available_filters)
		used_filters_map = helperlib.intersect(kw.keys(),available_filters_map)
		ufilters_vals = dict()
		used_filters += used_filters_map
		for filter in used_filters:
			kw[filter] = helperlib.listfix(kw[filter])
			filter_vals = helperlib.intersect(kw[filter],fdata[filter])
			filters = helperlib.fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		startdate = ""
		enddate = ""
		if 'startdate' in kw and 'enddate' in kw and helperlib.is_valid_date(kw['startdate']) and helperlib.is_valid_date(kw['enddate']):
			filters = helperlib.rangeconstruct(filters,"timestamp",kw['startdate'],kw['enddate'])
			used_filters.append("dates")
			ufilters_vals["dates"] = [kw['startdate'] + " to " + kw['enddate']]
			startdate = kw['startdate']
			enddate = kw['enddate']
		minkillerlev = ""
		maxkillerlev = ""
		minkilledlev = ""
		maxkilledlev = ""
		if 'minkillerlev' in kw and 'maxkillerlev' in kw and helperlib.is_valid_level(kw['minkillerlev']) and helperlib.is_valid_level(kw['maxkillerlev']):
			filters = helperlib.rangeconstruct(filters,"killer_level",kw['minkillerlev'],kw['maxkillerlev'])
			minkillerlev = kw['minkillerlev']
			maxkillerlev = kw['maxkillerlev']	
			used_filters.append("killer level range")
			ufilters_vals["killer level range"] = [kw['minkillerlev'] + " to " + kw['maxkillerlev']]
		if 'minkilledlev' in kw and 'maxkilledlev' in kw and helperlib.is_valid_level(kw['minkilledlev']) and helperlib.is_valid_level(kw['maxkilledlev']):
			filters = helperlib.rangeconstruct(filters,"killed_level",kw['minkilledlev'],kw['maxkilledlev'])
			minkilledlev = kw['minkilledlev']
			maxkilledlev = kw['maxkilledlev']	
			used_filters.append("killed level range")
			ufilters_vals["killed level range"] = [kw['minkilledlev'] + " to " + kw['maxkilledlev']]

		curs.execute("SELECT DISTINCT scenario_name,map_id FROM `KILLMAP` WHERE game_type='"+view_data[2]+"' GROUP BY map_id")
		maps = curs.fetchall()
		
		cur_map = kw.setdefault("map","mWF1l_gRtBBQvkNoqpYBPY3z2idTMM493ck_YjEk_WU=")
		#check for input sanity, we will be fetching map tiles based on this name
		if not ( cur_map.count("/") == 0 and cur_map.count(".") == 0 and len(cur_map) == 44):
			cur_map = ""
		cur_map_name = "None"
		for map in maps:
			if map[1] == cur_map:
				cur_map_name = map[0]
				break
		
		m_dimensions = ()
		if cur_map != "":
			m_dimensions = helperlib.get_map_dimensions(configuration.MAP_DIR+cur_map)
	
		#compute kill frequency per hex
		filters = helperlib.fconstruct(filters,"map_id",[cur_map])
		query = "SELECT position,COUNT(position) FROM `KILLMAP` INNER JOIN `" + games_tbl + "` USING (game_id) "+filters+" GROUP BY position ORDER BY COUNT(position) DESC LIMIT 0,100"
		curs.execute(query)
		hexdata = curs.fetchall()
		conn.close()
		
		#first item is 'hottest' -> red (255,0,0), last item is 'coldest' -> blue (0,0,255)
		#linearly interpolate hotness of all values inbetween
		grid_colors = ""
		if len(hexdata) != 0:
			max = hexdata[0][1]
			min = hexdata[len(hexdata)-1][1]
			#if there are very few results, the minimum frequency == maximum frequency -> divide by 0
			if max == min:
				max += 1
			for hex in hexdata:
				v = hex[1]
				red_val = 255.0*float(v-min)/float(max-min)	
				blue_val = 255.0 * (1 - (float(v-min)/float(max-min)))
				hex_color = "%.2x00%.2x" % (int(red_val),int(blue_val))
				#construct the javascript dicitonary that will store hex->color mappings
				grid_colors += '"%s":"%s",' % (hex[0],hex_color)
			grid_colors = grid_colors[:-1]
		return dict(maps=maps,cur_map=cur_map,dimensions=m_dimensions,
			grid_colors=grid_colors,startdate=startdate,enddate=enddate,
			minkillerlev=minkillerlev,maxkillerlev=maxkillerlev,
			minkilledlev=minkilledlev,maxkilledlev=minkilledlev,
			used_filters=used_filters,ufilters_vals=ufilters_vals,
			filters=available_filters+available_filters_map,
			fdata=fdata,cur_map_name=cur_map_name,title=view_data[0])
