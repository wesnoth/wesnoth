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
		   ../network_worker.cpp \
		   ../publish_campaign.cpp \
		   ../thread.cpp \
		   ../zipios++/xcoll.cpp \
		   ../game_events.hpp \
		   ../config.hpp \
		   ../filesystem.hpp \
		   ../game_config.hpp \
		   ../gettext.hpp \
		   ../log.hpp \
		   ../network.hpp \
		   ../network_worker.hpp \
		   ../publish_campaign.hpp \
		   ../thread.hpp \
		   ../zipios++/xcoll.hpp

AM_CXXFLAGS = @SDL_CFLAGS@ -DLOCALEDIR=\"$(LOCALEDIR)\" -I$(srcdir)/..
LDADD = @SDL_NET_LIBS@ @SDL_LIBS@ $(LIBZIPIOS)
