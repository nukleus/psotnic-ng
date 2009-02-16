

project( psotnic )

cmake_minimum_required( VERSION 2.6 )
#set( CMAKE_VERBOSE_MAKEFILE ON )

find_package( Perl REQUIRED )
find_package( Threads )
find_package( OpenSSL )

# build the list of files to compile
set( psotnic_SRC
   botcmd.cpp
   class-adns.cpp
   class-adns-firedns.cpp
   class-adns-pthread.cpp
   class-blowfish.cpp
   class-chan-actions.cpp
   class-chan.cpp
   class-chan-gotmode.cpp
   class-client.cpp
   class-clone.cpp
   class-comment.cpp
   class-customdata.cpp
   class-ent.cpp
   class-fifo.cpp
   class-http.cpp
   class-idle.cpp
   class-ignore.cpp
   class-inet.cpp
   class-listcmd.cpp
   class-masklist.cpp
   class-modeq.cpp
   class-offence.cpp
   class-options.cpp
   class-pchar.cpp
   class-penal.cpp
   class-ptime.cpp
   class-server.cpp
   class-shitlist.cpp
   class-socks5.cpp
   class-tcl.cpp
   class-update.cpp
   class-userlist.cpp
   class-wasop.cpp
   config-create.cpp
   config-load.cpp
   functions.cpp
   firedns.cpp
   firestring.cpp
   inet_pton.cpp
   main.cpp
   match.cpp
   md5.cpp
   md5func.cpp
   module.cpp
   parse-bot.cpp
   parse-botnet.cpp
   parse-ctcp.cpp
   parse-hub.cpp
   parse-irc.cpp
   parse-owner.cpp
   random.cpp
   signals.cpp )

# headers
set( psotnic_HDR away.h blowfish.h class-ent.h classes.h common.h ctcp.h defines.h fastptrlist.h firedns.h firemake.h firestring.h global-var.h grass.h hashlist.h isaac.h iterator.h match.h md5.h module.h numeric_def.h prots.h pstring.h ptrlist.h structs.h tiny_ptrlist.h topics.h blowfish.h2 )

set( PSOTNIC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} )


if( ${CMAKE_COMPILER_IS_GNUCXX} )
   add_definitions( "-Wall -Wno-non-virtual-dtor" )
endif( ${CMAKE_COMPILER_IS_GNUCXX} )

# todo:
# --with-antiptrace
# --no-irc-backtrace
# --disable-adns
# --with-ssl
# --with-firedns

############################
# generate make-hiddenseed #
############################
add_executable( make-hiddenseed make-hiddenseed.c )
add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hidden-seed-functions.cpp
   COMMAND make-hiddenseed > ${CMAKE_CURRENT_BINARY_DIR}/hidden-seed-functions.cpp
   DEPENDS make-hiddenseed )
set( psotnic_SRC ${psotnic_SRC} hidden-seed-functions.cpp )

# generate seed file
add_custom_target( genseed ALL
   COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/genseed.pl
   VERBATIM )

#####################
# generate config.h #
#####################
#if()
   set( HAVE_LITTLE_ENDIAN 1 )
#endif()

#if()
   set( HAVE_IPV6 1 )
#endif()

if( DEBUG )
   set( HAVE_DEBUG 1 )
endif( DEBUG )

if( WITH_FIREDNS )
   set( HAVE_FIREDNS 1 )
endif( WITH_FIREDNS )

if( WITH_ANTIPTRACE )
   set( HAVE_ANTIPTRACE 1 )
endif( WITH_ANTIPTRACE )

if( NOT NO_ADNS )
   set( HAVE_ADNS 1 )
   if( CMAKE_USE_PTHREADS_INIT )
      set( HAVE_ADNS_PTHREAD 1 )
      add_definitions( -DHAVE_ADNS_PTHREAD=1 -DHAVE_ADNS=1 )
   else( CMAKE_USE_PTHREADS_INIT )
      message( "Could not find pthreads, disabling multi-threaded functionalities." )
   endif( CMAKE_USE_PTHREADS_INIT )
endif( NOT NO_ADNS )

if( NOT NO_IRC_BACKTRACE )
   set( HAVE_IRC_BACKTRACE 1 )
endif( NOT NO_IRC_BACKTRACE )

if( WITH_SSL )
   if( OPENSSL_FOUND )
      set( HAVE_SSL 1 )
   else( OPENSSL_FOUND )
      MESSAGE( FATAL_ERROR "OpenSSL was not found on this system." )
   endif( OPENSSL_FOUND )
endif( WITH_SSL )

configure_file( config.cmake config.h )

# This is needed to include seed.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

add_executable( psotnic ${psotnic_SRC} )

target_link_libraries( psotnic dl )

if( HAVE_ADNS_PTHREAD )
   target_link_libraries( psotnic pthread )
endif( HAVE_ADNS_PTHREAD )


###########
# Modules #
###########
add_subdirectory( modules )

####################
# feature overview #
####################
message( "Feature overview\nPsotnic will be compiled with the following features:" )
message( "-----------------------------------------------------" )

if( HAVE_SSL )
   message( "SSL support                        Enabled" )
else( HAVE_SSL )
   message( "SSL support                        Disabled" )
endif( HAVE_SSL )

if( NOT NO_ADNS )
   message( "Asynchronous DNS resolver          Enabled" )
else( NOT NO_ADNS )
   message( "Asynchronous DNS resolver          Disabled" )
endif( NOT NO_ADNS )

if( HAVE_ADNS_PTHREAD )
   message( "ADNS multithreading                Enabled" )
else( HAVE_ADNS_PTHREAD )
   message( "ADNS multithreading                Disabled" )
endif( HAVE_ADNS_PTHREAD )

if( HAVE_ADNS_FIREDNS )
   message( "ADNS FireDNS resolver              Enabled" )
else( HAVE_ADNS_FIREDNS )
   message( "ADNS FireDNS resolver              Disabled" )
endif( HAVE_ADNS_FIREDNS )

if( HAVE_IRC_BACKTRACE )
   message( "IRC backtrace dump                 Enabled" )
else( HAVE_IRC_BACKTRACE )
   message( "IRC backtrace dump                 Disabled" )
endif( HAVE_IRC_BACKTRACE )
