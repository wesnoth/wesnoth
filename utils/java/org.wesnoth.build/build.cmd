@echo off

set ECLIPSEHOME=D:\bin\eclipse\
set ECLIPSEDELTAPACK=d:\work\java\eclipse\3.6.2\eclipse\
 
REM get path to equinox jar inside ECLIPSEHOME folder
for /f "delims= tokens=1" %%c in ('dir /B /S /OD %ECLIPSEHOME%\plugins\org.eclipse.equinox.launcher_*.jar') do set EQUINOXJAR=%%c
echo "Found equinox jar: " %EQUINOXJAR%

REM find pde build folder
for /f "delims= tokens=1" %%c in ('dir /B /S /OD %ECLIPSEHOME%\plugins\org.eclipse.pde.build_*') do set PDEBUILD_DIR=%%c
echo "Found pde folder: " %PDEBUILD_DIR%

java -cp %EQUINOXJAR% org.eclipse.core.launcher.Main -data workspace -application org.eclipse.ant.core.antRunner -Ddeltapack=%ECLIPSEDELTAPACK%  -Declipse.pdebuild.scripts=%PDEBUILD_DIR%\scripts -Declipse.pdebuild.templates=%PDEBUILD_DIR%\templates -buildfile build.xml

rm ../org.wesnoth/build.xml
rm ../org.wesnoth.wml/build.xml
rm ../org.wesnoth.wml.ui/build.xml
rm -rf workspace