
# build the list of files to compile
set( psotnic_SRC
   botcmd.cpp
   Adns.cpp
   AdnsFiredns.cpp
   AdnsPthread.cpp
   class-blowfish.cpp
   class-chan-actions.cpp
   class-chan.cpp
   class-chan-gotmode.cpp
   Chanlist.cpp
   Chanset.cpp
   Chanuser.cpp
   Client.cpp
   Clone.cpp
   Comment.cpp
   Config.cpp
   CustomDataStorage.cpp
   class-ent.cpp
   Fifo.cpp
   Http.cpp
   Idle.cpp
   Ignore.cpp
   Inet.cpp
   Inetconn.cpp
   class-listcmd.cpp
   Masklist.cpp
   Modeq.cpp
   Offence.cpp
   Options.cpp
   Pchar.cpp
   Penal.cpp
   PTime.cpp
   Server.cpp
   Protmodelist.cpp
   Socks5.cpp
   Tcl.cpp
   Update.cpp
   Userlist.cpp
   Wasop.cpp
   Settings.cpp
   Prvset.cpp
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
set( psotnic_HDR away.h blowfish.h class-ent.h classes.h common.h ctcp.h defines.h fastptrlist.h firedns.h firemake.h firestring.h global-var.h grass.h hashlist.h isaac.h iterator.h match.h md5.h module.h numeric_def.h pstring.h ptrlist.h structs.h tiny_ptrlist.h topics.h blowfish.h2 )

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
# generate seed file
add_custom_target( genseed ALL
   COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/genseed.pl
   VERBATIM )

add_executable( make-hiddenseed make-hiddenseed.c )
add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hidden-seed-functions.cpp
   COMMAND make-hiddenseed > ${CMAKE_CURRENT_BINARY_DIR}/hidden-seed-functions.cpp
   DEPENDS make-hiddenseed )
set( psotnic_SRC ${psotnic_SRC} hidden-seed-functions.cpp )

add_executable( psotnic ${psotnic_SRC} )

#####################
# generate config.h #
#####################
include( TestBigEndian )
TEST_BIG_ENDIAN( BE )
if( BE )
   add_definitions( -DHAVE_BIG_ENDIAN=1 )
   set( HAVE_BIG_ENDIAN 1 )
else( BE )
   add_definitions( -DHAVE_LITTLE_ENDIAN=1 )
   set( HAVE_LITTLE_ENDIAN 1 )
endif( BE )

#if()
   set( HAVE_IPV6 1 )
#endif()

if( DEBUG )
   set( HAVE_DEBUG 1 )
endif( DEBUG )


if( WITH_ANTIPTRACE )
   set( HAVE_ANTIPTRACE 1 )
endif( WITH_ANTIPTRACE )

if( NOT NO_ADNS )
   add_definitions( -DHAVE_ADNS )
   if( WITH_FIREDNS )
      set( HAVE_FIREDNS 1 )
      add_definitions( -DHAVE_ADNS_FIREDNS )
   else( WITH_FIREDNS )
      if( CMAKE_USE_PTHREADS_INIT )
         set( HAVE_ADNS_PTHREAD 1 )
         add_definitions( -DHAVE_ADNS_PTHREAD )
         target_link_libraries( psotnic ${CMAKE_THREAD_LIBS_INIT} )
      else( CMAKE_USE_PTHREADS_INIT )
         message( "Could not find pthreads, disabling multi-threaded functionalities." )
      endif( CMAKE_USE_PTHREADS_INIT )
   endif( WITH_FIREDNS )
endif( NOT NO_ADNS )

if( NOT NO_IRC_BACKTRACE )
   set( HAVE_IRC_BACKTRACE 1 )
endif( NOT NO_IRC_BACKTRACE )

if( WITH_SSL )
   if( OPENSSL_FOUND )
      set( HAVE_SSL 1 )
      add_definitions( -DHAVE_SSL )
      target_link_libraries( psotnic ${OPENSSL_LIBRARIES} )
   else( OPENSSL_FOUND )
      MESSAGE( FATAL_ERROR "OpenSSL was not found on this system." )
   endif( OPENSSL_FOUND )
endif( WITH_SSL )

# not documented elsewhere, no need for people to notice
# that psotnic theoretically supports tcl!
if( WITH_TCL )
   find_package( TCL )
   if( TCL_FOUND )
      set( HAVE_TCL 1 )
      target_link_libraries( psotnic ${TCL_LIBRARY} )
      include_directories( ${TCL_INCLUDE_PATH} )
   endif( TCL_FOUND )
endif( WITH_TCL )

configure_file( config.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

# This is needed to include seed.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

target_link_libraries( psotnic dl )

###########
# Modules #
###########
add_subdirectory( modules )

####################
# feature overview #
####################
message( "" )
message( "Feature overview\nPsotnic will be compiled with the following features:" )
message( "-----------------------------------------------------" )

if( HAVE_IPV6 )
   message( "> IPv6 support                     Enabled" )
else( HAVE_IPV6 )
   message( "> IPv6 support                     Disabled" )
endif( HAVE_IPV6 )

if( HAVE_SSL )
   message( "> SSL support                      Enabled" )
else( HAVE_SSL )
   message( "> SSL support                      Disabled" )
endif( HAVE_SSL )

if( NOT NO_ADNS )
   message( "> Asynchronous DNS resolver        Enabled" )
   if( HAVE_ADNS_PTHREAD )
      message( " > ADNS multithreading             Enabled" )
      message( " > FireDNS resolver                Disabled" )
   else( HAVE_ADNS_PTHREAD )
      message( " > ADNS multithreading             Disabled" )
      message( " > FireDNS resolver                Enabled" )
   endif( HAVE_ADNS_PTHREAD )
else( NOT NO_ADNS )
   message( "> Asynchronous DNS resolver          Disabled" )
   message( " > ADNS multithreading               Disabled" )
   message( " > FireDNS resolver                  Disabled" )
endif( NOT NO_ADNS )

if( HAVE_IRC_BACKTRACE )
   message( "> IRC backtrace dump               Enabled" )
else( HAVE_IRC_BACKTRACE )
   message( "> IRC backtrace dump               Disabled" )
endif( HAVE_IRC_BACKTRACE )

if( HAVE_ANTIPTRACE )
   message( "> Anti PTrace                      Enabled" )
else( HAVE_ANTIPTRACE )
   message( "> Anti PTrace                      Disabled" )
endif( HAVE_ANTIPTRACE )

if( HAVE_TCL )
   message( "> TCL support                      Enabled" )
endif( HAVE_TCL )
# not documenting that it is disabled, no need for people to notice
# that psotnic theoretically supports tcl!

message( "" )
