ISUBDIRS = icons
SUBDIRS = intl po m4 src $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

findfilterflags=-name CVS -prune -o -name ".cvs*" -o -name ".\#*" -o -type d

if LITE
findmoredata=;find images $(findfilterflags) -o -path "images/campaigns/*/story*" -o -path "images/campaigns/*/sotbe*" -o -print
else
findmoredata=;find images music $(findfilterflags) -o -print
endif

finddata=(cd $(top_srcdir) && find data fonts sounds $(findfilterflags) -o -print \
	 $(findmoredata) )
findnoinst=(cd $(top_srcdir) && find utils -name CVS -prune -o -name ".cvs*" -o -name ".\#*" -o -type d -o -print && find MANUAL*)

w_preparefileinstall = if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac;

install-data-local:
	@$(NORMAL_INSTALL)
	test -z "$(pkgdatadir)" || $(mkdir_p) "$(DESTDIR)$(pkgdatadir)"
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata) ) | while read p; do \
	  $(w_preparefileinstall) \
	  echo " $(install_sh_DATA) '$$d$$p' '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  $(install_sh_DATA) "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
if TINYGUI
	(cd $(top_srcdir) && find images/portraits $(findfilterflags) -o -print) | while read p; do \
	  $(w_preparefileinstall) \
	  convert -size 80x80 -resize 80x80 "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
	(cd $(top_srcdir) && find images/maps images/campaigns/*/maps images/campaigns/*/story $(findfilterflags) -o -print) | while read p; do \
	  $(w_preparefileinstall) \
	  convert -size 240x180 -resize 240x180 "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
endif

uninstall-local:
	@$(NORMAL_UNINSTALL)
	@srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata) ) | while read p; do \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac; \
	  echo " rm -f '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
dist-hook:
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata); $(findnoinst) ) | (cd $(top_srcdir) && tar cf - -T -) | (cd $(distdir) && tar xf -)

dist_noinst_DATA = copyright changelog wesnoth.dsp wesnoth.dsw
dist_man6_MANS = doc/man/wesnoth.6 doc/man/wesnothd.6 doc/man/wesnoth_editor.6

uninstall-hook:
	if test -d $(pkgdatadir) ; then rm -r $(pkgdatadir) ; fi
	if test -d $(mandir)/man6 ; then echo `rmdir $(mandir)/man6` ; fi
	if test -d $(mandir); then echo `rmdir $(mandir)` ; fi
	if test -d $(bindir) ; then echo `rmdir $(bindir)` ; fi
	if test -d $(datadir) ; then echo `rmdir $(datadir)` ; fi
	if test -d $(prefix) ; then echo `rmdir $(prefix)` ; fi

clean-local:
	rm -rf translations

BINARY_STAGING=/tmp/$(PACKAGE)-$(VERSION)-staging
BINARY_DIST=/tmp/$(PACKAGE)-binary-$(VERSION).tar.gz
DATA_DIST=/tmp/$(PACKAGE)-data-$(VERSION).tar.gz

binary-dist:
	@rm -rf $(BINARY_STAGING) $(BINARY_DIST)
	@make install-exec DESTDIR=$(BINARY_STAGING)
	@cd $(BINARY_STAGING) ; tar cf - * | gzip -c >$(BINARY_DIST)
	@rm -rf $(BINARY_STAGING)
	@echo "Precompiled binary tarball is in $(BINARY_DIST)"

data-dist:
	@rm -rf $(BINARY_STAGING) $(DATA_DIST)
	@make install-data DESTDIR=$(BINARY_STAGING)
	@cd $(BINARY_STAGING) ; tar cf - * | gzip -c >$(DATA_DIST)
	@rm -rf $(BINARY_STAGING)
	@echo "Precompiled data tarball is in $(DATA_DIST)"

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config/config.rpath
