@echo off
REM "run this: signing <keypass> <storepass>"
REM "this file should be run in 'plugin_feature' directory"
REM cn = common name
REM ou = organizational unit
REM o = organization
REM c = Country
echo generating key...
keytool -genkey -dname "cn=Battle for Wesnoth, ou=Wesnoth, o=Wesnoth" -alias wesnoth -keypass %1 -keystore keystore -storepass %2 -validity 1095
echo signing feature
jarsigner -keystore keystore -storepass %2 -keypass %1 ../update_site/features/Wesnoth_Eclipse_Plugin_1.0.0.jar wesnoth
echo signing org.wesnoth.wml_1.1.0.jar
jarsigner -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth.wml_1.1.0.jar wesnoth
echo signing org.wesnoth.wml.ui_1.1.0.jar
jarsigner -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth.wml.ui_1.1.0.jar wesnoth
echo signing org.wesnoth_1.0.0.jar
jarsigner -keystore keystore -storepass %2 -keypass %1 ../update_site/plugins/org.wesnoth_1.0.0.jar wesnoth
echo finished.
@pause
