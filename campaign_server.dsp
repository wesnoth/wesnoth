# Microsoft Developer Studio Project File - Name="campaign_server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=campaign_server - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "campaign_server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "campaign_server.mak" CFG="campaign_server - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "campaign_server - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "campaign_server - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "campaign_server - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "c:/projects/wesnoth/wesnothd/src" /I "c:/projects/wesnoth/include/SDL-1.2.9/include" /I "c:/projects/wesnoth/include" /I "c:/projects/wesnoth/wesnothd/src/sdl_ttf" /I "c:/program files/python 2.4/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include" /I "c:/projects/wesnoth/include/freetype-2.1.9/include/freetype" /I "c:/projects/wesnoth/include/SDL_image-1.2.4" /I "c:/projects/wesnoth/include/SDL_mixer-1.2.6" /I "c:/projects/wesnoth/include/SDL_net-1.2.5" /I "src/sdl_ttf" /I "c:/projects/wesnoth/include/libintl-devel/include" /I "c:/usr/include" /D "WIN32" /D "_NODEBUG" /D "_WINDOWS" /D "_MBCS" /D "NOT_HAVE_FRIBIDI" /D "NODEBUG_CONFIG" /FR"" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib SDL_mixer.lib SDL_net.lib SDL_image.lib libintl.lib freetype.lib Ws2_32.lib fribidi.lib /nologo /subsystem:windows /incremental:yes /debug /machine:I386 /out:"..\campaign_server.exe" /pdbtype:sept /libpath:"c:\projects\wesnoth\yogilib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "campaign_server - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "campaign_server - Win32 Release"
# Name "campaign_server - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=src\serialization\binary_or_text.cpp
# End Source File
# Begin Source File

SOURCE=src\serialization\binary_wml.cpp
# End Source File
# Begin Source File

SOURCE=src\campaign_server\campaign_server.cpp
# End Source File
# Begin Source File

SOURCE=src\color_range.cpp
# End Source File
# Begin Source File

SOURCE=src\config.cpp
# End Source File
# Begin Source File

SOURCE=src\filesystem.cpp
# End Source File
# Begin Source File

SOURCE=src\game_config.cpp
# End Source File
# Begin Source File

SOURCE=src\gettext.cpp
# End Source File
# Begin Source File

SOURCE=src\loadscreen_empty.cpp
# End Source File
# Begin Source File

SOURCE=src\log.cpp
# End Source File
# Begin Source File

SOURCE=src\network.cpp
# End Source File
# Begin Source File

SOURCE=src\network_worker.cpp
# End Source File
# Begin Source File

SOURCE=src\serialization\parser.cpp
# End Source File
# Begin Source File

SOURCE=src\publish_campaign.cpp
# End Source File
# Begin Source File

SOURCE=src\serialization\string_utils.cpp
# End Source File
# Begin Source File

SOURCE=src\thread.cpp
# End Source File
# Begin Source File

SOURCE=src\serialization\tokenizer.cpp
# End Source File
# Begin Source File

SOURCE=src\tstring.cpp
# End Source File
# Begin Source File

SOURCE=src\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
