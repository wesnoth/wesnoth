AUTOMAKE_OPTIONS = foreign
ISUBDIRS = icons
SUBDIRS = src $(ISUBDIRS)
pkgdatadir=$(datadir)/@DATADIR@

dist_noinst_DATA = @MANUAL_FILES@ copyright changelog @UTIL_FILES@
nobase_dist_pkgdata_DATA = @DATA_FILES@ @FONT_FILES@ @IMAGE_FILES@ @MUSIC_FILES@ @SOUND_FILES@
dist_man6_MANS = doc/man/wesnoth.6 doc/man/wesnothd.6 doc/man/wesnoth_editor.6

uninstall-local:
	rm -r $(prefix)
