# Microsoft Developer Studio Project File - Name="wesnoth" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wesnoth - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wesnoth.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wesnoth.mak" CFG="wesnoth - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wesnoth - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "wesnoth - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wesnoth - Win32 Debug"

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
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /Zi /Od /I "c:/projects/wesnoth/wesnothd/src" /I "c:/projects/wesnoth/include/SDL-1.2.9/include" /I "c:/projects/wesnoth/include" /I "c:/projects/wesnoth/wesnothd/src/sdl_ttf" /I "c:/program files/python 2.4/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include/freetype" /I "c:/projects/wesnoth/include/SDL_image-1.2.4" /I "c:/projects/wesnoth/include/SDL_mixer-1.2.6" /I "c:/projects/wesnoth/include/SDL_net-1.2.5" /I "src/sdl_ttf" /I "c:/projects/wesnoth/include/libintl-devel/include" /I "c:/usr/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "NOT_HAVE_FRIBIDI" /FAs /Fa"" /FR"" /Fp"Release/wesnoth.pch" /YX /Fo"Release/" /Fd"Release/" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"wesnoth.exe" /pdbtype:sept /libpath:"c:\projects\wesnoth\yogilib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "wesnoth - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "f:/wesnoth/src" /I "f:/SDL-1.2.7/include" /I "f:/SDL_image-1.2.3/include" /I "f:/SDL_mixer-1.2.5/include" /I "f:/SDL_net-1.2.5/include" /I "src/sdl_ttf" /I "f:/libintl-devel/include" /I "f:/intl/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "HAVE_FRIBIDI" /D "HAVE_PYTHON" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /I "c:/projects/wesnoth/wesnothd/src" /I "c:/projects/wesnoth/include/SDL-1.2.9/include" /I "c:/projects/wesnoth/include" /I "c:/projects/wesnoth/wesnothd/src/sdl_ttf" /I "c:/program files/python 2.4/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include/freetype" /I "c:/projects/wesnoth/include/SDL_image-1.2.4" /I "c:/projects/wesnoth/include/SDL_mixer-1.2.6" /I "c:/projects/wesnoth/include/SDL_net-1.2.5" /I "src/sdl_ttf" /I "c:/projects/wesnoth/include/libintl-devel/include" /I "c:/usr/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "NOT_HAVE_FRIBIDI" /D "BOOST_IOSTREAMS_NO_LIB" /FR /YX /FD /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_NODEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_NODEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /debug /machine:I386 /out:"wesnoth.exe" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib zdll.lib /nologo /subsystem:windows /incremental:yes /pdb:"Debug/wesnoth.pdb" /debug /machine:I386 /out:"wesnoth.exe" /pdbtype:sept /libpath:"c:\projects\wesnoth\yogilib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "wesnoth - Win32 Debug"
# Name "wesnoth - Win32 Release"
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

SOURCE=.\src\ai_python.cpp
# End Source File
# Begin Source File

SOURCE=.\src\animated_game.cpp
# End Source File
# Begin Source File

SOURCE=.\src\astarnode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\astarsearch.cpp
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

SOURCE=.\src\construct_dialog.cpp
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

SOURCE=.\src\widgets\file_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filechooser.cpp
# End Source File
# Begin Source File

SOURCE=.\src\filesystem.cpp
# End Source File
# Begin Source File

SOURCE=.\src\floating_textbox.cpp
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

SOURCE=.\src\game_display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game_events.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game_preferences.cpp
# End Source File
# Begin Source File

SOURCE=.\src\game_preferences_display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\gamestatus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\generate_report.cpp
# End Source File
# Begin Source File

SOURCE=.\src\generic_event.cpp
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

SOURCE=.\src\widgets\menu_style.cpp
# End Source File
# Begin Source File

SOURCE=.\src\minimap.cpp
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

SOURCE=.\src\play_controller.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playcampaign.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playmp_controller.cpp
# End Source File
# Begin Source File

SOURCE=.\src\playsingle_controller.cpp
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

SOURCE=.\src\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sha1.cpp
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

SOURCE=.\src\soundsource.cpp
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

SOURCE=.\src\terrain_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\terrain_translation.cpp
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

SOURCE=.\src\unit_abilities.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_animation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_display.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_frame.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_map.cpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_types.cpp
# End Source File
# Begin Source File

SOURCE=.\src\upload_log.cpp
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
# Begin Source File

SOURCE=.\src\wml_exception.cpp
# End Source File
# Begin Source File

SOURCE=..\boost_1_34_1\libs\iostreams\src\zlib.cpp
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

SOURCE=.\src\ai_dfool.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_interface.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_move.hpp
# End Source File
# Begin Source File

SOURCE=.\src\ai_python.hpp
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

SOURCE=.\src\attack_prediction.hpp
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

SOURCE=.\src\clipboard.hpp
# End Source File
# Begin Source File

SOURCE=.\src\color_range.hpp
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

SOURCE=.\src\construct_dialog.hpp
# End Source File
# Begin Source File

SOURCE=.\src\cursor.hpp
# End Source File
# Begin Source File

SOURCE=.\src\dialog_view.hpp
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

SOURCE=.\src\file_chooser.hpp
# End Source File
# Begin Source File

SOURCE=.\src\widgets\file_menu.hpp
# End Source File
# Begin Source File

SOURCE=.\src\filesystem.hpp
# End Source File
# Begin Source File

SOURCE=.\src\floating_textbox.hpp
# End Source File
# Begin Source File

SOURCE=.\src\font.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_config.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_display.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_errors.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_events.hpp
# End Source File
# Begin Source File

SOURCE=.\src\game_preferences.hpp
# End Source File
# Begin Source File

SOURCE=.\src\gamestatus.hpp
# End Source File
# Begin Source File

SOURCE=.\src\generic_event.hpp
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

SOURCE=.\src\loadscreen.hpp
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

SOURCE=.\src\play_controller.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playcampaign.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playmp_controller.hpp
# End Source File
# Begin Source File

SOURCE=.\src\playsingle_controller.hpp
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

SOURCE=.\src\widgets\scrollpane.hpp
# End Source File
# Begin Source File

SOURCE=.\src\sdl_utils.hpp
# End Source File
# Begin Source File

SOURCE=.\src\settings.hpp
# End Source File
# Begin Source File

SOURCE=.\src\sha1.hpp
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

SOURCE=.\src\soundsource.hpp
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

SOURCE=.\src\terrain_filter.hpp
# End Source File
# Begin Source File

SOURCE=.\src\terrain_translation.hpp
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

SOURCE="..\include\freetype-2.1.9\include\freetype\ttnameid.h"
# End Source File
# Begin Source File

SOURCE=.\src\unit.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_abilities.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_animation.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_display.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_frame.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_map.hpp
# End Source File
# Begin Source File

SOURCE=.\src\unit_types.hpp
# End Source File
# Begin Source File

SOURCE=.\src\upload_log.hpp
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

SOURCE=.\src\wml_exception.hpp
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
