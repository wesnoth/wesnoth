MAN_LANG = de
dist_man6_MANS = wesnoth.6 wesnothd.6 wesnoth_editor.6

install-data-hook:
	for i in $(MAN_LANG); do \
		mkdir -p $(mandir)/$$i/man6 ; \
		for j in $(dist_man6_MANS); do \
			$(INSTALL) -c -m 644 $$i/$$j $(mandir)/$$i/man6/$$j ; \
		done \
	done

uninstall-hook:
	for i in $(MAN_LANG); do \
		for j in $(dist_man6_MANS); do \
			rm -f $(mandir)/$$i/man6/$$j ; \
		done ; \
		if test -d $(mandir)/$$i/man6 ; then echo `rmdir $(mandir)/$$i/man6` ; fi ; \
		if test -d $(mandir)/$$i ; then echo `rmdir $(mandir)/$$i` ; fi ; \
	done
	if test -d $(mandir)/man6 ; then echo `rmdir $(mandir)/man6` ; fi
	if test -d $(mandir) ; then echo `rmdir $(mandir)` ; fi

