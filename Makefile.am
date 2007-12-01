if USESVN
SVNREV = $(shell svnversion -n .)
endif

ISUBDIRS = icons
SUBDIRS = po m4 src doc $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

bin_SCRIPTS =

findfilterflags=! \( -name .svn -prune -o -name ".\#*" -o -name "*~" -o -name "*bak" -o -type d \)

if LITE
exclude=| egrep -v '(/sounds|/music)' 
endif

finddata=(cd $(top_srcdir) && find data fonts icons images sounds $(findfilterflags) -print ) $(exclude)
finddatadirs=(cd $(top_srcdir) && find data fonts icons images sounds -type d \! \( -name .svn -prune \) -print ) $(exclude)
findnoinst=(cd $(top_srcdir) && find utils $(findfilterflags) -print )

finddocdist=(cd $(top_srcdir) && find doc/manual -name "*.html" -o -name "*.css" -o -name "*.jpg" -o -name "*.txt")
finddoc=($(finddocdist) | sed -e 's:doc/manual:manual:g' )
finddocdirs=(cd $(top_srcdir)/doc && find manual -type d \! \( -name .svn -prune \) -print; )

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

# Run this before release to check that POTs and data are in good shape
sanity-check:
	cd utils; ./sanity_check
	cd data/tools; make sanity-check

install-data-local:
	@$(NORMAL_INSTALL)
if OPTIPNG
	(cd $(top_srcdir) && find images -name '*.png' -print  ) | while read p; do \
	$(w_preparefileinstall) \
	optipng -o4 $$d$$p; \
	done
endif
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
	test -z "$(docdir)" || $(mkdir_p) "$(DESTDIR)$(docdir)"
	( $(finddocdirs) ) | while read p; do \
	  $(mkdir_p) "$(DESTDIR)$(docdir)/$$p"; \
	done; \
	( $(finddoc) ) | while read p; do \
	  cd doc && $(w_preparefileinstall) \
	  echo $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(docdir)/$$f"; \
	  $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(docdir)/$$f"; cd ..; \
	done
if DUMMYLOCALES
	$(mkdir_p) "$(DESTDIR)$(pkgdatadir)/locales"
	if test -d "$(DESTDIR)$(pkgdatadir)/locales/C" ; then rm -r "$(DESTDIR)$(pkgdatadir)/locales/C" ; fi
	echo | localedef --force "$(DESTDIR)$(pkgdatadir)/locales/C" 2> /dev/null; \
	for loc in `ls data/languages/*.cfg | sed -e's/data\/languages\/\(.*\)\.cfg/\1/'`; do \
	  loclnk="$(DESTDIR)$(pkgdatadir)/locales/$$loc"@wesnoth; \
	  if test -L "$$loclnk" ; then rm "$$loclnk" ; fi; \
	  ln -s C "$$loclnk"; done; \
	true
endif
if TINYGUI
	(cd $(top_srcdir) && find data/core/images data/campaigns images  \( $(findfilterflags) -o -name '*.png' -and -not -name 'bar-energy-tinygui.png' -print \) ) | while read p; do \
	  $(w_preparefileinstall) \
	  eval `identify $$d$$p | cut -f 3 -d ' ' | sed s/^/imagewidth=/ | sed s/x/\;imageheight=/` ;\
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
	( $(finddoc) ) | while read p; do \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac; \
	  echo " rm -f '$(DESTDIR)$(docadir)/$$f'"; \
	  rm -f "$(DESTDIR)$(docdir)/$$f"; \
	done
	( $(finddocdirs) | sed -e 's/ /\n/g' | sort -r ) | while read p; do \
	  d=$(DESTDIR)$(docdir)/$$p && \
	  if test -d $$d ; then echo "rmdir $$d"; echo `rmdir $$d` ; fi \
	done
endif

dist-hook:
	srcdirstrip=`echo "$(srcdir)" | sed 's|.|.|g'`; \
	( $(finddata); $(finddocdist); $(findnoinst) ) | (cd $(top_srcdir) && tar cf - -T -) | (cd $(distdir) && tar xf -)

dist_noinst_DATA = copyright changelog players_changelog wesnoth.dev editor.dev server.dev wesnoth.kdevelop campaign_server.dsp map_editor.dsp server.dsp wesnoth.dsp wesnoth.dsw

uninstall-hook:
	if test -d $(pkgdatadir) ; then rm -r $(pkgdatadir) ; fi
	if test -d $(bindir) ; then echo `rmdir $(bindir)` ; fi
	if test -d $(datadir) ; then echo `rmdir $(datadir)` ; fi
	if test -d $(prefix) ; then echo `rmdir $(prefix)` ; fi
	if test -d $(docdir) ; then echo `rmdir $(docdir)` ; fi

clean-local:
	rm -rf po/stamp* po/*/stamp*

distclean-local:
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

EXTRA_DIST = config/config.rpath config/mkinstalldirs config/py-compile

if PYTHON_INSTALL
pkgpython_PYTHON = data/tools/wesnoth/wmltools.py \
    data/tools/wesnoth/wmldata.py \
    data/tools/wesnoth/wmlparser.py \
    data/tools/wesnoth/campaignserver_client.py \
    data/tools/wesnoth/__init__.py
endif

update-po:
	@cd po && make $@ || exit $?
if MANUALUPDATE
	@cd doc/manual && make update-po || exit $?
endif

if PO4AUPDATE
update-po4a:
	@cd po && make update-po4a || exit $?
	@echo -n "Removing empty man directories... "
	@find doc/man/ -maxdepth 1 -type d -empty -exec rmdir {} \; && \
		echo "done." || exit $?
if MANUALUPDATE
	@cd doc/manual && make html || exit $?
endif
endif

if MANUALUPDATE
manual-en:
	@cd doc/manual && make manual.en.html || exit $?
endif

# latest graph-includes.pl can be found currently at http://ydirson.free.fr/soft/wesnoth/graphs/
wesnoth-deps.dot:
	PERL5LIB=$(top_srcdir) graph-includes -verbose --class wesnoth --consolidate 1-1 \
	  -sysI /usr/include/c++/4.0 -sysI /usr/include -sysI /usr/include/SDL \
	  --prefixstrip $(top_srcdir)/src/ -I $(top_srcdir)/src  $(top_srcdir)/src > $@ || \
		( rm $@; false )
%.ps: %.dot
	dot -Tps $< > $@
