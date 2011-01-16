# $Id$

cmake_minimum_required(VERSION 2.6)

project(ana)

add_subdirectory(src)

option(apps "Compile sample applications as well" OFF)

if(apps)
	message(STATUS "Compiling sample applications as well.")
	add_subdirectory( apps )
endif(apps)

option(tests "Compile test applications as well" OFF)

if(tests)
	message(STATUS "Compiling test applications as well.")
	add_subdirectory(test)
endif(tests)
