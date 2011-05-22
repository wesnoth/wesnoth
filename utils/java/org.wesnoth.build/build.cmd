@echo off

set ECLIPSEBIN=%1
set BUILDDIR=%TEMP%\eclipse_build

echo Clearing build dir...
IF EXIST %BUILDDIR%	rm -rf %BUILDDIR%
mkdir %BUILDDIR%

REM get path to equinox jar inside ECLIPSEBIN folder
for /f "delims= tokens=1" %%c in ('dir /B /S /OD %ECLIPSEBIN%\plugins\org.eclipse.equinox.launcher_*.jar') do set EQUINOXJAR=%%c

IF EXIST %EQUINOXJAR% (
echo Found equinox jar: %EQUINOXJAR%
) ELSE (
echo Couldn't find the equinox launcher jar
goto end
)

REM find pde build folder
for /f "delims= tokens=1" %%c in ('dir /B /S /OD %ECLIPSEBIN%\plugins\org.eclipse.pde.build_*') do set PDEBUILD_DIR=%%c

IF EXIST %PDEBUILD_DIR% (
echo Found pde folder: %PDEBUILD_DIR%
) ELSE (
echo Couldn't find the pde build plugin. Are you using a RCP eclipse version?
goto end
)

java -cp %EQUINOXJAR% org.eclipse.core.launcher.Main -data workspace -application org.eclipse.ant.core.antRunner -DbuildDirectory=%BUILDDIR% -Dbase=%ECLIPSEBIN% -DbaseLocation=%ECLIPSEBIN% -Ddeltapack=%ECLIPSEBIN%  -Declipse.pdebuild.scripts=%PDEBUILD_DIR%\scripts -Declipse.pdebuild.templates=%PDEBUILD_DIR%\templates -buildfile build.xml

IF EXIST ../org.wesnoth.feature/build.xml rm ../org.wesnoth.feature/build.xml
IF EXIST ../org.wesnoth/build.xml rm ../org.wesnoth/build.xml
IF EXIST ../org.wesnoth.wml/build.xml rm ../org.wesnoth.wml/build.xml
IF EXIST ../org.wesnoth.wml.ui/build.xml rm ../org.wesnoth.wml.ui/build.xml

:end
echo Script finished.
pause