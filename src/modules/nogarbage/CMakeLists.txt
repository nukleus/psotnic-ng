
project( nogarbage )

include_directories( ${PSOTNIC_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

add_library( nogarbage MODULE nogarbage.cpp )
