
project( plog )

set( plog_SRC main.cpp misc.cpp plog.cpp )
set( plog_HDR plog.h )
set( plog_EXTRA Readme.txt ) #used for install later

include_directories( ${PSOTNIC_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_library( plog MODULE ${plog_SRC} )
