# Microsoft Developer Studio Project File - Name="Map Editor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Map Editor - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Map Editor.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Map Editor.mak" CFG="Map Editor - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Map Editor - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "Map Editor - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Map Editor - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /I "c:\entwicklung\wesnoth\wesnoth-1.1.1\src" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_FRIBIDI" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /machine:I386 /out:"Release/editor.exe"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "Map Editor - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_FRIBIDI" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src" /I "f:/projets/include/sdl" /I "f:/projets/include" /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src/sdl_ttf" /I "c:\program files\python 2.4\include" /I "f:/wesnoth/src" /I "f:/SDL-1.2.7/include" /I "f:/SDL_image-1.2.3/include" /I "f:/SDL_mixer-1.2.5/include" /I "f:/SDL_net-1.2.5/include" /I "src/sdl_ttf" /I "f:/libintl-devel/include" /I "f:/intl/include" /I "f:/projets/gpl/" /D "WIN32" /D "_DEBUG" /D "DEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_FRIBIDI" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /machine:I386 /out:"Release/editor.exe"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/editor.exe" /libpath:"f:\projets\lib"
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "Map Editor - Win32 Release"
# Name "Map Editor - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\about.cpp
# End Source File
# Begin Source File

SOURCE=.\src\actions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\animated.cpp
# End Source File
# Begin Source File

SOURCE=.\src\astarnode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\attack_prediction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_or_text.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_wml.cpp
# End Source File
# Begin Source File

SOURCE=.\src\builder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\button.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cavegen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\checksum.cpp
# End Source File
# Begin Source File

SOURCE=.\src\clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\src\color_range.cpp
# End Source File
# Begin Source File

SOURCE=.\src\config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_layout.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_palettes.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_undo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\events.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\file_chooser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filechooser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\font.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game_config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game_events.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gamestatus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gettext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\halo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\help.cpp
# End Source File
# Begin Source File

SOURCE=.\src\hotkeys.cpp
# End Source File
# Begin Source File

SOURCE=.\src\image.cpp
# End Source File
# Begin Source File

SOURCE=.\src\key.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\label.cpp
# End Source File
# Begin Source File

SOURCE=.\src\language.cpp
# End Source File
# Begin Source File

SOURCE=.\src\loadscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\log.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map_create.cpp
# End Source File
# Begin Source File

SOURCE=.\src\map_label.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\map_manip.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mapgen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mapgen_dialog.cpp
# End Source File
# Begin Source File

SOURCE=".\src\marked-up_text.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\widgets\menu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\minimap.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mouse.cpp
# End Source File
# Begin Source File

SOURCE=.\src\mouse_events.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network.cpp
# End Source File
# Begin Source File

SOURCE=.\src\network_worker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\parser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pathfind.cpp
# End Source File
# Begin Source File

SOURCE=.\src\pathutils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playturn.cpp
# End Source File
# Begin Source File

SOURCE=.\src\preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\src\preferences_display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\preprocessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\progressbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\race.cpp
# End Source File
# Begin Source File

SOURCE=.\src\random.cpp
# End Source File
# Begin Source File

SOURCE=.\src\replay.cpp
# End Source File
# Begin Source File

SOURCE=.\src\reports.cpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\scenario_editor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollarea.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sdl_ttf\SDL_ttf.c
# End Source File
# Begin Source File

SOURCE=.\src\sdl_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\show_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\slider.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\src\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\string_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\team.cpp
# End Source File
# Begin Source File

SOURCE=.\src\terrain.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\textbox.cpp
# End Source File
# Begin Source File

SOURCE=.\src\theme.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tooltips.cpp
# End Source File
# Begin Source File

SOURCE=.\src\tstring.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_abilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_animation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_types.cpp
# End Source File
# Begin Source File

SOURCE=.\src\util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\variable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\video.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\widget.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\editor\editor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_dialogs.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_layout.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_palettes.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\editor_undo.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\map_manip.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\scenario_editor.hpp
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
