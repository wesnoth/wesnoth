@echo off
rem Don't show unrelated output (calls)
cd ..
rem Go to project root
svnversion -n > tempfile.tmp
rem Get the revision data
copy utils\svnhpp_1.bin /B + tempfile.tmp /B + utils\svnhpp_2.bin /B src\revision_stamp.hpp
rem Concatenate the other files with rev. number in binary mode
del src\game_config.o
del src\game_config.obj
rem Add here some more stuff to delete...
rem This is to force rebuild of the file which includes version data
del tempfile.tmp
rem Rid of the temp
