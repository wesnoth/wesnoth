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

__all__ = ['LineGraphController']

log = logging.getLogger("wesstats")

#one year = 12 months *31 days
TWOYEARS = 744
ONEYEAR = 372
SIXMONTHS = 186
THREEMONTHS = 93
ONEMONTH = 31
TWOWEEKS = 14
ONEWEEK = 7
ONEDAY = 1

class LineGraphController(BaseController):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.lineview")
	def default(self,**kw):
		#pull data on this view from DB
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform,tbl FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		log.debug("line chart request, here is SQL data for this view:")
		log.debug(view_data)

		tbl = view_data[7]

		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT " + filter + " FROM " + tbl)
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		#print fdata
		filters = ""
		used_filters = helperlib.intersect(kw.keys(),available_filters)
		ufilters_vals = dict()
		startdate = ""
		enddate = ""
		for filter in used_filters:
			kw[filter] = helperlib.listfix(kw[filter])
			filter_vals = helperlib.intersect(kw[filter],fdata[filter])
			filters = helperlib.fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		if 'startdate' in kw and 'enddate' in kw and helperlib.is_valid_date(kw['startdate']) and helperlib.is_valid_date(kw['enddate']):
			filters = helperlib.rangeconstruct(filters,"timestamp",kw['startdate'],kw['enddate'])
			used_filters.append("dates")
			ufilters_vals["dates"] = [kw['startdate'] + " to " + kw['enddate']]
			startdate = kw['startdate']
			enddate = kw['enddate']
		#calculate the number of days in the range
		daterange = TWOYEARS+1
		date_sampling_filter = ""
		if used_filters.__contains__("dates"):
			daterange = helperlib.get_date_range(kw['startdate'],kw['enddate'])
		
		if daterange > TWOYEARS:
			date_sampling_filter = " DAYOFMONTH(timestamp) = '1' AND MONTH(timestamp) % 2 "
		elif daterange > SIXMONTHS:
			date_sampling_filter = " DAYOFMONTH(timestamp) = '1' "
		elif daterange > THREEMONTHS:
			date_sampling_filter = " DAYOFMONTH(timestamp) % '14' = 0 "
		elif daterange > ONEMONTH:
			date_sampling_filter = " DAYOFMONTH(timestamp) % '7' = 0 "
		elif daterange > TWOWEEKS:
			date_sampling_filter = " DAYOFMONTH(timestamp) % '3' = 0 "
		elif daterange > ONEWEEK:
			date_sampling_filter = " "
		#must do special routines for hourly sampling
		elif daterange > ONEDAY:
			pass
		else:
			pass
		
		filters = helperlib.fconstruct_helper(filters,date_sampling_filter)
		#get columns and column transformations for this view
		y_xforms = view_data[6].split(',')
		y_data = view_data[2].split(',')
		y_data_str = ""
		y_group_str = ""
		#number of y columns and y transformations must be equal
		assert len(y_data) == len(y_xforms)
		#we currently only support time as the x axis
		assert view_data[1] == "timestamp"
		for i in range(len(y_data)):
			y_data_str += y_xforms[i] + "(" + y_data[i] + ")," + y_data[i] + ","
			y_group_str += y_data[i] + ","
		y_data_str = y_data_str[0:len(y_data_str)-1]
		y_group_str = y_group_str[0:len(y_group_str)-1]
		query = "SELECT CAST(timestamp as DATE),%s FROM %s %s GROUP BY CAST(timestamp as DATE),%s" % (y_data_str,tbl,filters,y_group_str)
		log.debug("SQL query:")
		log.debug(query)
		results = helperlib.scaled_query(curs,tbl,query,100,evaluators.simple_eval)
		#log.debug("query result:")
		#log.debug(results)
		data = LineGraphController.reformat_data(self,results)
		#generate JS datafields here because genshi templating can't emit JS...
		js_celldata = ""
		js_columnnames = ""
		dates = data[0].keys()
		dates.sort()
		for y in data[1]:
			js_columnnames += "data.addColumn('number', '" + y + "');\n"
		for i in range(len(dates)):
			js_celldata += "data.setCell(" + str(i) + ", 0, '" + dates[i].__str__() + "');\n"
			for j in range(len(data[0][dates[i]])):
				js_celldata += "data.setCell(" + str(i) + ", " + str(j+1) + ", " + str(data[0][dates[i]][j][1]) + ");\n"
		return dict(title=view_data[0],xlabel=view_data[3],js_celldata=js_celldata,
			filters=available_filters,used_filters=used_filters,js_columnnames=js_columnnames,
			ufilters_vals=ufilters_vals,fdata=fdata,numdates=len(dates),startdate=startdate,
			enddate=enddate)

	#returns a tuple of a dictionary with date:ydata and a list of ydata labels
	def reformat_data(self,sql_data):
		dates = dict()
		ydata = []
		#create an entry for each date and find all the ydata types listed
		for row in sql_data:
			dates[row[0]] = []
			if not ydata.__contains__(row[2]):
				ydata.append(row[2])
		for date in dates.keys():
			for y in ydata:
				for row in sql_data:
					if row[0] == date and row[2] == y:
						dates[date].append([row[2],row[1]])
						found = True
						break
		return [dates,ydata]
			
