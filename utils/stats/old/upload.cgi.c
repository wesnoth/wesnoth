#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>

#include "database.h"
#include "utils.h"

#define MAX_LOGS 10

int log_fd = STDERR_FILENO;

static const char *database_file(void)
{
	return getenv("TESTING_DATABASE") ?:
		"/home/rusty/wesnoth/wesnoth-uploads.db";
}

static const char *logfile_prefix(void)
{
	return getenv("TESTING_LOGFILE") ?:
		"/home/rusty/wesnoth/wesnoth-upload-log.";
}

/* We open a log file, and delete if if all goes well.  We
 * keep up a max of MAX_LOGS files, to avoid filling disk. */
static void maybe_log_to_file(char **filename)
{
	char name[strlen(logfile_prefix()) + sizeof(__stringify(MAX_LOGS))];
	unsigned int i;

	for (i = 0; i < MAX_LOGS; i++) {
		struct stat st;
		sprintf(name, "%s%u", logfile_prefix(), i);
		if (lstat(name, &st) != 0) {
			log_fd = open(name, O_WRONLY|O_CREAT|O_EXCL, 0640);
			if (log_fd >= 0) {
				*filename = strdup(name);
				return;
			}
		}
	}
	*filename = NULL;
	log_fd = open("/dev/null", O_WRONLY);
	if (log_fd < 0)
		barf_perror("Could not open /dev/null for logging");
}	

struct wml
{
	const char *name;
	unsigned int num_keys;
	const char **keyval;

	unsigned int num_children;
	struct wml **child;
};

static struct wml *new_wml(const char *name)
{
	struct wml *wml = new(struct wml);
	wml->name = name;
	wml->num_keys = wml->num_children = 0;
	wml->keyval = NULL;
	wml->child = NULL;
	return wml;
}

/* Deliberately simple parser (doesn't handle comments, for example).
 * We want canonical forms so we can enter into database.
 */
static char *get_line(char **string)
{
	char *start, *first_quote = NULL;

	/* Ignore whitespace. */
	while (isspace(**string))
		(*string)++;

	if (**string == '\0')
		return NULL;

	start = *string;
	while (**string != '\n') {
		switch (**string) {
		case '\0':
			barf("Unexpected end of input");
		case '+':
			if (!first_quote)
				barf("Cannot handle '+'");
			break;
		case '"':
			if (!first_quote)
				first_quote = *string;
			else {
				/* Only accept close of string then \n */
				if ((*string)[1] != '\n')
					barf("String must end of end of line");
			}
			break;
		default:
			break;
		}
		(*string)++;
	}
	if (first_quote) {
		/* Trim both quotes. */
		if ((*string)[-1] != '"')
			barf("Incomplete string");
		(*string)[-1] = '\0';
		memmove(first_quote, first_quote+1, strlen(first_quote));
	} else
		**string = '\0';

	(*string)++;
	return start;
}

static void wml_add(struct wml *wml, const char *line)
{
	wml->keyval = realloc_array(wml->keyval, wml->num_keys+1);
	wml->keyval[wml->num_keys++] = line;
}

static void wml_add_child(struct wml *wml, struct wml *child)
{
	wml->child = realloc_array(wml->child, wml->num_children+1);
	wml->child[wml->num_children++] = child;
}

#if 0 /* Unused */
static void wml_replace(struct wml *wml, const char *key, const char *value)
{
	unsigned int i, len = strlen(key);

	for (i = 0; i < wml->num_keys; i++) {
		if (memcmp(wml->keyval[i], key, len) == 0
		    && wml->keyval[i][len] == '=') {
			wml->keyval[i] = aprintf("%s=%s", key, value);
			return;
		}
	}
	barf("Could not find key '%s' to replace", key);
}
#endif

static struct wml *parse_data(char **string, const char *name)
{
	char *line;
	char end[2 + strlen(name ? name : "") + 1 + 1];
	struct wml *wml = new_wml(name);

	if (name)
		sprintf(end, "[/%s]", name);
	else
		end[0] = '\0';

	for (;;) {
		line = get_line(string);
		if (!line) {
			/* Only the top element can terminate this way. */
			if (!name)
				return wml;
			barf("Unexpected end of file during [%s]", name);
		}

		/* The end? */
		if (streq(line, end))
			return wml;

		/* Child? */
		if (*line == '[') {
			struct wml *child;
			if (line[1] == '/' || !strends(line, "]"))
				barf("Malformed line '%s' during [%s]",
				     line, name);

			line[strlen(line) - 1] = '\0';
			child = parse_data(string, line+1);
			wml_add_child(wml, child);
		} else {
			char *eq = strchr(line, '=');
			if (!eq)
				barf("Expected '=' in '%s'", line);
			wml_add(wml, line);
		}
	}
}

static bool write_all(int fd, const void *data, unsigned int len)
{
	while (len) {
		int done;

		done = write(fd, data, len);
		if (done < 0 && errno == EINTR)
			continue;
		if (done <= 0)
			return false;
		data += done;
		len -= done;
	}

	return true;
}

/* Simple parser for Wesnoth Markup Language. */
static struct wml *parse(int fd)
{
	unsigned long size;
	char *data = grab_input(fd, &size);

	logp("Content-length: %lu\n", size);
	if (!write_all(log_fd, data, size))
		barf("Could not write input to log");
	/* No NULs please */
	if (strlen(data) != size)
		barf("Contained embedded NUL chars");
	logp("=== END INPUT ===");
	return parse_data(&data, NULL);
}

/* For debygging */
void dump(const struct wml *wml, unsigned int level);
void dump(const struct wml *wml, unsigned int level)
{
	unsigned int i;

	char indent[level+1];
	memset(indent, '\t', level);
	indent[level] = '\0';

	for (i = 0; i < wml->num_keys; i++)
		printf("%s%s\n", indent, wml->keyval[i]);
	for (i = 0; i < wml->num_children; i++) {
		printf("%s[%s]\n", indent, wml->child[i]->name);
		dump(wml->child[i], level+1);
	}
}

#define INTEGER 1
#define TEXT 2
#define NAME_REFERENCE 3
#define UNIQUE_TEXT 4
#define UNIQUE_INTEGER 5

/* We don't put common strings into tables, but instead use a reference into
 * a table of names. */
static void __attribute__((sentinel)) 
create_table(void *h, const char *tablename, ...)
{
	va_list ap;
	const char *name;
	char sep = '(';
	char *cmd = aprintf("CREATE TABLE \"%s\" ", tablename);

	va_start(ap, tablename);
	while ((name = va_arg(ap, const char *)) != NULL) {
		switch (va_arg(ap, int)) {
		case INTEGER:
			cmd = aprintf_add(cmd, "%c%s INTEGER", sep, name);
			break;
		case UNIQUE_INTEGER:
			cmd = aprintf_add(cmd, "%c%s INTEGER UNIQUE",
					  sep, name);
			break;
		case TEXT:
			cmd = aprintf_add(cmd, "%c%s TEXT", sep, name);
			break;
		case UNIQUE_TEXT:
			cmd = aprintf_add(cmd, "%c%s TEXT UNIQUE",
					  sep, name);
			break;
		case NAME_REFERENCE: {
			char tblname[strlen(name)+1];
			cmd = aprintf_add(cmd, "%c%s INTEGER", sep, name);
			memcpy(tblname, name, strlen(name) - strlen("_ref"));
			tblname[strlen(name) - strlen("_ref")] = '\0';
			strcat(tblname, "s");
			create_table(h, tblname, "name", UNIQUE_TEXT, NULL);
			break;
		}
		default:
			barf("Unexpected type in create_table");
		}
		sep = ',';
	}
	va_end(ap);

	cmd = aprintf_add(cmd, ");");
	db_command(h, cmd);
}

static void create_tables(void *h)
{
	do {
		db_transaction_start(h);
		create_table(h, "players",
			     "id", UNIQUE_TEXT,
			     NULL);
		create_table(h, "game_count",
			     "player_ref", UNIQUE_INTEGER,
			     "games_received", INTEGER,
			     "last_upload", TEXT,
			     NULL);
		create_table(h, "campaigns",
			     "player_ref", INTEGER,
			     "campaign_name_ref", NAME_REFERENCE,
			     "difficulty_name_ref", NAME_REFERENCE,
			     NULL);
		create_table(h, "scenarios",
			     "campaign_ref", INTEGER,
			     "scenario_name_ref", NAME_REFERENCE,
			     NULL);
		create_table(h, "games",
			     "scenario_ref", INTEGER,
			     "start_turn", INTEGER,
			     "gold", INTEGER,
			     "start_time", INTEGER,
			     "version_name_ref", NAME_REFERENCE,
			     "game_number", INTEGER,
			     /* 0 = unknown/quit, 1 = victory, 2 = defeat */
			     "result", INTEGER,
			     "end_time", INTEGER,
			     "end_turn", INTEGER,
			     "num_turns", INTEGER,
			     /* This is only set on victory. */
			     "end_gold", INTEGER,
			     NULL);
		create_table(h, "special_units",
			     "game_ref", INTEGER,
			     "unit_name_ref", NAME_REFERENCE,
			     "level", INTEGER,
			     "experience", INTEGER,
			     NULL);
		create_table(h, "unit_types",
			     "name", TEXT,
			     "level", INTEGER,
			     NULL);
		create_table(h, "unit_tallies",
			     "game_ref", INTEGER,
			     "unit_type_ref", INTEGER,
			     "count", INTEGER,
			     NULL);
		create_table(h, "serial",
			     "id", UNIQUE_TEXT,
			     NULL);
		create_table(h, "bad_serial",
			     "id", UNIQUE_TEXT,
			     NULL);
		db_command(h, "CREATE VIEW campaign_view AS SELECT games.rowid AS game,games.end_time-games.start_time AS time,scenario_names.name AS scenario,games.result AS result,games.start_turn AS start_turn,games.end_turn AS end_turn,games.num_turns AS num_turns,games.gold AS gold,players.id AS player,campaign_names.name AS campaign,difficulty_names.name AS difficulty,version_names.name AS version FROM games INNER JOIN scenarios ON games.scenario_ref = scenarios.rowid INNER JOIN campaigns ON scenarios.campaign_ref = campaigns.rowid INNER JOIN difficulty_names ON campaigns.difficulty_name_ref = difficulty_names.rowid INNER JOIN version_names ON games.version_name_ref = version_names.rowid INNER JOIN scenario_names ON scenarios.scenario_name_ref = scenario_names.rowid INNER JOIN campaign_names ON campaigns.campaign_name_ref = campaign_names.rowid INNER JOIN players ON players.rowid = campaigns.player_ref;");
		db_command(h, "CREATE VIEW units_view AS SELECT scenario_names.name AS scenario,games.rowid AS game,unit_tallies.count AS count,unit_types.level AS level,players.id AS player,campaign_names.name AS campaign,difficulty_names.name AS difficulty,version_names.name AS version, games.start_turn AS start_turn FROM games INNER JOIN scenarios ON games.scenario_ref = scenarios.rowid INNER JOIN campaigns ON scenarios.campaign_ref = campaigns.rowid INNER JOIN difficulty_names ON campaigns.difficulty_name_ref = difficulty_names.rowid INNER JOIN version_names ON games.version_name_ref = version_names.rowid INNER JOIN unit_tallies ON games.rowid = unit_tallies.game_ref INNER JOIN unit_types ON unit_types.rowid = unit_tallies.unit_type_ref INNER JOIN scenario_names ON scenarios.scenario_name_ref = scenario_names.rowid INNER JOIN campaign_names ON campaigns.campaign_name_ref = campaign_names.rowid INNER JOIN players ON players.rowid = campaigns.player_ref;");
		db_command(h, "CREATE UNIQUE INDEX unit_tallies_idx ON unit_tallies (game_ref, unit_type_ref, count);");
	} while (!db_transaction_finish(h));
}

static const char *get_maybe(const struct wml *wml, const char *key)
{
	unsigned int i, len = strlen(key);

	for (i = 0; i < wml->num_keys; i++) {
		if (memcmp(wml->keyval[i], key, len) == 0
		    && wml->keyval[i][len] == '=')
			return wml->keyval[i] + len + 1;
	}
	return NULL;
}

/* Sanity check that this really is an int. */
static const char *is_int(const char *val)
{
	char *endp;
	strtoul(val, &endp, 10);
	if (*endp || endp == val)
		barf("Value '%s' is not a valid integer", val);
	return val;
}

static const char *get(const struct wml *wml, const char *key)
{
	const char *ret = get_maybe(wml, key);
	if (ret)
		return ret;

	if (!wml->name)
		barf("Did not find toplevel key '%s'", key);
	barf("Did not find key '%s' in [%s]", key, wml->name);
}

static struct wml *get_child(const struct wml *wml, const char *key)
{
	unsigned int i;
	for (i = 0; i < wml->num_children; i++) {
		if (streq(wml->child[i]->name, key))
			return wml->child[i];
	}
	return NULL;
}

/* Returns NULL or array of columns. */
static char **db_select(void *h, const char *table, const char *key,
			const char *val, ...)
{
	char *cmd;
	const char *col;
	char sep = ' ';
	va_list ap;
	struct db_query *query;

	cmd = aprintf("SELECT");
	va_start(ap, val);
	while ((col = va_arg(ap, const char *)) != NULL) {
		cmd = aprintf_add(cmd, "%c\"%s\"", sep, col);
		sep = ',';
	}
	va_end(ap);
	cmd = aprintf_add(cmd, " FROM \"%s\" WHERE \"%s\" = \"%s\";",
			  table, key, val);
	query = db_query(h, cmd);
	if (query->num_rows > 1)
		barf_perror("Query '%s' returned %i rows",
			    cmd, query->num_rows);
	return query->num_rows ? query->rows[0] : NULL;
}

static char *get_name_ref(void *h, const char *table, const char *name)
{
	char **answer;

	answer = db_select(h, table, "name", name, "ROWID", NULL);
	if (!answer) {
		char *cmd;
		cmd = aprintf("INSERT INTO \"%s\" VALUES(\"%s\");",table,name);
		db_command(h, cmd);
		answer = db_select(h, table, "name", name, "ROWID", NULL);
		if (!answer)
			barf("Cannot find name '%s' after insert in table %s",
			     name, table);
	}
	return answer[0];
}

static char *get_unit_type_ref(void *h, const char *name, const char *level)
{
	struct db_query *q;
	char *query, *cmd;

	query = aprintf("SELECT ROWID FROM \"unit_types\" where"
		      " \"name\" = \"%s\" AND \"level\" = \"%s\";",
		      name, level);
	q = db_query(h, query);
	if (q->num_rows)
		return q->rows[0][0];

	cmd = aprintf("INSERT INTO \"unit_types\" VALUES(\"%s\",\"%s\");",
		      name, level);
	db_command(h, cmd);

	q = db_query(h, query);
	if (q->num_rows != 1)
		barf("Cannot find unit_type '%s/%s' after insert",
		     name, level);
	return q->rows[0][0];
}

/* Return ROWID of this entry (key, value) pairs. */
static char *make_ref(void *h, bool might_exist, const char *table, ...)
{
	struct db_query *q;
	char *query, *cmd;
	const char *key, *val;
	const char *sep = "";
	va_list ap;

	query = aprintf("SELECT ROWID FROM \"%s\" WHERE ", table);
	va_start(ap, table);
	while ((key = va_arg(ap, const char *)) != NULL) {
		val = va_arg(ap, const char *);
		query = aprintf_add(query, "%s\"%s\" = \"%s\"",
				    sep, key, val);
		sep = " AND ";
	}
	va_end(ap);
	query = aprintf_add(query, ";");

	if (might_exist) {
		/* Try looking for it first? */
		q = db_query(h, query);
		if (q->num_rows)
			return q->rows[0][0];
	}

	/* Didn't find one, so make one. */
	cmd = aprintf("INSERT INTO \"%s\" (", table);
	sep = "";
	va_start(ap, table);
	while ((key = va_arg(ap, const char *)) != NULL) {
		val = va_arg(ap, const char *);
		cmd = aprintf_add(cmd, "%s\"%s\"", sep, key);
		sep = ",";
	}
	va_end(ap);
	cmd = aprintf_add(cmd, ") VALUES (");
	sep = "";
	va_start(ap, table);
	while ((key = va_arg(ap, const char *)) != NULL) {
		val = va_arg(ap, const char *);
		cmd = aprintf_add(cmd, "%s\"%s\"", sep, val);
		sep = ",";
	}
	va_end(ap);
	cmd = aprintf_add(cmd, ");");
	db_command(h, cmd);

	/* FIXME: doesn't insert return the ROWID? */
	q = db_query(h, query);
	if (q->num_rows != 1)
		barf("Entry '%s' did not exist after '%s'", query, cmd);
	return q->rows[0][0];
}

/* [5]
	[Elder Mage]
		count="1"
	[/Elder Mage]
   [/5]
 */
static void add_tally_level(void *h, const char *game_ref, struct wml *level)
{
	unsigned int i;
	for (i = 0; i < level->num_children; i++) {
		char *unit_type;

		unit_type = get_unit_type_ref(h, level->child[i]->name,
					      is_int(level->name));
		make_ref(h, false, "unit_tallies",
			 "game_ref", game_ref,
			 "unit_type_ref", unit_type,
			 "count", is_int(get(level->child[i],"count")),
			 NULL);
	}
}

/*
  [units-by-level]
	[2] ...
	[5] ...
  [/units-by-level]
*/
static void add_tally(void *h, const char *game_ref, struct wml *tally)
{
	unsigned int i;

	if (!tally)
		barf("No units-by-level entry");

	for (i = 0; i < tally->num_children; i++)
		add_tally_level(h, game_ref, tally->child[i]);
}

/* [special-unit]
	experience="22"
	level="3"
	name="Konrad"
   [/special-unit]
 */
static void add_special(void *h, const char *game_ref, struct wml *special)
{
	char *name;

	name = get_name_ref(h, "unit_names", get(special, "name"));
	make_ref(h, false, "special_units",
		 "game_ref", game_ref,
		 "unit_name_ref", name,
		 "level", get(special, "level"),
		 "experience", get(special, "experience"),
		 NULL);
}

/* [game]
	campaign="CAMPAIGN_HEIR_TO_THE_THRONE"
	difficulty="NORMAL"
	gold="549"
	scenario="Mountain_Pass"
	time="2052"
	start_turn="10"
	version="1.1-svn"
	[special-unit]...
	[units-by-level]...

One of:
	[victory]
		gold="749"
		time="3351"
		end_turn="19"
	[/victory]

OR:
	[defeat]
		time="5003"
		end_turn="19"
	[/defeat]
OR:
	[quit]
		time="5003"
		end_turn="19"
	[/quit]
 */
static void add_game(void *h,
		     const char *player_ref,
		     const char *version_ref,
		     unsigned int gamenum,
		     struct wml *game)
{
	struct wml *result;
	const char *campaign, *difficulty, *scenario;
	char *campaign_ref, *scenario_ref, *game_ref;
	const char *gold, *start_time, *start_turn, *game_number, *result_num,
		*end_time, *end_turn, *end_gold, *num_turns;
	unsigned int i;

	/* Get reference numbers for strings. */
	campaign = get_name_ref(h, "campaign_names", get(game,"campaign"));
	difficulty = get_name_ref(h,"difficulty_names",get(game,"difficulty"));
	scenario = get_name_ref(h, "scenario_names", get(game,"scenario"));

	/* Get reference for campaign entry. */
	campaign_ref = make_ref(h, true, "campaigns",
				"player_ref", player_ref,
				"campaign_name_ref", campaign,
				"difficulty_name_ref", difficulty, NULL);

	/* Get reference for scenario. */
	scenario_ref = make_ref(h, true, "scenarios",
				"campaign_ref", campaign_ref,
				"scenario_name_ref", scenario, NULL);

	gold = is_int(get(game, "gold"));
	start_time = is_int(get(game, "time"));
	num_turns = is_int(get(game, "num_turns"));
	/* We can tell between save at turn 1, and at end of last scenario.
	 * This matters: a save at turn 1 means maps is not random.
	 */
	start_turn = get_maybe(game, "start_turn");
	if (!start_turn)
		start_turn = "0";
	is_int(start_turn);
	game_number = aprintf("%i", gamenum);
	if ((result = get_child(game, "victory")) != NULL) {
		result_num = "1";
		end_time = is_int(get(result, "time"));
		end_turn = is_int(get(result, "end_turn"));
		end_gold = is_int(get(result, "gold"));
	} else if ((result = get_child(game, "defeat")) != NULL) {
		result_num = "2";
		end_time = is_int(get(result, "time"));
		end_turn = is_int(get(result, "end_turn"));
		end_gold = "0";
	} else if ((result = get_child(game, "quit")) != NULL) {
		result_num = "0";
		end_time = is_int(get(result, "time"));
		end_turn = is_int(get(result, "end_turn"));
		end_gold = "0";
	} else
		barf("No victory, defeat or quit!");

	/* Make entry for this game. */
	game_ref = make_ref(h, false, "games",
			    "scenario_ref", scenario_ref,
			    "start_turn", start_turn,
			    "gold", gold,
			    "start_time", start_time,
			    "version_name_ref", version_ref,
			    "game_number", game_number,
			    "result", result_num,
			    "end_time", end_time,
			    "end_turn", end_turn,
			    "end_gold", end_gold,
			    "num_turns", num_turns,
			    NULL);

	/* Make entry for each special-unit. */
	for (i = 0; i < game->num_children; i++) {
		if (streq(game->child[i]->name, "special-unit"))
			add_special(h, game_ref, game->child[i]);
	}

	add_tally(h, game_ref, get_child(game, "units-by-level"));
}

static const char *find_or_create_player(void *h,
					 const char *id,
					 unsigned int *game_number)
{
	char **answer, *cmd;

	answer = db_select(h, "players", "id", id, "ROWID", NULL);
	if (!answer) {
		cmd = aprintf("INSERT INTO \"players\" VALUES(\"%s\");", id);
		db_command(h, cmd);
		answer = db_select(h, "players", "id", id, "ROWID", NULL);
		*game_number = 1;
	} else {
		char **game;

		game = db_select(h, "game_count", "player_ref", answer[0],
				 "games_received", NULL);
		if (!game)
			barf("No game_count entry for %s", answer[0]);
		*game_number = atoi(game[0]);
	}
	return answer[0];
}

static void update_player_games(void *h,
				const char *player_ref, unsigned int num)
{
	char *cmd;
	cmd = aprintf("INSERT OR REPLACE INTO \"game_count\""
		      " VALUES(\"%s\",%u,CURRENT_DATE);", player_ref, num);
	db_command(h, cmd);
}

static void add_to_database(void *h, struct wml *wml)
{
	const char *version_ref, *id, *serial;
	const char *player_ref;
	unsigned int i, games;

	serial = get_maybe(wml, "serial");
	id = get(wml, "id");
	do {
		db_transaction_start(h);
		if (serial) {
			/* We already have it. */
			if (db_select(h, "serial", "id", serial, "ROWID",NULL))
				return;
			/* Manually-maintained list of known-bad files. */
			if (db_select(h, "bad_serial", "id", serial, "ROWID",
				      NULL))
				return;
			make_ref(h, false, "serial", "id", serial, NULL);
		}
		version_ref = get_name_ref(h, "version_names",
					   get(wml, "version"));
		player_ref = find_or_create_player(h, id, &games);
		for (i = 0; i < wml->num_children; i++) {
			if (!streq(wml->child[i]->name, "game"))
				barf("Unexpected toplevel element [%s]",
				     wml->child[i]->name);
			add_game(h, player_ref, version_ref, games + i,
				 wml->child[i]);
		}
		update_player_games(h, player_ref, games + i);
	} while (!db_transaction_finish(h));
}

/* Convert game to latest version. */
static struct wml *convert_game(struct wml *game, const char *version)
{
	return game;
}
	
/* Convert wml to the latest version. */
static struct wml *convert_wml(struct wml *wml, const char *version)
{
	unsigned int i;

	for (i = 0; i < wml->num_children; i++)
		if (streq(wml->child[i]->name, "game"))
			wml->child[i] = convert_game(wml->child[i], version);

	return wml;
}

static void receive(int fd)
{
	struct wml *wml;
	char *logfile;
	void *handle;

	maybe_log_to_file(&logfile);

	handle = db_open(database_file());

	wml = parse(fd);
	wml = convert_wml(wml, get(wml, "format_version"));

	add_to_database(handle, wml);

	/* We need this for a valid reply. */
	printf("Content-type: text/plain\n\n");
	if (logfile)
		unlink(logfile);
	db_close(handle);
}

/* For testing, takes a whole pile of files as input. */
int main(int argc, char *argv[])
{
	if (argc == 2 && streq(argv[1], "--initialize")) {
		create_tables(db_open(database_file()));
		exit(0);
	}

	nice(10);
	if (argc == 1)
		receive(STDIN_FILENO);
	else {
		int i;
		for (i = 1; i < argc; i++)
			receive(open(argv[i], O_RDONLY));
	}
	return 0;
}
