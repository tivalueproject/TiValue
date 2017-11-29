#include <fc/crypto/scrypt.hpp>
#include <fc/exception/exception.hpp>
#include "scrypt-jane.h"

namespace fc {

    unsigned log2( unsigned n )
    {
        if( n <= 0 ) FC_THROW_EXCEPTION( exception, "cannot take log2(${n})", ("n",n) );
        unsigned i = 0;
        while( n >>= 1 ) ++i;
        return i;
    }

    void scrypt_derive_key( const std::vector<unsigned char>& passphrase, const std::vector<unsigned char>& salt,
                            unsigned int n, unsigned int r, unsigned int p, std::vector<unsigned char>& key )
    {
        scrypt( passphrase.data(), passphrase.size(), salt.data(), salt.size(),
                log2( n ) - 1, log2( r ), log2( p ), key.data(), key.capacity() );
    }

} // namespace fc
