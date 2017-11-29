#
# Defines fc library target.

PROJECT( fc )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.12 )

MESSAGE(STATUS "Configuring project fc located in: ${CMAKE_CURRENT_SOURCE_DIR}")
SET( CMAKE_AUTOMOC OFF )

# Setup module path to make visible used CMake extensions
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/GitVersionGen")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

INCLUDE(GetPrerequisites)
INCLUDE( VersionMacros )
INCLUDE( SetupTargetMacros )
INCLUDE(GetGitRevisionDescription)

get_git_head_revision(GIT_REFSPEC FC_GIT_REVISION_SHA)
get_git_unix_timestamp(FC_GIT_REVISION_UNIX_TIMESTAMP)

SET( DEFAULT_HEADER_INSTALL_DIR include/\${target} )
SET( DEFAULT_LIBRARY_INSTALL_DIR lib/ )
SET( DEFAULT_EXECUTABLE_INSTALL_DIR bin/ )
SET( CMAKE_DEBUG_POSTFIX _debug )
SET( BUILD_SHARED_LIBS NO )
SET( ECC_IMPL openssl CACHE STRING "openssl or secp256k1 or mixed" )

set(platformBitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(platformBitness 64)
endif()

SET (ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

SET(BOOST_COMPONENTS)
LIST(APPEND BOOST_COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context locale iostreams)
SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )


IF( ECC_IMPL STREQUAL openssl )
SET( ECC_REST src/crypto/elliptic_impl_pub.cpp )
ELSE( ECC_IMPL STREQUAL openssl )
SET( ECC_LIB secp256k1 )
IF( ECC_IMPL STREQUAL mixed )
SET( ECC_REST src/crypto/elliptic_impl_priv.cpp src/crypto/elliptic_impl_pub.cpp )
ELSE( ECC_IMPL STREQUAL mixed )
SET( ECC_REST src/crypto/elliptic_impl_priv.cpp )
ENDIF( ECC_IMPL STREQUAL mixed )
ENDIF( ECC_IMPL STREQUAL openssl )

IF( WIN32 )
  MESSAGE(STATUS "Configuring fc to build on Win32")

  set( RPCRT4 Rpcrt4 )

  #boost
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
#  set(Boost_USE_DEBUG_PYTHON ON)
  set(Boost_USE_MULTITHREADED ON)
  set(BOOST_ALL_DYN_LINK OFF) # force dynamic linking for all libraries

  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS ${BOOST_COMPONENTS})
  # For Boost 1.53 on windows, coroutine was not in BOOST_LIBRARYDIR and do not need it to build,  but if boost versin >= 1.54, find coroutine otherwise will cause link errors
  IF(NOT "${Boost_VERSION}" MATCHES "1.53(.*)")
    SET(BOOST_LIBRARIES_TEMP ${Boost_LIBRARIES})
    FIND_PACKAGE(Boost 1.54 REQUIRED COMPONENTS coroutine)
    LIST(APPEND BOOST_COMPONENTS coroutine)
    SET(Boost_LIBRARIES ${BOOST_LIBRARIES_TEMP} ${Boost_LIBRARIES})
  ENDIF()

  set( PLATFORM_SPECIFIC_LIBS WS2_32.lib Userenv.lib)
  # iphlpapi.lib

ELSE(WIN32)
  MESSAGE(STATUS "Configuring fc to build on Unix/Apple")

  LIST(APPEND BOOST_COMPONENTS coroutine)

  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS ${BOOST_COMPONENTS}) 

  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")

  IF(NOT APPLE)
    # Linux or other unix
    SET(SALSA_SRC vendor/salsa20/salsa20.s)
    SET(rt_library rt )
    SET(pthread_library pthread)
  ENDIF(NOT APPLE)
ENDIF(WIN32)



IF(NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
  set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
  set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
  message(STATUS "Setting up OpenSSL root and include vars to ${OPENSSL_ROOT_DIR}, ${OPENSSL_INCLUDE_DIR}")
ENDIF()


find_package(OpenSSL REQUIRED)

set( CMAKE_FIND_LIBRARY_SUFFIXES ${ORIGINAL_LIB_SUFFIXES} )

option( UNITY_BUILD OFF )

set( fc_sources
     src/rpc/state.cpp
     src/uint128.cpp
     src/real128.cpp
     src/variant.cpp
     src/exception.cpp
     src/variant_object.cpp
     src/thread/thread.cpp
     src/thread/thread_specific.cpp
     src/thread/future.cpp
     src/thread/task.cpp
     src/thread/spin_lock.cpp 
     src/thread/spin_yield_lock.cpp 
     src/thread/mutex.cpp
     src/thread/non_preemptable_scope_check.cpp
     src/asio.cpp
     src/string.cpp
     src/shared_ptr.cpp
     src/time.cpp
     src/utf8.cpp
     src/io/iostream.cpp
     src/io/datastream.cpp
     src/io/buffered_iostream.cpp
     src/io/fstream.cpp
     src/io/sstream.cpp
     src/io/json.cpp
     src/io/varint.cpp
     src/io/console.cpp
     src/filesystem.cpp
     src/interprocess/process.cpp
     src/interprocess/signals.cpp
     src/interprocess/file_mapping.cpp
     src/interprocess/mmap_struct.cpp
     src/rpc/json_connection.cpp
     src/rpc/cli.cpp
     src/log/log_message.cpp 
     src/log/logger.cpp
     src/log/appender.cpp
     src/log/console_appender.cpp
     src/log/file_appender.cpp
     src/log/gelf_appender.cpp
     src/log/logger_config.cpp
     src/crypto/openssl.cpp
     src/crypto/aes.cpp
     src/crypto/crc.cpp
     src/crypto/city.cpp
     src/crypto/base32.cpp
     src/crypto/base36.cpp
     src/crypto/base58.cpp
     src/crypto/base64.cpp
     src/crypto/bigint.cpp
     src/crypto/hex.cpp
     src/crypto/sha1.cpp
     src/crypto/ripemd160.cpp
     src/crypto/sha256.cpp
     src/crypto/sha224.cpp
     src/crypto/sha512.cpp
     src/crypto/dh.cpp
     src/crypto/blowfish.cpp
     src/crypto/elliptic_common.cpp
     ${ECC_REST}
     src/crypto/elliptic_${ECC_IMPL}.cpp
     src/crypto/rand.cpp
     src/crypto/salsa20.cpp
     #src/crypto/scrypt.cpp
     src/crypto/romix.cpp
     src/network/tcp_socket.cpp
     src/network/udp_socket.cpp
     src/network/udt_socket.cpp
     src/network/http/http_connection.cpp
     src/network/http/http_server.cpp
     src/network/http/websocket.cpp
     src/network/ntp.cpp
     src/network/ip.cpp
     src/network/rate_limiting.cpp
     src/network/resolve.cpp
     src/network/url.cpp
     src/network/gntp.cpp
     src/compress/smaz.cpp
     src/compress/zlib.cpp
     src/compress/lzma.cpp
     vendor/cyoencode-1.0.2/src/CyoDecode.c
     vendor/cyoencode-1.0.2/src/CyoEncode.c
     #vendor/salsa20/ecrypt.c
     ${SALSA_SRC}
     )

     SET_PROPERTY( SOURCE 
                   vendor/salsa20/salsa20.s
                   PROPERTY LANGUAGE C)

file(GLOB_RECURSE fc_headers ${CMAKE_CURRENT_SOURCE_DIR} 
  *.hpp *.h)

set( sources
  ${fc_sources}
)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/git_revision.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git_revision.cpp" @ONLY)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/git_revision.cpp")
list(APPEND sources ${fc_headers})

add_subdirectory( vendor/easylzma )
add_subdirectory( vendor/websocketpp )
#add_subdirectory( vendor/scrypt-jane )
add_subdirectory( vendor/udt4 )

setup_library( fc SOURCES ${sources} LIBRARY_TYPE STATIC DONT_INSTALL_LIBRARY )

# begin readline stuff
find_package(Curses)
find_package(Readline)

file(GLOB HEADERS "include/bts/cli/*.hpp")

if (READLINE_FOUND)
  target_compile_definitions (fc PRIVATE HAVE_READLINE)
  set(readline_libraries ${Readline_LIBRARY})
  if (CURSES_FOUND)
    list(APPEND readline_libraries ${CURSES_LIBRARY})
  endif()
  set(readline_includes ${Readline_INCLUDE_DIR})
endif()
if(WIN32)
  target_compile_definitions( fc PRIVATE _CRT_NONSTDC_NO_DEPRECATE )
endif(WIN32)
# end readline stuff

IF(WIN32)
  target_compile_definitions(fc PUBLIC WIN32 NOMINMAX _WIN32_WINNT=0x0501 _CRT_SECURE_NO_WARNINGS
    _SCL_SERCURE_NO_WARNINGS 
    # Needed to disable MSVC autolinking feature (#pragma comment)
    BOOST_ALL_NO_LIB
  )
  # Activate C++ exception handling, assume extern C calls don't throw
  # Add /U options to be sure settings specific to dynamic boost link are ineffective 
  target_compile_options(fc PUBLIC /EHsc /UBOOST_ALL_DYN_LINK /UBOOST_LINKING_PYTHON /UBOOST_DEBUG_PYTHON)
ELSE()
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")

  IF(APPLE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++ -Wall")
  ELSE()
    if( NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
      target_compile_options(fc PUBLIC -std=c++11 -Wall -fnon-call-exceptions)
    endif()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fnon-call-exceptions")
  ENDIF()
ENDIF()
 
target_include_directories(fc
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    "${readline_includes}"

  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
  #${CMAKE_CURRENT_SOURCE_DIR}/vendor/scrypt-jane
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/cyoencode-1.0.2/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/boost_1.51/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/salsa20
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/easylzma/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/udt4/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/websocketpp
  )

#target_link_libraries( fc PUBLIC easylzma_static scrypt udt ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${RPCRT4} ${CMAKE_DL_LIBS} ${rt_library} ${ECC_LIB} )
target_link_libraries( fc PUBLIC -L/usr/local/lib easylzma_static udt ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${RPCRT4} ${CMAKE_DL_LIBS} ${rt_library} ${readline_libraries} ${ECC_LIB} )

if(MSVC)
  set_source_files_properties( src/network/http/websocket.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)


IF(NOT Boost_UNIT_TEST_FRAMEWORK_LIBRARY MATCHES "\\.(a|lib)$")
IF(WIN32)
add_definitions(/DBOOST_TEST_DYN_LINK)
ELSE(WIN32)
add_definitions(-DBOOST_TEST_DYN_LINK)
ENDIF(WIN32)
ENDIF()

add_executable( api tests/api.cpp )
target_link_libraries( api fc )

include_directories( vendor/websocketpp )

add_executable( ntp_test ntp_test.cpp )
target_link_libraries( ntp_test fc )

add_executable( task_cancel_test tests/task_cancel.cpp )
target_link_libraries( task_cancel_test fc )


add_executable( bloom_test tests/bloom_test.cpp )
target_link_libraries( bloom_test fc )

add_executable( real128_test tests/real128_test.cpp )
target_link_libraries( real128_test fc )


add_executable( udt_server tests/udts.cpp )
target_link_libraries( udt_server fc udt )

add_executable( udt_client tests/udtc.cpp )
target_link_libraries( udt_client fc udt )

add_executable( lzma_test tests/lzma_test.cpp )
target_link_libraries( lzma_test fc )

add_executable( ecc_test tests/ecc_test.cpp )
target_link_libraries( ecc_test fc )

#add_executable( test_compress tests/compress.cpp )
#target_link_libraries( test_compress fc )
#add_executable( test_aes tests/aes_test.cpp )
#target_link_libraries( test_aes fc ${rt_library} ${pthread_library} )
#add_executable( test_sleep tests/sleep.cpp )
#target_link_libraries( test_sleep fc )
#add_executable( test_rate_limiting tests/rate_limiting.cpp )
#target_link_libraries( test_rate_limiting fc )

if(WIN32)
   # add addtional import library on windows platform
   target_link_libraries( fc PUBLIC crypt32.lib)

   # now generate a list of the DLLs we're using to use during the install process
   include (ParseLibraryList)
   PARSE_LIBRARY_LIST(${Boost_LIBRARIES}
                        FOUND parseOk
                        DEBUG Boost_LIBRARIES_DEBUG
                        OPT   Boost_LIBRARIES_RELEASE
                        GENERAL Boost_LIBRARIES_GENERAL)

   #Variable will hold list of .pdb files generated for libraries the 'fc' module is linked to
   set(INTERFACE_LINK_PDB_RELEASE)

   set(SHARED_LIBRARIES_RELEASE)
   foreach(boost_import_lib ${Boost_LIBRARIES_RELEASE})
      get_filename_component(import_lib_name_root ${boost_import_lib} NAME_WE)
      get_filename_component(import_lib_path ${boost_import_lib} PATH)
      set(boost_dll "${import_lib_path}/${import_lib_name_root}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set(boost_lib_pdb "${import_lib_name_root}.pdb")

      FILE(GLOB_RECURSE boost_pdb_paths "${import_lib_path}/../../bin.v2/*/${boost_lib_pdb}")

      foreach(p ${boost_pdb_paths})
        if(p MATCHES ".*/address-model-${platformBitness}/")
          GP_APPEND_UNIQUE(INTERFACE_LINK_PDB_RELEASE ${p})
        endif()
      endforeach()

      if(EXISTS "${boost_dll}")
        set(SHARED_LIBRARIES_RELEASE ${SHARED_LIBRARIES_RELEASE} "${boost_dll}")
      endif()

   endforeach()
   
   set(INTERFACE_LINK_PDB_DEBUG)
   set(SHARED_LIBRARIES_DEBUG)
   foreach(boost_import_lib ${Boost_LIBRARIES_DEBUG})
      get_filename_component(import_lib_name_root ${boost_import_lib} NAME_WE)
      get_filename_component(import_lib_path ${boost_import_lib} PATH)
      set(boost_dll "${import_lib_path}/${import_lib_name_root}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      set(boost_lib_pdb "${import_lib_name_root}.pdb")

      FILE(GLOB_RECURSE boost_pdb_paths "${import_lib_path}/../../bin.v2/*/${boost_lib_pdb}")

      foreach(p ${boost_pdb_paths})
        if(p MATCHES ".*/address-model-${platformBitness}/")
          GP_APPEND_UNIQUE(INTERFACE_LINK_PDB_DEBUG ${p})
        endif()
      endforeach()
      if(EXISTS "${boost_dll}")
        set(SHARED_LIBRARIES_DEBUG ${SHARED_LIBRARIES_DEBUG} "${boost_dll}")
      endif()
   endforeach()

   # message(STATUS "openssl_libraries=${OPENSSL_LIBRARIES}")
   foreach(lib ${OPENSSL_LIBRARIES})
      get_filename_component(lib_name ${lib} NAME_WE)
      if (${lib_name} STREQUAL "libeay32")
         get_filename_component(lib_dir ${lib} DIRECTORY)
         get_filename_component(openssl_dir "${lib_dir}/.." REALPATH)
         set( eaydll "${openssl_dir}/bin/${lib_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
         set(eay_pdb "${openssl_dir}/bin/${lib_name}.pdb")
         message(STATUS "eay=${eaydll}")
         if(EXISTS ${eay_pdb})
           GP_APPEND_UNIQUE(INTERFACE_LINK_PDB_RELEASE ${eay_pdb})
           GP_APPEND_UNIQUE(INTERFACE_LINK_PDB_DEBUG ${eay_pdb})
         endif()

         set(SHARED_LIBRARIES_DEBUG   ${SHARED_LIBRARIES_DEBUG}   "${eaydll}")
         set(SHARED_LIBRARIES_RELEASE ${SHARED_LIBRARIES_RELEASE} "${eaydll}")
      endif()
   endforeach()

   set_property(TARGET fc PROPERTY INTERFACE_LINK_PDB_RELEASE ${INTERFACE_LINK_PDB_RELEASE})
   set_property(TARGET fc PROPERTY INTERFACE_LINK_PDB_DEBUG ${INTERFACE_LINK_PDB_DEBUG})
   set_property(TARGET fc PROPERTY SHARED_LIBRARIES_DEBUG ${SHARED_LIBRARIES_DEBUG})
   set_property(TARGET fc PROPERTY SHARED_LIBRARIES_RELEASE ${SHARED_LIBRARIES_RELEASE})

endif(WIN32)

IF(APPLE)
  # As of 10.10 yosemite, the OpenSSL static libraries shipped with os x have a dependency
  # on zlib, so any time you link in openssl you also need to link zlib.  .  We really want to detect whether openssl was configured with the --no-zlib
  # option or not when it was built, but that's difficult to do in practice, so we
  # just always try to link it in on mac.
  find_package( ZLIB REQUIRED )
ENDIF(APPLE)

SET(OPENSSL_CONF_TARGET )
IF(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  SET (OPENSSL_CONF_TARGET ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
ELSE()
  SET (OPENSSL_CONF_TARGET ${CMAKE_CURRENT_BINARY_DIR})
ENDIF()

    IF(WIN32)
SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
     COMMAND ${CMAKE_COMMAND} -E copy_if_different "${OPENSSL_ROOT_DIR}/ssl/openssl.cnf" "${OPENSSL_CONF_TARGET}/openssl.cnf")
    ENDIF(WIN32)

ADD_CUSTOM_COMMAND(TARGET fc POST_BUILD ${POST_BUILD_STEP_COMMANDS} 
  COMMENT "Copying OpenSSL/ssl/openssl.cnf into target directory."
)

MESSAGE(STATUS "Finished fc module configuration...")

