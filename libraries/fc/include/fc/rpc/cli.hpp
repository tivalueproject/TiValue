#pragma once
#include <fc/io/stdio.hpp>
#include <fc/io/json.hpp>
#include <fc/io/buffered_iostream.hpp>
#include <fc/io/sstream.hpp>
#include <fc/rpc/api_connection.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>

namespace fc { namespace rpc {

   /**
    *  Provides a simple wrapper for RPC calls to a given interface.
    */
   class cli : public api_connection
   {
      public:
         ~cli()
         {
            if( _run_complete.valid() )
            {
               stop();
            }
         }
         virtual variant send_call( api_id_type api_id, string method_name, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }
         virtual variant send_callback( uint64_t callback_id, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }
         virtual void    send_notice( uint64_t callback_id, variants args = variants() ) 
         {
            FC_ASSERT(false);
         }

         void start()
         {
            _run_complete = fc::async( [&](){ run(); } );
         }
         void stop()
         {
            _run_complete.cancel();
            _run_complete.wait();
         }
         void wait(){ _run_complete.wait(); }
         void format_result( const string& method, std::function<string(variant,const variants&)> formatter)
         {
            _result_formatters[method] = formatter;
         }

         virtual void getline( const fc::string& prompt, fc::string& line );

         void set_prompt( const string& prompt ) { _prompt = prompt; }

      private:
         void run()
         {
             while( !_run_complete.canceled() )
             {
                try {
                     std::string line;
                     try
                     {
                        getline( _prompt.c_str(), line );
                     }
                     catch ( const fc::eof_exception& e )
                     {
                        break;
                     }
                     std::cout << line << "\n";
                     line += char(EOF);
                     fc::variants args = fc::json::variants_from_string(line);;
                     if( args.size() == 0 ) continue;
                     
                     const string& method = args[0].get_string();

                     auto result = receive_call( 0, method, variants( args.begin()+1,args.end() ) );
                     auto itr = _result_formatters.find( method );
                     if( itr == _result_formatters.end() )
                     {
                        std::cout << fc::json::to_pretty_string( result ) << "\n";
                     }
                     else
                        std::cout << itr->second( result, args ) << "\n";
                }
                catch ( const fc::exception& e )
                {
                   std::cout << e.to_detail_string() << "\n";
                }
             } 
         }
         std::string      _prompt = ">>>";
         std::map<string,std::function<string(variant,const variants&)> > _result_formatters;
         fc::future<void> _run_complete;
   };
} } 
