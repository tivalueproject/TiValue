#include <fc/crypto/salsa20.hpp>
extern "C" {
#include <ecrypt-sync.h>
}

namespace fc 
{
  static bool salsa20_init = []() -> bool { ECRYPT_init(); return true; }();

  void salsa20_encrypt( const fc::sha256& key, uint64_t iv, const char* plain, char* cipher, uint64_t len )
  {
    ECRYPT_ctx ctx;
    ECRYPT_keysetup( &ctx, (unsigned char*)&key, ECRYPT_MAXIVSIZE, ECRYPT_MAXKEYSIZE );
    ECRYPT_ivsetup( &ctx, (unsigned char*)&iv );
   
    ECRYPT_encrypt_bytes( &ctx, (const unsigned char*)plain, (unsigned char*)cipher, len );
  }
  void salsa20_decrypt( const fc::sha256& key, uint64_t iv, const char* cipher, char* plain, uint64_t len  )
  {
  }
}
