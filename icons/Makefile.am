appicondir = $(datadir)/icons
appicon_DATA = ../images/wesnoth-icon.png

desktopdir = $(datadir)/applications
dist_desktop_DATA = wesnoth.desktop

if GNOME1
gnome1desktopdir = @GNOME1_DESKTOP@
gnome1desktop_DATA = $(dist_desktop_DATA)
endif

if KDE
kdedesktopdir = @KDE_DESKTOP@
kdedesktop_DATA = $(dist_desktop_DATA)
kdeicondir = @KDE_ICON@
kdeicon_DATA = $(appicon_DATA)
endif
