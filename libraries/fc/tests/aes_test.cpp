#include <iostream>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/city.hpp>
#include <fc/exception/exception.hpp>

#include <fc/variant.hpp>
int main( int argc, char** )
{
  std::string line;
  std::getline( std::cin, line );
  auto key = fc::sha512::hash( "hello", 5 );
  while( std::cin && line != "q" )
  {
    try {
      std::vector<char> data( line.c_str(),line.c_str()+line.size()+1 ); 
      std::vector<char> crypt = fc::aes_encrypt( key, data );
      std::vector<char> dcrypt = fc::aes_decrypt( key, crypt );

      std::cout<<"line.size:     '"<<line.size()<<"'\n";
      std::cout<<"data.size:     '"<<data.size()<<"'\n";
      std::cout<<"crypt.size:    '"<<crypt.size()<<"'\n";
      std::cout<<"dcrypt.size:   '"<<dcrypt.size()<<"'\n";
      std::cout<<"line:          '"<<line<<"'\n";
      std::cout<<"dcrypt:        '"<<dcrypt.data()<<"'\n";
      std::cout<<"dcrypt: "<<fc::variant(dcrypt).as_string()<<"\n";
      std::cout<<"crypt: "<<fc::variant(crypt).as_string()<<"\n";
      memset( crypt.data(), 0, crypt.size() );

      fc::aes_encoder enc;
      enc.init( fc::sha256::hash((char*)&key,sizeof(key) ), fc::city_hash_crc_128( (char*)&key, sizeof(key) ) ); 
      auto len = enc.encode( dcrypt.data(), dcrypt.size(), crypt.data() );
    //  enc.final_encode( crypt.data() + len );
      std::cout<<"crypt: "<<fc::variant(crypt).as_string()<<"\n";
      

      fc::aes_decoder dec;
      dec.init( fc::sha256::hash((char*)&key,sizeof(key) ), fc::city_hash_crc_128( (char*)&key, sizeof(key) ) ); 
    } 
    catch ( fc::exception& e ) 
    {
       std::cout<<e.to_detail_string()<<"\n";
    }
    std::getline( std::cin, line );
  }
  return 0;
}
