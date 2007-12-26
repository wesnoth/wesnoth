/* Query database to produce a graph of a campaign as SVG. */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "utils.h"
#include "database.h"

#define DATABASE_FILE "/home/rusty/wesnoth/wesnoth-uploads.db"
#define HEIGHT 200
#define WIDTH 130 		/* For each entry */
#define PROGRESS_WIDTH 150
#define BORDER 25
#define BAR_HEIGHT 10

int log_fd = STDERR_FILENO;

static void print_circle(unsigned int x, unsigned int val,
			 unsigned int repeats,
			 const char *player,
			 const char *campaign, const char *ver)
{
	/* Some queries (eg. # wins) not associated with particular player. */
	if (player)
		printf("<a xlink:href=\"index.cgi"
		       "?W_PLAYER=%s&amp;W_CAMPAIGN=%s&amp;W_VERSION=%s\""
		       " xlink:title=\"Player %s\" target=\"_blank\">",
		       player, campaign, ver, player);
	printf("<circle cx=\"%u\" cy=\"%u\" r=\"%f\" fill=\"red\"/>",
	       x, HEIGHT - val, sqrt(repeats)*2);
	if (player)
		printf("</a>");
	printf("\n");
}

static void plot(struct db_query *query, unsigned int x, float scale,
		 const char *campaign, const char *ver)
{
	unsigned int i, prev = 0;

	for (i = 0; i < query->num_rows; i++) {
		if (streq(query->rows[i][0], query->rows[prev][0]))
			continue;

		print_circle(x, atoi(query->rows[prev][0])*scale, i-prev,
			     query->rows[prev][1], campaign, ver);
		prev = i;
	}

	if (i != 0)
		print_circle(x, atoi(query->rows[prev][0])*scale, i-prev,
			     query->rows[prev][1], campaign, ver);
}

static void draw_graph(const char *view, const char *answer,
		       const char *select_cond, const char *campaign,
		       const char *diff, const char *ver,
		       const char *extra,
		       int argc, char *argv[])
{
	void *h;
	char *querystr;
	struct db_query *query[argc];
	int i, minimum = 0, maximum = 0;
	unsigned int j, span, grad;
	float scale;

	h = db_open(DATABASE_FILE);
	for (i = 0; i < argc; i++) {
		querystr = aprintf("SELECT %s,player FROM %s WHERE %s AND version='%s' AND difficulty='%s' AND scenario='%s' AND campaign='%s' %s;",
				   answer, view, select_cond, ver, diff,
				   argv[i], campaign, extra);
		query[i] = db_query(h, querystr);
		for (j = 0; j < query[i]->num_rows; j++) {
			maximum = max(atoi(query[i]->rows[j][0]), maximum);
			minimum = min(atoi(query[i]->rows[j][0]), minimum);
		}
#if 0
		printf("%s: ", argv[i]);
		for (j = 0; j < query[i]->num_rows; j++)
			printf("%s ", query[i]->rows[j][0]);
		printf("\n");
#endif
	}

	/* Header and axes. */
	printf("<?xml version=\"1.0\" standalone=\"no\"?>\n"
	       "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\" \"svg-20001102.dtd\">"
	       "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" viewBox=\"-%u -%u %u %u\" >"
	       "<path fill=\"none\" stroke=\"black\" d=\"M 0 %u l 0 -%u m -15 25 l 15 -25 l 15 25\"/>"
	       "<path fill=\"none\" stroke=\"black\" d=\"M 0 %u l %i 0 m -25 -15 l 25 15 l -25 15\"/>\n",
	       BORDER, BORDER, argc*WIDTH+BORDER*2, HEIGHT+BORDER*2,
	       HEIGHT, HEIGHT, HEIGHT, argc*WIDTH);

	/* Print names of scenarios and marks along x axis. */
	for (i = 0; i < argc; i++) {
		char *name = strndup(argv[i], 15);
		while (strchr(name, '_'))
			*strchr(name, '_') = ' ';
		printf("<text text-anchor=\"middle\" text-align=\"center\" font-size=\"16\" fill=\"black\" x=\"%u\" y=\"%u\">%s</text>\n"
		       "<line stroke=\"black\" x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\"/>\n",
		       i*WIDTH+WIDTH/2, HEIGHT+20, name,
		       i*WIDTH+WIDTH/2, HEIGHT, i*WIDTH+WIDTH/2, HEIGHT+5);
	}

	span = maximum - minimum;
	/* We want a Y size of HEIGHT. */
	scale = (float)HEIGHT / span;
	if (span > 200)
		grad = 100;
	else if (span > 20)
		grad = 10;
	else
		grad = 1;

	/* Print gradiations up y axis. */
	for (i = (minimum+grad-1)/grad*grad; i < maximum; i += grad)
		printf("<text fill=\"black\" x=\"-25\" y=\"%f\">%u</text>\n"
		       "<line stroke=\"black\" x1=\"-5\" y1=\"%f\" x2=\"0\" y2=\"%f\"/>\n",
		       HEIGHT - scale*i, i, (span-i)*scale, (span-i)*scale);

	/* Now plot results.  We compine multiples into bigger circles. */
	for (i = 0; i < argc; i++)
		plot(query[i], i*WIDTH+WIDTH/2, scale, campaign, ver);

	/* Draw average */
	printf("<path fill=\"none\" stroke=\"gray\" d=\"");
	for (i = 0; i < argc; i++) {
		unsigned int sum = 0;
		float avg;
		for (j = 0; j < query[i]->num_rows; j++)
			sum += atoi(query[i]->rows[j][0]);
		if (query[i]->num_rows == 0)
			avg = 0;
		else
			avg = (float)sum / query[i]->num_rows;
		printf("%s %u %f ", i == 0 ? "M" : "L", 
		       i*WIDTH+WIDTH/2, HEIGHT - avg*scale);
	}
	printf("\"/>\n");

	/* Print 0 line if not at bottom of graph. */
	if (minimum != 0)
		printf("<line stroke=\"red\" x1=\"%u\" y1=\"%f\" x2=\"%u\" y2=\"%f\"/>\n",
		       0, maximum*scale, argc*WIDTH, maximum*scale);

	printf("</svg>\n");
}

static void draw_one(struct db_query *query, unsigned height,
		     unsigned int argc, char **argv)
{
	unsigned int i, j;

	/* Axes */
	printf("<path fill=\"none\" stroke=\"black\" d=\"M 0 %u l 0 -%u m -15 25 l 15 -25 l 15 25\"/>"
	       "<path fill=\"none\" stroke=\"black\" d=\"M 0 %u l %i 0 m -25 -15 l 25 15 l -25 15\"/>\n",
	       height, height, height, argc*PROGRESS_WIDTH);

	/* Print names of scenarios along x axis, with lines. */
	for (i = 0; i < argc; i++) {
		char *name = strndup(argv[i], 15);
		while (strchr(name, '_'))
			*strchr(name, '_') = ' ';
		printf("<text text-anchor=\"middle\" text-align=\"center\" font-size=\"16\" fill=\"black\" x=\"%u\" y=\"%u\">%s</text>\n"
		       "<line stroke=\"black\" x1=\"%u\" y1=\"%u\" x2=\"%u\" y2=\"%u\"/>\n",
		       i*PROGRESS_WIDTH+PROGRESS_WIDTH/2, height+20, name,
		       i*PROGRESS_WIDTH+PROGRESS_WIDTH, 0,
		       i*PROGRESS_WIDTH+PROGRESS_WIDTH, height+5);
	}

	/* Draw each game. */
	for (i = 0; i < query->num_rows; i++) {
		int start, end, num_turns;

		/* Find which scenario was played. */
		for (j = 0; j < argc; j++) {
			if (streq(query->rows[i][0], argv[j]))
				break;
		}
		if (j == argc)
			continue;

		/* Bar goes from start to end turn. */
		start = atoi(query->rows[i][1]);
		end = atoi(query->rows[i][2]);
		/* Old data has no end 8( */
		if (end < start)
			end = start;

		/* We can end one turn *after* last turn (timeout). */
		num_turns = atoi(query->rows[i][3])+1;
		start /= (float)num_turns/PROGRESS_WIDTH;
		end /= (float)num_turns/PROGRESS_WIDTH;

		/* Link this to the specific game stats. */
		printf("<a xlink:href=\"index.cgi?W_GAME=%s\""
		       " xlink:title=\"Game %s\" target=\"_blank\">",
		       query->rows[i][5], query->rows[i][5]);
		printf("<rect x=\"%u\" y=\"%u\" width=\"%u\" height=\"%u\""
		       " stroke=\"black\" fill=\"%s\"/></a>\n",
		       PROGRESS_WIDTH*j+start, height-BAR_HEIGHT,
		       end-start?:1, BAR_HEIGHT,
		       streq(query->rows[i][4], "1") ? "green"
		       : streq(query->rows[i][4], "2") ? "#ff0d00"
		       : "#ece000");
		height -= BAR_HEIGHT;
	}
}

/* Draw the progress of this player. */
static void draw_progress_graph(const char *campaign, const char *diff,
				const char *ver, const char *player,
				unsigned int argc, char *argv[])
{
	void *h;
	char *querystr;
	struct db_query *query;
	unsigned int height, i, j;

	h = db_open(DATABASE_FILE);
	querystr = aprintf("SELECT scenario,start_turn,end_turn,num_turns,result,game FROM campaign_view WHERE campaign='%s' AND difficulty='%s' AND version='%s' AND player='%s' ORDER BY game;",
			   campaign, diff, ver, player);
	query = db_query(h, querystr);

	/* Remove scenarios not played by player. */
	for (i = 0; i < argc; i++) {
		for (j = 0; j < query->num_rows; j++) {
			if (streq(query->rows[j][0], argv[i]))
				break;
		}
		if (j == query->num_rows) {
			delete_arr(argv, argc, i, 1);
			argc--;
			i--;
		}
	}

	height = 0;
	/* For each one we're going to draw, add BAR_HEIGHT to height */
	for (i = 0; i < argc; i++) {
		for (j = 0; j < query->num_rows; j++) {
			if (streq(query->rows[j][0], argv[i]))
				height += BAR_HEIGHT;
		}
	}

	/* Header. */
	printf("<?xml version=\"1.0\" standalone=\"no\"?>\n"
	       "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\" \"svg-20001102.dtd\">"
	       "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" viewBox=\"-%u -%u %u %u\" preserveAspectRatio=\"none\">\n",
	       BORDER, BORDER, argc*PROGRESS_WIDTH+BORDER*2, height+BORDER*2);

	draw_one(query, height, argc, argv);
	printf("</svg>\n");
}

static void usage(const char *name)
{
	barf("Usage: %s <view> <answer> <select-cond> <campaign> <difficulty> <version> <extra-args> <scenario>...\n"
	     " OR:\n"
	     " %s --progress <campaign> <difficulty> <version> <scenario_name>...",
	     name, name);
}

/* FIXME: Allow multiple queries plotted with different colors? */
int main(int argc, char *argv[])
{
	if (argv[1] && streq(argv[1], "--progress")) {
		if (argc < 7)
			usage(argv[0]);

		draw_progress_graph(argv[2], argv[3], argv[4], argv[5],
				    argc-6, argv+6);
	} else {
		if (argc < 9)
			usage(argv[0]);

		draw_graph(argv[1], argv[2], argv[3], argv[4], argv[5], 
			   argv[6], argv[7], argc - 8, argv + 8);
	}
	return 0;
}
