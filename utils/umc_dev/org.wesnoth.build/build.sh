#!/bin/bash

eclipse_dir=$1
build_dir=/tmp/eclipse_build

echo Cleaning build directory
if [[ -d "$build_dir" ]]
	rm -rf "$build_dir"
fi


mkdir -p "$build_dir"
if [[ -z "$eclipse_dir" ]] 
then
	echo Please set the eclipse installation directory as the first parameter.
	exit
fi

echo Using $eclipse_dir directory as eclipse installation.
echo Using $build_dir as temporary directory.

echo Searching for eclipse launcher...
launcher_jar=`find "$eclipse_dir/plugins/" -type f -name 'org.eclipse.equinox.launcher_*.jar' -print0`
if [[ -f "$launcher_jar" ]]
then
	echo Found eclipse launcher jar: $launcher_jar
else
	echo Could not find eclipse launcher, exiting.
	exit
fi

echo Searching for eclipse pde build directory...
pdebuild_dir=`find "$eclipse_dir/plugins/" -type d -name 'org.eclipse.pde.build_*' -print0`
if [[ -d "$pdebuild_dir" ]]
then
	echo Found pde build directory: $pdebuild_dir
else
	echo Could not find pde build directory, exiting.
	exit
fi

echo Everything set-up. Starting the build...

java -cp $launcher_jar org.eclipse.core.launcher.Main -data workspace -application org.eclipse.ant.core.antRunner -DbuildDirectory=$build_dir -Dbase=$eclipse_dir -DbaseLocation=$eclipse_dir -Ddeltapack=$eclipse_dir -Declipse.pdebuild.scripts=$pdebuild_dir/scripts -Declipse.pdebuild.templates=$pdebuild_dir/templates -buildfile build.xml

echo Cleaning up...
if [[ -f "../org.wesnoth/build.xml" ]] 
then
	rm ../org.wesnoth/build.xml
fi
if [[ -f "../org.wesnoth.feature/build.xml" ]]
then
	rm ../org.wesnoth.feature/build.xml
fi
if [[ -f "../org.wesnoth.wml/build.xml" ]] 
then
	rm ../org.wesnoth.wml/build.xml
fi
if [[ -f "../org.wesnoth.wml.ui/build.xml" ]] 
then
	rm ../org.wesnoth.wml.ui/build.xml
fi
