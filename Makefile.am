ISUBDIRS = icons
SUBDIRS = po m4 src doc $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

bin_SCRIPTS =

# Ignore junk -- object files, editor backup files, wmllint backup files.
findfilterflags=! \( -name .svn -prune -o -name ".\#*" -o -name "*~" -o -name "*bak" -o -name 'Makefile' -o -name '*Makefile' -o -type d -o -regex "data/test/*" \)

# List all datafiles, ignoring junk
finddata=(cd $(top_srcdir) && find data fonts icons images sounds l10n-track $(findfilterflags) -print )
# List all data subdirectories
finddatadirs=(cd $(top_srcdir) && find data fonts icons images sounds -type d \! \( -name .svn -prune \) -print )
# List non-installable utility files
findnoinst=(cd $(top_srcdir) && find utils $(findfilterflags) -print )
# List all Header files
findnoinst_headers=(cd $(top_srcdir) && find src -name "*.hpp" -print && find src -name "*.h" -print && find src -name "*.ii" -print && find src -name "*.i" -print )
# List all files required for scons build recipe
findsconscript=(cd $(top_srcdir) && find . -name "SConscript" -print )
findsconstruct=(cd $(top_srcdir) && find . -name "SConstruct" -print )
findsconsdir=(cd $(top_srcdir) && find scons $(findfilterflags) -print )
# List all files required for cmake build recipe
findcmakelists=(cd $(top_srcdir) && find . -name "CMakeLists.txt" -print -o -name config.h.cmake -print )
findcmakedir=(cd $(top_srcdir) && find cmake $(findfilterflags) -print )
findcmakeextrafiles=(cd $(top_srcdir) && find po -name "po-update.sh" -print -o -name "pot-update.sh.in" -print -o -name "LINGUAS" -print )

# Find files derived from the manual that should go in a distribution
finddocdist=(cd $(top_srcdir) && find doc/manual -name "*.html" -o -name "*.css" -o -name "*.jpg" -o -name "*.txt")
# Same list as previous, but with doc/ stripped off the front
finddoc=($(finddocdist) | sed -e 's:doc/manual:manual:g' )
# List all document directories
finddocdirs=(cd $(top_srcdir)/doc && find manual -type d \! \( -name .svn -prune \) -print; )

# This is a workaround to include manpage and manual translation files in the
# tarball. Manual and manpages are pre-built, so these files aren't actually
# needed to build Wesnoth, but are included because .po files are the
# preferred source of modification.
findextrapo=(cd $(top_srcdir) && \
    find po/wesnoth-manpages po/wesnoth-manual \
         -name "*.pot" -o -name "*.po" -o -name "*.cfg")

w_preparefileinstall = if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  case $$p in \
	    $(srcdir)/*) f=`echo "$$p" | sed "s|^$$srcdirstrip/||"`;; \
	    *) f=$$p;; \
	  esac;

# Create Macintosh app bundle from the game 
bundle_contents=@PACKAGE_NAME@.app/Contents
@PACKAGE@_bundle: src/@PACKAGE@
	mkdir -p "$(bundle_contents)/MacOS"
	mkdir -p "$(bundle_contents)/Resources"
	echo "APPL????" > "$(bundle_contents)/PkgInfo"
	$(INSTALL_PROGRAM) $< "$(bundle_contents)/MacOS/@PACKAGE_NAME@"

# Run this before release to check that POTs and data are in good shape
sanity-check:
	cd utils; ./sanity_check
	cd data/tools; make sanity-check

install-data-local:
	@$(NORMAL_INSTALL)
if INSTALLDATA
	@echo "Replicate data directory skeleton under target shared-data directory."
	@echo "Then copy all corresponding files."
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
	@echo "Replicate doc directory skeleton under target doc subdirectory"
	@echo "Then copy all corresponding files."
	test -z "$(docdir)" || $(mkdir_p) "$(DESTDIR)$(docdir)"
	( $(finddocdirs) ) | while read p; do \
	  $(mkdir_p) "$(DESTDIR)$(docdir)/$$p"; \
	done; \
	( $(finddoc) ) | while read p; do \
	  cd doc && $(w_preparefileinstall) \
	  echo $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(docdir)/$$f"; \
	  $(INSTALL_DATA) "$$d$$p" "$(DESTDIR)$(docdir)/$$f"; cd ..; \
	done
if TINYGUI
	@echo "Shrink installed images by a factor of two."
	(cd $(top_srcdir) && find data/core/images data/campaigns images  \( $(findfilterflags) -a -name '*.png' -and -not -name 'bar-energy-tinygui.png' -print \) ) | while read p; do \
	  $(w_preparefileinstall) \
	  eval `identify $$d$$p | cut -f 3 -d ' ' | sed s/^/imagewidth=/ | sed s/x/\;imageheight=/` ;\
	  case $${imagewidth}x$$imageheight in \
	  1024x768) dstsize=320x240 ;; \
	  640x480)  dstsize=240x180 ;; \
	  205x205)  dstsize=80x80 ;; \
	  *) if [ $$imageheight -gt $$imagewidth ]; then max=`expr $$imageheight / 2`; else max=`expr $$imagewidth / 2`; fi ;\
	     dstsize=$${max}x$${max} ;; \
	  esac ; \
	  file=`basename "$$p"`; \
	  file "$$d$$p" | egrep "(RGBA|alpha)" >/dev/null 2>&1 && {\
	  	convert "$$d$$p" -filter point -resize $$dstsize "$(DESTDIR)$(pkgdatadir)/$$f";\
	  } || { \
	  	convert "$$d$$p" -resize $$dstsize "$(DESTDIR)$(pkgdatadir)/$$f"; \
	  } \
	done
endif
	@echo "Local installation is complete"
endif

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
	( $(finddata); $(finddocdist); $(findnoinst); $(findnoinst_headers); $(findextrapo); $(findsconscript); $(findsconstruct); $(findsconsdir); $(findcmakelists); $(findcmakedir); $(findcmakeextrafiles) ) | (cd $(top_srcdir) && tar cf - -T -) | (cd $(distdir) && tar xf -)

dist_noinst_DATA = copyright changelog players_changelog wesnoth.kdevelop

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
    data/tools/wesnoth/libsvn.py \
    data/tools/wesnoth/wescamp.py \
    data/tools/wesnoth/wmldata.py \
    data/tools/wesnoth/wmliterator.py \
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
