## some includes ##
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_PATH} )
INCLUDE_DIRECTORIES( ${SDLIMAGE_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${SDLMIXER_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${SDLNET_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${SDLTTF_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${SDL_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} )
# needed to get include paths in the subfolders correct
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/src/ )

########### libwesnoth-core ###############

SET(libwesnoth-core_STAT_SRC
    color_range.cpp
    config.cpp
    filesystem.cpp
    game_config.cpp
    gettext.cpp
    log.cpp
    map.cpp
    thread.cpp
    tstring.cpp
    util.cpp
    serialization/binary_or_text.cpp
    serialization/binary_wml.cpp
    serialization/parser.cpp
    serialization/preprocessor.cpp
    serialization/string_utils.cpp
    serialization/tokenizer.cpp
)

add_library(libwesnoth-core STATIC ${libwesnoth-core_STAT_SRC})

########### libwesnoth ###############

SET(libwesnoth_STAT_SRC
    astarnode.cpp
    astarsearch.cpp
    builder.cpp
    cavegen.cpp
    clipboard.cpp
    construct_dialog.cpp
    cursor.cpp
    display.cpp
    events.cpp
    filechooser.cpp
    font.cpp
    hotkeys.cpp
    image.cpp
    key.cpp
    language.cpp
    loadscreen.cpp
    map_create.cpp
    map_label.cpp
    mapgen.cpp
    mapgen_dialog.cpp
    marked-up_text.cpp
    minimap.cpp
    pathutils.cpp
    preferences.cpp
    preferences_display.cpp
    race.cpp
    random.cpp
    reports.cpp
    sdl_utils.cpp
    show_dialog.cpp
    sound.cpp
    soundsource.cpp
    terrain.cpp
    terrain_translation.cpp
    video.cpp
    theme.cpp
    widgets/button.cpp
    widgets/file_menu.cpp
    widgets/label.cpp
    widgets/menu.cpp
    widgets/menu_style.cpp
    widgets/progressbar.cpp
    widgets/scrollarea.cpp
    widgets/scrollbar.cpp
    widgets/slider.cpp
    widgets/textbox.cpp
    widgets/widget.cpp
    wml_exception.cpp
)

add_library(libwesnoth STATIC ${libwesnoth_STAT_SRC})

########### Wesnoth ###############

SET(wesnoth-main_SRC
    about.cpp
    actions.cpp
    ai.cpp
    ai_dfool.cpp
    ai_attack.cpp
    ai_move.cpp
    ai_python.cpp
    ai_village.cpp
    animated_game.cpp
    attack_prediction.cpp
    callable_objects.cpp
    config_adapter.cpp
    dialogs.cpp
    floating_textbox.cpp
    formula.cpp
    formula_ai.cpp
    formula_function.cpp
    formula_tokenizer.cpp
    game_display.cpp
    game_events.cpp
    game_preferences.cpp
    game_preferences_display.cpp
    gamestatus.cpp
    generate_report.cpp
    generic_event.cpp
    gui/dialogs/addon_connect.cpp
    gui/widgets/button.cpp
    gui/widgets/canvas.cpp
    gui/widgets/control.cpp
    gui/widgets/event_handler.cpp
    gui/widgets/grid.cpp
    gui/widgets/label.cpp
    gui/widgets/panel.cpp
    gui/widgets/settings.cpp
    gui/widgets/spacer.cpp
    gui/widgets/text.cpp
    gui/widgets/text_box.cpp
    gui/widgets/tooltip.cpp
    gui/widgets/helper.cpp
    gui/widgets/widget.cpp
    gui/widgets/window.cpp
    gui/widgets/window_builder.cpp
    halo.cpp
    help.cpp
    intro.cpp
    leader_list.cpp
    menu_events.cpp
    mouse_events.cpp
    multiplayer.cpp
    multiplayer_ui.cpp
    multiplayer_wait.cpp
    multiplayer_connect.cpp
    multiplayer_create.cpp
    multiplayer_lobby.cpp
    network.cpp
    network_worker.cpp
    pathfind.cpp
    playcampaign.cpp
    play_controller.cpp
    playmp_controller.cpp
    playsingle_controller.cpp
    playturn.cpp
    publish_campaign.cpp
    replay.cpp
    replay_controller.cpp
    sha1.cpp
    settings.cpp
    statistics.cpp
    team.cpp
    terrain_filter.cpp
    titlescreen.cpp
    tooltips.cpp
    unit.cpp
    unit_abilities.cpp
    unit_animation.cpp
    unit_display.cpp
    unit_frame.cpp
    unit_map.cpp
    unit_types.cpp
    upload_log.cpp
    variable.cpp
    variant.cpp
    widgets/combo.cpp
    widgets/scrollpane.cpp
)

SET(wesnoth_SRC
    game.cpp
    ${wesnoth-main_SRC}
)

ADD_EXECUTABLE(wesnoth ${wesnoth_SRC})
TARGET_LINK_LIBRARIES(wesnoth libwesnoth-core libwesnoth)

########### Editor ###############

SET(wesnoth_editor_SRC
    editor/editor.cpp
    editor/editor_layout.cpp
    editor/map_manip.cpp
    editor/editor_display.cpp
    editor/editor_palettes.cpp
    editor/editor_main.cpp
    editor/editor_dialogs.cpp
    editor/editor_undo.cpp
    animated_editor.cpp
    generic_event.cpp
    tooltips.cpp 
)

ADD_EXECUTABLE(wesnoth_editor ${wesnoth_editor_SRC})
TARGET_LINK_LIBRARIES(wesnoth_editor libwesnoth-core)

########### Wesnothd Server ###############

SET(wesnothd_SRC
    server/game.cpp
    server/input_stream.cpp
    server/metrics.cpp
    server/player.cpp
    server/proxy.cpp
    server/server.cpp
    server/simple_wml.cpp
    network.cpp
    network_worker.cpp
    loadscreen_empty.cpp 
)

ADD_EXECUTABLE(wesnothd ${wesnothd_SRC})
TARGET_LINK_LIBRARIES(wesnothd libwesnoth-core)

########### Campaign Server ###############

SET(campaignd_SRC
    campaign_server/campaign_server.cpp
    network.cpp
    network_worker.cpp
    publish_campaign.cpp
    loadscreen_empty.cpp
)

ADD_EXECUTABLE(campaignd ${campaignd_SRC})
TARGET_LINK_LIBRARIES(campaignd libwesnoth-core)

########### Castle building helpers ###############

SET(exploder_SRC
    tools/exploder.cpp
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    tools/exploder_composer.cpp
    tools/dummy_video.cpp
    sdl_utils.cpp
    loadscreen_empty.cpp
)

ADD_EXECUTABLE(exploder ${exploder_SRC})
TARGET_LINK_LIBRARIES(exploder libwesnoth-core)

SET(cutter_SRC
    tools/cutter.cpp
    tools/exploder_utils.cpp
    tools/exploder_cutter.cpp
    tools/dummy_video.cpp
    sdl_utils.cpp
    loadscreen_empty.cpp
)

ADD_EXECUTABLE(cutter ${cutter_SRC})
TARGET_LINK_LIBRARIES(cutter libwesnoth-core)

########### Unit tests ###############

SET(exploder_SRC
    tests/main.cpp
    tests/test_util.cpp
    ${wesnoth-main_SRC}
)

ADD_EXECUTABLE(test ${wesnoth_SRC})
TARGET_LINK_LIBRARIES(test libwesnoth-core libwesnoth)

########### install files ###############

#install(FILES  DESTINATION /share/@DATADIR@)



#original Makefile.am contents follow:

#AUTOMAKE_OPTIONS = subdir-objects
#
#bin_PROGRAMS =
#bin_SCRIPTS =
#
#if GAME
#bin_PROGRAMS += wesnoth
#endif
#
#if SERVER
#bin_PROGRAMS += wesnothd
#endif
#
#if CAMPAIGNSERVER
#bin_PROGRAMS += campaignd
#endif
#
#if TESTS
#bin_PROGRAMS += test
#endif
#
#CLEANFILES = revision.hpp
#
#if TOOLS
#bin_PROGRAMS += exploder cutter
#bin_SCRIPTS += wmlscope wmllint wmlindent wesnoth_addon_manager
#CLEANFILES += $(bin_SCRIPTS)
#endif
#
#if EDITOR
#bin_PROGRAMS += wesnoth_editor
#endif
#
#noinst_LIBRARIES = libwesnoth-core.a libwesnoth.a
#
#pkgdatadir=$(datadir)/@DATADIR@
#
#INTERNALLIBS = -lwesnoth-core -lwesnoth
#
#THELIBS = -L. $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(SDL_NET_LIBS) \
#	$(SDL_TTF_LIBS) $(SDL_LIBS) $(PYTHON_LIBS) $(LIBINTL) \
#	$(BOOST_IOSTREAMS_LIBS) $(BOOST_REGEX_LIBS)
#
#wesnoth_source = \
#	about.cpp \
#	actions.cpp \
#	ai.cpp \
#	ai_dfool.cpp \
#	ai_attack.cpp \
#	ai_move.cpp \
#	ai_python.cpp \
#	ai_village.cpp \
#	animated_game.cpp \
#	attack_prediction.cpp \
#	callable_objects.cpp \
#	config_adapter.cpp \
#	dialogs.cpp \
#	floating_textbox.cpp \
#	formula.cpp \
#	formula_ai.cpp \
#	formula_function.cpp \
#	formula_tokenizer.cpp \
#	game_display.cpp \
#	game_events.cpp \
#	game_preferences.cpp \
#	game_preferences_display.cpp \
#	gamestatus.cpp \
#	generate_report.cpp \
#	generic_event.cpp \
#	gui/dialogs/addon_connect.cpp \
#	gui/widgets/button.cpp \
#	gui/widgets/canvas.cpp \
#	gui/widgets/control.cpp \
#	gui/widgets/event_handler.cpp \
#	gui/widgets/grid.cpp \
#	gui/widgets/label.cpp \
#	gui/widgets/panel.cpp \
#	gui/widgets/settings.cpp \
#	gui/widgets/spacer.cpp \
#	gui/widgets/text.cpp \
#	gui/widgets/text_box.cpp \
#	gui/widgets/tooltip.cpp \
#	gui/widgets/helper.cpp \
#	gui/widgets/widget.cpp \
#	gui/widgets/window.cpp \
#	gui/widgets/window_builder.cpp \
#	halo.cpp \
#	help.cpp \
#	intro.cpp \
#	leader_list.cpp \
#	menu_events.cpp \
#	mouse_events.cpp \
#	multiplayer.cpp \
#	multiplayer_ui.cpp \
#	multiplayer_wait.cpp \
#	multiplayer_connect.cpp \
#	multiplayer_create.cpp \
#	multiplayer_lobby.cpp \
#	network.cpp \
#	network_worker.cpp \
#	pathfind.cpp \
#	playcampaign.cpp \
#	play_controller.cpp \
#	playmp_controller.cpp \
#	playsingle_controller.cpp \
#	playturn.cpp \
#	publish_campaign.cpp \
#	replay.cpp \
#	replay_controller.cpp \
#	sha1.cpp \
#	settings.cpp \
#	statistics.cpp \
#	team.cpp \
#	terrain_filter.cpp \
#	titlescreen.cpp \
#	tooltips.cpp \
#	unit.cpp \
#	unit_abilities.cpp \
#	unit_animation.cpp \
#	unit_display.cpp \
#	unit_frame.cpp \
#	unit_map.cpp \
#	unit_types.cpp \
#	upload_log.cpp \
#	variable.cpp \
#	variant.cpp \
#	widgets/combo.cpp \
#	widgets/scrollpane.cpp
#
##############################################################################
##    Wesnoth                                                                #
##############################################################################
#
#wesnoth_SOURCES = \
#	game.cpp \
#	$(wesnoth_source)
#	
#wesnoth_LDADD = $(INTERNALLIBS) $(THELIBS)
#wesnoth_DEPENDENCIES=libwesnoth-core.a libwesnoth.a
#
##############################################################################
##    Editor                                                                 #
##############################################################################
#
#wesnoth_editor_SOURCES = \
#	editor/editor.cpp \
#	editor/editor_layout.cpp \
#	editor/map_manip.cpp \
#	editor/editor_display.cpp \
#	editor/editor_palettes.cpp \
#	editor/editor_main.cpp \
#	editor/editor_dialogs.cpp \
#	editor/editor_undo.cpp \
#	animated_editor.cpp \
#	generic_event.cpp \
#	tooltips.cpp 
#
#wesnoth_editor_LDADD = $(INTERNALLIBS) $(THELIBS)
#wesnoth_editor_DEPENDENCIES=libwesnoth-core.a libwesnoth.a
#
##############################################################################
##    Wesnothd Server                                                        #
##############################################################################
#
#wesnothd_SOURCES = \
#	server/game.cpp \
#	server/input_stream.cpp \
#	server/metrics.cpp \
#	server/player.cpp \
#	server/proxy.cpp \
#	server/server.cpp \
#	server/simple_wml.cpp \
#	network.cpp \
#	network_worker.cpp \
#	loadscreen_empty.cpp 
#
#wesnothd_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
#wesnothd_DEPENDENCIES=libwesnoth-core.a
#
##############################################################################
##    Campaign Server                                                        #
##############################################################################
#
#campaignd_SOURCES = \
#	campaign_server/campaign_server.cpp \
#	network.cpp \
#	network_worker.cpp \
#	publish_campaign.cpp \
#	loadscreen_empty.cpp 
#
#campaignd_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBINTL)
#campaignd_DEPENDENCIES=libwesnoth-core.a
#
##############################################################################
##    Castle building helpers                                                #
##############################################################################
#
#exploder_SOURCES = \
#	tools/exploder.cpp \
#	tools/exploder_utils.cpp \
#	tools/exploder_cutter.cpp \
#	tools/exploder_composer.cpp \
#	tools/dummy_video.cpp \
#	sdl_utils.cpp \
#	loadscreen_empty.cpp 
#
#cutter_SOURCES = \
#	tools/cutter.cpp \
#	tools/exploder_utils.cpp \
#	tools/exploder_cutter.cpp \
#	tools/dummy_video.cpp \
#	sdl_utils.cpp \
#	loadscreen_empty.cpp 
#
#exploder_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
#exploder_DEPENDENCIES=libwesnoth-core.a
#cutter_LDADD = -L. -lwesnoth-core $(BOOST_IOSTREAMS_LIBS) @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBINTL) $(PNG_LIBS)
#cutter_DEPENDENCIES=libwesnoth-core.a
#
##############################################################################
##    Unit tests                                                             #
##############################################################################
#
#test_SOURCES = \
#	tests/main.cpp \
#	tests/test_util.cpp \
#	$(wesnoth_source)
#
#test_LDADD = $(INTERNALLIBS) $(THELIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS)
#test_DEPENDENCIES=libwesnoth-core.a libwesnoth.a
#
##############################################################################
##    Headers                                                                #
##############################################################################
#
#noinst_HEADERS = \
#	wesconfig.h \
#	animated.i \
#	filesystem_win32.ii
#
##############################################################################
##    Headers
##############################################################################
#
##color_range.cpp should be removed, but game_config depends on it.
##game_config has very few things that are needed elsewhere, it should be
##removed.  Requires moving path and version at least to other files.
#
#libwesnoth_core_a_SOURCES = \
#	color_range.cpp \
#	config.cpp \
#	filesystem.cpp \
#	game_config.cpp \
#	gettext.cpp \
#	log.cpp \
#	map.cpp \
#	thread.cpp \
#	tstring.cpp \
#	util.cpp \
#	serialization/binary_or_text.cpp \
#	serialization/binary_wml.cpp \
#	serialization/parser.cpp \
#	serialization/preprocessor.cpp \
#	serialization/string_utils.cpp \
#	serialization/tokenizer.cpp
#
#libwesnoth_a_SOURCES = \
#	astarnode.cpp \
#	astarsearch.cpp \
#	builder.cpp \
#	cavegen.cpp \
#	clipboard.cpp \
#	construct_dialog.cpp \
#	cursor.cpp \
#	display.cpp \
#	events.cpp \
#	filechooser.cpp \
#	font.cpp \
#	hotkeys.cpp \
#	image.cpp \
#	key.cpp \
#	language.cpp \
#	loadscreen.cpp \
#	map_create.cpp \
#	map_label.cpp \
#	mapgen.cpp \
#	mapgen_dialog.cpp \
#	marked-up_text.cpp \
#	minimap.cpp \
#	pathutils.cpp \
#	preferences.cpp \
#	preferences_display.cpp \
#	race.cpp \
#	random.cpp \
#	reports.cpp \
#	sdl_utils.cpp \
#	show_dialog.cpp \
#	sound.cpp \
#	soundsource.cpp \
#	terrain.cpp \
#	terrain_translation.cpp \
#	video.cpp \
#	theme.cpp \
#	widgets/button.cpp \
#	widgets/file_menu.cpp \
#	widgets/label.cpp \
#	widgets/menu.cpp \
#	widgets/menu_style.cpp \
#	widgets/progressbar.cpp \
#	widgets/scrollarea.cpp \
#	widgets/scrollbar.cpp \
#	widgets/slider.cpp \
#	widgets/textbox.cpp \
#	widgets/widget.cpp \
#	wml_exception.cpp 
#
## Until game_config.cpp is removed, this will be needed to ensure that the
## REVISION constant gets updated correctly.  If and when that definition is
## moved elsewhere, this dependency must follow it.
#game_config.o: revision.hpp 
#
## Should be touched each time the SVN revision number goes up 
## Inclusion of this file should be guarded by HAVE_REVISION so checkouts 
## into a Windows build environment won't break.
#REVISION = $(shell LC_ALL=C svnversion -n $(topdir) 2>/dev/null)
#.PRECIOUS: revision.hpp
#revision.hpp: FORCE
#	if [ "$(REVISION)" == "" ] || [ "$(REVISION)" == "exported" ]; then echo '' >/tmp/westemp$$$$; \
#	else echo '#define REVISION    "$(REVISION)"' >/tmp/westemp$$$$; fi;\
#	if cmp -s revision.hpp /tmp/westemp$$$$ 2>/dev/null; then :; else cp /tmp/westemp$$$$ revision.hpp; fi
#
#FORCE:
#
#AM_CXXFLAGS = -DHAVE_REVISION -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@  -DWESNOTH_PATH=\"$(pkgdatadir)\" \
#	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR) -DFIFODIR=\"$(fifodir)\"
#
#AM_CFLAGS = -DHAVE_REVISION -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
#	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR)
#
#if PYTHON
#AM_CXXFLAGS += @PYTHON_CFLAGS@
#AM_CFLAGS += @PYTHON_CFLAGS@
#endif
#
#if FRIBIDI
#    AM_CXXFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
#    AM_CFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
#    THELIBS += @FRIBIDI_LIBS@
#endif
#
#if X11
#    CXXFLAGS += -D_X11 @X_CFLAGS@
#    CFLAGS += -D_X11 @X_CFLAGS@
#endif
#if GCC
#    CXXFLAGS += -fno-omit-frame-pointer
#    CFLAGS += -fno-omit-frame-pointer
#endif
#if PREFSDIR
#    CXXFLAGS += -DPREFERENCES_DIR=\"$(prefsdir)\"
#endif
#
#if BOOST_TEST_DYN_LINK
#    CXXFLAGS += -DBOOST_TEST_DYN_LINK
#endif
#
#if BOOST_AUTO_TEST
#    CXXFLAGS += -DWESNOTH_BOOST_AUTO_TEST_MAIN
#else
#    CXXFLAGS += -DWESNOTH_BOOST_TEST_MAIN
#endif
#
#CXXLD = $(LDPREFIX) $(CXX)
#CXXFLAGS += $(BOOST_CPPFLAGS)
#
#
#if STATIC
#LDFLAGS += -all-static
#endif
#
#LDFLAGS += $(BOOST_IOSTREAMS_LDFLAGS)  $(BOOST_REGEX_LDFLAGS)
#
##if STATIC
##LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
##-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
##-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
##-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
##endif
#
#if SERVER
#install-exec-hook:
#	mkdir -p -m 0700 $(DESTDIR)$(fifodir)
#	-chown $(serveruid):$(servergid) $(DESTDIR)$(fifodir)
#
#uninstall-hook:
#	rm -rf $(DESTDIR)$(fifodir)
#else
#install-exec-hook:
#uninstall-hook:
#endif
#
##############################################################################
##    Tag support
##############################################################################
#
#CLEANFILES += TAGS
#
#TAGS:
#	etags -l c++ $(wesnoth_SOURCES) $(wesnoth_editor_SOURCES) $(wesnothd_SOURCES) $(campaignd_SOURCES) $(exploder_SOURCES) $(cutter_SOURCES) $(libwesnoth_core_a_SOURCES) $(libwesnoth__a_SOURCES) 
#
##############################################################################
##    Scripts
##############################################################################
#
#$(bin_SCRIPTS):
#	cp ../data/tools/$@ . 
