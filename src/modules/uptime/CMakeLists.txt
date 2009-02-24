
project( uptime )

include_directories( ${PSOTNIC_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_library( uptime MODULE uptime.cpp )
