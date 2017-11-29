if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -no-integrated-as" )
endif()

add_definitions( -DSCRYPT_SALSA )
add_definitions( -DSCRYPT_SHA256 )

set( scrypt_sources
     scrypt-jane.c
   )

add_library( scrypt ${scrypt_sources} )
