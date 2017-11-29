
IF( APPLE )
   add_definitions( -DOSX )
ELSEIF( WIN32 )

ELSE()
   add_definitions( -DLINUX )
ENDIF() 

set( udt_sources 
   src/api.cpp
   src/buffer.cpp
   src/cache.cpp
   src/ccc.cpp
   src/channel.cpp
   src/common.cpp
   src/core.cpp
   src/epoll.cpp
   src/list.cpp
   src/md5.cpp
   src/packet.cpp
   src/queue.cpp
   src/window.cpp
   )

add_library( udt ${udt_sources} )
