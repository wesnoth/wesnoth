/* SQLite3 database backend. */
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>
#include "database.h"
#include "utils.h"

/* sqlite3_busy_timeout sleeps for a *second*.  What a piece of shit. */
static int busy(void *unused __attribute__((unused)), int count)
{
	usleep(50000);

	/* If we've been stuck for 1000 iterations (at least 50
	 * seconds), give up. */
	return (count < 1000);
}

void *db_open(const char *file)
{
	sqlite3 *handle;

	int err = sqlite3_open(file, &handle);
	if (err != SQLITE_OK)
		barf("Error %i from sqlite3_open of db '%s'\n", err, file);
	sqlite3_busy_handler(handle, busy, NULL);

	return handle;
}

static int query_cb(void *data, int num, char**vals,
		    char**names __attribute__((unused)))
{
	int i;
	struct db_query *query = data;

	query->rows = realloc_array(query->rows, query->num_rows+1);
	query->rows[query->num_rows] = new_array(char *, num);
	for (i = 0; i < num; i++) {
		/* We don't count rows with NULL results
		 * (eg. count(*),player where count turns out to be
		 * zero. */
		if (!vals[i])
			return 0;
		query->rows[query->num_rows][i] = strdup(vals[i]);
	}
	query->num_rows++;
	return 0;
}

/* Runs query (SELECT).  Fails if > 1 row returned.  Fills in columns. */
struct db_query *db_query(void *h, const char *query)
{
	struct db_query *ret = new(struct db_query);
	char *err;

	ret->rows = NULL;
	ret->num_rows = 0;

	if (sqlite3_exec(h, query, query_cb, ret, &err) != SQLITE_OK)
		barf("Failed sqlite3 query '%s': %s", query, err);
	return ret;
}

/* Runs command (CREATE TABLE/INSERT) */
void db_command(void *h, const char *command)
{
	char *err;

	if (sqlite3_exec(h, command, NULL, NULL, &err) != SQLITE_OK)
		barf("Failed sqlite3 command '%s': %s", command, err);
}

/* Starts transaction.  Doesn't need to nest. */
void db_transaction_start(void *h)
{
	char *err;
	if (sqlite3_exec(h, "BEGIN EXCLUSIVE TRANSACTION", NULL, NULL, &err)!=SQLITE_OK)
		barf("Starting sqlite3 transaction: %s\n", err);
}

/* Finishes transaction, or rolls it back and caller needs to start again. */
bool db_transaction_finish(void *h)
{
	switch (sqlite3_exec(h, "COMMIT TRANSACTION;", NULL, NULL, NULL)) {
	case SQLITE_OK:
		return true;
	case SQLITE_BUSY:
		if (sqlite3_exec(h, "ROLLBACK TRANSACTION;", NULL, NULL, NULL)
		    != SQLITE_OK)
			barf("Ending sqlite3 busy rollback failed");
		return false;
	default:
		barf("Strange sqlite3 error return from COMMIT");
	}
}

/* Closes database (only called when everything OK). */
void db_close(void *h)
{
	sqlite3_close(h);
}

