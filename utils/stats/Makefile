CFLAGS=-Wall -g -Wunused-parameter -W -Wmissing-declarations
LDFLAGS=-lsqlite3 -lm

all: upload.cgi graph db2wml

upload.cgi: upload.cgi.o utils.o sqlite3_database.o
graph: graph.o utils.o sqlite3_database.o
db2wml: db2wml.o utils.o sqlite3_database.o

clean:
	rm -f graph upload.cgi *.o

# Testsuite: .input* are fed into database, then .test run, compared with .result.
check: upload.cgi
	@./run-tests.sh

clean-db: upload.cgi
	rm -f wesnoth-uploads.db
	./wesnoth-upload.cgi --initialize
