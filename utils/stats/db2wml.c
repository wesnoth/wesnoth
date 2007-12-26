/* Create WML upload file for a given game. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "database.h"
#include "utils.h"

#define DATABASE_FILE "/home/rusty/wesnoth/wesnoth-uploads.db"

int log_fd = STDERR_FILENO;

int main(int argc, char *argv[])
{
	void *h;
	char *q, *filename;
	unsigned int i;
	int level;
	struct db_query *query;

	if (argc != 3)
		barf("Usage: %s <database> <games.rowid>\n", argv[0]);

	h = db_open(argv[1]);
	/*
	  campaign="CAMPAIGN_HEIR_TO_THE_THRONE"
	  difficulty="NORMAL"
	  scenario="Mountain_Pass"
	  gold="549"
	  time="2052"
	  num_turns="25"
	  start_turn="10"
	  version="1.1-svn"
	*/
	q = aprintf("SELECT campaign_names.name,difficulty_names.name,scenario_names.name,games.gold,games.start_time,games.start_turn,version_names.name,games.result,games.end_time,games.end_gold,games.game_number,players.id FROM players,campaign_names,difficulty_names,scenario_names,campaigns,scenarios,games,version_names WHERE games.rowid='%s' AND games.scenario_ref = scenarios.rowid AND scenarios.campaign_ref = campaigns.rowid AND campaigns.difficulty_name_ref = difficulty_names.rowid AND campaigns.campaign_name_ref = campaign_names.rowid AND games.version_name_ref = version_names.rowid AND campaigns.player_ref = players.rowid AND scenario_names.rowid = scenarios.scenario_name_ref;", argv[2]);
	query = db_query(h, q);

	if (atoi(query->rows[0][8]) - atoi(query->rows[0][4]) < 100) {
		fprintf(stderr, "Only lasted from '%s' to '%s'\n",
			query->rows[0][4], query->rows[0][8]);
		exit(0);
	}

	filename = aprintf("%08i", atoi(query->rows[0][10]));
	dup2(open(filename, O_WRONLY|O_TRUNC|O_CREAT, 0666), STDOUT_FILENO);

	printf("format_version=\"1\"\n"
	       "version=\"%s\"\n"
	       "id=\"%s\"\n",
	       query->rows[0][6],
	       query->rows[0][11]);

	printf("[game]\n"
	       "\tcampaign=\"%s\"\n"
	       "\tdifficulty=\"%s\"\n"
	       "\tscenario=\"%s\"\n"
	       "\tgold=\"%s\"\n"
	       "\ttime=\"%s\"\n"
	       "\tstart_turn=\"%s\"\n",
	       query->rows[0][0],
	       query->rows[0][1],
	       query->rows[0][2],
	       query->rows[0][3],
	       query->rows[0][4],
	       query->rows[0][5]);

	if (streq(query->rows[0][7], "0")) {
		printf("\t[quit]\n"
		       "\t\ttime=\"%s\"\n"
		       "\t[/quit]\n",
		       query->rows[0][8]);
	} else if (streq(query->rows[0][7], "1")) {
		printf("\t[victory]\n"
		       "\t\ttime=\"%s\"\n"
		       "\t\tgold=\"%s\"\n"
		       "\t[/victory]\n",
		       query->rows[0][8],
		       query->rows[0][9]);
	} else {
		printf("\t[defeat]\n"
		       "\t\ttime=\"%s\"\n"
		       "\t[/defeat]\n",
		       query->rows[0][8]);
	}

	q = aprintf("SELECT unit_names.name,special_units.level,special_units.experience FROM unit_names,special_units WHERE special_units.game_ref = '%s' AND unit_names.rowid = special_units.unit_name_ref;", argv[2]);
	query = db_query(h, q);
	for (i = 0; i < query->num_rows; i++)
		printf("\t[special-unit]\n"
		       "\t\tname=\"%s\"\n"
		       "\t\tlevel=\"%s\"\n"
		       "\t\texperience=\"%s\"\n"
		       "\t[/special-unit]\n",
		       query->rows[i][0], query->rows[i][1],query->rows[i][2]);

	level = -1;
	printf("\t[units-by-level]\n");
	q = aprintf("SELECT unit_types.level,unit_types.name,unit_tallies.count FROM unit_tallies,unit_types WHERE unit_tallies.game_ref = '%s' AND unit_types.rowid = unit_tallies.unit_type_ref ORDER BY unit_types.level ASC;", argv[2]);
	query = db_query(h, q);
	for (i = 0; i < query->num_rows; i++) {
		if (atoi(query->rows[i][0]) != level) {
			if (i != 0)
				printf("\t\t[/%i]\n", level);
			level = atoi(query->rows[i][0]);
			printf("\t\t[%i]\n", level);
		}
		printf("\t\t\t[%s]\n"
		       "\t\t\t\tcount=\"%s\"\n"
		       "\t\t\t[/%s]\n",
		       query->rows[i][1], query->rows[i][2],query->rows[i][1]);
	}
	if (i != 0)
		printf("\t\t[/%i]\n", level);
	printf("\t[/units-by-level]\n"
		"[/game]\n");
	return 0;
}
		
		
