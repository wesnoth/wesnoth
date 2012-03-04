@echo off
REM "run this: signing <keypass> <storepass>"
REM "this file should be run in 'plugin_feature' directory"
REM cn = common name
REM ou = organizational unit
REM o = organization
REM c = Country
IF "%~1"=="" goto usage
IF "%~2"=="" goto usage
IF "%~3"=="" goto usage
IF EXIST keystore (
echo deleting previous keystore file
DEL keystore
)
echo generating key...
keytool.exe -genkey -dname "cn=Battle for Wesnoth, ou=Wesnoth, o=Wesnoth" -alias wesnoth -keypass %~2 -keystore keystore -storepass %~3 -validity 1095
echo signing feature
jarsigner.exe -keystore keystore -keypass %~2 -storepass %~3 %~1/features/org.wesnoth.*.jar wesnoth
echo signing core
jarsigner.exe -keystore keystore -keypass %~2 -storepass %~3 %~1/plugins/org.wesnoth_*.jar wesnoth
echo signing ui
jarsigner.exe -keystore keystore -keypass %~2 -storepass %~3 %~1/plugins/org.wesnoth.ui_*.jar wesnoth
echo finished.
goto end

:usage
echo Usage: %~0 ^<updates directory^> ^<keypass^> ^<storepass^>

:end
@pause
