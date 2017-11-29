#include <fc/bloom_filter.hpp>
#include <fc/exception/exception.hpp>
#include <fc/reflect/variant.hpp>
#include <iostream>
#include <fc/variant.hpp>
#include <fc/io/raw.hpp>
#include <fstream>
#include <fc/io/json.hpp>
#include <fc/crypto/base64.hpp>

using namespace fc;

int main( int argc, char** argv )
{
   try {

   bloom_parameters parameters;

   // How many elements roughly do we expect to insert?
   parameters.projected_element_count = 100000;

   // Maximum tolerable false positive probability? (0,1)
   parameters.false_positive_probability = 0.0001; // 1 in 10000

   // Simple randomizer (optional)
   parameters.random_seed = 0xA5A5A5A5;

   if (!parameters)
   {
      std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
      return 1;
   }

   parameters.compute_optimal_parameters();

   //Instantiate Bloom Filter
   bloom_filter filter(parameters);

   if( argc > 1 )
   {
      uint32_t count = 0;
      std::string line;
      std::ifstream in(argv[1]);
      std::ofstream words("words.txt");
      while( !in.eof() && count < 100000 )
      {
         std::getline(in, line);
         std::cout << "'"<<line<<"'\n";
         if( !filter.contains(line) )
         {
            filter.insert( line );
            words << line << "\n";
            ++count;
         }
      }
      wdump((filter));
      auto packed_filter = fc::raw::pack(filter);
      wdump((packed_filter.size()));
      wdump((packed_filter));
      std::ofstream out(argv[2]);
      std::string str = fc::json::to_string(packed_filter);
      auto b64 = fc::base64_encode( packed_filter.data(), packed_filter.size() );
      for( uint32_t i = 0; i < b64.size(); i += 1024 )
         out << '"' <<  b64.substr( i, 1024 ) << "\",\n";

      return 0;
   }




   std::string str_list[] = { "AbC", "iJk", "XYZ" };

   // Insert into Bloom Filter
   {
      // Insert some strings
      for (std::size_t i = 0; i < (sizeof(str_list) / sizeof(std::string)); ++i)
      {
         filter.insert(str_list[i]);
      }

      // Insert some numbers
      for (std::size_t i = 0; i < 100; ++i)
      {
         filter.insert(i);
      }
   }

   // Query Bloom Filter
   {
      // Query the existence of strings
      for (std::size_t i = 0; i < (sizeof(str_list) / sizeof(std::string)); ++i)
      {
         if (filter.contains(str_list[i]))
         {
            std::cout << "BF contains: " << str_list[i] << std::endl;
         }
      }

      // Query the existence of numbers
      for (std::size_t i = 0; i < 100; ++i)
      {
         if (filter.contains(i))
         {
            std::cout << "BF contains: " << i << std::endl;
         }
      }

      std::string invalid_str_list[] = { "AbCX", "iJkX", "XYZX" };

      // Query the existence of invalid strings
      for (std::size_t i = 0; i < (sizeof(invalid_str_list) / sizeof(std::string)); ++i)
      {
         if (filter.contains(invalid_str_list[i]))
         {
            std::cout << "BF falsely contains: " << invalid_str_list[i] << std::endl;
         }
      }

      // Query the existence of invalid numbers
      for (int i = -1; i > -100; --i)
      {
         if (filter.contains(i))
         {
            std::cout << "BF falsely contains: " << i << std::endl;
         }
      }
   }

   wdump((filter));
   auto packed_filter = fc::raw::pack(filter);
   wdump((packed_filter.size()));
   wdump((packed_filter));

     return 0;
   } 
   catch ( const fc::exception& e )
   {
      edump((e.to_detail_string()) );
   }
}
