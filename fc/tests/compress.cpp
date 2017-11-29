#include <iostream>
#include <fc/compress/smaz.hpp>
#include <fc/exception/exception.hpp>

int main( int argc, char** )
{
  std::string line;
  std::getline( std::cin, line );
  while( std::cin && line != "q" )
  {
    try {
      
      std::string compressed = fc::smaz_compress( line );
      std::cout<<"compressed size: "<<compressed.size()<<"\n";
      std::string decomp = fc::smaz_decompress( compressed );
      std::cout<<"decomp: '"<<decomp<<"'\n";
      std::cout<<"line:   '"<<line<<"'\n";
      FC_ASSERT( decomp == line );
    } 
    catch ( fc::exception& e ) 
    {
       std::cout<<e.to_detail_string()<<"\n";
    }
    std::getline( std::cin, line );
  }
  return 0;
}
