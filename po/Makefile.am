SUBDIRS = wesnoth wesnoth-editor wesnoth-lib wesnoth-tutorial wesnoth-httt wesnoth-tdh wesnoth-sotbe wesnoth-ei wesnoth-trow wesnoth-tb

EXTRA_DIST = Makefile.in.in

update-po:
	@for dir in $(SUBDIRS); do \
		( cd $$dir && make $@ || exit $? ); \
	done

distclean-local:
	@for dir in $(SUBDIRS); do \
		rm -f $$dir/Makefile.in.in ; \
	done
