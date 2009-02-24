project( psotnic )

cmake_minimum_required( VERSION 2.6 )

find_package( Perl REQUIRED )
find_package( Threads )
find_package( OpenSSL )

add_subdirectory( src )
