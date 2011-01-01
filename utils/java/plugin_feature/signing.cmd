@echo off
REM "run this: signing <keypass> <storepass>"
REM "this file should be run in 'plugin_feature' directory"
REM cn = common name
REM ou = organizational unit
REM o = organization
REM c = Country
SET PLUGIN_VERSION=1.0.2
SET PLUGIN_WML_VERSIOn=1.1.2
IF EXIST keystore (
echo deleting previous keystore file
DEL keystore
)
echo generating key...
keytool.exe -genkey -dname "cn=Battle for Wesnoth, ou=Wesnoth, o=Wesnoth" -alias wesnoth -keypass %1 -keystore keystore -storepass %2 -validity 1095
echo signing feature
jarsigner.exe -keystore keystore -storepass %2 -keypass %1 ../update_site/features/Wesnoth_Eclipse_Plugin_%PLUGIN_VERSION%.jar wesnoth
echo signing org.wesnoth.wml_%PLUGIN_WML_VERSION%.jar
jarsigner.exe -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth.wml_%PLUGIN_WML_VERSION%.jar wesnoth
echo signing org.wesnoth.wml.ui_%PLUGIN_WML_VERSION%.jar
jarsigner.exe -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth.wml.ui_%PLUGIN_WML_VERSION%.jar wesnoth
echo signing org.wesnoth_%PLUGIN_VERSION%.jar
jarsigner.exe -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth_%PLUGIN_VERSION%.jar wesnoth
echo finished.
@pause
