SUBDIRS = wesnoth wesnoth-editor wesnoth-lib wesnoth-units wesnoth-multiplayer wesnoth-tutorial wesnoth-did wesnoth-ei wesnoth-httt wesnoth-l wesnoth-nr wesnoth-sof wesnoth-sotbe wesnoth-tb wesnoth-trow wesnoth-tsg wesnoth-utbs

PO4A_SUBDIRS = wesnoth-man

EXTRA_DIST = Makefile.in.in

update-po:
	@for dir in $(SUBDIRS); do \
		( cd $$dir && make $@ || exit $? ); \
	done
if PO4AUPDATE
	@for dir in $(PO4A_SUBDIRS); do \
		( cd $$dir && $(PO4A) --no-translations --rm-backups $$dir.cfg || \
		  exit $? ); \
	done
endif

if PO4AUPDATE
update-po4a:
	@for dir in $(PO4A_SUBDIRS); do \
		( cd $$dir && $(PO4A) --rm-backups $$dir.cfg || exit $? ); \
	done
endif

distclean-local:
	@for dir in $(SUBDIRS); do \
		rm -f $$dir/Makefile.in.in ; \
	done
