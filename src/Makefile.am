AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS =
bin_SCRIPTS =

if GAME
bin_PROGRAMS += wesnoth
endif

if SERVER
bin_PROGRAMS += wesnothd
endif

if CAMPAIGNSERVER
bin_PROGRAMS += campaignd
endif

if TESTS
bin_PROGRAMS += test
endif

CLEANFILES = 

if TOOLS
bin_PROGRAMS += exploder cutter
bin_SCRIPTS += wmlscope wmllint wmlindent
CLEANFILES += $(bin_SCRIPTS)
endif

if EDITOR
bin_PROGRAMS += wesnoth_editor
endif

noinst_LIBRARIES = libwesnoth-core.a libwesnoth.a

pkgdatadir=$(datadir)/@DATADIR@

THELIBS = -L. -lwesnoth-core $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(SDL_NET_LIBS) \
	$(SDL_TTF_LIBS) $(SDL_LIBS) $(PYTHON_LIBS) $(FREETYPE_LIBS) $(LIBINTL) \
	$(BOOST_IOSTREAMS_LIBS) 

wesnoth_source = \
	about.cpp \
	actions.cpp \
	ai.cpp \
	ai_dfool.cpp \
	ai_attack.cpp \
	ai_move.cpp \
	ai_python.cpp \
	ai_village.cpp \
	animated_game.cpp \
	attack_prediction.cpp \
	config_adapter.cpp \
	dialogs.cpp \
	floating_textbox.cpp \
	game_display.cpp \
	game_events.cpp \
	game_preferences.cpp \
	game_preferences_display.cpp \
	gamestatus.cpp \
	generate_report.cpp \
	generic_event.cpp \
	halo.cpp \
	help.cpp \
	intro.cpp \
	leader_list.cpp \
	menu_events.cpp \
	mouse_events.cpp \
	multiplayer.cpp \
	multiplayer_ui.cpp \
	multiplayer_wait.cpp \
	multiplayer_connect.cpp \
	multiplayer_create.cpp \
	multiplayer_lobby.cpp \
	network.cpp \
	network_worker.cpp \
	pathfind.cpp \
	playcampaign.cpp \
	play_controller.cpp \
	playmp_controller.cpp \
	playsingle_controller.cpp \
	playturn.cpp \
	publish_campaign.cpp \
	replay.cpp \
	replay_controller.cpp \
	sha1.cpp \
	settings.cpp \
	statistics.cpp \
	team.cpp \
	terrain_filter.cpp \
	titlescreen.cpp \
	tooltips.cpp \
	unit.cpp \
	unit_abilities.cpp \
	unit_animation.cpp \
	unit_display.cpp \
	unit_frame.cpp \
	unit_map.cpp \
	unit_types.cpp \
	upload_log.cpp \
	variable.cpp \
	widgets/combo.cpp \
	widgets/scrollpane.cpp 

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = \
	game.cpp \
	$(wesnoth_source)
	
wesnoth_LDADD = $(THELIBS) libwesnoth.a
wesnoth_DEPENDENCIES=libwesnoth-core.a libwesnoth.a

#############################################################################
#    Editor                                                                 #
#############################################################################

wesnoth_editor_SOURCES = \
	editor/editor.cpp \
	editor/editor_layout.cpp \
	editor/map_manip.cpp \
	editor/editor_display.cpp \
	editor/editor_palettes.cpp \
	editor/editor_main.cpp \
	editor/editor_dialogs.cpp \
	editor/editor_undo.cpp \
	animated_editor.cpp \
	gamestatus_editor.cpp \
	generic_event.cpp \
	tooltips.cpp 

wesnoth_editor_LDADD = $(THELIBS) libwesnoth.a
wesnoth_editor_DEPENDENCIES=libwesnoth-core.a libwesnoth.a

#############################################################################
#    Wesnothd Server                                                        #
#############################################################################

wesnothd_SOURCES = \
	server/game.cpp \
	server/input_stream.cpp \
	server/metrics.cpp \
	server/player.cpp \
	server/proxy.cpp \
	server/server.cpp \
	server/monitor.cpp \
	network.cpp \
	network_worker.cpp \
	loadscreen_empty.cpp 

wesnothd_LDADD = -L. -lwesnoth-core $(GTOP_LIBS) $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
wesnothd_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Campaign Server                                                        #
#############################################################################

campaignd_SOURCES = \
	campaign_server/campaign_server.cpp \
	network.cpp \
	network_worker.cpp \
	publish_campaign.cpp \
	loadscreen_empty.cpp 

campaignd_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
campaignd_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Castle building helpers                                                #
#############################################################################

exploder_SOURCES = \
	tools/exploder.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/exploder_composer.cpp \
	tools/dummy_video.cpp \
	sdl_utils.cpp \
	loadscreen_empty.cpp 

cutter_SOURCES = \
	tools/cutter.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/dummy_video.cpp \
	sdl_utils.cpp \
	loadscreen_empty.cpp 

exploder_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
exploder_DEPENDENCIES=libwesnoth-core.a
cutter_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
cutter_DEPENDENCIES=libwesnoth-core.a

#############################################################################
#    Unit tests                                                             #
#############################################################################

test_SOURCES = \
	tests/main.cpp \
	tests/test_util.cpp \
	$(wesnoth_source)

test_LDADD = $(THELIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) libwesnoth.a 
test_DEPENDENCIES=libwesnoth-core.a libwesnoth.a

#############################################################################
#    Headers                                                                #
#############################################################################

noinst_HEADERS = \
	tools/exploder_composer.hpp \
	tools/exploder_utils.hpp \
	tools/exploder_cutter.hpp \
	serialization/tokenizer.hpp \
	serialization/parser.hpp \
	serialization/binary_or_text.hpp \
	serialization/binary_wml.hpp \
	serialization/preprocessor.hpp \
	serialization/string_utils.hpp \
	widgets/progressbar.hpp \
	widgets/textbox.hpp \
	widgets/combo.hpp \
	widgets/file_menu.hpp \
	widgets/scrollpane.hpp \
	widgets/menu.hpp \
	widgets/button.hpp \
	widgets/label.hpp \
	widgets/slider.hpp \
	widgets/scrollbar.hpp \
	widgets/widget.hpp \
	widgets/scrollarea.hpp \
	server/player.hpp \
	server/game.hpp \
	server/input_stream.hpp \
	server/proxy.hpp \
	server/metrics.hpp \
	editor/editor_undo.hpp \
	editor/map_manip.hpp \
	editor/editor_layout.hpp \
	editor/editor.hpp \
	editor/editor_palettes.hpp \
	editor/editor_dialogs.hpp \
	about.hpp \
	actions.hpp \
	ai.hpp \
	ai2.hpp \
	ai_dfool.hpp \
	ai_interface.hpp \
	ai_python.hpp \
	animated.hpp \
	animated.i \
	array.hpp \
	astarnode.hpp \
	attack_prediction.hpp \
	builder.hpp \
	cavegen.hpp \
	clipboard.hpp \
	color_range.hpp \
	config.hpp \
	config_adapter.hpp \
	construct_dialog.hpp \
	cursor.hpp \
	dialogs.hpp \
	display.hpp \
	events.hpp \
	file_chooser.hpp \
	filesystem.hpp \
	floating_textbox.hpp \
	font.hpp \
	game_config.hpp \
	game_display.hpp \
	game_errors.hpp \
	game_events.hpp \
	game_preferences.hpp \
	gamestatus.hpp \
	generic_event.hpp \
	gettext.hpp \
	global.hpp \
	halo.hpp \
	help.hpp \
	hotkeys.hpp \
	image.hpp \
	intro.hpp \
	key.hpp \
	language.hpp \
	leader_list.hpp \
	loadscreen.hpp \
	log.hpp \
	map.hpp \
	map_create.hpp \
	map_label.hpp \
	mapgen.hpp \
	mapgen_dialog.hpp \
	marked-up_text.hpp \
	menu_events.hpp \
	minimap.hpp \
	mouse_events.hpp \
	multiplayer.hpp \
	multiplayer_connect.hpp \
	multiplayer_create.hpp \
	multiplayer_lobby.hpp \
	multiplayer_ui.hpp \
	multiplayer_wait.hpp \
	network.hpp \
	network_worker.hpp \
	pathfind.hpp \
	pathutils.hpp \
	play_controller.hpp \
	playcampaign.hpp \
	playmp_controller.hpp \
	playsingle_controller.hpp \
	playturn.hpp \
	preferences.hpp \
	preferences_display.hpp \
	publish_campaign.hpp \
	race.hpp \
	random.hpp \
	replay.hpp \
	replay_controller.hpp \
	reports.hpp \
	scoped_resource.hpp \
	sha1.hpp \
	settings.hpp \
	sdl_utils.hpp \
	show_dialog.hpp \
	sound.hpp \
	soundsource.hpp \
	statistics.hpp \
	team.hpp \
	terrain.hpp \
	terrain_filter.hpp \
	terrain_translation.hpp \
	theme.hpp \
	thread.hpp \
	titlescreen.hpp \
	tooltips.hpp \
	tstring.hpp \
	unit.hpp \
	unit_abilities.hpp \
	unit_animation.hpp \
	unit_display.hpp \
	unit_frame.hpp \
	unit_map.hpp \
	unit_types.hpp \
	upload_log.hpp \
	util.hpp \
	variable.hpp \
	video.hpp \
	wml_separators.hpp \
	sdl_ttf/SDL_ttf.h \
	wesconfig.h \
	wml_exception.hpp \
	revision_stamp.hpp

BUILT_SOURCES = revision_stamp.hpp

#############################################################################
#    Headers
#############################################################################

#color_range.cpp should be removed, but game_config depends on it.
#game_config has very few things that are needed elsewhere, it should be
#removed.  Requires moving path and version at least to other files.

libwesnoth_core_a_SOURCES = \
	color_range.cpp \
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	gettext.cpp \
	log.cpp \
	map.cpp \
	thread.cpp \
	tstring.cpp \
	util.cpp \
	serialization/binary_or_text.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp

libwesnoth_a_SOURCES = \
	astarnode.cpp \
	astarsearch.cpp \
	builder.cpp \
	cavegen.cpp \
	clipboard.cpp \
	construct_dialog.cpp \
	cursor.cpp \
	display.cpp \
	events.cpp \
	filechooser.cpp \
	font.cpp \
	hotkeys.cpp \
	image.cpp \
	key.cpp \
	language.cpp \
	loadscreen.cpp \
	map_create.cpp \
	map_label.cpp \
	mapgen.cpp \
	mapgen_dialog.cpp \
	marked-up_text.cpp \
	minimap.cpp \
	pathutils.cpp \
	preferences.cpp \
	preferences_display.cpp \
	race.cpp \
	random.cpp \
	reports.cpp \
	sdl_utils.cpp \
	show_dialog.cpp \
	sound.cpp \
	soundsource.cpp \
	terrain.cpp \
	terrain_translation.cpp \
	video.cpp \
	theme.cpp \
	widgets/button.cpp \
	widgets/file_menu.cpp \
	widgets/label.cpp \
	widgets/menu.cpp \
	widgets/menu_style.cpp \
	widgets/progressbar.cpp \
	widgets/scrollarea.cpp \
	widgets/scrollbar.cpp \
	widgets/slider.cpp \
	widgets/textbox.cpp \
	widgets/widget.cpp \
	sdl_ttf/SDL_ttf.c \
	wml_exception.cpp 

# Should be touched each time the SVN revision number goes up 
revision_stamp.hpp: FORCE
	echo "#define SVNREV \"$(shell svnversion -n $(topdir) 2>/dev/null)\"" >revision_stamp.hpp

FORCE:

AM_CXXFLAGS = -I $(srcdir)/sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@  -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR) -DFIFODIR=\"$(fifodir)\"\
	$(GTOP_CFLAGS)

AM_CFLAGS = -I $(srcdir)/sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR) $(GTOP_CFGLAGS)

if PYTHON
AM_CXXFLAGS += @PYTHON_CFLAGS@
AM_CFLAGS += @PYTHON_CFLAGS@
endif

if FRIBIDI
    AM_CXXFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
    AM_CFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
    THELIBS += @FRIBIDI_LIBS@
endif

if X11
    CXXFLAGS += -D_X11 @X_CFLAGS@
    CFLAGS += -D_X11 @X_CFLAGS@
endif
if GCC
    CXXFLAGS += -fno-omit-frame-pointer
    CFLAGS += -fno-omit-frame-pointer
endif
if PREFSDIR
    CXXFLAGS += -DPREFERENCES_DIR=\"$(prefsdir)\"
endif

if BOOST_TEST_DYN_LINK
    CXXFLAGS += -DBOOST_TEST_DYN_LINK
endif

if BOOST_AUTO_TEST
    CXXFLAGS += -DWESNOTH_BOOST_AUTO_TEST_MAIN
else
    CXXFLAGS += -DWESNOTH_BOOST_TEST_MAIN
endif

CXXLD = $(LDPREFIX) $(CXX)
CXXFLAGS += $(BOOST_CPPFLAGS)
if SERVER_MONITOR
    CXXFLAGS += -DSERVER_MONITOR
endif

if STATIC
LDFLAGS += -all-static
endif

LDFLAGS += $(BOOST_IOSTREAMS_LDFLAGS)

#if STATIC
#LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
#-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
#-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
#-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
#endif

if SERVER
install-exec-hook:
	mkdir -p -m 0700 $(DESTDIR)$(fifodir)
	-chown $(serveruid):$(servergid) $(DESTDIR)$(fifodir)

uninstall-hook:
	rm -rf $(DESTDIR)$(fifodir)
else
install-exec-hook:
uninstall-hook:
endif

#############################################################################
#    Tag support
#############################################################################

CLEANFILES += TAGS

TAGS:
	etags -l c++ $(wesnoth_SOURCES) $(wesnoth_editor_SOURCES) $(wesnothd_SOURCES) $(campaignd_SOURCES) $(exploder_SOURCES) $(cutter_SOURCES) $(libwesnoth_core_a_SOURCES) $(libwesnoth__a_SOURCES) 

#############################################################################
#    Scripts
#############################################################################

$(bin_SCRIPTS):
	cp ../data/tools/$@ . 
