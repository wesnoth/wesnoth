separate_arguments(IMAGE_FILES)
file(GLOB_RECURSE IMAGES RELATIVE ${CMAKE_SOURCE_DIR} ${IMAGE_FILES})

foreach(IMAGE ${IMAGES})
  if(NOT EXISTS ${IMAGE_BUILD_DIR}/${IMAGE} OR ${IMAGE} IS_NEWER_THAN ${IMAGE_BUILD_DIR}/${IMAGE})
    execute_process(COMMAND ${IMAGEMAGICK_IDENTIFY_EXECUTABLE} 
                    ${IMAGE}
                    OUTPUT_VARIABLE IMAGE_PROP)

    if(IMAGE_PROP MATCHES " 1024x768 ")
      set(SIZE "320x240")
    elseif(IMAGE_PROP MATCHES " 640x480 ")
      set(SIZE "240x180")
    elseif(IMAGE_PROP MATCHES " 205x205 ")
      set(SIZE "80x80")
    else(IMAGE_PROP MATCHES " 1024x768 ")
      set(SIZE "50%")
    endif(IMAGE_PROP MATCHES " 1024x768 ")

    #copy the image first to create all subdirectories
    execute_process(COMMAND ${CMAKE_COMMAND} 
                    -E copy_if_different  ${IMAGE} ${IMAGE_BUILD_DIR}/${IMAGE})

    if(NOT ${IMAGE} STREQUAL "images/misc/bar-energy-tinygui.png")
        execute_process(COMMAND ${IMAGEMAGICK_CONVERT_EXECUTABLE} 
                    -filter point -resize ${SIZE} ${IMAGE} ${IMAGE_BUILD_DIR}/${IMAGE})
    endif(NOT ${IMAGE} STREQUAL "images/misc/bar-energy-tinygui.png")

    message(STATUS "Generating ${IMAGE}")

  endif(NOT EXISTS ${IMAGE_BUILD_DIR}/${IMAGE} OR ${IMAGE} IS_NEWER_THAN ${IMAGE_BUILD_DIR}/${IMAGE})
endforeach(IMAGE ${IMAGES})
