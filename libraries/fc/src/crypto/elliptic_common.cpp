#include <fc/crypto/base58.hpp>
#include <fc/crypto/elliptic.hpp>

/* stuff common to all ecc implementations */

namespace fc { namespace ecc {

    public_key public_key::from_key_data( const public_key_data &data ) {
        return public_key(data);
    }

    std::string public_key::to_base58( const public_key_data &key )
    {
      uint32_t check = (uint32_t)sha256::hash(key.data, sizeof(key))._hash[0];
      assert(key.size() + sizeof(check) == 37);
      array<char, 37> data;
      memcpy(data.data, key.begin(), key.size());
      memcpy(data.begin() + key.size(), (const char*)&check, sizeof(check));
      return fc::to_base58(data.begin(), data.size());
    }

    public_key public_key::from_base58( const std::string& b58 )
    {
        array<char, 37> data;
        size_t s = fc::from_base58(b58, (char*)&data, sizeof(data) );
        FC_ASSERT( s == sizeof(data) );

        public_key_data key;
        uint32_t check = (uint32_t)sha256::hash(data.data, sizeof(key))._hash[0];
        FC_ASSERT( memcmp( (char*)&check, data.data + sizeof(key), sizeof(check) ) == 0 );
        memcpy( (char*)key.data, data.data, sizeof(key) );
        return from_key_data(key);
    }

    bool public_key::is_canonical( const compact_signature& c ) {
        return !(c.data[1] & 0x80)
               && !(c.data[1] == 0 && !(c.data[2] & 0x80))
               && !(c.data[33] & 0x80)
               && !(c.data[33] == 0 && !(c.data[34] & 0x80));
    }

    private_key private_key::generate_from_seed( const fc::sha256& seed, const fc::sha256& offset )
    {
        ssl_bignum z;
        BN_bin2bn((unsigned char*)&offset, sizeof(offset), z);

        ec_group group(EC_GROUP_new_by_curve_name(NID_secp256k1));
        bn_ctx ctx(BN_CTX_new());
        ssl_bignum order;
        EC_GROUP_get_order(group, order, ctx);

        // secexp = (seed + z) % order
        ssl_bignum secexp;
        BN_bin2bn((unsigned char*)&seed, sizeof(seed), secexp);
        BN_add(secexp, secexp, z);
        BN_mod(secexp, secexp, order, ctx);

        fc::sha256 secret;
        assert(BN_num_bytes(secexp) <= int64_t(sizeof(secret)));
        auto shift = sizeof(secret) - BN_num_bytes(secexp);
        BN_bn2bin(secexp, ((unsigned char*)&secret)+shift);
        return regenerate( secret );
    }

    fc::sha256 private_key::get_secret( const EC_KEY * const k )
    {
       if( !k )
       {
          return fc::sha256();
       }

       fc::sha256 sec;
       const BIGNUM* bn = EC_KEY_get0_private_key(k);
       if( bn == NULL )
       {
         FC_THROW_EXCEPTION( exception, "get private key failed" );
       }
       int nbytes = BN_num_bytes(bn);
       BN_bn2bin(bn, &((unsigned char*)&sec)[32-nbytes] );
       return sec;
    }

    private_key private_key::generate()
    {
       EC_KEY* k = EC_KEY_new_by_curve_name( NID_secp256k1 );
       if( !k ) FC_THROW_EXCEPTION( exception, "Unable to generate EC key" );
       if( !EC_KEY_generate_key( k ) )
       {
          FC_THROW_EXCEPTION( exception, "ecc key generation error" );

       }

       return private_key( k );
    }

}

void to_variant( const ecc::private_key& var,  variant& vo )
{
    vo = var.get_secret();
}

void from_variant( const variant& var,  ecc::private_key& vo )
{
    fc::sha256 sec;
    from_variant( var, sec );
    vo = ecc::private_key::regenerate(sec);
}

void to_variant( const ecc::public_key& var,  variant& vo )
{
    vo = var.serialize();
}

void from_variant( const variant& var,  ecc::public_key& vo )
{
    ecc::public_key_data dat;
    from_variant( var, dat );
    vo = ecc::public_key(dat);
}

}
