#!/usr/bin/env python
from subprocess import Popen,PIPE
from time import clock, time
from pyPgSQL import PgSQL
import ConfigParser
import os
import string
import random

class GameResult:
	ai_config1 = ''
	ai_config2 = ''
	ai_ident1 = ''
	ai_ident2 = ''
	duration = '0'
	faction1 = ''
	faction2 = ''
	is_success = 'false'
	local_modifications = 'false'
	map = ''
	svn_release = '0'
	test = 'default'
	end_turn = '0'
	version_string = ''
	winner_side = '0'

	def __init__(self,_ai_config1,_ai_config2,_faction1,_faction2,_map,_test):
		self.ai_config1 = _ai_config1
		self.ai_config2 = _ai_config2
		self.faction1 = _faction1
		self.faction2 = _faction2
		self.map = _map
		self.test = _test

def filter_non_printable(str):
	return ''.join(c for c in str if ord(c) > 31 or ord(c) == 9)

def construct_command_line(cfg,ai1,ai2,f1,f2,map):
	wesnoth = cfg.get('default','path_to_wesnoth_binary')
	options= cfg.get('default','arguments_to_wesnoth_binary')
	ai_config1='--ai_config1='+ai1
	ai_config2='--ai_config2='+ai2
	if (map==''):
		optmap=''
	else:
		optmap='--scenario='+map
		
	return wesnoth+' '+options+' '+optmap+' '+ai_config1+' '+ai_config2

def do_filter(str,substring):
	n = str.find(substring)
	if (n>-1):
		return n,str[n+len(substring):].strip()
	return n,''

def run_game(cfg,game_result):

	command_line = construct_command_line(cfg,game_result.ai_config1,game_result.ai_config2, game_result.faction1, game_result.faction2, game_result.map)
	print 'Running: '+command_line
	start = time()
	p = Popen(command_line, shell=True, bufsize=10000000, stdout=PIPE, stderr=PIPE)
	# outlines = p.stdout.readlines()
	outerrlines = p.stderr.readlines()
	print 'Finished'
	for line in outerrlines:
		str = filter_non_printable(line.strip())
		n,s = do_filter(str,'info ai/testing: WINNER:')
		if (n>-1):
			#print 'AND THE WINNER IS: '+s
			game_result.winner_side = s
			game_result.is_success = 'true'
			continue

		n,s = do_filter(str,'info ai/testing: VERSION:')
		if (n>-1):
			#print 'AND THE VERSION IS: '+s
			game_result.version_string = s
			n1 = s.rfind('(')
			n2 = s.rfind(')')
			if ((n1>-1) and (n2>-1) and (n2>n1)):
				sz = s[n1+1:n2]
				#parse local_modifications
				n3 = sz.rfind('M')
				if (n3>-1):
					sz = sz[:n3]
					game_result.local_modifications = 1
				#parse svn_release
				game_result.svn_release = sz
			continue

		n,s = do_filter(str,'info ai/testing: GAME_END_TURN:')
		if (n>-1):
			#print 'AND THE VICTORY_TURN IS: '+s
			game_result.end_turn = s
			continue

		n,s = do_filter(str,'info ai/testing: AI_IDENTIFIER1:')
		if (n>-1):
			#print 'AND THE AI_IDENTIFIER1 IS: '+s
			game_result.ai_ident1 = s.strip()
			continue

		n,s = do_filter(str,'info ai/testing: AI_IDENTIFIER2:')
		if (n>-1):
			#print 'AND THE AI_IDENTIFIER2 IS: '+s
			game_result.ai_ident2 = s.strip()
			continue

		n,s = do_filter(str,'info ai/testing: FACTION1:')
		if (n>-1):
			#print 'AND THE FACTION1 IS: '+s
			game_result.faction1 = s
			continue

		n,s = do_filter(str,'info ai/testing: FACTION2:')
		if (n>-1):
			#print 'AND THE FACTION2 IS: '+s
			game_result.faction2 = s
			continue

	game_result.duration = time() - start
	if (game_result.is_success=='false'):
		print 'Warning: not success!'
		print '===================='
		print 'stderr:'
		for line in outerrlines:
			print filter_non_printable(line.strip())
		print '===================='

	return game_result

def save_result(cfg,game_result):
	print 'Saving to DB....'
	query = 'insert into game(ai_config1,ai_config2,ai_ident1,ai_ident2,duration,faction1,faction2,is_success,local_modifications,map,svn_release,test,end_turn,version_string,winner_side) values (%s,%s,%s,%s,cast(%s as double precision),%s,%s,cast(%s as boolean),cast(%s as boolean),%s,cast(%s as int),%s,cast(%s as int),%s,cast(%s as int))'
	db_ip = cfg.get('default','db_ip')
	db_port = cfg.getint('default','db_port')
	db_name = cfg.get('default','db_name')
	db_user = cfg.get('default','db_user')
	db_pass = cfg.get('default','db_pass')

	dbconnection = PgSQL.connect(database=db_name,host=db_ip,port=db_port,user=db_user,password=db_pass)
	cu = dbconnection.cursor()
	cu.execute(query, game_result.ai_config1, game_result.ai_config2, game_result.ai_ident1, game_result.ai_ident2, game_result.duration, game_result.faction1, game_result.faction2, game_result.is_success, game_result.local_modifications, game_result.map, game_result.svn_release, game_result.test, game_result.end_turn, game_result.version_string, game_result.winner_side)
	cu.execute('commit')
	dbconnection.close()
	print 'Saved to DB'

def maps(cfg):
	mp = 1
	while 1:
		try:
			yield cfg.get('default','map'+`mp`);
			mp= mp+1
		except:
			return

def tests(cfg):
	ai1=cfg.get('default','ai_config1').strip()
	ai2=cfg.get('default','ai_config2').strip()
	f1=cfg.get('default','faction1').strip()
	f2=cfg.get('default','faction2').strip()
	n=cfg.getint('default','number_of_tests')
	maplist = []
	for map in maps(cfg):
		maplist.append(map)
	random.seed()
	for i in range(0, n):
		map = random.choice(maplist)
		d = random.randint(0,1)
		print 'TEST: map '+map+' i='+str(i)+' d='+str(d)
		if (d==0):
			game_result = GameResult(ai1,ai2,f1,f2,map,'default')
		else:
			game_result = GameResult(ai2,ai1,f2,f1,map,'default')
		yield game_result

# main

cfg = ConfigParser.ConfigParser()
cfg.read('ai_test.cfg')

for test in tests(cfg):
	game_result = run_game(cfg,test)
	save_result(cfg,game_result)
