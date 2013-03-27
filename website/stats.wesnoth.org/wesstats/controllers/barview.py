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

__all__ = ['BarGraphController']

log = logging.getLogger("wesstats")

class BarGraphController(BaseController):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.pieview")
	def default(self,**kw):
		#pull data on this view from DB
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		log.debug("pie chart request, here is SQL data for this view:")
		log.debug(view_data)
		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT "+filter+" FROM GAMES_SP")
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		#print fdata
		filters = ""
		used_filters = helperlib.intersect(kw.keys(),available_filters)
		ufilters_vals = dict()
		for filter in used_filters:
			kw[filter] = helperlib.listfix(kw[filter])
			filter_vals = helperlib.intersect(kw[filter],fdata[filter])
			filters = helperlib.fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		if 'startdate' in kw and 'enddate' in kw and helperlib.isvalid(kw['startdate']) and helperlib.isvalid(kw['enddate']):
			filters = helperlib.dateconstruct(filters,kw['startdate'],kw['enddate'])
			used_filters.append("dates")
			ufilters_vals["dates"] = [kw['startdate'] + "-" + kw['enddate']]
		#get columns and column transformations for this view
		y_xforms = view_data[6].split(',')
		y_data = view_data[2].split(',')
		y_data_str = ""
		#they must be equal!
		assert len(y_data) == len(y_xforms)
		for i in range(len(y_data)):
			y_data_str += y_xforms[i] + "(" + y_data[i] + "),"
		y_data_str = y_data_str[0:len(y_data_str)-1]
		query = "SELECT "+view_data[1]+","+y_data_str+" FROM GAMES_SP "+filters+" GROUP BY "+view_data[1]
		log.debug("SQL query:")
		log.debug(query)
		results = helperlib.scaled_query(curs,query,100,evaluators.count_eval)
		log.debug("query result:")
		log.debug(results)
		#generate JS datafields here because genshi templating can't emit JS...
		data = ""
		for i in range(0,len(results)):
                        data += "data.setValue("+str(i)+",0,'"+results[i][0]+"');\n"
                        data += "data.setValue("+str(i)+",1,"+str(results[i][1])+");\n"
		return dict(title=view_data[0],xlabel=view_data[3],ylabel=view_data[4],piedata=results,
			data=data,filters=available_filters,used_filters=used_filters,
			ufilters_vals=ufilters_vals,fdata=fdata)
