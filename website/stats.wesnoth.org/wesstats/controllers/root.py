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
import types
import configuration
import evaluators
import time

from tg import expose, flash, require, url, request, redirect
from pylons.i18n import ugettext as _, lazy_ugettext as l_
from catwalk.tg2 import Catwalk
from repoze.what import predicates

from wesstats.lib.base import BaseController
from wesstats.model import DBSession, metadata
from wesstats.controllers.error import ErrorController
from wesstats import model
from wesstats.controllers.secure import SecureController

__all__ = ['RootController']

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
			print "query took " + str(time.time()-s_time) + " seconds"
			return results
	print "samples too small, using entire table"
	curs.execute(query)
	print "query took " + str(time.time()-s_time) + " seconds"
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

class RootController(BaseController):
	@expose(template="wesstats.templates.index")
	def index(self):
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		curs.execute("SELECT title,url FROM _wsviews")
		views = curs.fetchall()
		conn.close()
		return dict(views=views)

	@expose(template="wesstats.templates.addview")
	def addview(self):
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
		else:
			view = NotFoundController(url)

		return view, remainder

class PieGraphController(object):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.pieview")
	def default(self,**kw):
		#pull data on this view from DB
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		print view_data
		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT "+filter+" FROM GAMES")
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		#print fdata
		filters = ""
		used_filters = intersect(kw.keys(),available_filters)
		ufilters_vals = dict()
		for filter in used_filters:
			kw[filter] = listfix(kw[filter])
			filter_vals = intersect(kw[filter],fdata[filter])
			filters = fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		if 'startdate' in kw and 'enddate' in kw and isvalid(kw['startdate']) and isvalid(kw['enddate']):
			filters = dateconstruct(filters,kw['startdate'],kw['enddate'])
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
		query = "SELECT "+view_data[1]+","+y_data_str+" FROM GAMES "+filters+" GROUP BY "+view_data[1]
		print query
		results = scaled_query(curs,query,100,evaluators.count_eval)
		print results
		#generate JS datafields here because genshi templating can't emit JS...
		data = ""
		for i in range(0,len(results)):
                        data += "data.setValue("+str(i)+",0,'"+results[i][0]+"');\n"
                        data += "data.setValue("+str(i)+",1,"+str(results[i][1])+");\n"
		return dict(title=view_data[0],xlabel=view_data[3],ylabel=view_data[4],piedata=results,
			data=data,filters=available_filters,used_filters=used_filters,
			ufilters_vals=ufilters_vals,fdata=fdata)

class BarGraphController(object):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.barview")
	def default(self,**kw):
		#pull data on this view from DB
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		print view_data
		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT "+filter+" FROM GAMES")
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		#print fdata
		filters = ""
		for filter in available_filters:
			print filter		
		print kw
		used_filters = intersect(kw.keys(),available_filters)
		ufilters_vals = dict()
		for filter in used_filters:
			kw[filter] = listfix(kw[filter])
			filter_vals = intersect(kw[filter],fdata[filter])
			print filter_vals
			filters = fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		#get columns and column transformations for this view
		y_xforms = view_data[6].split(',')
		y_data = view_data[2].split(',')
		y_data_str = ""
		#they must be equal!
		assert len(y_data) == len(y_xforms)
		for i in range(len(y_data)):
			y_data_str += y_xforms[i] + "(" + y_data[i] + "),"
		y_data_str = y_data_str[0:len(y_data_str)-1]
		query = "SELECT "+view_data[1]+","+y_data_str+" FROM GAMES "+filters+" GROUP BY "+view_data[1]
		print query
		results = scaled_query(curs,query,100,evaluators.count_eval)
		print results
		#generate JS datafields here because genshi templating can't emit JS...
		data = ""
		for i in range(0,len(results)):
                        data += "data.setValue("+str(i)+",0,'"+results[i][0]+"');\n"
                        data += "data.setValue("+str(i)+",1,"+str(results[i][1])+");\n"
		return dict(title=view_data[0],xlabel=view_data[3],ylabel=view_data[4],piedata=results,
			data=data,filters=available_filters,used_filters=used_filters,
			ufilters_vals=ufilters_vals,fdata=fdata)

class LineGraphController(object):
	def __init__(self,url):
		self.url = url
	
	@expose(template="wesstats.templates.lineview")
	def default(self,**kw):
		#pull data on this view from DB
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME,use_unicode=True)
		curs = conn.cursor()
		
		curs.execute("SELECT title,xdata,ydata,xlabel,ylabel,filters,y_xform FROM _wsviews WHERE url = %s", (self.url,))
		view_data = curs.fetchall()[0]
		print view_data
		#fetch the relevant filters for this template and their possible values
		available_filters = view_data[5].split(',')
		fdata = dict()
		for filter in available_filters:
			curs.execute("SELECT DISTINCT "+filter+" FROM GAMES")
			#curs.fetchall() returns a list of lists, we convert this to a plain list for ease of handling
			raw_fdata = curs.fetchall()
			fdata[filter] = []
			for i in raw_fdata:
				fdata[filter].append(i[0])
		#print fdata
		filters = ""
		used_filters = intersect(kw.keys(),available_filters)
		ufilters_vals = dict()
		for filter in used_filters:
			kw[filter] = listfix(kw[filter])
			filter_vals = intersect(kw[filter],fdata[filter])
			filters = fconstruct(filters,filter,filter_vals)
			ufilters_vals[filter] = filter_vals
		#get columns and column transformations for this view
		y_xforms = view_data[6].split(',')
		y_data = view_data[2].split(',')
		y_data_str = ""
		#they must be equal!
		assert len(y_data) == len(y_xforms)
		for i in range(len(y_data)):
			y_data_str += y_xforms[i] + "(" + y_data[i] + "),"
		y_data_str = y_data_str[0:len(y_data_str)-1]
		query = "SELECT "+view_data[1]+","+y_data_str+" FROM GAMES "+filters+" GROUP BY "+view_data[1]
		print query
		results = scaled_query(curs,query,100,evaluators.count_eval)
		print results
		#generate JS datafields here because genshi templating can't emit JS...
		data = ""
		for i in range(0,len(results)):
                        data += "data.setValue("+str(i)+",0,'"+results[i][0]+"');\n"
                        data += "data.setValue("+str(i)+",1,"+str(results[i][1])+");\n"
		return dict(title=view_data[0],xlabel=view_data[3],ylabel=view_data[4],piedata=results,
			data=data,filters=available_filters,used_filters=used_filters,
			ufilters_vals=ufilters_vals,fdata=fdata)

class NotFoundController(object):
	def __init__(self,url):
		self.url = url
