ISUBDIRS = icons
SUBDIRS = intl po m4 src doc $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

bin_SCRIPTS = utils/wmlxgettext

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
if INSTALLDATA
	test -z "$(pkgdatadir)" || $(mkdir_p) "$(DESTDIR)$(pkgdatadir)"
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata) ) | while read p; do \
	  $(w_preparefileinstall) \
	  echo " $(install_sh_DATA) '$$d$$p' '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  $(install_sh_DATA) "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
if TINYGUI
	(cd $(top_srcdir) && find images images/terrain \( $(findfilterflags) -o -name '*.png' -print \) ) | while read p; do \
	  $(w_preparefileinstall) \
	  eval `pngmeta --all $$d$$p | grep -E 'image-(width|height):' | tr -d ' -' | tr : =` ;\
	  case $${imagewidth}x$$imageheight in \
	  1024x768) dstsize=320x240 ;; \
	  640x480)  dstsize=240x180 ;; \
	  205x205)  dstsize=80x80 ;; \
	  *) if [ $$imageheight -gt $$imagewidth ]; then max=`expr $$imageheight / 2`; else max=`expr $$imagewidth / 2`; fi ;\
	     dstsize=$${max}x$${max} ;; \
	  esac ; \
	  convert -size $$dstsize -resize $$dstsize "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
endif
endif

zip-install: install
	$(top_srcdir)/utils/splittree "$(DESTDIR)$(pkgdatadir)" --zip

uninstall-local:
	@$(NORMAL_UNINSTALL)
if INSTALLDATA
	@srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata) ) | while read p; do \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac; \
	  echo " rm -f '$(DESTDIR)$(pkgdatadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
endif

dist-hook:
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata); $(findnoinst) ) | (cd $(top_srcdir) && tar cf - -T -) | (cd $(distdir) && tar xf -)

dist_noinst_DATA = copyright changelog wesnoth.dsp wesnoth.dsw

uninstall-hook:
	if test -d $(pkgdatadir) ; then rm -r $(pkgdatadir) ; fi
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

# latest graph-includes.pl can be found currently at http://ydirson.free.fr/soft/wesnoth/graphs/
deps.dot:
	(cd $(top_srcdir)/src && graph-includes.pl -verbose -class wesnoth -group 1 1 \
	-color 2 widgets=linen,editor=cyan,server=pink,multiplayer=palegreen,serialization=steelblue1 \
	`find -name '*.[ch]pp'|sed s,\./,,`) > $@
%.ps: %.dot
	dot -Tps $< > $@
