#include <fc/crypto/dh.hpp>
#include <openssl/dh.h>

namespace fc {
   bool diffie_hellman::generate_params( int s, uint8_t g )
   {
        DH* dh = DH_generate_parameters( s, g, NULL, NULL );
        p.resize( BN_num_bytes( dh->p ) );
        if( p.size() )
            BN_bn2bin( dh->p, (unsigned char*)&p.front()  );
        this->g = g;

        int check;
        DH_check(dh,&check);
        DH_free(dh);

        if( check & DH_CHECK_P_NOT_SAFE_PRIME )
            return valid = false;
        return valid = true;
   }
   bool diffie_hellman::validate()
   {
        if( !p.size() ) 
            return valid = false;
        DH* dh = DH_new(); 
        dh->p = BN_bin2bn( (unsigned char*)&p.front(), p.size(), NULL );
        dh->g = BN_bin2bn( (unsigned char*)&g, 1, NULL );

        int check;
        DH_check(dh,&check);
        DH_free(dh);
        if( check & DH_CHECK_P_NOT_SAFE_PRIME )
            return valid = false;
        return valid = true;
   }

   bool diffie_hellman::generate_pub_key()
   {
        if( !p.size() ) 
            return valid = false;
        DH* dh = DH_new(); 
        dh->p = BN_bin2bn( (unsigned char*)&p.front(), p.size(), NULL );
        dh->g = BN_bin2bn( (unsigned char*)&g, 1, NULL );

        int check;
        DH_check(dh,&check);
        if( check & DH_CHECK_P_NOT_SAFE_PRIME )
        {
            DH_free(dh);
            return valid = false;
        }
        DH_generate_key(dh);

        pub_key.resize( BN_num_bytes( dh->pub_key ) );
        priv_key.resize( BN_num_bytes( dh->priv_key ) );
        if( pub_key.size() )
            BN_bn2bin( dh->pub_key, (unsigned char*)&pub_key.front()  );
        if( priv_key.size() )
            BN_bn2bin( dh->priv_key, (unsigned char*)&priv_key.front()  );

        DH_free(dh);
        return valid = true;
   }
   bool diffie_hellman::compute_shared_key( const char* buf, uint32_t s ) {
        DH* dh = DH_new(); 
        dh->p = BN_bin2bn( (unsigned char*)&p.front(), p.size(), NULL );
        dh->pub_key = BN_bin2bn( (unsigned char*)&pub_key.front(), pub_key.size(), NULL );
        dh->priv_key = BN_bin2bn( (unsigned char*)&priv_key.front(), priv_key.size(), NULL );
        dh->g = BN_bin2bn( (unsigned char*)&g, 1, NULL );

        int check;
        DH_check(dh,&check);
        if( check & DH_CHECK_P_NOT_SAFE_PRIME )
        {
            DH_free(dh);
            return valid = false;
        }


        BIGNUM* pk = BN_bin2bn( (unsigned char*)buf, s, NULL ); 
        shared_key.resize( DH_size(dh) ); 
        DH_compute_key( (unsigned char*)&shared_key.front(), pk, dh );
        BN_free(pk);
        DH_free(dh);
        return valid = true;
   }
   bool diffie_hellman::compute_shared_key( const std::vector<char>& pubk ) {
      return compute_shared_key( &pubk.front(), pubk.size() );
   }

}
