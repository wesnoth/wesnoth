set(SDL_gpu_SRCS
	SDL_gpu.c
	SDL_gpuShapes.c
	SDL_gpu_Renderer.c
	GL_common/SDL_gpu_GL_matrix.c
	OpenGL_1_BASE/SDL_gpu_OpenGL_1_BASE.c
	OpenGL_1/SDL_gpu_OpenGL_1.c
	OpenGL_2/SDL_gpu_OpenGL_2.c
	OpenGL_3/SDL_gpu_OpenGL_3.c
	GLES_1/SDL_gpu_GLES_1.c
	GLES_2/SDL_gpu_GLES_2.c
)

set(SDL_gpu_HDRS
	SDL_gpu.h
	GL_common/SDL_gpu_GL_matrix.h
	GL_common/SDL_gpu_GL_common.inl
	GL_common/SDL_gpuShapes_GL_common.inl
	OpenGL_1_BASE/SDL_gpu_OpenGL_1_BASE.h
	OpenGL_1/SDL_gpu_OpenGL_1.h
	OpenGL_2/SDL_gpu_OpenGL_2.h
	OpenGL_3/SDL_gpu_OpenGL_3.h
	GLES_1/SDL_gpu_GLES_1.h
	GLES_2/SDL_gpu_GLES_2.h
)

if(STBI_FOUND)
	include_directories(${STBI_INCLUDE_DIR})
	link_libraries (${STBI_LIBRARY})
else(STBI_FOUND)
	SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/stb_image/stb_image.c)
	SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/stb_image/stb_image.h)
endif(STBI_FOUND)

if(STBI_WRITE_FOUND)
	include_directories(${STBI_WRITE_INCLUDE_DIR})
	link_libraries (${STBI_WRITE_LIBRARY})
else(STBI_WRITE_FOUND)
	SET(SDL_gpu_SRCS ${SDL_gpu_SRCS} externals/stb_image/stb_image_write.c)
	SET(SDL_gpu_HDRS ${SDL_gpu_HDRS} externals/stb_image/stb_image_write.h)
endif(STBI_WRITE_FOUND)


set(SDL_gpu_install_HDRS
	SDL_gpu.h
	GL_common/SDL_gpu_GL_matrix.h
	OpenGL_1_BASE/SDL_gpu_OpenGL_1_BASE.h
	OpenGL_1/SDL_gpu_OpenGL_1.h
	OpenGL_2/SDL_gpu_OpenGL_2.h
	OpenGL_3/SDL_gpu_OpenGL_3.h
	GLES_1/SDL_gpu_GLES_1.h
	GLES_2/SDL_gpu_GLES_2.h
)

# Set the appropriate library name so SDL_gpu and SDL2_gpu can co-exist.
if ( SDL_gpu_USE_SDL1)
	set (SDL_gpu_OUTPUT_NAME "SDL_gpu")
else ( SDL_gpu_USE_SDL1)
	set (SDL_gpu_OUTPUT_NAME "SDL2_gpu")
endif ( SDL_gpu_USE_SDL1)

# Build the shared library (.so or .dll)
if(SDL_gpu_BUILD_SHARED)
	add_library(SDL_gpu_shared SHARED
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES
		OUTPUT_NAME ${SDL_gpu_OUTPUT_NAME}
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu_shared PROPERTIES LINKER_LANGUAGE C)
endif()

# Build the static library (.a)
if(SDL_gpu_BUILD_STATIC)
	add_library(SDL_gpu STATIC
		${SDL_gpu_HDRS}
		${SDL_gpu_SRCS}
	)
	set_target_properties(SDL_gpu PROPERTIES
		OUTPUT_NAME ${SDL_gpu_OUTPUT_NAME}
		CLEAN_DIRECT_OUTPUT 1
		VERSION ${SDL_gpu_VERSION}
	)
	set_target_properties(SDL_gpu PROPERTIES LINKER_LANGUAGE C)
endif()

# These are used to create visual studio folders.
source_group(Common FILES ${SDL_gpu_SRCS} ${SDL_gpu_HDRS})

# Install the library
if(SDL_gpu_INSTALL)
	# install headers
	install(FILES ${SDL_gpu_install_HDRS} DESTINATION include/SDL)

	# install libraries
	if(SDL_gpu_BUILD_SHARED)
		install(TARGETS SDL_gpu_shared EXPORT SDL_gpu-targets DESTINATION lib)
	endif()
	if(SDL_gpu_BUILD_STATIC)
		install(TARGETS SDL_gpu EXPORT SDL_gpu-targets DESTINATION lib)
	endif()

	# install build system hooks for third-party apps
	#install(EXPORT SDL_gpu-targets DESTINATION ./SDL_gpu)
	#install(FILES SDL_gpuConfig.cmake DESTINATION ./SDL_gpu)
endif(SDL_gpu_INSTALL)
