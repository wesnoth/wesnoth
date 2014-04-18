README.txt
==========
NEW WAY (updating the VC9 project file):

Please refer to the guide on wiki.wesnoth.org
http://wiki.wesnoth.org/CompilingWesnothOnWindows#Visual_Studio_2010_and_later


==========
OLD WAY (using cmake):

Wesnoth 1.9 (since r41888) and branches/1.8 (since r42011)
can be compiled using cmake+msvc9

   1. make sure that git (formerly svn) command-line executable is in your %PATH%
   2. for wesnoth 1.8 and early pre-1.9 versions: download
      ftp://ftp.terraninfo.net/wesnoth/msvc9/external.zip (55mb download) and
      unpack it into wesnoth's directory - so, there'll be an 'external'
      folder in there.
   3. for wesnoth 1.9: download
      ftp://ftp.terraninfo.net/wesnoth/msvc9/external_19.zip (59mb download)
      and unpack it into wesnoth's directory - so, there'll be an 'external'
      folder in there.
   4. install cmake http://www.cmake.org/cmake/resources/software.html and
      put it into %PATH% (that can be done by the installer)
   5. to allow the game to run, put full path to external/dll into %PATH%
   6. run external/msvc9-cmake.bat
   7. open generated project file in msvc9 and compile. the project should
      pick up changes in cmake files automatically. Also, you can re-run the
      .bat file to regenerate it.


==========
Building the prerequisites

We'll assume to have two 'General Folders' for libraries and includes to store
most of the prerequisites. (Otherwise each configuration setting for each
project would need to have each and every 'lib' and 'include' folder added.)
      C:\projects\_include
      C:\projects\_lib

We'll now start downloading and compiling/copying the prerequisites into the necessary folders:


   1. >=zlib-1.2.3 ( http://www.zlib.net/ )

      Download the zlib source code
      	zlib123.zip
      and unpack it into the projects folder to get:
      	C:\projects\zlib123\

      Open the file zlib.dsw in the folder
      	C:\projects\zlib123\projects\visualc6\
      with MSVC and select 'Yes To All' to convert it into a VC9 project.

      Open the properties of the 'zlib' solution and in the
      'Configuration Properties' select the 'DLL Release'
      'Configuration'. Select 'OK' and then right-click the 'zlib'
      project and select 'Build'. After a successful Build, close that
      instance of MSVC.


   2. >=libsdl-1.2.7 ( http://www.libsdl.org )

      Download the development library file for Win32 as well as the
      source code
      	SDL-1.2.13.zip
      	SDL-devel-1.2.13-VC8.zip
      and unpack them into the projects folder to get:
      	C:\projects\SDL-1.2.13\

      Copy the 'SDL.dll' and the 'SDL.lib' from the 'lib' folder
      and the contents of 'include' folder into the corresponding
      'General Folder' ( ...\_include\ resp. ...\_lib\ ).

      Unpack the 'VisualC.zip' and open the 'SDL.sln' file with MSVC.
      Follow the instructions to convert the project to VC9.

      Right-click the 'SDLmain' project and select 'Build'. After a
      successful Build, close that instance of MSVC.

      Copy 'SDLmain.lib' from
      	C:\projects\SDL-1.2.13\VisualC\SDLmain\Debug
      into the 'lib' 'General Folder'.


   3. >=libboost-1.33.0 ( http://www.boost.org/ )

      Download Boost and unpack it into the folder
      	C:\projects\

      This should give you a folder structure like
      	C:\projects\boost_1_38_0\

      Download the pre-built bjam executable
      	boost-jam-3.1.17-1-ntx86.zip
      from the bjam download page on sourceforge
      	http://sourceforge.net/project/showfiles.php?group_id=7586&package_id=72941
      and unpack and move the bjam executable it into the base folder
      of boost
      	C:\projects\boost_1_38_0\

      Now start the command prompt (CMD.exe) and invoke the following
      commands:
      	C:\> set ZLIB_SOURCE=C:\projects\zlib123\
      	C:\> set ZLIB_LIBPATH=C:\projects\zlib123\projects\visualc6\Win32_DLL_Release
      	C:\> set ZLIB_BINARY=zlib1.dll
      	C:\> cd C:\projects\boost_1_38_0
      	C:\projects\boost_1_38_0> bjam ^
      	More? --toolset=msvc ^
      	More? --build-type=complete ^
      	More? --prefix="C:\projects\Boost\" install

      If the upper last command doesn't work, replace the last line with:
      	--prefix="C:\projects\Boost" install

      Now bjam will compile all variants of the boost library binaries
      for MSVC 2008 and copies them into the folder
      	C:\projects\Boost\lib\
      while all required headers are copied into
      	C:\projects\Boost\include\boost-1_38\boost

      Use the time bjam is busy compiling (possibly several hours) as
      you wish. (It is possible to continue downloading and copying
      the include and lib files since all required compiling is done.)

      Once bjam has finished building/copying, you then can delete
      the folder
      	C:\projects\boost_1_38_0\
      to free up space.


   4. >=sdl-image-1.2 (with PNG support)
      	( http://www.libsdl.org/projects/SDL_image )

      Download the binary file for Win32
      	SDL_image-devel-1.2.7-VC9.zip
      and unpack it into the projects folder to get:
      	C:\projects\SDL_image-1.2.7\

      Copy the contents of each the 'lib' and the 'include' folder
      into the corresponding 'General Folder'.


   5. >=sdl-mixer-1.2 (with Vorbis support)
      	( http://www.libsdl.org/projects/SDL_mixer )

      Download the binary file for Win32
      	SDL_mixer-devel-1.2.8-VC8.zip
      and unpack it into the projects folder to get:
      	C:\projects\SDL_mixer-1.2.8\

      Copy the contents of each the 'lib' and the 'include' folder
      into the corresponding 'General Folder'.


   6. >=sdl-net-1.2 ( http://www.libsdl.org/projects/SDL_net )

      Download the binary file for Win32
      	SDL_net-devel-1.2.7-VC8.zip
      and unpack it into the projects folder to get:
      	C:\projects\SDL_net-1.2.7\

      Copy the contents of each the 'lib' and the 'include' folder
      into the corresponding 'General Folder'.


   7. >=sdl-ttf-2.0.8 ( http://www.libsdl.org/projects/SDL_ttf )

      Download the binary file for Win32
      	SDL_ttf-devel-2.0.9-VC8.zip
      and unpack it into the projects folder to get:
      	C:\projects\SDL_ttf-2.0.9\

      Copy the contents of each the 'lib' and the 'include' folder
      into the corresponding 'General Folder'.


   8. >=libintl-0.14.4
      	( http://gnuwin32.sourceforge.net/packages/gettext.htm )

      Download the 'Binaries', 'Dependencies' and the
      'Developer files'
      	gettext-0.14.4-bin.zip
      	gettext-0.14.4-dep.zip
      	gettext-0.14.4-lib.zip
      and unpack them all into the projects folder to get:
      	C:\projects\gettext-0.14.4\

      Copy the contents of each the 'lib' and the 'include' folder
      into the corresponding 'General Folder'.
      Copy all dll's from the 'bin' folder into the '_lib'
      'General Folder'.


   9. >=libfontconfig-2.4.1
      	( http://www.gtk.org/download-windows.html )

      Download the 'Fontconfig' package 'Binaries' from the
      'Third Party Dependencies'
      	fontconfig-2.4.2-tml-20071015.zip
      and unpack it into the projects folder to get:
      	C:\projects\fontconfig-2.4.2\

      Copy the 'libfontconfig-1.dll' from the 'bin' folder into the
      'lib' 'General Folder'.


  10. >=lua-5.1.4 ( http://luabinaries.luaforge.net/download.html )

      Download the 'Windows x86 DLL and Includes (Visual C++ 2005
      Compatible)'
      	lua5_1_4_Win32_dll8_lib.zip
      and unpack it into the projects folder to get:
      	C:\projects\lua-5.1.4\

      Copy the dll's and the lib's into the 'lib' 'General Folder' and
      the content of the 'include' folder into the corresponding '
      General Folder'.


  11. >=pangocairo-1.24.4 ( http://www.gtk.org/download-windows.html )

      Download the following 'Binaryies' and 'Dev' packages from the
      'GTK+ individual packages'
      	cairo_1.8.6-1_win32.zip
      	cairo-dev_1.8.6-1_win32.zip
      	glib_2.20.0-1_win32.zip
      	glib-dev_2.20.0-1_win32.zip
      	pango_1.24.0-1_win32.zip
      	pango-dev_1.24.0-1_win32.zip
      and unpack them into the projects folder to get:
      	C:\projects\pangocairo\

      Copy all dll's from the 'bin' folder into the '_lib'
      'General Folder'.
      Copy the contents (but not the subfolders) of the 'lib' folder
      into the corresponding 'General Folder'.
      Copy the 'glibconfig.h' file from
      C:\projects\pangocairo\lib\glib-2.0\include into the '_include'
      'General Folder'.
      Copy the contens (including subfolders) of the following folders
      into the '_include' 'General Folder':
      	C:\projects\pangocairo\include\cairo
      	C:\projects\pangocairo\include\glib-2.0
      	C:\projects\pangocairo\include\pango-1.0
