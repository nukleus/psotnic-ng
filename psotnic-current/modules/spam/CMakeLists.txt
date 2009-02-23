
project( antispam )

include_directories( ${PSOTNIC_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_library( antispam MODULE spam.cpp repeat.cpp )
