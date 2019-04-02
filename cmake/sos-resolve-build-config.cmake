

if(SOS_BUILD_CONFIG)
  message( STATUS "SOS BUILD CONFIG PROVIDED: " ${SOS_BUILD_CONFIG})
else()
  # See if the director ends in _link or _arm to do the build
  message( STATUS "CMAKE DIRECTORY: " ${CMAKE_BINARY_DIR})

  string(LENGTH ${CMAKE_BINARY_DIR} STR_LENGTH)
  string(FIND ${CMAKE_BINARY_DIR} "_link" LINK_POS REVERSE)
  string(FIND ${CMAKE_BINARY_DIR} "_arm" ARM_POS)

  math(EXPR IS_LINK "${STR_LENGTH} - ${LINK_POS}")
  math(EXPR IS_ARM "${STR_LENGTH} - ${ARM_POS}")

  if( ${IS_LINK} STREQUAL 5 )
    set(SOS_BUILD_CONFIG link CACHE INTERNAL "sos build config is link")
    message( STATUS "Set build config to link" )
  elseif( ${IS_ARM} STREQUAL 4 )
    set(SOS_BUILD_CONFIG arm CACHE INTERNAL "sos build config is link")
  else()
    message( FATAL_ERROR "No Configuration available build in *_link or *_arm directory or say -DSOS_BUILD_CONFIG:STRING=<arm|link>")
  endif()
endif()


