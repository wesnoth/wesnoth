#!/usr/bin/env python2
from subprocess import Popen, PIPE
from time import clock, time
import datetime
import sqlite3
import ConfigParser
import os
import string
import random
import sys

# Wording conversations:
# We have one 'Test'
# One 'Test' includes multiple 'Executions'
# One 'Execution' has one or more 'Games'

class Test:
	ai_config1 = ''
	ai_config2 = ''
	ai_ident1 = ''
	ai_ident2 = ''
	map = ''
	version_string = ''
	faction1 = ''
	faction2 = ''
	time = ''
	test_id = 0
	title = ''
	game_results = []

	def __init__(self, _ai_config1, _ai_config2, _faction1, _faction2, _map, _title):
		self.ai_config1 = _ai_config1
		self.ai_config2 = _ai_config2
		self.faction1 = _faction1
		self.faction2 = _faction2
		self.map = _map
		self.title = _title

		today = datetime.datetime.now()
		self.time = today.strftime('%Y-%m-%d  %H:%M')

def filter_non_printable(str):
	return ''.join(c for c in str if ord(c) > 31 or ord(c) == 9)

def construct_command_line(cfg, test, switched_side):
	wesnoth = cfg.get('default', 'path_to_wesnoth_binary')
	options = cfg.get('default', 'additional_arguments')
	repeats = cfg.getint('default', 'repeat')
	repeats_param = '--multiplayer-repeat ' + str(repeats)
	if repeats > 1:
		print 'Be patient, ' + str(repeats) + ' repeats are going to take a while.'

	side1 = test.ai_config1 if not switched_side else test.ai_config2
	side2 = test.ai_config2 if not switched_side else test.ai_config1
	faction1 = test.faction1 if not switched_side else test.faction2
	faction2 = test.faction2 if not switched_side else test.faction1
	ai_param1 = '--ai-config 1:' + side1 if side1 else ''
	ai_param2 = '--ai-config 2:' + side2 if side2 else ''
	faction_param1 = '--side 1:"' + faction1 + '"' if faction1 else ''
	faction_param2 = '--side 2:"' + faction2 + '"' if faction2 else ''
	map_param = '--scenario=' + test.map if test.map else ''

	if len(sys.argv) > 1 and sys.argv[1] == '-p':
		gui = '--debug'
	else:
		gui = '--nogui'

	statics = '--log-info=ai/testing,mp/connect/engine --multiplayer'
	return (wesnoth + ' ' + options + ' ' + map_param + ' ' + ai_param1 + ' ' + ai_param2 + 
		' ' + faction_param1 + ' ' + faction_param2 + ' ' + gui + ' ' + repeats_param + ' ' + statics)

def do_filter(str, substring):
	n = str.find(substring)
	if (n > -1):
		temp = str[n + len(substring):].strip()
		c = temp.find(',')
		if (c > -1):
			return n, temp[:c].strip()
		else:
			return n, temp
	return n, ''

def run_game(cfg, test, switched_side):
	command_line = construct_command_line(cfg, test, switched_side)
	print 'Running: ' + command_line

	game_results = []
	game_result = None
	faction1 = ''
	faction2 = ''
	debugout = ''

	p = Popen(command_line, shell=True, bufsize=10000000, stdout=PIPE, stderr=PIPE)

	for line in p.stderr:
		l = filter_non_printable(line.strip())
		debugout += l + '\n'

		n, s = do_filter(l , 'side 1: faction=')
		if (n > -1):
			faction1 = s
			continue

		n, s = do_filter(l , 'side 2: faction=')
		if (n > -1):
			faction2 = s
			continue

		n, s = do_filter(l , 'info ai/testing: VERSION:')
		if (n > -1):
			test.version_string = s
			continue

		n, s = do_filter(l , 'info ai/testing: AI_IDENTIFIER1:')
		if(n > -1):
			if switched_side:
				test.ai_ident2 = s
			else:
				test.ai_ident1 = s

			# this is the first line of a game.
			# We'll do some initializations here.

			game_result = {}
			game_result['switched_side'] = switched_side
			game_result['is_success'] = False
			continue

		n, s = do_filter(l , 'info ai/testing: AI_IDENTIFIER2:')
		if(n > -1):
			if switched_side:
				test.ai_ident1 = s
			else:
				test.ai_ident2 = s
			continue

		n, s = do_filter(l , 'info ai/testing: WINNER:')
		if (n > -1):
			if (int(s) == 1) != switched_side:
				winner = 1
			else:
				winner = 2
			print 'AND THE WINNER IS: ' + str(winner)
			if game_result.has_key('winner'):
				game_result['is_success'] = False
				break
			game_result['winner'] = winner
			game_result['is_success'] = True
			continue

		n, s = do_filter(l , 'info ai/testing: DRAW:')
		if(n > -1):
			print 'AND THE WINNER IS: DRAW'
			if game_result.has_key('winner'):
				game_result['is_success'] = False
				break
			game_result['winner'] = 0
			game_result['is_success'] = True
			continue

		n, s = do_filter(l , 'info ai/testing: GAME_END_TURN:')
		if (n > -1):
			# this is the last line printed in a game
			# so we do some checking here and adding
			# game_result to game_results.

			print 'AND THE VICTORY_TURN IS: ' + s

			if game_result.has_key('end_turn'):
				game_result['is_success'] = False
				break

			game_result['end_turn'] = int(s)
			game_result['faction1'] = faction1 if not switched_side else faction2
			game_result['faction2'] = faction2 if not switched_side else faction1

			if not game_result['is_success']:
				print_error(debugout)

			game_results.append(game_result)
			continue

		n, s = do_filter(l , 'error')
		if(n > -1):
			# forward errors from stderr.
			print 'stderr give: error ' + s
			continue


	if game_result is None or not game_result['is_success']:
		print_error(debugout)
	return game_results

def print_error(debugout):
	print 'Warning: not success!'
	print '===================='
	print 'stderr:'
	print debugout
	print '===================='

def save_result_logfile(cfg, test, game_result, log_file):
	print 'Saving to log file....'
	log_file.write('"' + test.ai_config1 + '", "' +
			test.ai_config2 + '", "' +
			test.ai_ident1 + '", "' +
			test.ai_ident2 + '", "' +
			str(game_result['switched_side']) + '", "' +
			game_result['faction1'] + '", "' +
			game_result['faction2'] + '", "' +
			str(game_result['is_success']) + '", "' +
			test.map + '", "' +
			str(game_result['end_turn']) + '", "' +
			str(test.version_string) + '", "' +
			str(game_result['winner']) + '"\n');

	log_file.flush();
	print 'Saved to log file'

def save_result_database(cfg, test, game_result, sqlite_file):
	print 'Saving to DB....'
	query = ('INSERT INTO games("test_id","faction1","faction2","switched_side","is_success","end_turn","winner")' + 
			'VALUES (?,?,?,?,?,?,?)')

	conn = sqlite3.connect(sqlite_file)
	cur = conn.cursor()
	cur.execute(query, (
		test.test_id,
		game_result['faction1'],
		game_result['faction2'],
		game_result['switched_side'],
		game_result['is_success'],
		game_result['end_turn'],
		game_result['winner']))
	conn.commit()
	conn.close()
	print 'Saved to DB'

def executions(cfg, test):
	structured = cfg.getboolean('default', 'structured_test')
	if structured:
		factions = ['Drakes', 'Rebels', 'Undead', 'Northerners', 'Knalgan Alliance', 'Loyalists']
		i = 1
		for faction1 in factions:
			for faction2 in factions:
				print 'EXECUTION: ' + str(i) + '/36 --- ' + faction1 + ' against ' + faction2
				test.faction1 = faction1
				test.faction2 = faction2
				game_results = run_game(cfg, test, False)
				yield game_results
				i = i + 1
		test.faction1 = 'structured'
		test.faction2 = 'structured'
	else:
		n = cfg.getint('default', 'number_of_tests')
		randomize = cfg.getboolean('default', 'randomize_sides')
		random.seed()

		for i in range(0, n):
			switched_side = (random.randint(0, 1) == 1) if randomize else False
			print 'EXECUTION ' + str(i + 1)
			game_results = run_game(cfg, test, switched_side)
			yield game_results

		if test.faction1 == '':
			test.faction1 = 'random'
		if test.faction2 == '':
			test.faction2 = 'random'

# main

cfg = ConfigParser.ConfigParser()
cfg.read('ai_test.cfg')

ai1 = cfg.get('default', 'ai_config1').strip()
ai2 = cfg.get('default', 'ai_config2').strip()
faction1 = cfg.get('default', 'faction1').strip()
faction2 = cfg.get('default', 'faction2').strip()
map = cfg.get('default', 'map').strip()
if cfg.has_option('default', 'title'):
	title = cfg.get('default', 'title')
else:
	title = ''

test = Test(ai1, ai2, faction1, faction2, map, title)

# only 'test the test' with GUI / start one game then exit
if len(sys.argv) > 1 and sys.argv[1] == '-p':
	executions(cfg, test).next()
	sys.exit(0)

log_file = None
if cfg.has_option('default', 'log_file'):
	log_file = open(datetime.datetime.now().strftime(cfg.get('default', 'log_file').strip())  , 'w')
	log_file.write('"ai_config1", "ai_config2", "ai_ident1", "ai_ident2", "switched_side", ' +
			'"faction1", "faction2", "is_success", "local_modifications", ' +
			'"map", "repo_release", "end_turn", "version_string", "winner_side"\n');
	log_file.flush();
sqlite_file = None
if cfg.has_option('default', 'sqlite_file'):
	sqlite_file = cfg.get('default', 'sqlite_file')
	conn = sqlite3.connect(sqlite_file)
	cur = conn.cursor()
	cur.execute('INSERT INTO tests ("title","ai_config1","ai_config2","map","time") ' +
			'VALUES ("' + test.title + '","' + test.ai_config1 + '","' + test.ai_config2 + '","' +
					test.map + '","' + test.time + '")')
	test.test_id = cur.lastrowid
	conn.commit()
	conn.close();

# the following variables are for generating a print output only
total = 0
ai1_won = 0
ai2_won = 0
draw = 0

for game_results in executions(cfg, test):
	for game_result in game_results:
		if log_file:
			save_result_logfile(cfg, test, game_result, log_file)
		if sqlite_file:
			save_result_database(cfg, test, game_result, sqlite_file)

		total = total + 1
		if game_result['winner'] == 0:
			draw = draw + 1
		elif game_result['winner'] == 1:
			ai1_won = ai1_won + 1
		elif game_result['winner'] == 2:
			ai2_won = ai2_won + 1
	print '\n=====Status====='
	print 'Total games: ' + str(total)
	print 'AI1(' + ai1 + ') won: ' + str(ai1_won) + "/" + str(ai1_won * 100 / total) + '%'
	print 'AI2(' + ai2 + ') won: ' + str(ai2_won) + "/" + str(ai2_won * 100 / total) + '%'
	print 'Draws: ' + str(draw) + "/" + str(draw * 100 / total) + '%\n'
if sqlite_file:
	conn = sqlite3.connect(sqlite_file)
	cur = conn.cursor()
	query = "UPDATE tests SET ai_ident1 = ?, ai_ident2 = ?, version = ? , faction1 = ?, faction2 = ? WHERE id = ?;"
	cur.execute(query, (test.ai_ident1,
				test.ai_ident2,
				test.version_string,
				test.faction1,
				test.faction2,
				test.test_id))
	conn.commit()
	conn.close();
