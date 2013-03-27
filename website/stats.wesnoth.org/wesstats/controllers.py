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

import turbogears as tg
import MySQLdb
import types
import configuration
import evaluators
import time

from turbogears import controllers, expose, flash
# from wesstats import model
from turbogears import identity, redirect
from cherrypy import request, response
# from wesstats import json
# import logging
# log = logging.getLogger("wesstats.controllers")

class Root(controllers.RootController):
	@expose(template="wesstats.templates.index")
	def index(self):
		return dict()
	@expose(template="wesstats.templates.index_new")
	def index2(self):
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME)
		curs = conn.cursor()
		curs.execute("SELECT title FROM _wsviews")
		views = curs.fetchall()
		conn.close()
		return dict(views=views)

	def listfix(self,l):
		if not isinstance(l,types.ListType):
			return [l]
		return l

	def scaled_query(self,curs,query,threshold,evaluator):
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
			print nquery
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

	def fconstruct(self,filters,colname,list):
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
		
	@expose(template="wesstats.templates.platform")
	def platform(self, versions=["all"], **kw):	
		#hack to work around how a single value in a GET is interpreted
		versions = Root.listfix(self,versions)
		
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME)
		curs = conn.cursor()
		filters = ""

		filters = Root.fconstruct(self,filters,"version",versions)
		query = "SELECT platform, COUNT(platform) FROM GAMES "+filters+" GROUP BY platform"
		results = Root.scaled_query(self,curs,query,100,evaluators.count_eval)
		
		total = 0
		for result in results:
			total += result[1]
		#we process part of the IMG string here because it's a pain in KID templating
		chartdata = ""
		chartnames = ""
		for result in results:
			value = (100.0*result[1])/total
			chartdata += str(value) + ","
			if len(result[0]) == 0:
				chartnames += "unspecified" + " " + str(value) + "%|"
			else:
				chartnames += result[0] + " " + str(value) + "%|"
		chartdata = chartdata[0:len(chartdata)-1] 
		chartnames = chartnames[0:len(chartnames)-1] 

		curs.execute("SELECT DISTINCT version FROM GAMES")
		vlist = curs.fetchall()
		
		conn.close()
		return dict(chd=chartdata,chn=chartnames,vers=versions,vlist=vlist,total=total)

	@expose(template="wesstats.templates.breakdown")
	def breakdown(self, campaigns=["all"], diffs=["all"], versions=["all"], scens=["all"], **kw):
		
		#hack to work around how a single value in a GET is interpreted
		campaigns = Root.listfix(self,campaigns)
		versions = Root.listfix(self,versions)
		diffs = Root.listfix(self,diffs)
		scens = Root.listfix(self,scens)
		
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME)
		curs = conn.cursor()
	
		filters = ""

		filters = Root.fconstruct(self,filters,"campaign",campaigns)
		filters = Root.fconstruct(self,filters,"difficulty",diffs)
		filters = Root.fconstruct(self,filters,"version",versions)
		filters = Root.fconstruct(self,filters,"scenario",scens)

		query = "SELECT result, COUNT(result) FROM GAMES " + filters + " GROUP BY result"
		#curs.execute("SELECT result, COUNT(result) FROM GAMES " + filters + " GROUP BY result")
		results = Root.scaled_query(self,curs,query,100,evaluators.count_eval)
		total = 0
		for result in results:
			total += result[1]
		#we process part of the IMG string here because it's a pain in KID templating
		chartdata = ""
		chartnames = ""
		for result in results:
			value = (100.0*result[1])/total
			chartdata += str(value) + ","
			chartnames += result[0] + " " + str(value) + "%|"
		chartdata = chartdata[0:len(chartdata)-1] 
		chartnames = chartnames[0:len(chartnames)-1] 

		curs.execute("SELECT DISTINCT campaign FROM GAMES")
		clist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT difficulty FROM GAMES")
		dlist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT version FROM GAMES")
		vlist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT scenario FROM GAMES")
		slist = curs.fetchall()
		
		conn.close()
		return dict(chd=chartdata,chn=chartnames,camps=campaigns,diffs=diffs,vers=versions,scens=scens,clist=clist, dlist=dlist, vlist=vlist, slist=slist,total=total)

	def barparse(self,parts,min,max,results):
		#divide the data into 3 ranges
		width = (max-min)/parts
		count = []
		for i in range(parts):
			count.append(0)
		for i in range(parts):
			curmin = min + (i*width) + 1
			if i == 0:
				curmin -= 1
			curmax = min + ((i+1)*width)
			#we overcount slightly because the intervals overlap
			for result in results:
				if result[0] >= curmin and result[0] <= curmax:
					count[i] += 1
		chd = ""
		chxl = "0:|"
		for i in range(parts):
			chd += str(count[i]) + ","
			chxl += str(min+(i*width)+1)+"+to+"+str(min+((i+1)*width))+"|"
		chd = chd[0:len(chd)-1]
		return [chd,chxl]

	@expose(template="wesstats.templates.gold")
	def gold(self, campaigns=["all"], diffs=["all"], versions=["all"], scens=["all"], gresults=["all"], granularity="3bar", **kw):
		
		#hack to work around how a single value in a GET is interpreted
		campaigns = Root.listfix(self,campaigns)
		versions = Root.listfix(self,versions)
		diffs = Root.listfix(self,diffs)
		scens = Root.listfix(self,scens)
		gresults = Root.listfix(self,gresults)
		
		conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME)
		curs = conn.cursor()
	
		filters = ""

		filters = Root.fconstruct(self,filters,"campaign",campaigns)
		filters = Root.fconstruct(self,filters,"difficulty",diffs)
		filters = Root.fconstruct(self,filters,"version",versions)
		filters = Root.fconstruct(self,filters,"scenario",scens)
		filters = Root.fconstruct(self,filters,"result",gresults)
		
		curs.execute("SELECT gold, COUNT(gold) FROM GAMES " + filters + " GROUP BY gold")
		results = curs.fetchall()
		
		results2 = []
		goldlist = []
		vallist = []
		for result in results:
			if result[0] > -1000 and result[0] < 1000:
				results2.append(result)
				goldlist.append(result[0])
				vallist.append(result[1])
		results = results2
		total = len(results)
		
		goldlist.sort()
		vallist.sort()
		max = goldlist[-1]
		min = goldlist[0]
		maxval = vallist[-1]
		minval = vallist[0]	
	
		if granularity == "3bar":
			cdata = Root.barparse(self,3,min,max,results)
		elif granularity == "5bar":
			cdata = Root.barparse(self,5,min,max,results)
		elif granularity == "7bar":
			cdata = Root.barparse(self,7,min,max,results)
		else:
			cdata = Root.barparse(self,5,min,max,results)
		chd = cdata[0]
		chxl = cdata[1]

		curs.execute("SELECT DISTINCT campaign FROM GAMES")
		clist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT difficulty FROM GAMES")
		dlist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT version FROM GAMES")
		vlist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT scenario FROM GAMES")
		slist = curs.fetchall()
		
		curs.execute("SELECT DISTINCT result FROM GAMES")
		rlist = curs.fetchall()
		
		conn.close()
		return dict(chd=chd,chxl=chxl,camps=campaigns,diffs=diffs,results=gresults,
			vers=versions,scens=scens,clist=clist,minval=minval,maxval=maxval,
			dlist=dlist,vlist=vlist,slist=slist,rlist=rlist,total=total)

	@expose()
	def default(self, *args, **kw):
		return "WIP"
