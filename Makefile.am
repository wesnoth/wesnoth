ISUBDIRS = icons
SUBDIRS = po m4 src doc $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

bin_SCRIPTS = utils/wmlxgettext

findfilterflags=-name .svn -prune -o -name ".\#*" -o -type d

if LITE
findmoredata=;find images $(findfilterflags) -o -path "images/campaigns/*/story*" -o -path "images/campaigns/*/sotbe*" -o -print
findmoredatadirs=find images -type d \! \( -name .svn -prune -o -path "images/campaigns/*/story*" -o -path "images/campaigns/*/sotbe*" \) -print
else
findmoredata=;find images music $(findfilterflags) -o -print
findmoredatadirs=find images music -type d \! \( -name .svn -prune \) -print
endif

finddata=(cd $(top_srcdir) && find data fonts sounds $(findfilterflags) -o -print \
	 $(findmoredata) )
finddatadirs=(cd $(top_srcdir) && find data fonts sounds -type d \! \( -name CVS -prune \) -print; $(findmoredatadirs) )
findnoinst=(cd $(top_srcdir) && find utils -name .svn -prune -o -name ".\#*" -o -type d -o -print && find MANUAL*)

w_preparefileinstall = if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac;

bundle_contents=@PACKAGE_NAME@.app/Contents
@PACKAGE@_bundle: src/@PACKAGE@
	mkdir -p "$(bundle_contents)/MacOS"
	mkdir -p "$(bundle_contents)/Resources"
	echo "APPL????" > "$(bundle_contents)/PkgInfo"
	$(INSTALL_PROGRAM) $< "$(bundle_contents)/MacOS/@PACKAGE_NAME@"

bundle_editor_contents=@PACKAGE_NAME@ Editor.app/Contents
@PACKAGE@_editor_bundle: src/@PACKAGE@_editor
	mkdir -p "$(bundle_editor_contents)/MacOS"
	mkdir -p "$(bundle_editor_contents)/Resources"
	echo "APPL????" > "$(bundle_editor_contents)/PkgInfo"
	$(INSTALL_PROGRAM) $< "$(bundle_editor_contents)/MacOS/@PACKAGE_NAME@ Editor"

install-data-local:
	@$(NORMAL_INSTALL)
if INSTALLDATA
	test -z "$(pkgdatadir)" || $(mkdir_p) "$(DESTDIR)$(pkgdatadir)"
	( $(finddatadirs) ) | while read p; do \
	  $(mkdir_p) "$(DESTDIR)$(pkgdatadir)/$$p"; \
	done; \
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata) ) | while read p; do \
	  $(w_preparefileinstall) \
	  echo $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	  $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(pkgdatadir)/$$f"; \
	done
if DUMMYLOCALES
	$(mkdir_p) "$(DESTDIR)$(pkgdatadir)/locales"
	if test -d "$(DESTDIR)$(pkgdatadir)/locales/C" ; then rm -r "$(DESTDIR)$(pkgdatadir)/locales/C" ; fi
	echo | localedef --force "$(DESTDIR)$(pkgdatadir)/locales/C" 2> /dev/null; \
	for loc in `sed -n 's/^.*language_def..\(.._..\).*$$/\1/p' "$(srcdir)/src/language.cpp"`; do \
	  loclnk="$(DESTDIR)$(pkgdatadir)/locales/$$loc"@wesnoth; \
	  if test -L "$$loclnk" ; then rm "$$loclnk" ; fi; \
	  ln -s C "$$loclnk"; done; \
	true
endif
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

update-po:
	@cd po && make $@ || exit $?

# latest graph-includes.pl can be found currently at http://ydirson.free.fr/soft/wesnoth/graphs/
wesnoth-deps.dot:
	(cd $(top_srcdir) && graph-includes -verbose --class wesnoth --group 1-1 \
	  -sysI /usr/include/c++/3.3 -sysI /usr/include -sysI /usr/include/SDL \
	  --prefixstrip src/ -I src  src) > $@
%.ps: %.dot
	dot -Tps $< > $@
