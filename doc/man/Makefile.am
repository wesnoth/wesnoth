MAN_LANG = de
dist_man6_MANS = wesnoth.6 wesnothd.6 wesnoth_editor.6

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
