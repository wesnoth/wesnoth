SUBDIRS = .

if SERVER
SUBDIRS += server
endif

if CAMPAIGNSERVER
SUBDIRS += campaign_server
endif

if TOOLS
SUBDIRS += tools
endif

bin_PROGRAMS = wesnoth

if EDITOR
bin_PROGRAMS += wesnoth_editor
endif

if WZIP
SUBDIRS += zip
endif

pkgdatadir=$(datadir)/@DATADIR@

#############################################################################
#    Wesnoth                                                                #
#############################################################################

wesnoth_SOURCES = about.cpp \
                  actions.cpp \
		  ai.cpp \
		  ai_attack.cpp \
		  ai_move.cpp \
		  astarnode.cpp \
		  builder.cpp \
		  cavegen.cpp \
		  clipboard.cpp \
		  config.cpp \
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
		  log.cpp \
		  map.cpp \
		  map_label.cpp \
		  mapgen.cpp \
		  mapgen_dialog.cpp \
		  mouse.cpp \
		  multiplayer.cpp \
		  multiplayer_client.cpp \
		  multiplayer_connect.cpp \
		  multiplayer_lobby.cpp \
		  network.cpp \
		  network_worker.cpp \
		  pathfind.cpp \
		  pathutils.cpp \
		  playlevel.cpp \
		  playturn.cpp \
		  preferences.cpp \
		  publish_campaign.cpp \
		  race.cpp \
		  replay.cpp \
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
		  unit.cpp \
		  unit_display.cpp \
		  unit_types.cpp \
		  video.cpp \
		  wassert.cpp \
		  widgets/button.cpp \
		  widgets/combo.cpp \
		  widgets/label.cpp \
		  widgets/menu.cpp \
		  widgets/progressbar.cpp \
		  widgets/scrollarea.cpp \
		  widgets/scrollbar.cpp \
		  widgets/scrollpane.cpp \
		  widgets/slider.cpp \
		  widgets/textbox.cpp \
		  widgets/widget.cpp \
		  zipios++/xcoll.cpp \
		  sdl_ttf/SDL_ttf.c \
		  about.hpp \
		  actions.hpp \
		  ai.hpp \
		  ai2.hpp \
		  ai_attack.hpp \
		  ai_interface.hpp \
		  ai_move.hpp \
		  animated.hpp \
		  array.hpp \
		  astarnode.hpp \
		  builder.hpp \
		  cavegen.hpp \
		  clipboard.hpp \
		  cursor.hpp \
		  config.hpp \
		  dialogs.hpp \
		  display.hpp \
		  events.hpp \
		  filesystem.hpp \
		  font.hpp \
		  game.hpp \
		  game_config.hpp \
		  game_events.hpp \
		  gamestatus.hpp \
		  gettext.hpp \
		  global.hpp \
		  halo.hpp \
		  help.hpp \
		  hotkeys.hpp \
		  image.hpp \
		  intro.hpp \
		  key.hpp \
		  language.hpp \
		  log.hpp \
		  map.hpp \
		  map_label.hpp \
		  mapgen.hpp \
		  mapgen_dialog.hpp \
		  mouse.hpp \
		  multiplayer.hpp \
		  multiplayer_client.hpp \
		  multiplayer_connect.hpp \
		  multiplayer_lobby.hpp \
		  network.hpp \
		  network_worker.hpp \
		  pathfind.hpp \
		  pathutils.hpp \
		  playlevel.hpp \
		  playturn.hpp \
		  preferences.hpp \
		  publish_campaign.hpp \
		  race.hpp \
		  replay.hpp \
		  reports.hpp \
		  scoped_resource.hpp \
		  sdl_utils.hpp \
		  show_dialog.hpp \
		  sound.hpp \
		  statistics.hpp \
		  team.hpp \
		  terrain.hpp \
		  theme.hpp \
		  thread.hpp \
		  titlescreen.hpp \
		  tooltips.hpp \
		  unit.hpp \
		  unit_display.hpp \
		  unit_types.hpp \
		  util.hpp \
		  video.hpp \
		  wassert.hpp \
		  widgets/button.hpp \
		  widgets/combo.hpp \
		  widgets/label.hpp \
		  widgets/menu.hpp \
		  widgets/progressbar.hpp \
		  widgets/scrollarea.hpp \
		  widgets/scrollbar.hpp \
		  widgets/scrollpane.hpp \
		  widgets/slider.hpp \
		  widgets/textbox.hpp \
		  widgets/widget.hpp \
		  zipios++/xcoll.hpp \
		  sdl_ttf/SDL_ttf.h \
		  wesconfig.h

#############################################################################
#    Editor                                                                 #
#############################################################################

wesnoth_editor_SOURCES = editor/editor.cpp \
			 editor/editor_layout.cpp \
			 editor/map_manip.cpp \
			 editor/editor_palettes.cpp \
			 editor/editor_main.cpp \
			 editor/editor_dialogs.cpp \
			 editor/editor_undo.cpp \
			 about.cpp \
			 actions.cpp \
			 astarnode.cpp \
			 builder.cpp \
			 cavegen.cpp \
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
		         map_label.cpp \
			 map.cpp \
			 mapgen.cpp \
			 mapgen_dialog.cpp \
			 mouse.cpp \
			 network.cpp \
			 network_worker.cpp \
			 pathutils.cpp \
			 pathfind.cpp \
			 playturn.cpp \
			 preferences.cpp \
			 race.cpp \
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
			 unit.cpp \
			 unit_display.cpp \
			 unit_types.cpp \
			 video.cpp \
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
			 sdl_ttf/SDL_ttf.c \
			 editor/editor.hpp \
			 editor/editor_layout.hpp \
			 editor/map_manip.hpp \
			 editor/editor_palettes.hpp \
			 editor/editor_dialogs.hpp \
			 editor/editor_undo.hpp \
			 about.hpp \
			 actions.hpp \
			 ai.hpp \
			 ai_attack.hpp \
			 ai_move.hpp \
			 array.hpp \
			 astarnode.hpp \
		         builder.hpp \
		 	 cavegen.hpp \
			 clipboard.hpp \
			 config.hpp \
			 cursor.hpp \
			 dialogs.hpp \
			 display.hpp \
			 events.hpp \
			 filesystem.hpp \
			 font.hpp \
			 game_config.hpp \
			 game_events.hpp \
			 gamestatus.hpp \
			 gettext.hpp \
			 halo.hpp \
			 help.hpp \
			 hotkeys.hpp \
			 image.hpp \
			 intro.hpp \
			 key.hpp \
			 language.hpp \
			 log.hpp \
			 map_label.hpp \
			 map.hpp \
			 mapgen.hpp \
			 mapgen_dialog.hpp \
			 mouse.hpp \
			 network.hpp \
			 network_worker.hpp \
			 pathfind.hpp \
			 pathutils.hpp \
			 playlevel.hpp \
			 playturn.hpp \
			 preferences.hpp \
			 race.hpp \
			 replay.hpp \
			 reports.hpp \
			 sdl_utils.hpp \
			 show_dialog.hpp \
			 sound.hpp \
			 statistics.hpp \
			 team.hpp \
			 terrain.hpp \
			 theme.hpp \
			 thread.hpp \
			 tooltips.hpp \
			 unit.hpp \
			 unit_display.hpp \
			 unit_types.hpp \
			 video.hpp \
			 widgets/button.hpp \
			 widgets/file_chooser.hpp \
			 widgets/label.hpp \
			 widgets/menu.hpp \
			 widgets/progressbar.hpp \
			 widgets/textbox.hpp \
			 widgets/scrollarea.hpp \
			 widgets/scrollbar.hpp \
			 widgets/slider.hpp \
			 widgets/widget.hpp \
			 zipios++/xcoll.hpp \
			 sdl_ttf/SDL_ttf.h \
			 wesconfig.h

AM_CXXFLAGS = -I sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR)

AM_CFLAGS = -I sdl_ttf -I../intl -I$(top_srcdir)/intl @SDL_CFLAGS@ -DWESNOTH_PATH=\"$(pkgdatadir)\" \
	-DLOCALEDIR=\"$(LOCALEDIR)\" -DHAS_RELATIVE_LOCALEDIR=$(HAS_RELATIVE_LOCALEDIR)

if X11
    CXXFLAGS += -D_X11 @X_CFLAGS@
    CFLAGS += -D_X11 @X_CFLAGS@
endif
if GCC
    CXXFLAGS += -fno-omit-frame-pointer
    CFLAGS += -fno-omit-frame-pointer
endif

CXXLD = $(LDPREFIX) $(CXX)


LIBS = $(SDL_IMAGE_LIBS) $(SDL_MIXER_LIBS) $(SDL_NET_LIBS) \
	$(SDL_TTF_LIBS) $(SDL_LIBS) $(LIBZIPIOS) $(FREETYPE_LIBS) $(LIBINTL)

if STATIC
LDFLAGS += -all-static
endif

#if STATIC
#LIBS += -static -L/usr/lib -Wl,-rpath,/usr/lib -L/usr/X11R6/lib -lSDL_image \
#-lSDL_mixer -lSDL_net -lSDL_ttf -lSDL -lpthread -ldl -laudio -lXt -lX11 -lXext \
#-lSDL_image -lSDL_mixer -lSDL_net -lSDL_ttf -lstdc++ -lm -lSM -lICE -ljpeg \
#-lpng -lz -lvorbisfile -lvorbis -logg -lsmpeg -lfreetype
#endif
