
project( chanlog )

include_directories( ${PSOTNIC_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_library( chanlog MODULE Log.cpp )
