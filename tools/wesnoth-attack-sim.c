/* Simulate Wesnoth combat. */
#define _GNU_SOURCE
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

/* from the Linux Kernel:
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

struct unit
{
	unsigned damage, num_attacks, hp, max_hp;
	unsigned hit_chance;
	bool slowed, slows, drains, berserk, swarm, firststrike;
	bool touched;
};

static void __attribute__((noreturn, format(printf,1,2)))
barf(const char *fmt, ...)
{
	char *str;
	va_list arglist;

	fprintf(stderr, "FATAL: ");

	va_start(arglist, fmt);
	vasprintf(&str, fmt, arglist);
	va_end(arglist);

	fprintf(stderr, "%s\n", str);
	free(str);
	exit(1);
}

static bool hits(unsigned int chance)
{
	return (random() % 100) < chance;
}

static void strike(struct unit *attacker, struct unit *defender)
{
	unsigned damage = attacker->damage;

	if (!hits(attacker->hit_chance))
		return;

	if (damage > defender->hp)
		damage = defender->hp;
	if (attacker->slows && !defender->slowed) {
		defender->slowed = true;
		defender->damage /= 2;
	}
	if (attacker->drains) {
		attacker->hp += damage/2;
		if (attacker->hp > attacker->max_hp)
			attacker->hp = attacker->max_hp;
	}
	defender->hp -= damage;
	defender->touched = true;
}

// A attacks B.
static void simulate_attack(struct unit *a, struct unit *b)
{
	unsigned int i, j;

	for (i = 0; i < ((a->berserk || b->berserk) ? 30: 1); i++) {
		for (j = 0; j < max(a->num_attacks, b->num_attacks); j++) {
			if (j < a->num_attacks) {
				strike(a, b);
				if (b->hp == 0)
					return;
			}
			if (j < b->num_attacks) {
				strike(b, a);
				if (a->hp == 0)
					return;
			}
		}
	}
}

static unsigned int num_attacks(unsigned base, unsigned max, unsigned hp,
				bool swarm)
{
	if (!swarm)
		return base;
	/* Swarm scales num attacks by hp. */
	return base - (base*(max-hp) / max);
}

/* This gives a max variation of around 1%. */
static void calculate_attack(const struct unit *defender,
			     double defender_res[],
			     double *defender_touched,
			     double attacker_touched[],
			     const struct unit *attackers[],
			     double *attacker_res[],
			     unsigned num_attackers,
			     unsigned num_sims)
{
	unsigned int i, j;

	*defender_touched = 0;

	for (j = 0; j < num_attackers; j++)
		attacker_touched[j] = 0;

	for (i = 0; i < num_sims; i++) {
		struct unit def = *defender;

		def.slowed = false;
		def.touched = false;
		for (j = 0; j < num_attackers && def.hp; j++) {
			struct unit att = *attackers[j];

			att.slowed = false;
			att.touched = false;
			att.num_attacks	= num_attacks(att.num_attacks,
						      att.max_hp, att.hp,
						      att.swarm);
			def.num_attacks = num_attacks(defender->num_attacks,
						      defender->max_hp, def.hp,
						      def.swarm);
			if (def.firststrike && !att.firststrike)
				simulate_attack(&def, &att);
			else
				simulate_attack(&att, &def);
			attacker_res[j][att.hp]++;
			if (att.touched)
				attacker_touched[j]++;
		}
		defender_res[def.hp]++;
		if (def.touched)
			(*defender_touched)++;
	}

	/* Now normalize each one by number of battles it was in. */
	for (i = 0; i <= defender->max_hp; i++)
		defender_res[i] /= num_sims;
	*defender_touched /= num_sims;

	for (i = 0; i < num_attackers; i++) {
		unsigned int battles = 0;
		for (j = 0; j <= attackers[i]->max_hp; j++) {
			battles += attacker_res[i][j];
			attacker_res[i][j] /= num_sims;
		}
		/* Any battle we weren't in, we're unscathed. */
		attacker_res[i][attackers[i]->hp]
			+= (1.0*num_sims-battles)/num_sims;

		/* If this attacker wasn't in more than 1% of battles, don't
		 * pretend to know this probability. */
		if (battles <= num_sims / 100)
			attacker_touched[i] = -1.0;
		else
			/* FIXME: attack_prediction doesn't take into account
			 * that opponent might already be dead. */
			attacker_touched[i] /= num_sims;
	}
}

static struct unit *parse_unit(char ***argv)
{
	struct unit *u = malloc(sizeof(*u));

	u->damage = atoi((*argv)[1]);
	u->num_attacks = atoi((*argv)[2]);
	u->hp = u->max_hp = atoi((*argv)[3]);
	u->hit_chance = atoi((*argv)[4]);
	u->slows = false;
	u->slowed = false;
	u->drains = false;
	u->berserk = false;
	u->swarm = false;
	u->firststrike = false;
	if ((*argv)[5] && atoi((*argv)[5]) == 0) {
		char *max = strstr((*argv)[5], "maxhp=");
		if (max) {
			u->max_hp = atoi(max + strlen("maxhp="));
			if (u->max_hp < u->hp)
				barf("maxhp must be > hitpoints");
		}
		if (strstr((*argv)[5], "drain")) {
			if (!max)
				barf("drain needs maxhp set");
			u->drains = true;
		}
		if (strstr((*argv)[5], "slow"))
			u->slows = true;
		if (strstr((*argv)[5], "berserk"))
			u->berserk = true;
		if (strstr((*argv)[5], "firststrike"))
			u->firststrike = true;
		if (strstr((*argv)[5], "swarm")) {
			if (!max)
				barf("swarm needs maxhp set");
			u->swarm = true;
		}
		*argv += 5;
	} else
		*argv += 4;

	return u;
}

#if 0
static void graph_prob(unsigned int hp, double prob)
{
	unsigned int i, percent;

	percent = (prob + 1/200.0) * 100;

	printf("%-3u %3u%% |", hp, percent);
	for (i = 0; i < percent; i++)
		printf("#");
	printf("\n");
}
#endif

static void draw_results(const double res[], const struct unit *u,
			 double touched,
			 const char label[])
{
	unsigned int i;

	printf("#0: %s: %u %u %u %u%% ",
	       label, u->damage, u->num_attacks, u->hp, u->hit_chance);
	if (u->drains)
		printf("drains,");
	if (u->slows)
		printf("slows,");
	if (u->berserk)
		printf("berserk,");
	if (u->swarm)
		printf("swarm,");
	if (u->firststrike)
		printf("firststrike,");
	printf("maxhp=%u ", u->max_hp);
	if (touched == -1)
		printf("touched:unknown ");
	else
		printf("touched:%.2f%% ", touched*100);
	for (i = 0; i < u->max_hp+1; i++)
		printf(" %.2f", res[i]*100);
	printf("\n");
}

static void compare_results(const double res[], const struct unit *u,
			    const char label[], unsigned battle,
			    double touched, FILE *f)
{
	unsigned int i;
	char line[128], cmp[128];
	double val;

	sprintf(cmp, "#%u: %s: %u %u %u %u%% ", battle,
		label, u->damage, u->num_attacks, u->hp, u->hit_chance);
	if (u->drains)
		sprintf(cmp+strlen(cmp), "drains,");
	if (u->slows)
		sprintf(cmp+strlen(cmp), "slows,");
	if (u->berserk)
		sprintf(cmp+strlen(cmp), "berserk,");
	if (u->swarm)
		sprintf(cmp+strlen(cmp), "swarm,");
	if (u->firststrike)
		sprintf(cmp+strlen(cmp), "firststrike,");
	sprintf(cmp+strlen(cmp), "maxhp=%u", u->max_hp);

	if (fread(line, strlen(cmp), 1, f) != 1)
		barf("Unexpected end of file on battle %u", battle);

	if (strncmp(line, cmp, strlen(cmp)) != 0)
		barf("Battle %u is different: '%.*s' should be '%s'",
		     battle, strlen(cmp), line, cmp);

	if (fscanf(f, " %lf", &val) != 1)
		barf("Malformed untouched: %s battle %u", 
		     label, battle);

	/* We *must* have result for defender and attacker 1. */
	if (touched == -1)
		assert(strcmp(label, "Attacker #2") == 0);
	else if (abs((val - (1.0 - touched))*100) > 1.0)
		printf("Warning: expected %f untouched, got %f battle %u %s\n",
		       1.0 - touched, val, battle, label);

	for (i = 0; i < u->max_hp+1; i++) {
		if (fscanf(f, " %lf", &val) != 1)
			barf("Malformed hp line: %s hp %u battle %u", 
			     label, i, battle);
#if 0
		if (abs(val - res[i]*100) > 5.0)
			barf("Battle %u %s hp %u %f should be %f",
			     battle, label, i, val, res[i]*100);
#endif
		if (abs(val - res[i]*100) > 1.0)
			printf("Warning: battle %u %s hp %u %f should be %f\n",
			       battle, label, i, val, res[i]*100);
	}
	fscanf(f, "\n");
}

static void check_sum(double *arr, unsigned int num)
{
	unsigned int i;
	double sum = 0;

	for (i = 0; i <= num; i++)
		sum += arr[i];

	assert(sum > 0.999 && sum < 1.001);
}

#define NUM_UNITS 50
static void check(const char *filename)
{
	/* N^2 battles. */
	struct unit u[NUM_UNITS];
	unsigned int i, j, k, battle = 0, percent;
	FILE *f = fopen(filename, "r");
	if (!f)
		barf("Could not open %s for reading: %s",
		     filename, strerror(errno));

	printf("Creating %i units...\n", NUM_UNITS);
	for (i = 0; i < NUM_UNITS; i++) {
		u[i].hp = 1 + ((i*3)%23);
		u[i].max_hp = u[i].hp + (i+7)%17;
		u[i].damage = (i % 7) + 2;
		u[i].num_attacks = (i % 4) + 1;
		u[i].slows = (i % 8) == 0;
		u[i].drains = (i % 9) == 0;
		u[i].berserk = (i % 5) == 0;
		u[i].hit_chance = 30 + (i % 6)*10;
		u[i].swarm = ((i+4) % 4) == 0;
//		u[i].swarm = false;
		u[i].firststrike = ((i+3) % 5) == 0;
	}

	printf("Beginning battle...\n");

	percent = NUM_UNITS*(NUM_UNITS-1)*(NUM_UNITS-2)/100;
	srandom(time(NULL));
	for (i = 0; i < NUM_UNITS; i++) {
		for (j = 0; j < NUM_UNITS; j++) {
			if (i == j)
				continue;
			for (k = 0; k < NUM_UNITS; k++) {
				if (k == i || k == j)
					continue;
				double i_result[u[i].max_hp+1];
				double j_result[u[j].max_hp+1];
				double k_result[u[k].max_hp+1];
				double i_touched;
				double *attacker_res[2];
				const struct unit *attackers[2];
				double touched[2];

				memset(i_result, 0, sizeof(i_result));
				memset(j_result, 0, sizeof(j_result));
				memset(k_result, 0, sizeof(k_result));

				attacker_res[0] = j_result;
				attacker_res[1] = k_result;
				attackers[0] = &u[j];
				attackers[1] = &u[k];
				calculate_attack(&u[i], i_result, &i_touched,
						 touched,
						 attackers, attacker_res, 2,
						 10000);
				battle++;
				check_sum(i_result, u[i].max_hp);
				check_sum(j_result, u[j].max_hp);
				check_sum(k_result, u[k].max_hp);
				compare_results(i_result, &u[i], "Defender",
						battle, i_touched, f);
				compare_results(j_result, &u[j], "Attacker #1",
						battle, touched[0], f);
				compare_results(k_result, &u[k], "Attacker #2",
						battle, touched[1], f);
				if ((battle % percent) == 0) {
					printf(".");
					fflush(stdout);
				}
			}
		}
	}
	printf("\nTotal combats: %i\n", NUM_UNITS*(NUM_UNITS-1)*(NUM_UNITS-2));
	exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int i;
	double *res_def, *res_att[argc / 4], def_touched, att_touched[argc/4];
	const struct unit *def, *attacker[argc / 4 + 1];

	if (argc == 3 && strcmp(argv[1], "--check") == 0)
		check(argv[2]);

	if (argc < 9)
		barf("Usage: %s --check <results-file>\n"
		     "\t%s <damage> <attacks> <hp> <hitprob> [drain,slow,swarm,firststrike,berserk,maxhp=<num>] <damage> <attacks> <hp> <hitprob> [drain,slow,berserk,firststrike,swarm,maxhp=<num>] ...",
		     argv[0], argv[0]);

	def = parse_unit(&argv);
	res_def = calloc(sizeof(double), def->max_hp+1);
	for (i = 0; argv[1]; i++) {
		attacker[i] = parse_unit(&argv);
		res_att[i] = calloc(sizeof(double), attacker[i]->max_hp+1);
	}
	attacker[i] = NULL;

	srandom(time(NULL));
	calculate_attack(def, res_def, &def_touched, att_touched,
			 attacker, res_att, i, 10000);
	draw_results(res_def, def, def_touched, "Defender");
	for (i = 0; attacker[i]; i++)
		draw_results(res_att[i], attacker[i], att_touched[i],
			     "Attacker");
	return 0;
}
