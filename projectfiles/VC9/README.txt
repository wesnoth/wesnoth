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
