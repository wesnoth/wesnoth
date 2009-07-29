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
	curs.execute("SELECT count(*) FROM information_schema.tables WHERE table_schema = 'corn' AND table_name = '"+tblname+"'")
	results = curs.fetchall()
	exists = results[0][0] == 1
	if exists:
		curs.execute("DROP TABLE IF EXISTS "+tblname)
	curs.execute("""
	CREATE TABLE `"""+tblname+"""` (
	  `game_id` int(11) NOT NULL auto_increment,
	  `timestamp` datetime NOT NULL,
	  `user_id` char(32) NOT NULL,
	  `serial` char(18) NOT NULL,
	  `platform` char(8) default NULL,
	  `version` char(14) default NULL,
	  `campaign` char(40) default NULL,
	  `difficulty` char(20) default NULL,
	  `gold` int(11) default NULL,
	  `turns` int(11) default NULL,
	  `scenario` char(40) default NULL,
	  `start_turn` int(11) default NULL,
	  `time` int(11) default NULL,
	  `result` enum('victory','defeat','quit') default NULL,
	  `end_time` int(11) default NULL,
	  `end_gold` int(11) default NULL,
	  `end_turn` int(11) default NULL,
	  PRIMARY KEY  (`game_id`)
	) ENGINE=MyISAM AUTO_INCREMENT=2450740 DEFAULT CHARSET=utf8 """)
	
	#randomly pick size number of entries from the main DB and put them into this sample
	choices = random.sample(range(1,max_id),size)
	for c in choices:
		curs.execute("SELECT * FROM GAMES WHERE `game_id`=%s",c)
		results = curs.fetchall()
		if len(results) != 0:
			#print results[0]
			curs.execute("""INSERT INTO %s (game_id,timestamp,user_id,serial,platform,version,
				campaign,difficulty,gold,turns,scenario,start_turn,time,result,end_time,end_gold,end_turn) VALUES(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)""",results[0])
	conn.close()

sample(100000)
