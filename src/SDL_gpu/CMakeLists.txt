set(SDL_gpu_INSTALL_BY_DEFAULT OFF)

option(SDL_gpu_INSTALL "Install SDL_gpu libs, includes, and CMake scripts" ${SDL_gpu_INSTALL_BY_DEFAULT})
option(SDL_gpu_BUILD_DEBUG "Build with debugging symbols" ON)
option(SDL_gpu_BUILD_SHARED "Build SDL_gpu shared libraries" OFF)
option(SDL_gpu_BUILD_STATIC "Build SDL_gpu static libraries" ON)
option(SDL_gpu_USE_SDL1 "Use SDL 1.2 headers and library instead of SDL 2" ON)
option(SDL_gpu_DISABLE_OPENGL "Disable OpenGL renderers.  Overrides specific OpenGL renderer flags." OFF)
option(SDL_gpu_DISABLE_GLES "Disable OpenGLES renderers.  Overrides specific GLES renderer flags." ON)
option(SDL_gpu_DISABLE_OPENGL_1_BASE "Disable OpenGL 1 BASE renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_1 "Disable OpenGL 1.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_2 "Disable OpenGL 2.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_3 "Disable OpenGL 3.X renderer" OFF)
option(SDL_gpu_DISABLE_OPENGL_4 "Disable OpenGL 4.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_1 "Disable OpenGLES 1.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_2 "Disable OpenGLES 2.X renderer" OFF)
option(SDL_gpu_DISABLE_GLES_3 "Disable OpenGLES 3.X renderer" OFF)

set(SDL_gpu_VERSION 0.8.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./CMake_scripts")

if ( SDL_gpu_BUILD_DEBUG )
	SET(CMAKE_BUILD_TYPE Debug)
else ( SDL_gpu_BUILD_DEBUG )
	SET(CMAKE_BUILD_TYPE Release)
endif ( SDL_gpu_BUILD_DEBUG )

# Find the package for SDL or SDL2
if ( SDL_gpu_USE_SDL1 )
	find_package(SDL REQUIRED)
	
	if ( NOT SDL_FOUND )
	   message ( FATAL_ERROR "SDL not found!" )
	endif ( NOT SDL_FOUND )
	
	include_directories(${SDL_INCLUDE_DIR})
	link_libraries(${SDL_LIBRARY})
else ( SDL_gpu_USE_SDL1 )
	find_package(SDL2 REQUIRED)
	
	if ( NOT SDL2_FOUND )
	   message ( FATAL_ERROR "SDL2 not found!" )
	endif ( NOT SDL2_FOUND )
	
	if ( NOT SDL2MAIN_LIBRARY )
		message ( WARNING "SDL2MAIN_LIBRARY is NOTFOUND" )
		SET( SDL2MAIN_LIBRARY "" )
	endif ( NOT SDL2MAIN_LIBRARY )

	include_directories(${SDL2_INCLUDE_DIR})
	link_libraries(${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})
endif( SDL_gpu_USE_SDL1 )

# Find the package for OpenGL
if (SDL_gpu_DISABLE_OPENGL)
	add_definitions("-DSDL_GPU_DISABLE_OPENGL")
else (SDL_gpu_DISABLE_OPENGL)

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	link_libraries (${OPENGL_LIBRARIES})
	
	if (SDL_gpu_DISABLE_OPENGL_1_BASE)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1_BASE")
	endif (SDL_gpu_DISABLE_OPENGL_1_BASE)
	if (SDL_gpu_DISABLE_OPENGL_1)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_1")
	endif (SDL_gpu_DISABLE_OPENGL_1)
	if (SDL_gpu_DISABLE_OPENGL_2)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_2")
	endif (SDL_gpu_DISABLE_OPENGL_2)
	if (SDL_gpu_DISABLE_OPENGL_3)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_3")
	endif (SDL_gpu_DISABLE_OPENGL_3)
	if (SDL_gpu_DISABLE_OPENGL_4)
		add_definitions("-DSDL_GPU_DISABLE_OPENGL_4")
	endif (SDL_gpu_DISABLE_OPENGL_4)
	
endif (SDL_gpu_DISABLE_OPENGL)

# Find the package for OpenGLES
if (SDL_gpu_DISABLE_GLES)
	add_definitions("-DSDL_GPU_DISABLE_GLES")
else (SDL_gpu_DISABLE_GLES)

	find_package(OpenGLES REQUIRED)
	include_directories(${OPENGLES_INCLUDE_DIR})
	link_libraries (${OPENGLES_LIBRARIES})
	
	if (SDL_gpu_DISABLE_GLES_1)
		add_definitions("-DSDL_GPU_DISABLE_GLES_1")
	endif (SDL_gpu_DISABLE_GLES_1)
	if (SDL_gpu_DISABLE_GLES_2)
		add_definitions("-DSDL_GPU_DISABLE_GLES_2")
	endif (SDL_gpu_DISABLE_GLES_2)
	if (SDL_gpu_DISABLE_GLES_3)
		add_definitions("-DSDL_GPU_DISABLE_GLES_3")
	endif (SDL_gpu_DISABLE_GLES_3)
	
endif (SDL_gpu_DISABLE_GLES)

if(NOT WIN32)
  find_library(M_LIB m)
  link_libraries(${M_LIB})
endif (NOT WIN32)

# Make local includes look in the right places
include_directories(SDL_gpu)
include_directories(SDL_gpu/GL_common)
include_directories(SDL_gpu/OpenGL_1_BASE)
include_directories(SDL_gpu/OpenGL_1)
include_directories(SDL_gpu/OpenGL_2)
include_directories(SDL_gpu/OpenGL_3)
include_directories(SDL_gpu/GLES_1)
include_directories(SDL_gpu/GLES_2)
include_directories(SDL_gpu/externals/glew)
include_directories(SDL_gpu/externals/stb_image)


add_definitions("-Wall -std=c99 -pedantic -DSTBI_FAILURE_USERMSG")

# Build the SDL_gpu library.
add_subdirectory(SDL_gpu)
