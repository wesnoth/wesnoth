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

import MySQLdb
import random

import configuration

TBLSTRING = "SMPL"

def sample(size):
	conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_WRITE_USERNAME,configuration.DB_WRITE_PASSWORD,configuration.DB_NAME)
	curs = conn.cursor()

	curs.execute("SELECT DISTINCT `table` FROM _wsviews")
	results = curs.fetchall()
	
	for tbl in results:
		tbl = tbl[0]
		
		sample_tbl = "%s%s%s%d" % (configuration.DB_TABLE_PREFIX,TBLSTRING,tbl,size)
		curs.execute("DROP TABLE IF EXISTS %s" % (sample_tbl))
		curs.execute("CREATE TABLE %s LIKE %s" % (sample_tbl,tbl))
		
		curs.execute("SELECT MAX(game_id) FROM %s" % (tbl,)) #we assume game_id is always the primary key
		results = curs.fetchall()
		max_id = int(results[0][0])

		if max_id < size:
			size = max_id

		choices = random.sample(range(0,max_id),size)
		inserts = 0
		for c in choices:
			curs.execute("SELECT * FROM %s WHERE game_id=%d" % (tbl,c))
			results = curs.fetchall()
			if len(results) != 0:
				query = "INSERT INTO %s SELECT * FROM %s WHERE game_id=%d" % (sample_tbl,tbl,c)
				curs.execute(query)
	conn.commit()
	conn.close()

sample(10000)
sample(100000)
