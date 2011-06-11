README.txt

Compiling wesnoth on windows using CodeBlocks 10.05
(June 2011, development version 1.9.6+svn r49843,
earlier source versions untested)

(Whenever you enter directories or names, you should do it
by using the "..." button to the right of the field.)
1. Get the source by downloading from from http://www.wesnoth.org/
or by using svn (if you know how to). The folder where you put the
source is referred to as wesnoth_root/ in this README.
2. Install CodeBlocks http://www.codeblocks.org/
MinGw is not needed. CodeBlocks' installation folder is referred to as
codeblocks_root/ in this README.
3. Install tdm-gcc-4.5.2 from http://tdm-gcc.tdragon.net/download
4. In CodeBlocks, open
wesnoth_root/projectfiles/CodeBlocks/wesnoth.workspace
5. In CodeBlocks, goto (menu bar)>settings>compiler and debugger
>global compiler settings> (tab) toolchain executables.
In the field "compiler's installation directory", enter the folder
into which you did install tdm-gcc-4.5.2.
Into the fields "c compiler", "c++ compiler" and "linker for dynamic libs"
enter "g++.exe".
Into the field "linker for static libs", enter "ar.exe".
6. Download WesnothCodeBlocksWinSDK.zip from
http://www.mediafire.com/?7sc58qmmmeufhxo and extract
this archive so that you have directories codeblocks_root/include and
codeblocks_root/lib
7. In CodeBlocks, goto (menu bar)>settings>compiler and debugger
>global compiler settings> (tab) search directories> (tab) compiler>Add.
Enter codeblocks_root/include.
8. In CodeBlocks, goto (menu bar)>settings>compiler and debugger
>global compiler settings> (tab) search directories> (tab) linker>Add.
Enter codeblocks_root/lib.
9. In CodeBlocks, choose (menu bar)>build>build workspace.
wesnoth.exe and wesnothd.exe should appear in wesnoth_root/
when finished building.
10. To be able to execute the program, copy all *.dll files from your
installation folder of the most recent official windows wesnoth development
release into wesnoth_root/.
