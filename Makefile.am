AUTOMAKE_OPTIONS = foreign
ISUBDIRS = icons
SUBDIRS = src $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

dist_noinst_DATA = @MANUAL_FILES@ copyright changelog @UTIL_FILES@
nobase_dist_pkgdata_DATA = @DATA_FILES@ @FONT_FILES@ @IMAGE_FILES@ @MUSIC_FILES@ @SOUND_FILES@
dist_man6_MANS = doc/man/wesnoth.6 doc/man/wesnothd.6 doc/man/wesnoth_editor.6

uninstall-hook:
	if test -d $(pkgdatadir) ; then rm -r $(pkgdatadir) ; fi
	if test -d $(mandir)/man6 ; then echo `rmdir $(mandir)/man6` ; fi
	if test -d $(mandir); then echo `rmdir $(mandir)` ; fi
	if test -d $(bindir) ; then echo `rmdir $(bindir)` ; fi
	if test -d $(datadir) ; then echo `rmdir $(datadir)` ; fi
	if test -d $(prefix) ; then echo `rmdir $(prefix)` ; fi
