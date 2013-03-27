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
	conn = MySQLdb.connect(configuration.DB_HOSTNAME,configuration.DB_USERNAME,configuration.DB_PASSWORD,configuration.DB_NAME)
	curs = conn.cursor()

	#find the maximum game_id in the DB
	curs.execute("SELECT MAX(game_id) FROM GAMES")
	results = curs.fetchall()
	max_id = results[0][0]

	#look for an existing table with this sample size and drop it if it exists, then create a new one
	tblname = configuration.DB_TABLE_PREFIX+TBLSTRING+str(size)
	curs.execute("SELECT count(*) FROM information_schema.tables WHERE table_schema = 'corn' AND table_name = %s",(tblname,))
	results = curs.fetchall()
	exists = results[0][0] == 1
	if exists:
		curs.execute("DROP TABLE IF EXISTS "+tblname)
	curs.execute("CREATE TABLE "+tblname+" LIKE GAMES")
	
	#randomly pick size number of entries from the main DB and put them into this sample
	choices = random.sample(range(1,max_id),size)
	for c in choices:
		curs.execute("INSERT INTO "+tblname+" SELECT * FROM GAMES WHERE `game_id`=%s",c)
	conn.close()

sample(10**6) #1 mil row subsample
sample(10**5) #100k row subsample
sample(10**4) #10k row subsample
