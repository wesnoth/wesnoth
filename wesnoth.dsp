# Microsoft Developer Studio Project File - Name="wesnoth" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wesnoth - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wesnoth.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wesnoth.mak" CFG="wesnoth - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wesnoth - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wesnoth - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wesnoth - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Ot /Oa /Ow /Og /Oi /Op /Oy /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src" /I "f:/projets/include/sdl" /I "f:/projets/include" /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src/sdl_ttf" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# SUBTRACT CPP /Ox
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:"wesnoth.exe" /libpath:"f:\projets\lib"

!ELSEIF  "$(CFG)" == "wesnoth - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src" /I "f:/projets/include/sdl" /I "f:/projets/include" /I "f:/projets/gpl/wesnoth/wesnoth-trunk/src/sdl_ttf" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"wesnoth.exe" /pdbtype:sept /libpath:"f:\projets\lib"

!ENDIF 

# Begin Target

# Name "wesnoth - Win32 Release"
# Name "wesnoth - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\about.cpp
# End Source File
# Begin Source File

SOURCE=.\src\actions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_attack.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_dfool.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_move.cpp
# End Source File
# Begin Source File

SOURCE=.\src\animated.cpp
# End Source File
# Begin Source File

SOURCE=.\src\astarnode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_or_text.cpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_wml.cpp
# End Source File
# Begin Source File

SOURCE=".\src\boilerplate-header.cpp"
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

SOURCE=.\src\widgets\combo.cpp
# End Source File
# Begin Source File

SOURCE=.\src\config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\config_adapter.cpp
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

SOURCE=.\src\game.cpp
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

SOURCE=.\src\widgets\image_button.cpp
# End Source File
# Begin Source File

SOURCE=.\src\intro.cpp
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

SOURCE=.\src\leader_list.cpp
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

SOURCE=.\src\menu_events.cpp
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

SOURCE=.\src\multiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_connect.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_create.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_lobby.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_ui.cpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_wait.cpp
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

SOURCE=.\src\playcampaign.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playlevel.cpp
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

SOURCE=.\src\publish_campaign.cpp
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

SOURCE=.\src\replay_controller.cpp
# End Source File
# Begin Source File

SOURCE=.\src\reports.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollarea.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollbar.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollpane.cpp
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

SOURCE=.\src\titlescreen.cpp
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

SOURCE=.\src\wassert.cpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\widget.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\about.hpp
# End Source File
# Begin Source File

SOURCE=.\src\actions.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai2.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_attack.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_interface.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_move.hpp
# End Source File
# Begin Source File

SOURCE=.\src\animated.hpp
# End Source File
# Begin Source File

SOURCE=.\src\array.hpp
# End Source File
# Begin Source File

SOURCE=.\src\astarnode.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_or_text.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\binary_wml.hpp
# End Source File
# Begin Source File

SOURCE=.\src\builder.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\button.hpp
# End Source File
# Begin Source File

SOURCE=.\src\cavegen.hpp
# End Source File
# Begin Source File

SOURCE=.\src\checksum.hpp
# End Source File
# Begin Source File

SOURCE=.\src\clipboard.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\combo.hpp
# End Source File
# Begin Source File

SOURCE=.\src\config.hpp
# End Source File
# Begin Source File

SOURCE=.\src\config_adapter.hpp
# End Source File
# Begin Source File

SOURCE=.\src\cursor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\dialogs.hpp
# End Source File
# Begin Source File

SOURCE=.\src\display.hpp
# End Source File
# Begin Source File

SOURCE=.\src\events.hpp
# End Source File
# Begin Source File

SOURCE=.\src\filesystem.hpp
# End Source File
# Begin Source File

SOURCE=.\src\font.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_config.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_errors.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_events.hpp
# End Source File
# Begin Source File

SOURCE=.\src\gamestatus.hpp
# End Source File
# Begin Source File

SOURCE=.\src\gettext.hpp
# End Source File
# Begin Source File

SOURCE=.\src\global.hpp
# End Source File
# Begin Source File

SOURCE=.\src\halo.hpp
# End Source File
# Begin Source File

SOURCE=.\src\help.hpp
# End Source File
# Begin Source File

SOURCE=.\src\hotkeys.hpp
# End Source File
# Begin Source File

SOURCE=.\src\image.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\image_button.hpp
# End Source File
# Begin Source File

SOURCE=.\src\intro.hpp
# End Source File
# Begin Source File

SOURCE=.\src\key.hpp
# End Source File
# Begin Source File

SOURCE=.\src\language.hpp
# End Source File
# Begin Source File

SOURCE=.\src\leader_list.hpp
# End Source File
# Begin Source File

SOURCE=.\src\log.hpp
# End Source File
# Begin Source File

SOURCE=.\src\map.hpp
# End Source File
# Begin Source File

SOURCE=.\src\map_create.hpp
# End Source File
# Begin Source File

SOURCE=.\src\map_label.hpp
# End Source File
# Begin Source File

SOURCE=.\src\mapgen.hpp
# End Source File
# Begin Source File

SOURCE=.\src\mapgen_dialog.hpp
# End Source File
# Begin Source File

SOURCE=".\src\marked-up_text.hpp"
# End Source File
# Begin Source File

SOURCE=.\src\widgets\menu.hpp
# End Source File
# Begin Source File

SOURCE=.\src\menu_events.hpp
# End Source File
# Begin Source File

SOURCE=.\src\minimap.hpp
# End Source File
# Begin Source File

SOURCE=.\src\mouse.hpp
# End Source File
# Begin Source File

SOURCE=.\src\mouse_events.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_client.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_connect.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_create.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_lobby.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_ui.hpp
# End Source File
# Begin Source File

SOURCE=.\src\multiplayer_wait.hpp
# End Source File
# Begin Source File

SOURCE=.\src\network.hpp
# End Source File
# Begin Source File

SOURCE=.\src\network_worker.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\parser.hpp
# End Source File
# Begin Source File

SOURCE=.\src\pathfind.hpp
# End Source File
# Begin Source File

SOURCE=.\src\pathutils.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playcampaign.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playlevel.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playturn.hpp
# End Source File
# Begin Source File

SOURCE=.\src\preferences.hpp
# End Source File
# Begin Source File

SOURCE=.\src\preferences_display.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\preprocessor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\progressbar.hpp
# End Source File
# Begin Source File

SOURCE=.\src\publish_campaign.hpp
# End Source File
# Begin Source File

SOURCE=.\src\race.hpp
# End Source File
# Begin Source File

SOURCE=.\src\random.hpp
# End Source File
# Begin Source File

SOURCE=.\src\replay.hpp
# End Source File
# Begin Source File

SOURCE=.\src\replay_controller.hpp
# End Source File
# Begin Source File

SOURCE=.\src\reports.hpp
# End Source File
# Begin Source File

SOURCE=.\src\editor\scenario_editor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\scoped_resource.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollarea.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\scrollbar.hpp
# End Source File
# Begin Source File

SOURCE=.\src\sdl_utils.hpp
# End Source File
# Begin Source File

SOURCE=.\src\shared_ptr.hpp
# End Source File
# Begin Source File

SOURCE=.\src\show_dialog.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\slider.hpp
# End Source File
# Begin Source File

SOURCE=.\src\sound.hpp
# End Source File
# Begin Source File

SOURCE=.\src\statistics.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\string_utils.hpp
# End Source File
# Begin Source File

SOURCE=.\src\team.hpp
# End Source File
# Begin Source File

SOURCE=.\src\terrain.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\textbox.hpp
# End Source File
# Begin Source File

SOURCE=.\src\theme.hpp
# End Source File
# Begin Source File

SOURCE=.\src\thread.hpp
# End Source File
# Begin Source File

SOURCE=.\src\titlescreen.hpp
# End Source File
# Begin Source File

SOURCE=.\src\serialization\tokenizer.hpp
# End Source File
# Begin Source File

SOURCE=.\src\tooltips.hpp
# End Source File
# Begin Source File

SOURCE=.\src\tstring.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_display.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_types.hpp
# End Source File
# Begin Source File

SOURCE=.\src\util.hpp
# End Source File
# Begin Source File

SOURCE=.\src\variable.hpp
# End Source File
# Begin Source File

SOURCE=.\src\video.hpp
# End Source File
# Begin Source File

SOURCE=.\src\wassert.hpp
# End Source File
# Begin Source File

SOURCE=.\src\wesconfig.h
# End Source File
# Begin Source File

SOURCE=.\src\widgets\widget.hpp
# End Source File
# Begin Source File

SOURCE=.\src\wml_separators.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\wesnoth.ico
# End Source File
# End Group
# End Target
# End Project
