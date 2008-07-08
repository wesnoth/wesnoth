/* Simple SQL-style database ops.  Currently implemented for sqlite3. */
#ifndef _UPLOAD_ANALYSIS_DATABASE_H
#define _UPLOAD_ANALYSIS_DATABASE_H
#include <stdbool.h>

/* Returns handle to the database.. */
void *db_open(const char *file);

/* Runs query (SELECT).  Fills in columns. */
struct db_query
{
	unsigned int num_rows;
	char ***rows;
};

struct db_query *db_query(void *h, const char *query);

/* Runs command (CREATE TABLE/INSERT) */
void db_command(void *h, const char *command);

/* Starts transaction.  Doesn't need to nest. */
void db_transaction_start(void *h);

/* Finishes transaction, or rolls it back and caller needs to start again. */
bool db_transaction_finish(void *h);

/* Closes database (only called when everything OK). */
void db_close(void *h);

#endif /* _UPLOAD_ANALYSIS_DATABASE_H */
