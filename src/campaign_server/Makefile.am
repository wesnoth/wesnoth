bin_PROGRAMS = campaignd

#############################################################################
#    Campaign Server                                                        #
#############################################################################

campaignd_SOURCES = campaign_server.cpp \
		   ../server/variable.cpp \
		   ../config.cpp \
		   ../filesystem.cpp \
		   ../game_config.cpp \
		   ../gettext.cpp \
		   ../log.cpp \
		   ../network.cpp \
		   ../publish_campaign.cpp \
		   ../game_events.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../gettext.hpp \
		   ../log.hpp \
		   ../network.hpp \
		   ../publish_campaign.hpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DLOCALEDIR=\"$(LOCALEDIR)\"
AM_LDFLAGS = @SDL_LIBS@ @SDL_NET_LIBS@
