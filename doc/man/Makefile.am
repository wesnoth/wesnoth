MAN_LANG = de
dist_man6_MANS =

if GAME
dist_man6_MANS += wesnoth.6
endif

if SERVER
dist_man6_MANS += wesnothd.6
endif

if EDITOR
dist_man6_MANS += wesnoth_editor.6
endif

install-data-hook:
	for i in $(MAN_LANG); do \
		mkdir -p $(DESTDIR)$(mandir)/$$i/man6 ; \
		for j in $(dist_man6_MANS); do \
			$(INSTALL) -c -m 644 $(srcdir)/$$i/$$j $(DESTDIR)$(mandir)/$$i/man6/$$j ; \
		done \
	done

uninstall-hook:
	for i in $(MAN_LANG); do \
		for j in $(dist_man6_MANS); do \
			rm -f $(DESTDIR)$(mandir)/$$i/man6/$$j ; \
		done ; \
		if test -d $(DESTDIR)$(mandir)/$$i/man6 ; then echo `rmdir $(DESTDIR)$(mandir)/$$i/man6` ; fi ; \
		if test -d $(DESTDIR)$(mandir)/$$i ; then echo `rmdir $(DESTDIR)$(mandir)/$$i` ; fi ; \
	done
	if test -d $(DESTDIR)$(mandir)/man6 ; then echo `rmdir $(DESTDIR)$(mandir)/man6` ; fi
	if test -d $(DESTDIR)$(mandir) ; then echo `rmdir $(DESTDIR)$(mandir)` ; fi

dist-hook:
	for i in $(MAN_LANG); do \
		mkdir -p $(distdir)/$$i ; \
		for j in $(dist_man6_MANS); do \
			$(INSTALL) -c -m 644 $(srcdir)/$$i/$$j $(distdir)/$$i/$$j ; \
		done \
	done
