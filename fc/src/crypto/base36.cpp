#include <fc/crypto/bigint.hpp>
#include <fc/io/sstream.hpp>
#include <fc/log/logger.hpp>

namespace fc
{
    fc::string to_base36( const char* data, size_t len )
    {
       if( len == 0 ) return fc::string();
       fc::bigint value( data, len );
       auto base36 = "0123456789abcdefghijklmnopqrstuvwxyz";
       std::vector<char> out( static_cast<size_t>(len * 1.6) + 1 );
       int pos = out.size() - 1;
       out[pos] = '\0';
       fc::bigint _36(36);
       do {
         if( value ) {
           --pos;
           out[pos] = base36[(value % _36).to_int64()];
         }
       } while (value /= _36);
      
       return &out[pos]; //fc::string( &out[pos], out.size() - pos);
    }

    fc::string to_base36( const std::vector<char>& vec )
    {
      return to_base36( (const char*)vec.data(), vec.size() );
    }

    std::vector<char> from_base36( const fc::string& b36 )
    {
       fc::bigint value;

       fc::bigint pos = 0; 
       fc::bigint _36(36);
       for( auto itr = b36.begin(); itr != b36.end(); ++itr )
       {
          if( *itr - '0' < 10 )      value = value +  _36.exp(pos) * fc::bigint(*itr - '0');
          else if( *itr - 'a' < 26 ) value = value + (_36.exp(pos) *  fc::bigint(10+*itr - 'a'));
          else if( *itr - 'A' < 26 ) value = value + (_36.exp(pos) *  fc::bigint(10+*itr - 'A'));
          else
          {
             wlog("unknown '${char}'", ("char",fc::string(&*itr,1)) );
          }
          ++pos;
       }
       return value;
    }
}
