AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS =

if GAME
bin_PROGRAMS += wesnoth
endif

if SERVER
bin_PROGRAMS += wesnothd
endif

if CAMPAIGNSERVER
bin_PROGRAMS += campaignd
endif

if TOOLS
bin_PROGRAMS += exploder cutter
endif

if EDITOR
bin_PROGRAMS += wesnoth_editor
endif

pkgdatadir=$(datadir)/@DATADIR@

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = \
	about.cpp \
	actions.cpp \
	ai.cpp \
	ai_dfool.cpp \
	ai_attack.cpp \
	ai_move.cpp \
	animated.cpp \
	astarnode.cpp \
	builder.cpp \
	cavegen.cpp \
	checksum.cpp \
	clipboard.cpp \
	config.cpp \
	config_adapter.cpp \
	cursor.cpp \
	dialogs.cpp \
	display.cpp \
	events.cpp \
	filesystem.cpp \
	font.cpp \
	game.cpp \
	game_config.cpp \
	game_events.cpp \
	gamestatus.cpp \
	gettext.cpp \
	halo.cpp \
	help.cpp \
	hotkeys.cpp \
	image.cpp \
	intro.cpp \
	key.cpp \
	language.cpp \
	leader_list.cpp \
	log.cpp \
	map.cpp \
	map_create.cpp \
	map_label.cpp \
	mapgen.cpp \
	mapgen_dialog.cpp \
	marked-up_text.cpp \
	menu_events.cpp \
	minimap.cpp \
	mouse.cpp \
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
	pathutils.cpp \
	playcampaign.cpp \
	playlevel.cpp \
	playturn.cpp \
	preferences.cpp \
	preferences_display.cpp \
	publish_campaign.cpp \
	race.cpp \
	random.cpp \
	replay.cpp \
	replay_controller.cpp \
	reports.cpp \
	sdl_utils.cpp \
	show_dialog.cpp \
	sound.cpp \
	statistics.cpp \
	team.cpp \
	terrain.cpp \
	theme.cpp \
	thread.cpp \
	titlescreen.cpp \
	tooltips.cpp \
	tstring.cpp \
	unit.cpp \
	unit_display.cpp \
	unit_types.cpp \
	util.cpp \
	variable.cpp \
	video.cpp \
	wassert.cpp \
	serialization/binary_or_text.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	widgets/button.cpp \
	widgets/combo.cpp \
	widgets/label.cpp \
	widgets/menu.cpp \
	widgets/image_button.cpp \
	widgets/progressbar.cpp \
	widgets/scrollarea.cpp \
	widgets/scrollbar.cpp \
	widgets/scrollpane.cpp \
	widgets/slider.cpp \
	widgets/textbox.cpp \
	widgets/widget.cpp \
	zipios++/xcoll.cpp \
	sdl_ttf/SDL_ttf.c

#############################################################################
#    Editor                                                                 #
#############################################################################

wesnoth_editor_SOURCES = \
	editor/editor.cpp \
	editor/editor_layout.cpp \
	editor/map_manip.cpp \
	editor/editor_palettes.cpp \
	editor/editor_main.cpp \
	editor/editor_dialogs.cpp \
	editor/editor_undo.cpp \
	about.cpp \
	actions.cpp \
	animated.cpp \
	astarnode.cpp \
	builder.cpp \
	cavegen.cpp \
	checksum.cpp \
	clipboard.cpp \
	config.cpp \
	cursor.cpp \
	dialogs.cpp \
	display.cpp \
	events.cpp \
	filechooser.cpp \
	filesystem.cpp \
	font.cpp \
	game_config.cpp \
	game_events.cpp \
	gamestatus.cpp \
	gettext.cpp \
	halo.cpp \
	help.cpp \
	hotkeys.cpp \
	image.cpp \
	key.cpp \
	language.cpp \
	log.cpp \
	map_create.cpp \
	map_label.cpp \
	map.cpp \
	mapgen.cpp \
	mapgen_dialog.cpp \
	marked-up_text.cpp \
	minimap.cpp \
	mouse.cpp \
	network.cpp \
	network_worker.cpp \
	pathutils.cpp \
	pathfind.cpp \
	playturn.cpp \
	preferences.cpp \
	preferences_display.cpp \
	race.cpp \
	random.cpp \
	replay.cpp \
	reports.cpp \
	sdl_utils.cpp \
	show_dialog.cpp \
	sound.cpp \
	statistics.cpp \
	team.cpp \
	terrain.cpp \
	thread.cpp \
	theme.cpp \
	tooltips.cpp \
	tstring.cpp \
	unit.cpp \
	unit_display.cpp \
	unit_types.cpp \
	util.cpp \
	variable.cpp \
	video.cpp \
	serialization/binary_or_text.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	widgets/button.cpp \
	widgets/file_chooser.cpp \
	widgets/label.cpp \
	widgets/menu.cpp \
	widgets/progressbar.cpp \
	widgets/textbox.cpp \
	widgets/scrollarea.cpp \
	widgets/scrollbar.cpp \
	widgets/slider.cpp \
	widgets/widget.cpp \
	zipios++/xcoll.cpp \
	sdl_ttf/SDL_ttf.c

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
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	gettext.cpp \
	log.cpp \
	network.cpp \
	network_worker.cpp \
	thread.cpp \
	tstring.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	zipios++/xcoll.cpp

wesnothd_LDADD = @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBZIPIOS) $(LIBINTL)

#############################################################################
#    Campaign Server                                                        #
#############################################################################

campaignd_SOURCES = \
	campaign_server/campaign_server.cpp \
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	gettext.cpp \
	log.cpp \
	network.cpp \
	network_worker.cpp \
	publish_campaign.cpp \
	thread.cpp \
	tstring.cpp \
	serialization/binary_wml.cpp \
	serialization/parser.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	zipios++/xcoll.cpp

campaignd_LDADD = @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBZIPIOS) $(LIBINTL)

#############################################################################
#    Castle building helpers                                                #
#############################################################################

exploder_SOURCES = \
	tools/exploder.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/exploder_composer.cpp \
	tools/dummy_video.cpp \
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	sdl_utils.cpp \
	log.cpp \
	tstring.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	zipios++/xcoll.cpp

cutter_SOURCES = \
	tools/cutter.cpp \
	tools/exploder_utils.cpp \
	tools/exploder_cutter.cpp \
	tools/dummy_video.cpp \
	config.cpp \
	filesystem.cpp \
	game_config.cpp \
	sdl_utils.cpp \
	log.cpp \
	tstring.cpp \
	serialization/parser.cpp \
	serialization/preprocessor.cpp \
	serialization/string_utils.cpp \
	serialization/tokenizer.cpp \
	zipios++/xcoll.cpp

#############################################################################
#    Headers
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
	widgets/file_chooser.hpp \
	widgets/scrollpane.hpp \
	widgets/menu.hpp \
	widgets/button.hpp \
	widgets/label.hpp \
	widgets/slider.hpp \
	widgets/scrollbar.hpp \
	widgets/widget.hpp \
	widgets/scrollarea.hpp \
	widgets/image_button.hpp \
	zipios++/xcoll.hpp \
	server/player.hpp \
	server/game.hpp \
	server/input_stream.hpp \
	server/proxy.hpp \
	server/metrics.hpp \
	editor/scenario_editor.hpp \
	editor/editor_undo.hpp \
	editor/map_manip.hpp \
	editor/editor_layout.hpp \
	editor/editor.hpp \
	editor/editor_palettes.hpp \
	editor/editor_dialogs.hpp \
	statistics.hpp \
	network.hpp \
	ai2.hpp \
	cursor.hpp \
	tooltips.hpp \
	halo.hpp \
	variable.hpp \
	hotkeys.hpp \
	mouse.hpp \
	unit_display.hpp \
	theme.hpp \
	pathutils.hpp \
	mapgen.hpp \
	cavegen.hpp \
	race.hpp \
	multiplayer_connect.hpp \
	array.hpp \
	wassert.hpp \
	dialogs.hpp \
	map.hpp \
	global.hpp \
	checksum.hpp \
	video.hpp \
	ai_interface.hpp \
	multiplayer_wait.hpp \
	gettext.hpp \
	builder.hpp \
	publish_campaign.hpp \
	playlevel.hpp \
	game_config.hpp \
	terrain.hpp \
	config.hpp \
	network_worker.hpp \
	util.hpp \
	language.hpp \
	multiplayer_lobby.hpp \
	scoped_resource.hpp \
	sound.hpp \
	pathfind.hpp \
	reports.hpp \
	map_label.hpp \
	random.hpp \
	filesystem.hpp \
	help.hpp \
	ai.hpp \
	replay.hpp \
	wml_separators.hpp \
	show_dialog.hpp \
	map_create.hpp \
	actions.hpp \
	sdl_utils.hpp \
	game_errors.hpp \
	font.hpp \
	astarnode.hpp \
	multiplayer_ui.hpp \
	titlescreen.hpp \
	image.hpp \
	team.hpp \
	game_events.hpp \
	clipboard.hpp \
	events.hpp \
	playcampaign.hpp \
	multiplayer.hpp \
	thread.hpp \
	gamestatus.hpp \
	playturn.hpp \
	log.hpp \
	key.hpp \
	tstring.hpp \
	multiplayer_create.hpp \
	intro.hpp \
	preferences.hpp \
	unit_types.hpp \
	display.hpp \
	leader_list.hpp \
	about.hpp \
	unit.hpp \
	animated.hpp \
	mapgen_dialog.hpp \
	marked-up_text.hpp \
	ai_dfool.hpp \
	minimap.hpp \
	mouse_events.hpp \
	menu_events.hpp \
	preferences_display.hpp \
	replay_controller.hpp \
	config_adapter.hpp \
	sdl_ttf/SDL_ttf.h \
	wesconfig.h

exploder_LDADD = @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBZIPIOS) $(LIBINTL) $(PNG_LIBS)
cutter_LDADD = @SDL_IMAGE_LIBS@ @SDL_LIBS@ $(LIBZIPIOS) $(LIBINTL) $(PNG_LIBS)

AM_CXXFLAGS = -I $(srcdir)/sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR) -DFIFODIR=\"$(fifodir)\"

AM_CFLAGS = -I $(srcdir)/sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR)

if FRIBIDI
    AM_CXXFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
    AM_CFLAGS += -DHAVE_FRIBIDI @FRIBIDI_CFLAGS@
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

CXXLD = $(LDPREFIX) $(CXX)


THELIBS = $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(SDL_NET_LIBS) \
	$(SDL_TTF_LIBS) $(SDL_LIBS) $(LIBZIPIOS) $(FREETYPE_LIBS) $(LIBINTL) \
	@FRIBIDI_LIBS@

wesnoth_LDADD = $(THELIBS)
wesnoth_editor_LDADD = $(THELIBS)

if STATIC
LDFLAGS += -all-static
endif

#if STATIC
#LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
#-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
#-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
#-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
#endif

if SERVER
install-exec-hook:
	mkdir -p -m 0700 $(DESTDIR)$(fifodir)
	chown $(serveruid):$(servergid) $(DESTDIR)$(fifodir)

uninstall-hook:
	rm -rf $(DESTDIR)$(fifodir)
else
install-exec-hook:
uninstall-hook:
endif
