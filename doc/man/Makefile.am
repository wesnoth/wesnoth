MAN_LANG = af ar bg ca ca_ES@valencia cs da de el en_GB en@shaw es eo et eu fi fr fur_IT gl he hr hu id is it ja ko la lt lv mk mr nl nb_NO pl pt pt_BR racv ro ru sk sl sr sr@ijekavian sr@ijekavianlatin sr@latin sv tl tr vi zh_CN zh_TW
MAN_PAGES = wesnoth.6 wesnothd.6
INSTALL_MAN_PAGES =

if GAME
INSTALL_MAN_PAGES += wesnoth.6
endif

if SERVER
INSTALL_MAN_PAGES += wesnothd.6
endif

install-data-hook:
	mkdir -p $(DESTDIR)$(mandir)/man6 ; \
	for j in $(INSTALL_MAN_PAGES); do \
		$(INSTALL) -c -m 644 $(srcdir)/$$j $(DESTDIR)$(mandir)/man6/$$j ; \
	done
	for i in $(MAN_LANG); do \
		mkdir -p $(DESTDIR)$(mandir)/$$i/man6 ; \
		for j in $(INSTALL_MAN_PAGES); do \
			if test -f $(srcdir)/$$i/$$j; then \
				$(INSTALL) -c -m 644 $(srcdir)/$$i/$$j $(DESTDIR)$(mandir)/$$i/man6/$$j ; \
			fi \
		done \
	done

uninstall-hook:
	for i in $(MAN_LANG); do \
		for j in $(INSTALL_MAN_PAGES); do \
			rm -f $(DESTDIR)$(mandir)/$$i/man6/$$j ; \
		done ; \
		if test -d $(DESTDIR)$(mandir)/$$i/man6 ; then echo `rmdir $(DESTDIR)$(mandir)/$$i/man6` ; fi ; \
		if test -d $(DESTDIR)$(mandir)/$$i ; then echo `rmdir $(DESTDIR)$(mandir)/$$i` ; fi ; \
	done
	for j in $(INSTALL_MAN_PAGES); do \
		rm -f $(DESTDIR)$(mandir)/man6/$$j ; \
	done
	if test -d $(DESTDIR)$(mandir)/man6 ; then echo `rmdir $(DESTDIR)$(mandir)/man6` ; fi
	if test -d $(DESTDIR)$(mandir) ; then echo `rmdir $(DESTDIR)$(mandir)` ; fi

dist-hook:
	mkdir -p $(distdir)/ ; \
	for j in $(MAN_PAGES); do \
		$(INSTALL) -c -m 644 $(srcdir)/$$j $(distdir)/$$j ; \
	done
	for i in $(MAN_LANG); do \
		mkdir -p $(distdir)/$$i ; \
		for j in $(MAN_PAGES); do \
			if test -f $(srcdir)/$$i/$$j; then \
				$(INSTALL) -c -m 644 $(srcdir)/$$i/$$j $(distdir)/$$i/$$j ; \
			fi \
		done \
	done
